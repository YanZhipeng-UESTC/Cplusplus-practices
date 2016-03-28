#ifndef HIERARCHICAL_H
#define HIERARCHICAL_H
#include <vector>
#include <list>
#include <fstream>
using namespace std;

struct MinDistance
{
	double minVal;
	int firstnode;
	int secondnode;
	MinDistance() :minVal(-1.0), firstnode(-1), secondnode(-1){}
	MinDistance(double val, int fst, int scd) :minVal(val), firstnode(fst), secondnode(scd){}
};
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

struct Node
{
	int index;            //data point index; -1 means non-valid
	size_t leafcount;
	Dpoint* pData;   
	Node* leftchild;
	Node* rightchild;
	double level;       //data point level
	Node() :index(-1), leafcount(0), pData(nullptr), leftchild(nullptr), rightchild(nullptr), level(-1.0){}
	Node(double tx, double ty, double tz, double tw) :index(-1), leafcount(0), pData(new Dpoint(tx, ty, tz, tw)), leftchild(nullptr), rightchild(nullptr), level(0){}
	~Node()
	{
		if (pData)
		{
			delete pData;
			pData = nullptr;
		}
	}
};

class Hierarchical
{
public:
	Hierarchical() :root(nullptr), count(0){}
	~Hierarchical(){}
	void ClusterInit(const char* filename);
	void CreateDisMatrix();
	void CreateTree();
	void ComputeClusters(size_t n);
	void OutputResult();
private:
	Node* root;			//root node of binarytree
	vector<Node> dataSet;
	list<Node*> clusters;		//clusters saved as subtrees
	size_t count;		//data count
	vector<vector<double>> dis;		//distance matrix
	MinDistance find1stDMVal();
	void TraverseSubtree(const Node* subtree,fstream& fout);
	double ComputeDis(const Node& node1, const Node& node2)const;
};

#endif