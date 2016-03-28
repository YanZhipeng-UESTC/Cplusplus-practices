#ifndef DBSCAN_H
#define DBSCAN_H
#include <vector>
#include <map>
using namespace std;

struct Dpoint
{
	double x;
	double y;
	double z;
	double w;
	Dpoint(double tx, double ty, double tz, double tw)
	{
		x = tx;
		y = ty;
		z = tz;
		w = tw;
	}
	Dpoint(const Dpoint&) = delete;
	Dpoint& operator=(const Dpoint&) = delete;
};
struct DataNode
{
	Dpoint *data;
	size_t clustNo;
	bool isVisited;
	DataNode() = default;
	~DataNode() 
	{
		if (!data)
		{
			delete data;
			data = nullptr;
		}
	}
};
class DBSCAN
{
private:
	vector<DataNode> dataSet;
	vector<vector<double>> dis;
	map<size_t, vector<size_t>> neibormap;
	size_t count;
	double Esp;
	size_t minPts;
public:
	DBSCAN() = default;
	~DBSCAN() = default;
	void ClusterInit(const char* filename);
	void CreateDisMatrix();
	void ComputeClustArgs();
	void Clust();
	void OutputResult();
private:
	double ComputeDis(const DataNode& d1, const DataNode& d2)const;
	void RecursiveClust(size_t clustNo,size_t coreNo);
};
#endif