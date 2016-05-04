#ifndef ASTAR_HPP
#define ASTAR_HPP

#include <iostream>
#include <vector>
#include <cmath>
#include <set>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <tuple>
#include <list>
#include "tsp.h"
//#define DEBUG

struct AStar
{
	struct compNodePtr
	{
		bool operator()(const TSP::Node lhs,const TSP::Node rhs)
		{
			return (lhs) < (rhs);
		}
	};

	AStar(TSP p) : mProblem(p), mStart(p.mStart, NULL, 0.0f)
	{}

	std::vector<TSP::Node> solve()
	{
		std::vector<TSP::Node> frontier;
		std::list<TSP::Node> visited;
		frontier.push_back(mStart);
		visited.push_back(mStart);

#ifdef DEBUG
		expandedNodes = 0;
#endif

		while (!frontier.empty())
		{
			for (unsigned int i = 0; i < frontier.size(); ++i)
			{
				visited.push_back(frontier.front());

				if (mProblem.checkSolution(visited.back())) return buildSolution(visited.back());

				std::pop_heap(frontier.begin(), frontier.end());
				frontier.pop_back();

				std::vector<TSP::Node> successors = mProblem.getSuccessors(visited.back());
				computeHeuristics(successors);
#ifdef DEBUG
				expandedNodes++;
#endif
				for (unsigned int j = 0; j < successors.size(); ++j)
				{
					if (std::find(visited.begin(), visited.end(), successors[j]) == visited.end())
					{
						unsigned int k;
						for (k = 0; k != frontier.size(); ++k)
						{
							if (successors[j].mData.back().mVerts.second == frontier[k].mData.back().mVerts.second)
							{
								if (successors[j].mF < frontier[k].mF)
								{
									frontier[k] = successors[j];
									std::make_heap(frontier.begin(),frontier.end());
									break;
								}
								else break;
							}
						}
						if (k == frontier.size())
						{
							frontier.push_back(successors[j]);
							std::push_heap(frontier.begin(), frontier.end());
						}
					}
				}
			}
		}
		
		return std::vector<TSP::Node>();
	}

	std::vector<TSP::Node> buildSolution(TSP::Node n)
	{
		std::vector<TSP::Node> result;
		const TSP::Node* it = &n;
		while (it != NULL)
		{
			result.push_back(*it);
			it = it->mPrev;
		}
		std::reverse(result.begin(), result.end());
#ifdef DEBUG		
		solutionDepth = result.size();
#endif
		return result;
	}

	void computeHeuristics(std::vector<TSP::Node>& succ)
	{
		for (unsigned int i = 0; i < succ.size(); ++i)
		{
			succ[i].mF += mProblem.heuristic(succ[i].mData);
		}
	}

	TSP mProblem;
	TSP::Node mStart;

#ifdef DEBUG
	int solutionDepth;
	int expandedNodes;
#endif
};

#endif

