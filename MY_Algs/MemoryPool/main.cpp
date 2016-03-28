#include "pool_alloc.h"
#include <iostream>
#include <windows.h>
using namespace std;
int main()
{
	pool_alloc<int> alloc;
	int* arr[10];
	DWORD time_start = GetTickCount();
	for (size_t i = 0; i != 1000000; ++i)
	{
		for (size_t j = 0; j != 10; ++j)
		{
			 arr[j]= alloc.allocate(2);
		}
		for (size_t j = 0; j != 10; ++j)
		{
			alloc.deallocate(arr[j],sizeof(int));
		}
	}
	DWORD time_end = GetTickCount();
	cout << (time_end - time_start) << endl;
	return 0;
}