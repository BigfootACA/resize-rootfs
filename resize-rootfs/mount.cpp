/*
 *
 * Copyright (C) 2025 BigfootACA <bigfoot@classfun.cn>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include<stdexcept>
#include<libmount/libmount.h>
#include<fmt/format.h>
#include"mount.h"

using std::runtime_error;
using std::invalid_argument;
using std::make_shared;
using std::shared_ptr;
using fmt::format;

MountEntry::MountEntry():fs(mnt_new_fs()){
	if(!fs)throw runtime_error("alloc mount fs failed");
}

MountEntry::MountEntry(libmnt_fs*fs):fs(fs){
	if(!fs)throw invalid_argument("bad mount fs");
	mnt_ref_fs(fs);
}

string MountEntry::GetSource(){
	auto val=mnt_fs_get_source(fs);
	return val?val:"";
}

string MountEntry::GetTarget(){
	auto val=mnt_fs_get_target(fs);
	return val?val:"";
}

string MountEntry::GetFSType(){
	auto val=mnt_fs_get_fstype(fs);
	return val?val:"";
}

MountEntry::~MountEntry(){
	if(fs)mnt_unref_fs(fs);
}

MountTable::MountTable():table(mnt_new_table()){
	if(!table)throw runtime_error("alloc mount table failed");
}

MountTable::MountTable(libmnt_table*table):table(table){
	if(!table)throw invalid_argument("bad mount table");
	mnt_ref_table(table);
}

MountTable::~MountTable(){
	if(table)mnt_unref_table(table);
}

shared_ptr<MountEntry>MountTable::FindTarget(const string&tgt){
	auto fs=mnt_table_find_target(table,tgt.c_str(),0);
	return fs?make_shared<MountEntry>(fs):nullptr;
}

shared_ptr<MountEntry>MountTable::FindSource(const string&src){
	auto fs=mnt_table_find_source(table,src.c_str(),0);
	return fs?make_shared<MountEntry>(fs):nullptr;
}

string MountTable::GetTargetFromSource(const string&src){
	auto fs=FindSource(src);
	if(!fs)throw runtime_error(format("no such mountpoint with source {}",src));
	return fs->GetTarget();
}

string MountTable::GetSourceFromTarget(const string&tgt){
	auto fs=FindTarget(tgt);
	if(!fs)throw runtime_error(format("no such mountpoint with target {}",tgt));
	return fs->GetSource();
}

shared_ptr<MountTable>MountTable::FromMTAB(const string&mtab){
	auto tab=make_shared<MountTable>();
	int r=mnt_table_parse_mtab(tab->table,mtab.empty()?nullptr:mtab.c_str());
	if(r<0)throw runtime_error(format("parse mtab failed: {}",r));
	return tab;
}
