#include "stdafx.h"
#include "DBSCAN.h"
#include <fstream>
#include <cmath>
#include <cassert>
#include <algorithm>
using namespace std;
double DBSCAN::ComputeDis(const DataNode& d1, const DataNode& d2)const
{
	return sqrt(
		pow(d1.data->x - d2.data->x, 2) +
		pow(d1.data->y - d2.data->y, 2) +
		pow(d1.data->z - d2.data->z, 2) +
		pow(d1.data->w - d2.data->w, 2)
		);
}
void DBSCAN::ClusterInit(const char* filename)
{
	fstream fin(filename, ios::in | ios::binary);
	assert(fin.is_open() == true);
	auto off_beg = fin.tellg();
	fin.seekg(0, ios::end);
	auto off_end = fin.tellg();
	count = (off_end - off_beg) / (4*sizeof(double));
	dataSet.assign(count, DataNode());
	fin.seekg(0, ios::beg);
	char* pc = new char[4 * sizeof(double)];
	double *pd = reinterpret_cast<double*>(pc);
	for (size_t i = 0; i != count; ++i)
	{
		fin.read(pc, 4 * sizeof(double));
		dataSet[i].data = new Dpoint(pd[0], pd[1], pd[2], pd[3]);
		dataSet[i].clustNo = 0;
		dataSet[i].isVisited = false;
	}
	delete [] pc;
	dis.assign(count, vector<double>(count, 0.0));
	fin.close();
}
void DBSCAN::CreateDisMatrix()
{
	for (size_t i = 1; i != count; ++i)
	{
		for (size_t j = 0; j != i; ++j)
		{
			dis[i][j] = ComputeDis(dataSet[i], dataSet[j]);
			dis[j][i] = dis[i][j];
		}
	}
}
void DBSCAN::ComputeClustArgs()
{
	vector<vector<double>> KNN = dis;
	for (size_t i = 0; i != KNN.size(); ++i)
	{
		sort(KNN[i].begin(), KNN[i].end());
	}
	double u1, b1, Esp1, n1;
	double u2, b2, Esp2, n2;
	double sumx,sum1x;
	vector<bool> core;
	size_t noises;
	for (size_t k = 1; k != count; ++k)
	{
		sumx = sum1x = 0.0;
		for (size_t i = 0; i != count; ++i)
		{
			sumx += KNN[i][k];
			sum1x += 1.0 / KNN[i][k];
		}
		u2 = sumx / count;
		b2 = u2*u2*(sum1x / count) - u2;
		Esp2 = (sqrt(9 * b2*b2 + 4 * u2*u2) - 3 * b2) / 2.0;
		core.assign(count, false);
		noises = 0;
		for (size_t i = 0; i != count; ++i)
		{
			if (KNN[i][k] <= Esp2)
				core[i] = true;
		}
		for (size_t i = 0; i != count; ++i)
		{
			if (core[i]==true)
				continue;
			size_t j;
			for (j = 0; j != count; ++j)
			{
				if (dis[i][j]<=Esp2&&core[j]==true)
					break;
			}
			if (j == count)
				++noises;
		}
		n2 = noises;
		if (n2 < k)
		{
			if (abs(n2 - k) <= abs(n1 - k + 1))
			{
				Esp = Esp2;
				minPts = k;
			}
			else
			{
				Esp = Esp1;
				minPts = k - 1;
			}
			break;
		}
		u1 = u2;
		b1 = b2;
		Esp1 = Esp2;
		n1 = n2;
	}
	for (size_t i = 0; i != count; ++i)
	{
		if (KNN[i][minPts] <= Esp)
		{
			vector<size_t> neibor;
			for (size_t j = 0; j != count; ++j)
			{
				if (j != i&&dis[i][j] <= Esp)
					neibor.push_back(j);
			}
			assert(neibor.size() >= minPts);
			neibormap.insert(make_pair(i, neibor));
		}
	}
}
void DBSCAN::Clust()
{
	size_t clusterNumber = 1;
	for (auto iter = neibormap.cbegin(); iter != neibormap.cend(); ++iter)
	{
		if (!dataSet[iter->first].isVisited)
		{
			dataSet[iter->first].isVisited = true;
			dataSet[iter->first].clustNo = clusterNumber;
			RecursiveClust(clusterNumber, iter->first);
			++clusterNumber;
		}
	}
}
void DBSCAN::RecursiveClust(size_t clustNo, size_t coreNo)
{
	auto coreIt = neibormap.find(coreNo);
	assert(coreIt != neibormap.end());
	for (size_t i : coreIt->second)
	{
		if (dataSet[i].isVisited)
			continue;
		dataSet[i].isVisited = true;
		dataSet[i].clustNo = clustNo;
		if (neibormap.find(i) != neibormap.end())
			RecursiveClust(clustNo, i);
	}
}
void DBSCAN::OutputResult()
{
	fstream fout("result.txt", ios::out);
	assert(fout.is_open() == true);
	sort(dataSet.begin(), dataSet.end(), [](const DataNode& node1, const DataNode& node2)->bool
	{
		return node1.clustNo < node2.clustNo;
	});
	for (size_t i = 0; i != count; ++i)
	{
		Dpoint* temp = dataSet[i].data;
		fout << temp->x << '\t' << temp->y << '\t' << temp->z << '\t' << temp->w << '\t' << dataSet[i].clustNo << endl;
	}
	fout.close();
}