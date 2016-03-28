#include "cache_line.h"
#include "FileOp.h"
#include <iostream>
using namespace std;
int main()
{
	FileOp *fp=FileOp::getInstance();
	if(!fp)
	{
		cerr<<"file open error"<<endl;
		return 0;
	}
	for(int i=0;i!=10000;i++)
	{
		fp->WriteFile("hello world! ");
		fp->WriteFile("my name is xxx123\t");
	}
	char s[101];
	fp->Lseek(4000,Begin);
	fp->ReadFile(s,100);
	cout<<s<<endl;
	const char *ss="this file is end.";
	fp->Lseek(0,End);
	fp->WriteFile(ss);
	return 0;
}
