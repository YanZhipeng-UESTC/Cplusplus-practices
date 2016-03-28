// DBSCAN.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "DBSCAN.h"

int _tmain(int argc, _TCHAR* argv[])
{
	DBSCAN test;
	test.ClusterInit("iris_Inputs.dat");
	test.CreateDisMatrix();
	test.ComputeClustArgs();
	test.Clust();
	test.OutputResult();
	return 0;
}

