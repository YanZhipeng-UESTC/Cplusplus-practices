#ifndef PCA_H
#define PCA_H
#include <vector>
using namespace std;
class PCA
{
private:
	mutable vector<vector<double>> data;
	mutable vector<vector<double>> corvariance;
	mutable vector<vector<double>> eigenVector;
	mutable vector<double> eigenValue;
	const double err;
	size_t dimensionBefore;
	size_t dimensionAfter;
	size_t count;
public:
	PCA();
	~PCA(){}
	void ParaInit(const char* filename, size_t dimenBf, size_t dimenAf);
	void CalculateCorv()const;
	void CalculateEigen()const;
	void OutputResult()const;
private:
	double getMax(int& nRow, int& nCol)const;
	void CreateJacobian(const int i, const int j, const double rPhi)const;
};
#endif // PCA_H