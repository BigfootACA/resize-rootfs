/*
 *
 * Copyright (C) 2025 BigfootACA <bigfoot@classfun.cn>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef EXEC_H
#define EXEC_H
#include<vector>
#include<string>

using std::vector;
using std::string;

extern int get_max_fd();
extern int close_all_fd(const vector<int>&exclude={});
extern vector<string>array_to_vector(const char**arr);
extern char**vector_to_string(const vector<string>&strs);
extern vector<string>parse_path(const string&path);
extern int run_external(const string&prog,const vector<string>&args,const vector<string>&env={});
extern int run_external(const vector<string>&args,const vector<string>&env={});
#endif