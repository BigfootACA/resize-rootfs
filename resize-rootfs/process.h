/*
 *
 * Copyright (C) 2025 BigfootACA <bigfoot@classfun.cn>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef PROCESS_H
#define PROCESS_H
#include<string>

using std::string;

extern void get_folder_device(string&folder,string&device);
extern void get_disk_partn(const string&device,string&disk,int&partn);
extern bool do_partiton(const string&disk,int partn);
extern void resize_filesystem(const string&dev);
#endif
