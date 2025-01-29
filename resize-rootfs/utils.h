/*
 *
 * Copyright (C) 2025 BigfootACA <bigfoot@classfun.cn>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef UTILS_H
#define UTILS_H
#include<string>
#include<vector>
#include<algorithm>

using std::find;
using std::vector;
using std::string;

extern bool path_is_type(const string&path,mode_t type);
extern bool path_is_block(const string&path);
extern bool path_is_char(const string&path);
extern bool path_is_dir(const string&path);
extern bool path_is_file(const string&path);
extern bool path_is_link(const string&path);
extern dev_t path_get_dev(const string&path);
extern string read_tag(const string&path,const string&tag);
extern string read_uevent(const string&folder,const string&tag);
extern string dev_to_string(dev_t dev);
extern string device_to_sysfs(const string&dev);
extern string part_get_parent(const string&part);
extern int part_get_num(const string&part);

template<typename T>
static bool contains(const vector<T>&vec,T val){
	return find(vec.begin(),vec.end(),val)!=vec.end();
}

#endif
