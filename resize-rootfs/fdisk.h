/*
 *
 * Copyright (C) 2025 BigfootACA <bigfoot@classfun.cn>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef FDISK_H
#define FDISK_H
#include<string>
#include<vector>
#include<memory>

typedef struct fdisk_context fdisk_context;
typedef struct fdisk_table fdisk_table;
typedef struct fdisk_iter fdisk_iter;
typedef struct fdisk_partition fdisk_partition;
using std::string;
using std::vector;
using std::shared_ptr;
using std::make_shared;

enum FdiskLayout{
	LAYOUT_NONE = 0,
	LAYOUT_GPT,
	LAYOUT_MBR,
};

class FdiskIterator{
	public:
		FdiskIterator(int dir=0);
		~FdiskIterator();
		fdisk_iter*iter=nullptr;
};

class FdiskPartition{
	public:
		FdiskPartition();
		FdiskPartition(fdisk_partition*part);
		~FdiskPartition();
		void SetStart(uint64_t off);
		void UnsetStart();
		uint64_t GetStart();
		bool HasStart();
		int CompareStart(shared_ptr<FdiskPartition>other);
		void SetSize(uint64_t off);
		void UnsetSize();
		uint64_t GetSize();
		bool HasSize();
		uint64_t GetEnd();
		bool HasEnd();
		void SetPartNumber(uint64_t off);
		void UnsetPartNumber();
		uint64_t GetPartNumber();
		bool HasPartNumber();
		int ComparePartNumber(shared_ptr<FdiskPartition>other);
		bool IsNested();
		bool IsContainer();
		bool IsUsed();
		bool IsBootable();
		bool IsWholedisk();
		bool IsFreeSpace();
		fdisk_partition*part=nullptr;
};

class FdiskTable{
	public:
		FdiskTable();
		FdiskTable(fdisk_table*table);
		~FdiskTable();
		vector<shared_ptr<FdiskPartition>>GetPartitions();
		void Print(uint64_t sector,int partn=0);
		fdisk_table*table=nullptr;
};

class FdiskContext{
	public:
		FdiskContext();
		FdiskContext(fdisk_context*ctx);
		~FdiskContext();
		void Save();
		void Assign(const string&path);
		FdiskLayout GetLayout();
		string GetLayoutName(string def="(Unknown)");
		uint64_t GetSectorSize();
		shared_ptr<FdiskTable>GetPartitionsTable();
		shared_ptr<FdiskTable>GetAllPartitionsTable();
		std::vector<shared_ptr<FdiskPartition>>GetPartitions();
		std::vector<shared_ptr<FdiskPartition>>GetAllPartitions();
		void SetPartition(uint64_t partno,shared_ptr<FdiskPartition>part);
		shared_ptr<FdiskPartition>GetPartition(uint64_t partno);
		void PrintPartitions(int partn=0);
		void RereadTable();
		void RereadChanges(shared_ptr<FdiskTable>table);
		fdisk_context*ctx=nullptr;
};

extern string FdiskLayoutToString(FdiskLayout layout,string def="(Unknown)");
extern void PrintTable(vector<shared_ptr<FdiskPartition>>table,uint64_t sector,int partn);
#endif
