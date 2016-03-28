#include "stdafx.h"
#include<cmath>
#include<cassert>
#include <iostream>
#include "hierarchical.h"
using namespace std;

double Hierarchical::ComputeDis(const Node& node1, const Node& node2)const
{
	/*whether the two nodes are valid*/
	if (node1.index == -1 || node2.index == -1)
		return 0;
	if (node1.index == node2.index)
		return 0;
	return sqrt(
		pow(node1.pData->x - node2.pData->x, 2) +
		pow(node1.pData->y - node2.pData->y, 2) +
		pow(node1.pData->z - node2.pData->z, 2) +
		pow(node1.pData->w - node2.pData->w, 2)
		);
}
MinDistance Hierarchical::find1stDMVal()
{
	for (size_t i = 1; i != count; ++i)
	{
		for (size_t j = 0; j != i;++j)
			if (dis[i][j] > -1.0)
				return MinDistance(dis[i][j], i, j);
	}
	return MinDistance();	//dis is full of -1
}
void Hierarchical::ClusterInit(const char* filename)
{
	fstream fin(filename, ios::in | ios::binary);
	assert(fin.is_open() == true);
	auto off_beg = fin.tellg();
	fin.seekg(0, ios::end);
	auto off_end = fin.tellg();
	count = (off_end - off_beg) / (sizeof Dpoint);
	dataSet.assign(2 * count - 1, Node());
	fin.seekg(0, ios::beg);
	char* pc = new char[4 * sizeof(double)];
	double *pd = reinterpret_cast<double*>(pc);
	for (size_t i = 0; i != count; ++i)
	{
		fin.read(pc, 4 * sizeof(double));
		dataSet[i].pData = new Dpoint(pd[0], pd[1], pd[2], pd[3]);
		dataSet[i].index = i;
		dataSet[i].leafcount = 1;
	}
	delete [] pc;
	dis.assign(count, vector<double>(count,0.0));
	fin.close();
}
void Hierarchical::CreateDisMatrix()
{
	for (size_t i = 1; i != count; ++i)
	{
		for (size_t j = 0; j != i; ++j)
		{
			dis[i][j] = ComputeDis(dataSet[i], dataSet[j]);
			dis[j][i] = dis[i][j];
			//debug
			//if (dis[i][j] == 0)
				//cout << i << '\t' << j << endl;
		}
	}
}
void Hierarchical::CreateTree()
{
	/*k represents non-leaf node*/
	size_t k = count;
	MinDistance currentMin;
	for (k; k != 2 * count - 1; ++k)
	{
		currentMin = find1stDMVal();
		if (currentMin.minVal == -1.0)
			break;
		for (size_t i = 1; i != count; ++i)
		{	
			for (size_t j = 0; j != i; ++j)
			{
				if (dis[i][j] == -1.0)
					continue;
				if (dis[i][j] < currentMin.minVal)
				{
					currentMin.minVal = dis[i][j];
					currentMin.firstnode = i;
					currentMin.secondnode = j;
				}
			}
		}
		/*find the min dis*/
		dataSet[k].index = currentMin.firstnode;
		dataSet[k].level = currentMin.minVal;
		//debug
		//cout << currentMin.minVal << '\t';
		/*n means index of dataSet*/
		for (size_t n = 0; n != k; ++n)
		{
			if (dataSet[n].index == currentMin.secondnode)
			{
				dataSet[k].rightchild = &dataSet[n];
				dataSet[n].index = -1;
			}
			else if (dataSet[n].index == currentMin.firstnode)
			{
				dataSet[k].leftchild = &dataSet[n];
				dataSet[n].index = -1;
			}
		}
		size_t countL = dataSet[k].leftchild->leafcount;
		size_t countR = dataSet[k].rightchild->leafcount;
		dataSet[k].leafcount = countL + countR;
		/*n means index of dis[i]*/
		//Ñ­»·ÖØÐ´
		/*
		for (size_t n = 0; n != count; ++n)
		{
			if (n == currentMin.firstnode || n == currentMin.secondnode|| dis[currentMin.firstnode][n]==0||dis[currentMin.secondnode][n]==0)
				continue;
			dis[currentMin.firstnode][n] = (dis[currentMin.firstnode][n]*countL + dis[currentMin.secondnode][n]*countR)/(countL+countR);
			dis[n][currentMin.firstnode]=dis[currentMin.firstnode][n];
		}
		*/

		for (size_t n = 0; n != k; ++n)
		{
			int di=dataSet[n].index;
			if ( di != -1)
			{
				dis[currentMin.firstnode][di] = (dis[currentMin.firstnode][di] * countL + dis[currentMin.secondnode][di] * countR) / (countL + countR);
				dis[di][currentMin.firstnode] = dis[currentMin.firstnode][di];
			}
		}

		/*n means index of dis[i]*/
		for (size_t n = 0; n != count; ++n)
		{
			dis[currentMin.secondnode][n] = -1.0;
			dis[n][currentMin.secondnode] = -1.0;
		}
	}
	assert(k == 2 * count - 1);
	root = &dataSet[k-1];
}
void Hierarchical::ComputeClusters(size_t nClusters)
{
	clusters.push_back(root);
	Node* head=nullptr;
	for (size_t i = 0; i != nClusters-1; ++i)
	{
		head = clusters.front();
		if (head->level == -1.0)
			break;
		if (head->leftchild->level > head->rightchild->level)
		{
			clusters.push_back(head->leftchild);
			clusters.push_back(head->rightchild);
		}
		else
		{
			clusters.push_back(head->rightchild);
			clusters.push_back(head->leftchild);
		}
		clusters.pop_front();
	}
}
void Hierarchical::OutputResult()
{
	fstream fout("result.txt", ios::out);
	assert(fout.is_open() == true);
	size_t clusterNum = 0;
	for (auto iter = clusters.cbegin(); iter != clusters.cend(); ++iter)
	{
		fout << "Cluster:" << ++clusterNum << endl;
		TraverseSubtree(*iter,fout);
		fout << endl;
	}
	fout.close();
}
void Hierarchical::TraverseSubtree(const Node* subtree,fstream& fout)
{
	if (subtree->level == -1.0)
	{
		const Dpoint* p=subtree->pData;
		fout << p->x << '\t' << p->y << '\t' << p->z << '\t' << p->w << '\n';
		return;
	}
	else
	{
		TraverseSubtree(subtree->leftchild, fout);
		TraverseSubtree(subtree->rightchild, fout);
		return;
	}
}