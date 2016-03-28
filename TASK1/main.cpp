#include <fcntl.h>
#include <unistd.h>
#include <iostream>
using namespace std;
int main()
{
	int fd=open("test.txt",O_RDWR|O_APPEND);
	if(fd<0)
	{
		cerr<<"file open failed!"<<endl;
		return 0;
	}
	off_t old_offset=lseek(fd,0,SEEK_END);
	cout<<"old offset is :"<<old_offset<<endl;
	off_t new_offset=lseek(fd,0,SEEK_SET);
	cout<<"new offset is :"<<new_offset<<endl;
	char c;
	read(fd,&c,1);
	cout<<c<<endl;
	return 0;
}
