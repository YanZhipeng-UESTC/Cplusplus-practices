#ifndef FILEOP_H
#define FILEOP_H
#define Cache_Line_Count 16
#include <stddef.h>
struct cache_line;
enum Where{Begin,Cur,End};
class FileOp
{
public:
	static FileOp* getInstance();
	bool ReadFile(char* const buffer,const size_t count);
	bool WriteFile(const char *buffer);
	bool Lseek(long offset,Where w);
private:
	FileOp();
	~FileOp();
	FileOp(const FileOp&);
	FileOp& operator=(const FileOp&);
	bool FlushCache();
	static void OnProcessExit();
	//size_t SwapLine(size_t LineFlag);
	size_t CatchLine(size_t LineFlag);

	int FD;
	static FileOp *SingleFile;
	size_t File_Offset;
	size_t File_end;
	typedef cache_line Cache[Cache_Line_Count];
	Cache FileCache;

};
#endif
