/*
 *
 * Copyright (C) 2025 BigfootACA <bigfoot@classfun.cn>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include<string>
#include<memory>
#include<stdexcept>
#include<fmt/format.h>
#include"exec.h"
#include"utils.h"
#include"mount.h"
#include"fdisk.h"
#include"readable.h"

using std::string;
using std::make_shared;
using std::invalid_argument;
using std::runtime_error;
using fmt::format;
using fmt::println;

void get_folder_device(string&folder,string&device){
	auto mtab=MountTable::FromMTAB();
	if(folder.empty()&&device.empty())
		throw invalid_argument("no target specified");
	if(folder.empty()&&!device.empty())
		folder=mtab->GetTargetFromSource(device);
	if(!folder.empty()&&device.empty())
		device=mtab->GetSourceFromTarget(folder);
	if(!path_is_dir(folder))
		throw invalid_argument(format("bad folder {}",device));
	if(!path_is_block(device))
		throw invalid_argument(format("bad device {}",folder));
	println("Target folder: {}",folder);
	println("Target partition: {}",device);
}

void get_disk_partn(const string&device,std::string&disk,int&partn){
	disk=part_get_parent(device);
	partn=part_get_num(device);
	println("Target disk: {}",disk);
	println("Target partition number: {}",partn);
}

bool do_partiton(const string&disk,int partn){
	auto ctx=make_shared<FdiskContext>();
	ctx->Assign(disk);
	auto layout=ctx->GetLayoutName("");
	if(layout.empty())throw runtime_error(format(
		"unknown layout in {}",disk
	));
	println("Disk layout: {}",layout);
	auto sector=ctx->GetSectorSize();
	auto old_table=ctx->GetPartitionsTable();
	auto parts=ctx->GetAllPartitions();
	PrintTable(parts,sector,partn);
	size_t idx=SIZE_MAX;
	for(size_t i=0;i<parts.size();i++){
		auto part=parts[i];
		auto num=part->GetPartNumber();
		if(part->IsFreeSpace())continue;
		if(num+1!=(uint64_t)partn)continue;
		idx=i;
		break;
	}
	if(idx==SIZE_MAX)throw runtime_error("partition not found");
	if(idx+1>=parts.size()||!parts[idx+1]->IsFreeSpace()){
		println("No freespace found");
		return false;
	}
	auto old_size=parts[idx]->GetSize();
	auto new_size=old_size+parts[idx+1]->GetSize();
	println(
		"Resize partition from {} sectors ({}) to {} sectors ({})",
		old_size,FormatSize(old_size*sector),
		new_size,FormatSize(new_size*sector)
	);
	parts[idx]->SetSize(new_size);
	ctx->SetPartition(partn-1,parts[idx]);
	println("Saving disk layout...");
	ctx->Save();
	try{
		ctx->RereadTable();
	}catch(...){
		ctx->RereadChanges(old_table);
	}
	return true;
}

void resize_filesystem(const string&dev){
	int ret=0;
	auto mtab=MountTable::FromMTAB();
	auto mnt=mtab->FindSource(dev);
	if(!mnt)throw runtime_error(format(
		"mountpoint for {} not found",dev
	));
	auto folder=mnt->GetTarget();
	auto fstype=mnt->GetFSType();
	println("Filesystem type: {}",fstype);
	if(contains({"ext2","ext3","ext4"},fstype))
		ret=run_external({"resize2fs",dev});
	else if(fstype=="btrfs")
		ret=run_external({"btrfs","filesystem","resize","max",folder});
	else if(fstype=="xfs")
		ret=run_external({"xfs_growfs",folder});
	else throw runtime_error(format("unsupported filesystem {}",fstype));
	if(ret!=0)throw runtime_error(format("resize filesystem failed: {}",ret));
}
