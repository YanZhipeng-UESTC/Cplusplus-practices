#ifndef TSP_HPP
#define TSP_HPP

#include <iostream>
#include <vector>
#include <cmath>
#include <set>
#include <algorithm>
#include <iterator>
#include <cstdlib>
#include <ctime>
#include <tuple>
#include <list>
#include <fstream>

struct TSP
{
	struct Edge
	{
		Edge(std::pair<int, int> verts, double cost) :
		mVerts(verts), mCost(cost){}
		Edge& operator=(const Edge& e)
		{
			mVerts = e.mVerts;
			mCost = e.mCost;
			return *this;
		}
		friend bool operator==(const Edge& a, const Edge& b)
		{
			return a.mVerts == b.mVerts;
		}

		friend bool operator!=(const Edge& a, const Edge& b)
		{
			return !(a == b);
		}

		friend bool operator<(const Edge& a, const Edge& b)
		{
			return a.mCost < b.mCost;
		}

		std::pair<int, int> mVerts;
		double mCost;
	};

	typedef std::vector<Edge> NodeData;

	struct Node
	{
		Node(NodeData nd, Node* previous, double f) : mData(nd), mPrev(previous), mF(f){}
		Node& operator=(const Node& n)
		{
			mData = n.mData;
			mPrev = n.mPrev;
			mF = n.mF;
			return *this;
		}
		friend bool operator<(const Node& a, const Node& b)
		{
			return a.mF > b.mF;
		}

		friend bool operator!=(const Node& a, const Node& b)
		{
			return a.mData != b.mData;
		}

		friend bool operator==(const Node& a, const Node& b)
		{
			if (a.mData.size() != b.mData.size()) return false;
			for (size_t i = 0; i<a.mData.size(); ++i)
			{
				if (a.mData[i] != b.mData[i]) return false;
			}
			return true;
		}

		NodeData mData;
		Node* mPrev;
		double mF;
	};

	TSP(const char* filename)
	//TSP(const char* filename,unsigned int start)
	{
		//START = start;
		std::ifstream fin(filename);
		if (!fin.is_open())
		{
			std::cerr << "can not open file " << filename << std::endl;
			exit(-1);
		}
		fin.ignore(256, '\n');
		fin.ignore(256, '\n');
		fin.ignore(256, '\n');
		fin.ignore(256, ':');
		int count;
		fin >> count;
		if (count < 0)
		{
			std::cerr << "dataset count is invalid" << std::endl;
			exit(-1);
		}
		unsigned int nVertices = count;
		std::srand(unsigned(time(NULL)));
		START = std::rand()%count;
		adjacencyVec.assign(nVertices, std::vector<Edge>());
		fin.ignore();
		fin.ignore(256, '\n');
		fin.ignore(256, '\n');
		unsigned int z, x, y;
		for (unsigned int i = 0; i != nVertices; ++i)
		{
			fin >> z >> x >> y;
			vertices.push_back({ x, y });
			fin.ignore();
		}
		fin.close();
		for (unsigned int i = 0; i<nVertices - 1; ++i)
		{
			for (unsigned int j = i + 1; j<nVertices; ++j)
			{
		//		double dist = sqrt(pow(vertices[i].first - vertices[j].first, 2) +
		//			pow(vertices[i].second - vertices[j].second, 2));
				double dist = sqrt((pow(vertices[i].first - vertices[j].first, 2) +
					pow(vertices[i].second - vertices[j].second, 2))/10);
				adjacencyVec[i].push_back(Edge(std::make_pair(i, j), dist));
				adjacencyVec[j].push_back(Edge(std::make_pair(j, i), dist));
			}
		}

		for (size_t i = 0; i<adjacencyVec.size(); ++i)
		{
			for (size_t j = 0; j<adjacencyVec[i].size(); ++j)
			{
				sortedEdges.push_back(adjacencyVec[i][j]);
			}
		}
		std::sort(sortedEdges.begin(), sortedEdges.end());

	}

	std::vector<Node> getSuccessors(Node& n)
	{
		std::vector<Node> succ;

		if (n.mData.empty())
		{
			for (unsigned int i = 0; i<adjacencyVec[START].size(); ++i)
			{
				NodeData newSol;
				newSol.push_back(adjacencyVec[START][i]);
				succ.push_back(Node(newSol, NULL, newSol[0].mCost));
			}
			return succ;
		}

		if (n.mData.back().mVerts.second == n.mData.front().mVerts.first)
		{
			return succ;
		}

		int lastV = n.mData.back().mVerts.second;
		for (unsigned int i = 0; i<adjacencyVec[lastV].size(); ++i)
		{
			if (!checkVertexInSolution(n.mData, adjacencyVec[lastV][i].mVerts.second))
			{
				std::vector<Edge> newSol = n.mData;
				newSol.push_back(adjacencyVec[lastV][i]);
				double mG = 0.0f;
				for (auto e : newSol)
					mG += e.mCost;
				succ.push_back(Node(newSol, &n, mG));
				//succ.push_back(Node(newSol, &n, n.mF + adjacencyVec[lastV][i].mCost));
			}
			else if (n.mData.size() == vertices.size() - 1 &&
				adjacencyVec[lastV][i].mVerts.second == n.mData.front().mVerts.first)
			{
				std::vector<Edge> newSol = n.mData;
				newSol.push_back(adjacencyVec[lastV][i]);
				double mG = 0.0f;
				for (auto e : newSol)
					mG += e.mCost;
				succ.push_back(Node(newSol, &n, mG));
				//succ.push_back(Node(newSol, &n, n.mF + adjacencyVec[lastV][i].mCost));
			}
		}
		return succ;
	}

	bool checkVertexInSolution(const std::vector<Edge>& s, int v)
	{
		for (size_t i = 0; i<s.size(); ++i)
		{
			if (s[i].mVerts.first == v || s[i].mVerts.second == v)
				return true;
		}
		return false;
	}

	bool checkSolution(const Node& n)
	{
		if (n.mData.size() == vertices.size() &&
			n.mData.back().mVerts.second == n.mData.front().mVerts.first)
			return true;
		return false;
	}
	double heuristic(const NodeData& n)
	{
		double result = 0.0f;
		NodeData mst;
		std::list<std::vector<unsigned int>> subtrees(adjacencyVec.size());
		size_t i = 0;
		for (auto iter = subtrees.begin(); iter != subtrees.end(); ++iter)
		{
			iter->push_back(i);
			++i;
		}
		for (auto iter1 = n.begin(); iter1 != n.end() - 1; ++iter1)
		{
			for (auto iter2 = subtrees.begin(); iter2 != subtrees.end(); ++iter2)
			{
				if (iter1->mVerts.second == iter2->front())
				{
					subtrees.erase(iter2);
					break;
				}
			}
		}
		for (size_t j = 0; j != sortedEdges.size(); ++j)
		{
			auto set1 = find_set(sortedEdges[j].mVerts.first, subtrees);
			auto set2 = find_set(sortedEdges[j].mVerts.second, subtrees);
			if (set1 != subtrees.end() && set2 != subtrees.end() && set1 != set2)
			{
				mst.push_back(sortedEdges[j]);
				std::copy(set2->begin(), set2->end(), std::back_inserter(*set1));
				subtrees.erase(set2);
			}
			if (subtrees.size() == 1)
				break;
		}
		for (auto iterm : mst)
			result += iterm.mCost;
		return result;
	}
	std::list<std::vector<unsigned int>>::iterator find_set(unsigned int v, std::list<std::vector<unsigned int>>& subtrees)
	{
		for (auto iter = subtrees.begin(); iter != subtrees.end(); ++iter)
		{
			for (auto iterm : *iter)
			if (v == iterm)
				return iter;
		}
		return subtrees.end();
	}
	std::vector<Edge> sortedEdges;
	std::vector<std::vector<Edge> > adjacencyVec;
	std::vector< std::pair<double,double> > vertices;
	NodeData mStart;
	unsigned int START;
};


#endif
