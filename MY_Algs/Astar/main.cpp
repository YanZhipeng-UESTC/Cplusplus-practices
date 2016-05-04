#include "astar.h"
#include "tsp.h"

int main(int argc, char* argv[])
{
	std::cout << "each edge of the final path are as follows..." << std::endl;
	std::cout << '\n' << "Cost" << '\t' << "FirstNode" << '\t' << "SecondNode" << std::endl;
	//for (unsigned int st =0; st != 48; ++st)
	//{
		clock_t start, end;
		TSP tsp("att48.tsp");
		AStar astar(tsp);
		double totalcost = 0.0f;
		start = clock();
		auto path = astar.solve();
		for (auto& edge : path.back().mData)
		{
			std::cout << edge.mCost << '\t' << edge.mVerts.first << '\t' << edge.mVerts.second << std::endl;
			totalcost += edge.mCost;
		}
		end = clock();
		unsigned long msecs = ((double)(end - start)) * 1000 / CLOCKS_PER_SEC;
		std::cout << '\n' << "Cost: " << totalcost <<'\t'<< "Time: " << msecs << std::endl;
	//}

	return 0;
}
