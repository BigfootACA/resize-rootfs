/*
 *
 * Copyright (C) 2025 BigfootACA <bigfoot@classfun.cn>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include<string>
#include<stdexcept>
#include<fmt/format.h>
#include"utils.h"
#include"process.h"

using std::string;
using std::exception;
using std::invalid_argument;
using fmt::format;
using fmt::println;

int main(int argc,char**argv){
	string folder="/";
	string device="";
	string disk="";
	int partn=0;
	try{
		if(argc==2&&argv[1]){
			string a=argv[1];
			if(path_is_dir(a))device="",folder=a;
			else if(path_is_block(a))device=a,folder="";
			else throw invalid_argument(format("bad path {}",a));
		}else if(argc!=1){
			println(stderr,"Usage: resize-rootfs [FOLDER|DEVICE]");
			return 1;
		}
	}catch(exception&ex){
		println(stderr,"bad argument: {}",ex.what());
		return 1;
	}
	try{
		get_folder_device(folder,device);
		get_disk_partn(device,disk,partn);
		if(do_partiton(disk,partn))
			resize_filesystem(device);
	}catch(exception&ex){
		println(stderr,ex.what());
		return 1;
	}
	return 0;
}
