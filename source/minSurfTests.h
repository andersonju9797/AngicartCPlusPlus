#ifndef _MINSURFTESTS
#define _MINSURFTESTS 1

#include "wxMinSurfTests.h"

#include <vector>

using namespace std;

extern vector<vector<vector<unsigned int> > > backbonesGlobal, branchpointsGlobal;
extern vector<vector<double> > volumesGlobal; // stored as a double for partial voxels, but is the voxel count, not the physical volume
extern vector<vector<vector<bool> > > branchAtFrontGlobal;
extern vector<vector<double> > aveRadSolidGlobal, aveRadSurfGlobal;
extern double voxdimsGlobal[3];
extern unsigned int voldimsGlobal[3];
extern string lengthUnitGlobal;
extern double defaultPointColor[4];

int main(int argc, char *argv[]);
void wxTest();
void streamlineTest();
void sphereCoarsenTest();
void findClosestSegment(double x, double y, double z);
void normalizedCoord(unsigned int i, double &nx, double &ny, double &nz);
double backboneLength(const vector<unsigned int> &backbone);

template <class S, class T>
double radFromVolLen(S vol, T len){
	return sqrt((vol/len)/acos(-1.0));
}

#endif