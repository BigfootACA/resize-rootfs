/*
 *
 * Copyright (C) 2025 BigfootACA <bigfoot@classfun.cn>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef MOUNT_H
#define MOUNT_H
#include<string>
#include<memory>

typedef struct libmnt_fs libmnt_fs;
typedef struct libmnt_table libmnt_table;
using std::string;
using std::shared_ptr;

class MountEntry{
	public:
		MountEntry();
		MountEntry(libmnt_fs*fs);
		string GetSource();
		string GetTarget();
		string GetFSType();
		~MountEntry();
		libmnt_fs*fs=nullptr;
};

class MountTable{
	public:
		MountTable();
		MountTable(libmnt_table*table);
		~MountTable();
		shared_ptr<MountEntry>FindTarget(const string&tgt);
		shared_ptr<MountEntry>FindSource(const string&src);
		string GetTargetFromSource(const string&src);
		string GetSourceFromTarget(const string&tgt);
		libmnt_table*table=nullptr;
		static shared_ptr<MountTable>FromMTAB(const string&mtab="");
};
#endif
