/*
 *
 * Copyright (C) 2025 BigfootACA <bigfoot@classfun.cn>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include<vector>
#include<algorithm>
#include<dirent.h>
#include<unistd.h>
#include<stdexcept>
#include<fmt/format.h>
#include<sys/wait.h>
#include<sys/resource.h>
#include"utils.h"
#include"exec.h"

using std::max;
using std::vector;
using std::string;
using std::runtime_error;
using std::invalid_argument;
using fmt::format;
using fmt::println;

int get_max_fd(){
	rlimit rl;
	rlim_t m;
	if(getrlimit(RLIMIT_NOFILE,&rl)<0)return -errno;
	m=max(rl.rlim_cur,rl.rlim_max);
	if(m<FD_SETSIZE)return FD_SETSIZE-1;
	if(m==RLIM_INFINITY||m>INT_MAX)return INT_MAX;
	return (int)(m-1);
}

int close_all_fd(const vector<int>&exclude){
	DIR*d=NULL;
	int r=0,fd,max_fd;
	if(!(d=opendir("/proc/self/fd"))){
		if((max_fd=get_max_fd())<0)return max_fd;
		for(fd=3;fd>=0;fd=fd<max_fd?fd+1:-1)
			if(!contains(exclude,fd))close(fd);
		return r;
	}
	dirent*de;
	while((de=readdir(d))){
		try{fd=std::stoi(de->d_name);}catch(...){continue;}
		if(fd<3||fd==dirfd(d))continue;
		if(!contains(exclude,fd))close(fd);
	}
	closedir(d);
	return r;
}

vector<string>array_to_vector(const char**arr){
	vector<string>ret{};
	for(size_t i=0;arr[i];i++)
		ret.push_back(arr[i]);
	return ret;
}

char**vector_to_string(const vector<string>&strs){
	size_t idx_len=sizeof(char*);
	size_t str_len=0;
	for(auto str:strs){
		idx_len+=sizeof(char*);
		str_len+=str.length()+1;
	}
	auto len=idx_len+str_len;
	auto area=malloc(len);
	if(!area)return nullptr;
	memset(area,0,len);
	auto x_idx=(char**)area;
	auto x_str=(char*)area+idx_len;
	size_t idx=0;
	for(auto str:strs){
		x_idx[idx++]=x_str;
		strcpy(x_str,str.c_str());
		x_str+=str.length()+1;
	}
	return x_idx;
}

vector<string>parse_path(const string&path){
	string remain=path;
	vector<string>ret{};
	while(!remain.empty()){
		auto idx=remain.find(':');
		if(idx==string::npos)break;
		ret.push_back(remain.substr(0,idx));
		remain=remain.substr(idx+1);
	}
	if(!remain.empty())
		ret.push_back(remain);
	return ret;
}

string find_exec_path(const string&name){
	auto p=getenv("PATH");
	if(p)for(auto dir:parse_path(p)){
		auto path=format("{}/{}",dir,name);
		if(access(path.c_str(),X_OK)==0)
			return path;
	}
	throw runtime_error(format("{} not found",name));
}

int run_external(const string&prog,const vector<string>&args,const vector<string>&env){
	int ret=0;
	vector<string>envs=env;
	char**x_args=nullptr;
	char**x_envs=nullptr;
	if(envs.empty())envs=array_to_vector((const char**)environ);
	try{
		x_args=vector_to_string(args);
		x_envs=vector_to_string(envs);
		fflush(stdout);
		fflush(stderr);
		auto f=fork();
		if(f<0)throw runtime_error(format(
			"fork failed: {}",strerror(errno)
		));
		else if(f==0){
			close_all_fd();
			int r=execve(prog.c_str(),x_args,x_envs);
			if(r!=0)println(
				stderr,"execve {} failed: {}",
				prog,strerror(errno)
			);
			free(x_args);
			free(x_envs);
			_exit(255);
		}else if(f>0){
			int st=0,sig;
			while(waitpid(f,&st,0)!=f&&errno!=ECHILD);
			if(WIFEXITED(st))ret=WEXITSTATUS(st);
			else if(WIFSIGNALED(st)&&(sig=WTERMSIG(st))>0){
				ret=128|sig;
			}
			free(x_args);
			free(x_envs);
		}
	}catch(...){
		if(x_args)free(x_args);
		if(x_envs)free(x_envs);
		throw;
	}
	return ret;
}

int run_external(const vector<string>&args,const vector<string>&env){
	if(args.size()<1)throw invalid_argument("no program specified");
	return run_external(find_exec_path(args[0]),args,env);
}
