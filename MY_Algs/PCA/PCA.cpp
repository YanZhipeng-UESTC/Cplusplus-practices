// PCA.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <fstream>
#include <cassert>
#include <cmath>
#include "PCA.h"
using namespace std;

PCA::PCA() :err(0.00001), count(0)
{}
void PCA::ParaInit(const char* filename, size_t dimenBf, size_t dimenAf)
{
	assert(dimenBf >= dimenAf);
	dimensionBefore = dimenBf;
	dimensionAfter = dimenAf;
	fstream fin(filename, ios::in | ios::binary);
	assert(fin.is_open() == true);
	auto off_beg = fin.tellg();
	fin.seekg(0, ios::end);
	auto off_end = fin.tellg();
	count = (off_end - off_beg) / (dimensionBefore*sizeof(double));
	data.assign(count, vector<double>(dimensionBefore, 0));
	fin.seekg(0, ios::beg);
	char* pc = new char[dimensionBefore*sizeof(double)];
	double* pd = reinterpret_cast<double*>(pc);
	for (size_t i = 0; i != count; ++i)
	{
		fin.read(pc, dimensionBefore*sizeof(double));
		for (size_t j = 0; j != dimensionBefore; ++j)
			data[i][j] = pd[j];
	}
	delete[] pc;
	fin.close();
	eigenVector.assign(dimensionBefore, vector<double>(dimensionBefore+1, 0));
	eigenValue.assign(dimensionBefore, 0);
	corvariance.assign(dimensionBefore, vector<double>(dimensionBefore, 0));
}
double PCA::getMax(int& nRow, int& nCol)const
{
	double rMax = corvariance[0][1];
	nRow = 0;
	nCol = 1;

	for (size_t i = 0; i<dimensionBefore; ++i)
		for (size_t j = 0; j<dimensionBefore; ++j)
			if (i != j)
				if (abs(corvariance[i][j]) > rMax)
				{
					rMax = abs(corvariance[i][j]);
					nRow = i;
					nCol = j;
				}
	return rMax;
}
void PCA::CreateJacobian(const int i, const int j, const double rPhi)const
{
	double rSp = sin(rPhi);
	double rCp = cos(rPhi);
	vector<vector<double>> Temp(dimensionBefore);
	for (size_t ii = 0; ii != dimensionBefore; ++ii)
	{
		Temp[ii].assign(corvariance[ii].begin(), corvariance[ii].end());
		corvariance[ii].assign(dimensionBefore, 0);
	}

	for (size_t ii = 0; ii != dimensionBefore; ++ii)
	{
		for (size_t jj = 0; jj != dimensionBefore; ++jj)
		{
			if (ii == i)
			{     // row i
				if (jj == i) 
					corvariance[ii][jj] = Temp[i][i] * rCp*rCp + Temp[j][j] * rSp*rSp + 2 * Temp[i][j] * rCp*rSp;
				else if (jj == j) 
					corvariance[ii][jj] = (Temp[j][j] - Temp[i][i])*rSp*rCp + Temp[i][j] * (rCp*rCp - rSp*rSp);
				else 
					corvariance[ii][jj] = Temp[i][jj] * rCp + Temp[j][jj] * rSp;
			}
			else if (ii == j) 
			{// row j
				if (jj == i) 
					corvariance[ii][jj] = (Temp[j][j] - Temp[i][i])*rSp*rCp + Temp[i][j] * (rCp*rCp - rSp*rSp);
				else if (jj == j) 
					corvariance[ii][jj] = Temp[i][i] * rSp*rSp + Temp[j][j] * rCp*rCp - 2 * Temp[i][j] * rCp*rSp;
				else 
					corvariance[ii][jj] = Temp[j][jj] * rCp - Temp[i][jj] * rSp;
			}
			else {            // row l ( l!=i,j )
				if (jj == i) 
					corvariance[ii][jj] = Temp[i][ii] * rCp + Temp[j][ii] * rSp;
				else if (jj == j) 
					corvariance[ii][jj] = Temp[j][ii] * rCp - Temp[i][ii] * rSp;
				else 
					corvariance[ii][jj] = Temp[ii][jj];
			}
		}
	}
}
void PCA::CalculateCorv()const
{
	vector<double> average(dimensionBefore, 0);
	for (size_t i = 0; i != count; ++i)
		for (size_t j = 0; j != dimensionBefore; ++j)
			average[j] += data[i][j] / count;

	//Calculate TempData
	vector<vector<double>> tempData(count);
	for (size_t i = 0; i != count; ++i)
		tempData[i].assign(data[i].begin(), data[i].end());
	for (size_t i = 0; i != count; ++i)
		for (size_t j = 0; j != dimensionBefore; ++j)
			tempData[i][j] -= average[j];
	for (size_t i = 0; i != dimensionBefore; ++i)
		for (size_t j = 0; j != dimensionBefore; ++j)
			for (size_t k = 0; k != count; ++k)
				corvariance[i][j] += tempData[k][i] * tempData[k][j] / count;
}
void PCA::CalculateEigen()const
{
	vector<vector<double>>eTemp(dimensionBefore,vector<double>(dimensionBefore,0));
	vector<vector<double>>eVec(dimensionBefore,vector<double>(dimensionBefore,0));
	vector<vector<double>>eC(dimensionBefore,vector<double>(dimensionBefore,0));
	for (size_t i = 0; i != dimensionBefore; ++i)
		eTemp[i][i] = 1;
	while (1)
	{
		int i = 0, j = 0;
		double rMax = getMax(i, j);
		if (rMax <= err) break;

		double rPhi = atan2(2 * corvariance[i][j], corvariance[i][i] - corvariance[j][j]) / 2;
		CreateJacobian(i, j, rPhi);
		for (size_t x = 0; x!=dimensionBefore; ++x)
			eC[x][x] = 1;
		eC[j][j] = eC[i][i] = cos(rPhi);
		eC[j][i] = sin(rPhi);
		eC[i][j] = -eC[j][i];

		for (size_t x = 0; x!=dimensionBefore; ++x) // for eigenvectors
			for (size_t y = 0; y!=dimensionBefore; ++y)
				for (size_t z = 0; z!=dimensionBefore; ++z)
					eVec[x][y] = eVec[x][y] + eTemp[x][z] * eC[z][y];

		for (size_t x = 0; x!=dimensionBefore; ++x)
			for (size_t y = 0; y!=dimensionBefore; ++y)
			{
				eTemp[x][y] = eVec[x][y];
				eVec[x][y] = 0;
				eC[x][y] = 0;
			}
	}
	for (size_t i = 0; i!=dimensionBefore; ++i)
		eigenValue[i] = eigenVector[i][dimensionBefore] = corvariance[i][i];
	for (size_t i = 0; i!=dimensionBefore; ++i)
		for (size_t j = 0; j!=dimensionBefore; ++j)
			eigenVector[i][j] = eTemp[j][i];

	//Sort the EigenVector order of the EigenValue
	vector<double> TempEigenVector(dimensionBefore, 0);

	for (size_t i = 0; i!=dimensionBefore; ++i)
		for (size_t j = i; j!=dimensionBefore - 1; ++j)
			if (eigenValue[j]<eigenValue[j + 1])
			{
				double TempEigenValue = eigenValue[j + 1];
				eigenValue[j + 1] = eigenVector[j + 1][dimensionBefore] = eigenValue[j];
				eigenValue[j] = eigenVector[j][dimensionBefore] = TempEigenValue;
				for (size_t k = 0; k!=dimensionBefore; ++k)
					TempEigenVector[k] = eigenVector[j + 1][k];
				for (size_t k = 0; k!=dimensionBefore; ++k)
					eigenVector[j + 1][k] = eigenVector[j][k];
				for (size_t k = 0; k!=dimensionBefore; ++k)
					eigenVector[j][k] = TempEigenVector[k];
			}
}
void PCA::OutputResult()const
{
	vector<vector<double>> result(count, vector<double>(dimensionAfter, 0));
	for (size_t i = 0; i != count;++i)
		for (size_t j = 0; j != dimensionAfter;++j)
			for (size_t k = 0; k != dimensionBefore; ++k)
				{
					result[i][j] += data[i][k] * eigenVector[j][k];
				}
	fstream fout("result2.txt", ios::out);
	assert(fout.is_open());
	for (size_t i = 0; i != count; ++i)
	{
		for (size_t j = 0; j != dimensionAfter; ++j)
			fout << result[i][j] << '\t';
		fout << endl;
	}
	fout.close();
}