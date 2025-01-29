/*
 *
 * Copyright (C) 2025 BigfootACA <bigfoot@classfun.cn>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include<cerrno>
#include<string>
#include<cstdlib>
#include<stdexcept>
#include<sys/stat.h>
#include<fmt/format.h>
#include<sys/sysmacros.h>
#include"utils.h"

using std::string;
using std::runtime_error;
using fmt::format;

bool path_is_type(const string&path,mode_t type){
	struct stat st;
	int r=lstat(path.c_str(),&st);
	return r==0&&(st.st_mode&S_IFMT)==type;
}

bool path_is_block(const string&path){
	return path_is_type(path,S_IFBLK);
}

bool path_is_char(const string&path){
	return path_is_type(path,S_IFCHR);
}

bool path_is_dir(const string&path){
	return path_is_type(path,S_IFDIR);
}

bool path_is_file(const string&path){
	return path_is_type(path,S_IFREG);
}

bool path_is_link(const string&path){
	return path_is_type(path,S_IFLNK);
}

dev_t path_get_dev(const string&path){
	struct stat st;
	int r=stat(path.c_str(),&st);
	if(r!=0)throw runtime_error(format(
		"stat {} failed: {}",path,strerror(errno)
	));
	if(!S_ISBLK(st.st_mode)&&!S_ISCHR(st.st_mode))
		throw runtime_error(format("{} is not a device",path));
	return st.st_rdev;
}

string read_tag(const string&path,const string&tag){
	char buf_line[4096];
	auto f=fopen(path.c_str(),"r");
	if(!f)throw runtime_error(format("open {} failed",path));
	auto ctag=tag.c_str();
	string value="";
	while(fgets(buf_line,sizeof(buf_line),f)){
		auto pos=strchr(buf_line,'=');
		if(!pos)continue;
		auto len=strlen(buf_line);
		while(isspace(buf_line[len-1]))buf_line[--len]=0;
		*pos=0;
		if(strcasecmp(buf_line,ctag)==0)
			value=pos+1;
	}
	fclose(f);
	if(value.empty())throw runtime_error(format("{} not found",tag));
	return value;
}

string read_uevent(const string&folder,const string&tag){
	return read_tag(format("{}/uevent",folder),tag);
}

string dev_to_string(dev_t dev){
	return format("{}:{}",major(dev),minor(dev));
}

string device_to_sysfs(const string&dev){
	string type;
	auto dn=path_get_dev(dev);
	if(path_is_block(dev))type="block";
	else if(path_is_char(dev))type="char";
	else throw runtime_error(format("{} is not a device",dev));
	auto dir=format("/sys/dev/{}/{}",type,dev_to_string(dn));
	if(!path_is_link(dir))throw runtime_error(format("invalid sysfs {}",dir));
	return dir;
}

string part_get_parent(const string&part){
	auto sysfs=device_to_sysfs(part);
	if(!path_is_file(format("{}/partition",sysfs)))
		throw runtime_error(format("device {} is not a block partition",part));
	auto pkname=read_uevent(format("{}/..",sysfs),"DEVNAME");
	if(pkname.empty())throw runtime_error(format("no parent device for {}",part));
	auto pkpath=format("/dev/{}",pkname);
	if(!path_is_block(pkpath))throw runtime_error(format("{} is not a block device",part));
	return pkpath;
}

int part_get_num(const string&part){
	auto sysfs=device_to_sysfs(part);
	auto partn=read_uevent(sysfs,"PARTN");
	if(partn.empty())throw runtime_error(format("no part number for {}",part));
	return stoi(partn);
}
