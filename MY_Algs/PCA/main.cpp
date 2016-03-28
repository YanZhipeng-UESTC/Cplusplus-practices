#include "stdafx.h"
#include <iostream>
#include "PCA.h"
using namespace std;
int main()
{
	PCA test;
	test.ParaInit("iris_Inputs.dat", 4, 2);
	test.CalculateCorv();
	test.CalculateEigen();
	test.OutputResult();
}