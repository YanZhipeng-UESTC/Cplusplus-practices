// Hierarchical.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include "hierarchical.h"

int _tmain(int argc, _TCHAR* argv[])
{
	Hierarchical h;
	h.ClusterInit("iris_Inputs.dat");
	h.CreateDisMatrix();
	h.CreateTree();
	h.ComputeClusters(3);
	h.OutputResult();
	return 0;
}

