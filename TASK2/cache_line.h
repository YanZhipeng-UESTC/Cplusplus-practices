#ifndef CACHE_LINE_H
#define CACHE_LINE_H
#define Cache_Line_Size 4096
#include <stddef.h>
struct cache_line
{
	char* buf;
	size_t line_flag;
	size_t rw_times;
	size_t used_bytes;
	cache_line();
	~cache_line();
};

#endif
