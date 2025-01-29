/*
 *
 * Copyright (C) 2025 BigfootACA <bigfoot@classfun.cn>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include<string>
#include<stdexcept>
#include<libfdisk/libfdisk.h>
#include<fmt/format.h>
#include"fdisk.h"
#include"readable.h"

using std::string;
using std::runtime_error;
using std::invalid_argument;
using fmt::format;
using fmt::println;

string FdiskLayoutToString(FdiskLayout layout,string def){
	switch(layout){
		case LAYOUT_GPT:return "gpt";
		case LAYOUT_MBR:return "mbr";
		default:return def;
	}
}

void PrintTable(vector<shared_ptr<FdiskPartition>>table,uint64_t sector,int partn){
	println("Partitions:");
	println(
		"     No |"
		"         Start         |"
		"          End          |"
		"          Size         |"
	);
	for(auto part:table){
		string str_num="";
		string str_sec_start="",str_sec_end="",str_sec_size="";
		string str_fmt_start="",str_fmt_end="",str_fmt_size="";
		auto num=part->GetPartNumber();
		if(!part->IsFreeSpace())
			str_num=std::to_string(num+1);
		if(part->HasStart()){
			auto val=part->GetStart();
			str_fmt_start=FormatSize(val*sector);
			str_sec_start=format("{}s",val);
		}
		if(part->HasEnd()){
			auto val=part->GetEnd();
			str_fmt_end=FormatSize(val*sector);
			str_sec_end=format("{}s",val);
		}
		if(part->HasSize()){
			auto val=part->GetSize();
			str_fmt_size=FormatSize(val*sector);
			str_sec_size=format("{}s",val);
		}
		println(
			"  {:c}  {:>2s} | "
			"{:>10s} {:>10s} | "
			"{:>10s} {:>10s} | "
			"{:>10s} {:>10s} |",
			num+1==(uint64_t)partn?'*':' ',str_num,
			str_sec_start,str_fmt_start,
			str_sec_end,str_fmt_end,
			str_sec_size,str_fmt_size
		);
	}
}

FdiskIterator::FdiskIterator(int dir):iter(fdisk_new_iter(dir)){
	if(!iter)throw runtime_error("alloc fdisk iterator failed");
}

FdiskIterator::~FdiskIterator(){
	if(iter)fdisk_free_iter(iter);
}

FdiskPartition::FdiskPartition():part(fdisk_new_partition()){
	if(!part)throw runtime_error("alloc fdisk partition failed");
}

FdiskPartition::FdiskPartition(fdisk_partition*part):part(part){
	if(!part)throw invalid_argument("bad fdisk partition");
	fdisk_ref_partition(part);
}

FdiskPartition::~FdiskPartition(){
	if(part)fdisk_unref_partition(part);
}

void FdiskPartition::SetStart(uint64_t off){
	int r=fdisk_partition_set_start(part,off);
	if(r<0)throw runtime_error(format("failed to set partition start: %d",r));
}

void FdiskPartition::UnsetStart(){
	int r=fdisk_partition_unset_start(part);
	if(r<0)throw runtime_error(format("failed to unset partition start: %d",r));
}

uint64_t FdiskPartition::GetStart(){
	return fdisk_partition_get_start(part);
}

bool FdiskPartition::HasStart(){
	return fdisk_partition_has_start(part)!=0;
}

int FdiskPartition::CompareStart(shared_ptr<FdiskPartition>other){
	return fdisk_partition_cmp_start(part,other->part);
}

void FdiskPartition::SetSize(uint64_t off){
	int r=fdisk_partition_set_size(part,off);
	if(r<0)throw runtime_error(format("failed to set partition size: %d",r));
}

void FdiskPartition::UnsetSize(){
	int r=fdisk_partition_unset_size(part);
	if(r<0)throw runtime_error(format("failed to unset partition size: %d",r));
}

uint64_t FdiskPartition::GetSize(){
	return fdisk_partition_get_size(part);
}

bool FdiskPartition::HasSize(){
	return fdisk_partition_has_size(part)!=0;
}

uint64_t FdiskPartition::GetEnd(){
	return fdisk_partition_get_end(part);
}

bool FdiskPartition::HasEnd(){
	return fdisk_partition_has_end(part)!=0;
}

void FdiskPartition::SetPartNumber(uint64_t id){
	int r=fdisk_partition_set_partno(part,id);
	if(r<0)throw runtime_error(format("failed to set partition number: %d",r));
}

void FdiskPartition::UnsetPartNumber(){
	int r=fdisk_partition_unset_partno(part);
	if(r<0)throw runtime_error(format("failed to unset partition number: %d",r));
}

uint64_t FdiskPartition::GetPartNumber(){
	return fdisk_partition_get_partno(part);
}

bool FdiskPartition::HasPartNumber(){
	return fdisk_partition_has_partno(part)!=0;
}

int FdiskPartition::ComparePartNumber(shared_ptr<FdiskPartition>other){
	return fdisk_partition_cmp_start(part,other->part);
}

bool FdiskPartition::IsNested(){
	return fdisk_partition_is_nested(part);
}

bool FdiskPartition::IsContainer(){
	return fdisk_partition_is_container(part);
}

bool FdiskPartition::IsUsed(){
	return fdisk_partition_is_used(part);
}

bool FdiskPartition::IsBootable(){
	return fdisk_partition_is_bootable(part);
}

bool FdiskPartition::IsWholedisk(){
	return fdisk_partition_is_wholedisk(part);
}

bool FdiskPartition::IsFreeSpace(){
	return fdisk_partition_is_freespace(part);
}

FdiskTable::FdiskTable():table(fdisk_new_table()){
	if(!table)throw runtime_error("alloc fdisk table failed");
}

FdiskTable::FdiskTable(fdisk_table*table):table(table){
	if(!table)throw invalid_argument("bad fdisk table");
	fdisk_ref_table(table);
}

FdiskTable::~FdiskTable(){
	if(table)fdisk_unref_table(table);
}

vector<shared_ptr<FdiskPartition>>FdiskTable::GetPartitions(){
	fdisk_partition*part=nullptr;
	FdiskIterator iter{};
	vector<shared_ptr<FdiskPartition>>ret{};
	while(fdisk_table_next_partition(table,iter.iter,&part)==0)
		ret.push_back(make_shared<FdiskPartition>(part));
	return ret;
}

void FdiskTable::Print(uint64_t sector,int partn){
	PrintTable(GetPartitions(),sector,partn);
}

FdiskContext::FdiskContext():ctx(fdisk_new_context()){
	if(!ctx)throw runtime_error("alloc fdisk context failed");
}

FdiskContext::FdiskContext(fdisk_context*ctx):ctx(ctx){
	if(!ctx)throw invalid_argument("bad fdisk context");
	fdisk_ref_context(ctx);
}

FdiskContext::~FdiskContext(){
	if(ctx)fdisk_unref_context(ctx);
}

void FdiskContext::Assign(const std::string&path){
	int r=fdisk_assign_device(ctx,path.c_str(),0);
	if(r!=0)throw runtime_error(format("open device {} failed: {}",path,r));
}

void FdiskContext::Save(){
	int r=fdisk_write_disklabel(ctx);
	if(r!=0)throw runtime_error(format("save disk label failed: {}",r));
}

FdiskLayout FdiskContext::GetLayout(){
	if(fdisk_is_labeltype(ctx,FDISK_DISKLABEL_GPT))
		return LAYOUT_GPT;
	if(fdisk_is_labeltype(ctx,FDISK_DISKLABEL_DOS))
		return LAYOUT_MBR;
	return LAYOUT_NONE;
}

string FdiskContext::GetLayoutName(string def){
	return FdiskLayoutToString(GetLayout(),def);
}

uint64_t FdiskContext::GetSectorSize(){
	return fdisk_get_sector_size(ctx);
}

shared_ptr<FdiskTable>FdiskContext::GetPartitionsTable(){
	int r;
	fdisk_table*table=nullptr;
	r=fdisk_get_partitions(ctx,&table);
	if(r!=0)throw runtime_error(format("get partitions failed: {}",r));
	auto ret=std::make_shared<FdiskTable>(table);
	fdisk_unref_table(table);
	return ret;
}

shared_ptr<FdiskTable>FdiskContext::GetAllPartitionsTable(){
	int r;
	fdisk_table*table=nullptr;
	r=fdisk_get_partitions(ctx,&table);
	if(r!=0)throw runtime_error(format("get partitions failed: {}",r));
	r=fdisk_get_freespaces(ctx,&table);
	if(r!=0)throw runtime_error(format("get free spaces failed: {}",r));
	auto ret=std::make_shared<FdiskTable>(table);
	fdisk_unref_table(table);
	return ret;
}

std::vector<shared_ptr<FdiskPartition>>FdiskContext::GetPartitions(){
	return GetPartitionsTable()->GetPartitions();
}

std::vector<shared_ptr<FdiskPartition>>FdiskContext::GetAllPartitions(){
	return GetAllPartitionsTable()->GetPartitions();
}

void FdiskContext::SetPartition(uint64_t partno,shared_ptr<FdiskPartition>part){
	int r=fdisk_set_partition(ctx,partno,part->part);
	if(r!=0)throw runtime_error(format("set partition {} failed: {}",partno,r));
}

shared_ptr<FdiskPartition>FdiskContext::GetPartition(uint64_t partno){
	fdisk_partition*part=nullptr;
	int r=fdisk_get_partition(ctx,partno,&part);
	if(r!=0||!part)throw runtime_error(format("get partition {} failed: {}",partno,r));
	return make_shared<FdiskPartition>(part);
}

void FdiskContext::PrintPartitions(int partn){
	PrintTable(GetPartitions(),GetSectorSize(),partn);
}

void FdiskContext::RereadTable(){
	int r=fdisk_reread_partition_table(ctx);
	if(r!=0)throw runtime_error(format("reread partition table failed: {}",r));
}

void FdiskContext::RereadChanges(shared_ptr<FdiskTable>table){
	int r=fdisk_reread_changes(ctx,table->table);
	if(r!=0)throw runtime_error(format("reread partition changes failed: {}",r));
}
