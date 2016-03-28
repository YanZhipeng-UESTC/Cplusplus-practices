#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <cassert>
#include <iostream>
#include "cache_line.h"
#include "FileOp.h"
#define FILE_NAME "test.txt"
using namespace std;

FileOp* FileOp::SingleFile=0;
FileOp::FileOp()
{
	FD=open(FILE_NAME,O_RDWR|O_CREAT,S_IWUSR|S_IRUSR);
	if(FD==-1)
		throw "in FileOp() file open failed";
	File_Offset=0;
	File_end=lseek(FD,0,SEEK_END);
}

FileOp::~FileOp()
{
	if(FD!=-1)
		close(FD);
}

FileOp* FileOp::getInstance()
{
	if(SingleFile==0)
	{
		try
		{
			SingleFile=new FileOp();
		}catch(const char* s)
		{
			cerr<<s<<endl;
			return 0;
		}
		if(atexit(OnProcessExit)!=0)
		{
			return 0;
		}
	}
	return SingleFile;
}

bool FileOp::ReadFile(char* const buffer,const size_t count)
{
	if(buffer==0)
		return false;
	if(count==0)
		return true;
	size_t head=File_Offset/Cache_Line_Size;
	size_t offset=File_Offset%Cache_Line_Size;
	if(offset+count<=Cache_Line_Size)
	{
		size_t curCacheLine=CatchLine(head);
		size_t allInUsed=FileCache[curCacheLine].used_bytes;
		if(offset>=allInUsed)
			return false;
		else if(offset+count>allInUsed)
		{
			memcpy(buffer,FileCache[curCacheLine].buf+offset,allInUsed-offset);
			buffer[allInUsed-offset]='\0';
			File_Offset=File_Offset+allInUsed-offset;
			FileCache[curCacheLine].rw_times+=1;
			return true;
		}
		else
		{
			memcpy(buffer,FileCache[curCacheLine].buf+offset,count);
			buffer[count]='\0';
			File_Offset+=count;
			FileCache[curCacheLine].rw_times+=1;
			return true;
		}
	}
	
	size_t curCacheLine;
	size_t lastCount=count;
	size_t availableCount;
	char *pBuf=buffer;
	size_t allInUsed;
	while(lastCount>0)
	{
		curCacheLine=CatchLine(head);
		allInUsed=FileCache[curCacheLine].used_bytes;
		if(offset>=allInUsed)
			return false;
		else if(allInUsed<Cache_Line_Size)
		{
			memcpy(pBuf,FileCache[curCacheLine].buf+offset,allInUsed-offset);
			pBuf[allInUsed-offset]='\0';
			File_Offset=File_Offset+allInUsed-offset;
			FileCache[curCacheLine].rw_times+=1;
			return true;
		}
		availableCount=Cache_Line_Size-offset;
		if(lastCount<availableCount)
		{
			memcpy(pBuf,FileCache[curCacheLine].buf+offset,lastCount);
			pBuf[lastCount]='\0';
			File_Offset+=count;
			FileCache[curCacheLine].rw_times+=1;
			return true;
		}
		else
		{
			memcpy(pBuf,FileCache[curCacheLine].buf+offset,availableCount);
			pBuf+=availableCount;
			FileCache[curCacheLine].rw_times+=1;
			offset=0;
			head+=1;
			lastCount-=availableCount;
		}
	}
}

bool FileOp::WriteFile(const char* buffer)
{
	if(buffer==0)
		return false;
	const size_t count=strlen(buffer);
	if(count==0)
		return true;
	size_t head=File_Offset/Cache_Line_Size;
	size_t offset=File_Offset%Cache_Line_Size;
	if(offset+count<=Cache_Line_Size)
	{
		size_t curCacheLine=CatchLine(head);
		size_t allInUsed=FileCache[curCacheLine].used_bytes;
		memcpy(FileCache[curCacheLine].buf+offset,buffer,count);
		File_Offset+=count;
		FileCache[curCacheLine].rw_times+=1;
		if(allInUsed<offset+count)
		{
			File_end=File_Offset;
			FileCache[curCacheLine].used_bytes=offset+count;
		}
		return true;
	}

	size_t curCacheLine;
	size_t lastCount=count;
	size_t availableCount;
	const char *pBuf=buffer;
	size_t allInUsed;
	while(lastCount>0)
	{
		curCacheLine=CatchLine(head);
		allInUsed=FileCache[curCacheLine].used_bytes;
		availableCount=Cache_Line_Size-offset;
		if(lastCount<availableCount)
		{
			memcpy(FileCache[curCacheLine].buf+offset,pBuf,lastCount);
			File_Offset+=count;
			FileCache[curCacheLine].rw_times+=1;
			if(allInUsed<offset+lastCount)
			{
				File_end=File_Offset;
				FileCache[curCacheLine].used_bytes=offset+count;
			}
			return true;
		}
		else
		{
			memcpy(FileCache[curCacheLine].buf+offset,pBuf,availableCount);
			pBuf+=availableCount;
			FileCache[curCacheLine].rw_times+=1;
			if(allInUsed<Cache_Line_Size)
				FileCache[curCacheLine].used_bytes=Cache_Line_Size;
			offset=0;
			head+=1;
			lastCount-=availableCount;
		}

	}
}

bool FileOp::Lseek(off_t offset,Where w)
{
	switch (w)
	{
		case Begin:
			if(offset<0)
				return false;
			File_Offset=offset;
			break;
		case Cur:
			{
				off_t temp=File_Offset+offset;
				if(temp<0)
					return false;
				File_Offset=temp;
			}
			break;
		case End:
			{
				off_t temp=File_end+offset;
				if(temp<0)
					return false;
				File_Offset=temp;
			}
			break;
		default:
			return false; 
	}
	return true;
}

//size_t FileOp::SwapLine(size_t LineFlag)
//{
		
//}

size_t FileOp::CatchLine(size_t LineFlag)
{
	if(FD==-1)
		return -1;
	size_t UnusedForLong=0;
	size_t curRWtimes=FileCache[UnusedForLong].rw_times;
	for(int i=0;i!=Cache_Line_Count;++i)
	{
		if(FileCache[i].rw_times<curRWtimes)
		{
			curRWtimes=FileCache[i].rw_times;
			UnusedForLong=i;
		}
		if(LineFlag==FileCache[i].line_flag)
			return i;
	}
	
	if(FileCache[UnusedForLong].rw_times!=0)
	{
		lseek(FD,FileCache[UnusedForLong].line_flag*Cache_Line_Size,SEEK_SET);
		if(write(FD,FileCache[UnusedForLong].buf,FileCache[UnusedForLong].used_bytes)==-1)
			return -1;
	}
	lseek(FD,LineFlag*Cache_Line_Size,SEEK_SET);
	size_t ReadCount=read(FD,FileCache[UnusedForLong].buf,Cache_Line_Size);
	if(ReadCount==-1)
		return -1;
	FileCache[UnusedForLong].line_flag=LineFlag;
	FileCache[UnusedForLong].used_bytes=ReadCount;
	return UnusedForLong;
}

bool FileOp::FlushCache()
{
	for(int i=0;i!=Cache_Line_Count;++i)
	{
		if(FileCache[i].rw_times!=0)
		{
			lseek(FD,FileCache[i].line_flag*Cache_Line_Size,SEEK_SET);
			if(write(FD,FileCache[i].buf,FileCache[i].used_bytes)==-1)
				return false;
		}
	}
	return true;
}

void FileOp::OnProcessExit()
{
	FileOp *pFile=FileOp::getInstance();
	if(pFile!=0)
		assert(pFile->FlushCache()==true);
}
