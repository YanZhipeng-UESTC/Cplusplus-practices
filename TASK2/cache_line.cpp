#include "cache_line.h"
cache_line::cache_line()
{
	buf=new char[Cache_Line_Size];
	line_flag=0;
	rw_times=0;
	used_bytes=0;
}
cache_line::~cache_line()
{
	delete [] buf;
}
