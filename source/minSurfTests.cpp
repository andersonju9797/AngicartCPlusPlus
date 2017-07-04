/**
 minSurfTests.cpp
 Purpose: Runnable file that analyzes structure and creates 3D visualization
 
 @author David Hunt
 @version 1.1 7/3/17
 */

#include <algorithm>
#include <ctime>
#include <iostream>
#include <fstream>
#include <map>
#include <queue>
#include <sstream>
#include "kiss.h"
#include "lodepng.h"
#include "BinaryVolume.h"
#include "Lumens.h"
#include "minSurfTests.h"
#include "pngMinSurf.h"
#include "utilMinSurfTests.h"
#if TRY_WX == 1
#include "wxMinSurfTests.h"
#endif

time_t startTime;
vector<vector<vector<unsigned int> > > backbonesGlobal, branchpointsGlobal;
vector<vector<double> > volumesGlobal, aveRadSolidGlobal, aveRadSurfGlobal;
vector<vector<vector<bool> > > branchAtFrontGlobal;
double voxdimsGlobal[3] = {0.0, 0.0, 0.0};
unsigned int voldimsGlobal[3] = {0, 0, 0};
double defaultPointColor[] = {0.0, 0.0, 0.0, 0.2};
string lengthUnitGlobal("units");

/** 
 * Returns true if all elements of v are false
 * @param v vector
 */
bool allFalse(const vector<bool> &v){
	for(unsigned int i(0); i < v.size(); i++){
		if(v[i])
			return false;
	}
	return true;
}

/**
 * Returns index of first false found; If not found, returns size of v
 * @param v vector
 */
unsigned int findFirstFalse(const vector<bool> &v){
	for(unsigned int i(0); i < v.size(); i++){
		if(!v[i])
			return i;
	}
	return (unsigned int)v.size();
}

/**
 * Returns true if v is all true
 * @param v vector
 */
bool allTrue(const vector<bool> &v){
	return findFirstFalse(v) == v.size();
}

bool xyz(unsigned int i, unsigned int &x, unsigned int &y, unsigned int &z){
	if(voldimsGlobal[0] == 0 || voldimsGlobal[1] == 0 || voldimsGlobal[2] == 0)
		return false;
	x = i/(voldimsGlobal[1]*voldimsGlobal[2]);
	y = (i%(voldimsGlobal[1]*voldimsGlobal[2]))/voldimsGlobal[2];
	z = i%voldimsGlobal[2];
	return true;
}

template <class T>
T EuclideanDistanceSq(T x1, T y1, T z1, T x2, T y2, T z2){
	T x(x2 - x1), y(y2 - y1), z(z2 - z1);
	return x*x + y*y + z*z;
}

unsigned int EuclideanDistanceSq(unsigned int x1, unsigned int y1, unsigned int z1, unsigned int x2, unsigned int y2, unsigned int z2){
	unsigned int x(absDiff(x2, x1)), y(absDiff(y2, y1)), z(absDiff(z2, z1));
	return x*x + y*y + z*z;
}

double EuclideanDistanceSq(const BinaryVolume &B, unsigned int a, unsigned int b){
	return EuclideanDistanceSq(B.x(a), B.y(a), B.z(a), B.x(b), B.y(b), B.z(b));
}

double EuclideanDistanceSq(const Lumens &L, unsigned int a, unsigned int b){
	return EuclideanDistanceSq(L.x(a), L.y(a), L.z(a), L.x(b), L.y(b), L.z(b));
}

double EuclideanDistance(const Lumens &L, unsigned int a, unsigned int b){
	return sqrt(double(EuclideanDistanceSq(L.x(a), L.y(a), L.z(a), L.x(b), L.y(b), L.z(b))));
}

double EuclideanDistance(const BinaryVolume &B, unsigned int a, unsigned int b){
	return sqrt(double(EuclideanDistanceSq(B.x(a), B.y(a), B.z(a), B.x(b), B.y(b), B.z(b))));
}

template <class T>
double EuclideanDistance(T x1, T y1, T z1, T x2, T y2, T z2){
	return sqrt(double(EuclideanDistanceSq(x1, y1, z1, x2, y2, z2)));
}

double separationSq3D(double ax, double ay, double az, double bx, double by, double bz){
	return (ax - bx)*(ax - bx) + (ay - by)*(ay - by) + (az - bz)*(az - bz);
}

/**
 * Use voxdimsGlobal to computer physical separation
 * @param B generated binary volume
 * @param i
 * @param j
 */
double separation3D(const BinaryVolume &B, unsigned int i, unsigned int j){
	return sqrt(separationSq3D(B.x(i)*voxdimsGlobal[0], B.y(i)*voxdimsGlobal[1], B.z(i)*voxdimsGlobal[2],
		B.x(j)*voxdimsGlobal[0], B.y(j)*voxdimsGlobal[1], B.z(j)*voxdimsGlobal[2]));
}

/**
 * Uses voxdimsGLobal to compute physical separation and xyz to find 3D coordinates
 * @param i
 * @param j
 */
double separation3D(unsigned int i, unsigned int j){
	unsigned int xi, yi, zi, xj, yj, zj;
	xyz(i, xi, yi, zi);
	xyz(j, xj, yj, zj);
	return sqrt(separationSq3D(xi*voxdimsGlobal[0], yi*voxdimsGlobal[1], zi*voxdimsGlobal[2],
		xj*voxdimsGlobal[0], yj*voxdimsGlobal[1], zj*voxdimsGlobal[2]));
}

/**
 * Produces a BinaryVolume from the intensities in L based on the threshold (thresh)
 * @param L lumen
 * @param thresh given thresholds
 */
BinaryVolume threshThrash(const Lumens &L, double thresh){
	double minL(L.minLumen()), maxL(L.maxLumen());
	BinaryVolume B(L.size[0], L.size[1], L.size[2]);
	if(minL == maxL)
		return B;
	for(unsigned int x(0); x < L.size[0]; x++){
		for(unsigned int y(0); y < L.size[1]; y++){
			for(unsigned int z(0); z < L.size[2]; z++){
				if((L.lumens[x][y][z] - minL)/(maxL - minL) > thresh)
					B.t(x, y, z);
			}
		}
	}
	return B;
}

bool inNeighborhood(unsigned int i, unsigned int c, const BinaryVolume &B){
	return absDiff(B.x(i), B.x(c)) < 2 && absDiff(B.y(i), B.y(c)) < 2 && absDiff(B.z(i), B.z(c)) < 2;
}

/**
 * Finds indices in v that are in the neighborhood of i
 * Will include i itself if i is in v
 * @param B binary volume
 * @param i
 * @param v vector
 */
vector<unsigned int> findNeighbors(const BinaryVolume &B, unsigned int i, const vector<unsigned int> &v){
	vector<unsigned int> nh;
	for(unsigned int j(0); j < v.size(); j++){
		if(inNeighborhood(i, v[j], B))
			nh.push_back(v[j]);
	}
	return nh;
}

/**
 * Returns a vector of sets of frontiers that are connected components
 * @param frontier
 * @param B binary volume
 */
vector<vector<unsigned int> > segregateConnectedComponents(const vector<unsigned int> &frontier, const BinaryVolume &B){
	vector<unsigned int> fRemains(frontier);
	vector<vector<unsigned int> > ccs;
	while(fRemains.size() > 0){ // to get all connected components
		ccs.push_back(vector<unsigned int>(1, fRemains[0]));
		vector<unsigned int> fAdd(1, fRemains[0]);
		fRemains.erase(fRemains.begin());
		while(fAdd.size() > 0 && fRemains.size() > 0){ // to get all points in connected component
			for(int i(0); i < (int)fRemains.size(); i++){
				if(inNeighborhood(fAdd[0], fRemains[i], B)){
					fAdd.push_back(fRemains[i]);
					ccs.back().push_back(fAdd.back());
					fRemains.erase(fRemains.begin() + i);
					i--;
				}
			}
			fAdd.erase(fAdd.begin());
		}
	}
	return ccs;
}

// does not include point (x, y, z) itself
vector<unsigned int> vesselBlocksInNeighborhood(const BinaryVolume &B, unsigned int x, unsigned int y, unsigned int z){
	unsigned int xStart(0), yStart(0), zStart(0);
	if(x > 0)
		xStart = x - 1;
	if(y > 0)
		yStart = y - 1;
	if(z > 0)
		zStart = z - 1;
	vector<unsigned int> vesselBlockIndices;
	for(unsigned int i(xStart); i < x + 2 && i < B.size[0]; i++){
		for(unsigned int j(yStart); j < y + 2 && j < B.size[1]; j++){
			for(unsigned int k(zStart); k < z + 2 && k < B.size[2]; k++){
				if(i == x && j == y && k == z) // might be unnecessary when optimized...
					continue;
				if(B.is(i, j, k))
					vesselBlockIndices.push_back(B.indexOf(i, j, k));
			}
		}
	}
	return vesselBlockIndices;
}

// does not include point (x, y, z) itself
vector<unsigned int> blocksInNeighborhood(const BinaryVolume &B, unsigned int x, unsigned int y, unsigned int z){
	unsigned int xStart(0), yStart(0), zStart(0);
	if(x > 0)
		xStart = x - 1;
	if(y > 0)
		yStart = y - 1;
	if(z > 0)
		zStart = z - 1;
	vector<unsigned int> blockIndices;
	for(unsigned int i(xStart); i < x + 2 && i < B.size[0]; i++){
		for(unsigned int j(yStart); j < y + 2 && j < B.size[1]; j++){
			for(unsigned int k(zStart); k < z + 2 && k < B.size[2]; k++){
				if(i == x && j == y && k == z) // might be unnecessary when optimized...
					continue;
				blockIndices.push_back(B.indexOf(i, j, k));
			}
		}
	}
	return blockIndices;
}

// does not include point i itself
vector<unsigned int> blocksInNeighborhood(const BinaryVolume &B, unsigned int i){
	unsigned int x(B.x(i)), y(B.y(i)), z(B.z(i)), xStart(0), yStart(0), zStart(0);
	if(x > 0)
		xStart = x - 1;
	if(y > 0)
		yStart = y - 1;
	if(z > 0)
		zStart = z - 1;
	vector<unsigned int> blockIndices;
	for(unsigned int i(xStart); i < x + 2 && i < B.size[0]; i++){
		for(unsigned int j(yStart); j < y + 2 && j < B.size[1]; j++){
			for(unsigned int k(zStart); k < z + 2 && k < B.size[2]; k++){
				if(i == x && j == y && k == z) // might be unnecessary when optimized...
					continue;
				blockIndices.push_back(B.indexOf(i, j, k));
			}
		}
	}
	return blockIndices;
}

// does not include index itself
vector<unsigned int> vesselBlocksInNeighborhood(const BinaryVolume &B, unsigned int index){
	return vesselBlocksInNeighborhood(B, B.x(index), B.y(index), B.z(index));
}

// does not include index itself
unsigned int numVesselBlocksInNeighborhood(const BinaryVolume &B, unsigned int index){
	unsigned int x(B.x(index)), y(B.y(index)), z(B.z(index)), xStart(0), yStart(0), zStart(0);
	if(x > 1)
		xStart = x - 1;
	if(y > 1)
		yStart = y - 1;
	if(z > 1)
		zStart = z - 1;
	unsigned int numVesselBlocks(0);
	for(unsigned int i(xStart); i < x + 2 && i < B.size[0]; i++){
		for(unsigned int j(yStart); j < y + 2 && j < B.size[1]; j++){
			for(unsigned int k(zStart); k < z + 2 && k < B.size[2]; k++){
				if(i == x && j == y && k == z) // might be unnecessary when optimized...
					continue;
				if(B.is(i, j, k))
					numVesselBlocks++;
			}
		}
	}
	return numVesselBlocks;
}

/**
 * Returns the index in the binary volume of the center of mass of the frontier, each point weighed identically as a cube
 * @param B binary volume
 * @param frontier
 */
unsigned int centerOfMass(const BinaryVolume &B, const vector<unsigned int> &frontier){
	if(frontier.size() < 1)
		return B.totalSize();
	unsigned int xSum(0), ySum(0), zSum(0);
	for(unsigned int i(0); i < frontier.size(); i++){
		xSum += B.x(frontier[i]);
		ySum += B.y(frontier[i]);
		zSum += B.z(frontier[i]);
	}
	return B.indexOf(xSum/frontier.size(), ySum/frontier.size(), zSum/frontier.size());
}

/**
 * Returns the index in B of the point in frontier that is nearest to the center off mass of frontier, each point weighted identically as cube * @param L lumen
 * @param B binary volume
 * @param frontier
 */
unsigned int centerOfMassVessel(const BinaryVolume &B, const vector<unsigned int> &frontier){
	if(frontier.size() < 1)
		return B.totalSize();
	unsigned int com(centerOfMass(B, frontier));
	if(com == B.totalSize())
		return com;
	unsigned int bestFrontier(frontier[0]);
	double bestDistanceSq(EuclideanDistanceSq(B, com, frontier[0]));
	for(unsigned int i(1); i < frontier.size(); i++){
		double thisDistanceSq(EuclideanDistanceSq(B, com, frontier[i]));
		if(bestDistanceSq > thisDistanceSq){
			bestDistanceSq = thisDistanceSq;
			bestFrontier = frontier[i];
		}
	}
	return bestFrontier;
}

/**
 * Returns the index in v that is the closest to the center of v
 * @param B binary volume
 * @param v vector
 */
unsigned int centerOfMassFromSet(const BinaryVolume &B, const vector<unsigned int> &v){
	if(v.empty())
		return B.totalSize();
	double x(0.0), y(0.0), z(0.0);
	for(unsigned int i(0); i < v.size(); i++){
		x += B.x(v[i]);
		y += B.y(v[i]);
		z += B.z(v[i]);
	}
	x /= v.size();
	y /= v.size();
	z /= v.size();
	double bestDistSq(EuclideanDistanceSq(x, y, z, B.x(v[0]), B.y(v[0]), B.z(v[0])));
	unsigned int best(0);
	for(unsigned int i(1); i < v.size(); i++){
		double distSq(EuclideanDistanceSq(x, y, z, B.x(v[i]), B.y(v[i]), B.z(v[i])));
		if(bestDistSq > distSq){
			bestDistSq = distSq;
			best = i;
		}
	}
	return v[best];
}

/**
 * Finds the indices m in a and n in b where the separation between m[a] and n[b] is minimized
 * Returns distance of nearest passing -1.0 if a and b are not appropriate
 * Does not address multiple overlaps
 * @param B binary volume
 * @param a
 * @param b
 * @param m
 * @param n
 */
double nearestPass(const BinaryVolume &B, const vector<unsigned int> &a, const vector<unsigned int> &b, unsigned int &m, unsigned int &n){
	m = 0;
	n = 0;
	if(a.size() < 1 || b.size() < 1)
		return -1.0;
	double shortDist(EuclideanDistance(B.x(a[0]), B.y(a[0]), B.z(a[0]), B.x(b[0]), B.y(b[0]), B.z(b[0])));
	for(unsigned int i(0); i < a.size(); i++){
		unsigned int ax(B.x(a[i])), ay(B.y(a[i])), az(B.z(a[i]));
		for(unsigned int j(0); j < b.size(); j++){
			double dist(EuclideanDistance(ax, ay, az, B.x(b[j]), B.y(b[j]), B.z(b[j])));
			if(shortDist > dist){
				m = i;
				n = j;
				shortDist = dist;
			}
		}
	}
	return shortDist;
}

void normalizedCoord(unsigned int x, unsigned int y, unsigned int z, double &nx, double &ny, double &nz){
	double largestExtent(voldimsGlobal[0]*voxdimsGlobal[0]);
	if(largestExtent < voldimsGlobal[1]*voxdimsGlobal[1])
		largestExtent = voldimsGlobal[1]*voxdimsGlobal[1];
	if(largestExtent < voldimsGlobal[2]*voxdimsGlobal[2])
		largestExtent = voldimsGlobal[2]*voxdimsGlobal[2];
	double extentFactors[] = {voldimsGlobal[0]*voxdimsGlobal[0]/largestExtent, voldimsGlobal[1]*voxdimsGlobal[1]/largestExtent, voldimsGlobal[2]*voxdimsGlobal[2]/largestExtent};
	nx = extentFactors[0]*(double(x)/double(voldimsGlobal[0]) - 0.5);
	ny = extentFactors[1]*(double(y)/double(voldimsGlobal[1]) - 0.5);
	nz = extentFactors[2]*(double(z)/double(voldimsGlobal[2]) - 0.5);
}

void normalizedCoord(unsigned int i, double &nx, double &ny, double &nz){
	unsigned int x(0), y(0), z(0);
	xyz(i, x, y, z);
	normalizedCoord(x, y, z, nx, ny, nz);
}

/**
 * Returns false if there are no backbones or the point is too far outside the space
 * @param x
 * @param y
 * @param z
 */
void findClosestSegment(double x, double y, double z){
	segIsSelected = false;
	if(backbonesGlobal.empty())
		return;
	if(backbonesGlobal[0].empty())
		return;
	if(backbonesGlobal[0][0].empty())
		return;
	if(2.0 < x || x < -2.0 || 2.0 < y || y < -2.0 || 2.0 < z || z < -2.0)
		return;
	double nx(0.0), ny(0.0), nz(0.0);
	normalizedCoord(backbonesGlobal[0][0][0], nx, ny, nz);
	double sepSq(separationSq3D(x, y, z, nx, ny, nz));
	ccSelected = bbSelected = vertSelected = 0;
	for(unsigned int i(0); i < backbonesGlobal.size(); i++){
		for(unsigned int j(0); j < backbonesGlobal[i].size(); j++){
			for(unsigned int k(0); k < backbonesGlobal[i][j].size(); k++){
				normalizedCoord(backbonesGlobal[i][j][k], nx, ny, nz);
				double ss(separationSq3D(x, y, z, nx, ny, nz));
				if(sepSq > ss){
					sepSq = ss;
					ccSelected = i;
					bbSelected = j;
					vertSelected = k;
				}
			}
		}
	}
	segIsSelected = true;
}

unsigned int lowestNotIn(const vector<unsigned int> &v){
	unsigned int i(0);
	while(isIn(i, v))
		i++;
	return i;
}

vector<vector<vector<unsigned char> > > segColors(const vector<vector<vector<unsigned int> > > &backbones,
		const vector<vector<vector<unsigned int> > > &branchpoints, unsigned int offset = 0, unsigned int numCol = 9){
	vector<vector<vector<unsigned char> > > sc(branchpoints.size(), vector<vector<unsigned char> >());
	if(branchpoints.size() == 0){
		sc.push_back(vector<vector<unsigned char> >(1, vector<unsigned char>(4, 255)));
		rainbowColor(offset, numCol, sc[0][0][0], sc[0][0][1], sc[0][0][2]);
		return sc;
	}
	
	if(numCol%2 == 0)
		numCol++;
	vector<vector<unsigned char> > cols(numCol, vector<unsigned char>(4, 255));
	for(unsigned int i(0); i < numCol; i++)
		rainbowColor(i, numCol, cols[i][0], cols[i][1], cols[i][2]);

	vector<map<unsigned int, vector<unsigned int> > > adj (branchpoints.size(), map<unsigned int, vector<unsigned int> >());
	vector<vector<unsigned int> > label (branchpoints.size(), vector<unsigned int>());
	for(unsigned int i(0); i < branchpoints.size(); i++){ // each cc
		for(unsigned int j(0); j < branchpoints[i].size(); j++){ // each branchpoint in cc
			for(unsigned int k(0); k < branchpoints[i][j].size(); k++){ // each backbone in branchpoint in cc
				for(unsigned int m(0); m < k; m++)
					pushUnique(branchpoints[i][j][m], adj[i][branchpoints[i][j][k]]);
				for(unsigned int m(k + 1); m < branchpoints[i][j].size(); m++)
					pushUnique(branchpoints[i][j][m], adj[i][branchpoints[i][j][k]]);
			}
		}

		// define labels
		label[i] = vector<unsigned int>(backbones[i].size(), 0);
		for(unsigned int j(0); j < backbones[i].size(); j++)
			label[i][j] = j%numCol;
		for(unsigned int j(0); j < backbones[i].size(); j++){
			vector<unsigned int> nhCols;
			for(unsigned int k(0); k < adj[i][j].size(); k++)
				pushUnique(label[i][adj[i][j][k]], nhCols);
			if(isIn(label[i][j], nhCols))
				label[i][j] = lowestNotIn(nhCols);
		}
		cout << endl;
	}

	for(unsigned int i(0); i < branchpoints.size(); i++){
		sc[i] = vector<vector<unsigned char> >(backbones[i].size(), vector<unsigned char>(4, 255));
		for(unsigned int j(0); j < backbones[i].size(); j++){
			for(unsigned int k(0); k < 3; k++)
				sc[i][j][k] = cols[label[i][j]][k];
		}
	}

	return sc;
}

vector<double> lineCols(const BinaryVolume &B, const double voxdims[], const vector<vector<vector<unsigned int> > > &backbones,
		const vector<vector<vector<unsigned int> > > &branchpoints, unsigned int offset = 0){
	double largestExtent(B.size[0]*voxdims[0]);
	if(largestExtent < B.size[1]*voxdims[1])
		largestExtent = B.size[1]*voxdims[1];
	if(largestExtent < B.size[2]*voxdims[2])
		largestExtent = B.size[2]*voxdims[2];
	double extentFactors[] = {B.size[0]*voxdims[0]/largestExtent, B.size[1]*voxdims[1]/largestExtent, B.size[2]*voxdims[2]/largestExtent};
	vector<vector<vector<unsigned char> > > sc(segColors(backbones, branchpoints, offset));
	vector<double> bc;
	for(unsigned int i(0); i < backbones.size(); i++){ // each cc
		for(unsigned int j(0); j < backbones[i].size(); j++){ // each backbone
			if(backbones[i][j].size() > 0){
				bc.push_back(extentFactors[0]*(double(B.x(backbones[i][j][0]))/double(B.size[0]) - 0.5));
				bc.push_back(extentFactors[1]*(double(B.y(backbones[i][j][0]))/double(B.size[1]) - 0.5));
				bc.push_back(extentFactors[2]*(double(B.z(backbones[i][j][0]))/double(B.size[2]) - 0.5));
				bc.push_back(0.0);
				bc.push_back(0.0);
				bc.push_back(0.0);
				bc.push_back(0.0);
			}
			for(unsigned int k(0); k < backbones[i][j].size(); k++){ // each vertebra
				bc.push_back(extentFactors[0]*(double(B.x(backbones[i][j][k]))/double(B.size[0]) - 0.5));
				bc.push_back(extentFactors[1]*(double(B.y(backbones[i][j][k]))/double(B.size[1]) - 0.5));
				bc.push_back(extentFactors[2]*(double(B.z(backbones[i][j][k]))/double(B.size[2]) - 0.5));
				bc.push_back(sc[i][j][0]/255.0);
				bc.push_back(sc[i][j][1]/255.0);
				bc.push_back(sc[i][j][2]/255.0);
				bc.push_back(sc[i][j][3]/255.0);
			}
			if(backbones[i][j].size() > 0){
				bc.push_back(extentFactors[0]*(double(B.x(backbones[i][j].back()))/double(B.size[0]) - 0.5));
				bc.push_back(extentFactors[1]*(double(B.y(backbones[i][j].back()))/double(B.size[1]) - 0.5));
				bc.push_back(extentFactors[2]*(double(B.z(backbones[i][j].back()))/double(B.size[2]) - 0.5));
				bc.push_back(0.0);
				bc.push_back(0.0);
				bc.push_back(0.0);
				bc.push_back(0.0);
			}
		}
	}
	return bc;
}

/**
 * Uses voxdimsGlobal to calculate average radius
 * @param sB
 * @param backbone
 * @param aveRadSolid average radius solid
 * @param aveRadSurf average radius surface
 */
void averageRadius(const BinaryVolume &sB, const vector<unsigned int> &backbone, double &aveRadSolid, double &aveRadSurf){
	aveRadSolid = 0.0;
	aveRadSurf = 0.0;
	if(backbone.empty())
		return;
	unsigned int solidCount(0), surfCount(0);
	for(unsigned int i(sB.findFirstAtOrAfter(0)); i < sB.totalSize(); i = sB.findFirstAtOrAfter(i + 1)){
		double minSep(separation3D(sB, i, backbone[0]));
		unsigned int closestIndex(backbone[0]);
		for(unsigned int k(1); k < backbone.size(); k++){
			double sep(separation3D(sB, i, backbone[k]));
			if(minSep > sep){
				minSep = sep;
				closestIndex = backbone[k];
			}
		}
		aveRadSolid += minSep;
		solidCount++;
		if(numVesselBlocksInNeighborhood(sB, i) < 26){
			aveRadSurf += minSep;
			surfCount++;
		}
	}
	if(solidCount > 0)
		aveRadSolid /= solidCount;
	if(surfCount > 0)
		aveRadSurf /= surfCount;
}

/**
 * Finds volume of the connected component connected to seed_index
 * Assumes seed_index is true and finds all segments in neighborhood
 * @param b binary volume
 * @param seed_index
 */
unsigned int connectedComponentVolume(const BinaryVolume &B, unsigned int seed_index){
	queue<unsigned int> q;
	q.push(seed_index);
	BinaryVolume vB(B); // unvisited voxels are true
	unsigned int vol(1);
	vB.f(seed_index);
	while(q.size() > 0){
		vector<unsigned int> nh(vesselBlocksInNeighborhood(vB, q.front()));
		for(unsigned int i(0); i < nh.size(); i++){
			vB.f(nh[i]);
			q.push(nh[i]);
		}
		vol += nh.size();
		q.pop();
	}
	return vol;
}

/**
 * Computes backbone lengths using voxdimsGlobal
 * @param backbone
 */
double backboneLength(const vector<unsigned int> &backbone){
	if(backbone.size() < 2)
		return 0.5;
	double len(0.0);
	for(unsigned int i(1); i < backbone.size(); i++)
		len += separation3D(backbone[i - 1], backbone[i]);
	return len;
}

template <class T>
map<double, double> normalizedBins(vector<T> v, unsigned int numBins = 0){
	map<double, double> dist;
	if(v.size() < 2)
		return dist;
	if(numBins < 1){
		if(v.size() < 15)
			numBins = 3;
		else if(v.size() < 100)
			numBins = ((unsigned int)v.size())/5;
		else
			numBins = ((unsigned int)v.size())/10;
	}
	T minV(minimum(v)), maxV(maximum(v));
	double binWidth((maxV - minV)/double(numBins));
	if(binWidth == 0.0)
		binWidth = 1.0;
	vector<double> binBottoms(numBins, 0.0);
	for(unsigned int i(0); i < numBins; i++)
		binBottoms[i] = minV + i*binWidth;
	vector<unsigned int> counts(v.size(), 0);
	for(unsigned int i(0); i < v.size(); i++){
		unsigned int j(numBins - 1);
		while(v[i] < binBottoms[j])
			j--;
		counts[j]++;
	}
	for(unsigned int i(0); i < numBins; i++)
		dist[binBottoms[i] + binWidth/2.0] = counts[i]/double(v.size())/binWidth;
	return dist;
}

string makeStringMap(map<double, double> &m){
	string s("\n");
	for(map<double, double>::iterator it(m.begin()); it != m.end(); it++)
		s += makeString(it->first) + "\t" + makeString(it->second) + "\n";
	return s;
}

#if TRY_WX == 1

/**
 * Creates the properly formatted lists for points with colors
 * @param B binary volume
 * @param voxdims voxel dimensions
 * @param sets
 */
vector<double> pointsWithColors(const BinaryVolume &B, const double voxdims[], const vector<vector<unsigned int> > &sets,
		const vector<double> &setCols, map<unsigned int, unsigned int> &pm){
	double largestExtent(B.size[0]*voxdims[0]);
	if(largestExtent < B.size[1]*voxdims[1])
		largestExtent = B.size[1]*voxdims[1];
	if(largestExtent < B.size[2]*voxdims[2])
		largestExtent = B.size[2]*voxdims[2];
	double extentFactors[] = {B.size[0]*voxdims[0]/largestExtent, B.size[1]*voxdims[1]/largestExtent, B.size[2]*voxdims[2]/largestExtent};
	map<unsigned int, unsigned int> pointSets;
	for(unsigned int i(B.findFirstAtOrAfter(0)); i < B.totalSize(); i = B.findFirstAtOrAfter(i + 1))
		pointSets[i] = (unsigned int)sets.size();
	for(unsigned int i(0); i < sets.size(); i++){
		for(unsigned int j(0); j < sets[i].size(); j++)
			pointSets[sets[i][j]] = i;
	}
	vector<double> v(7*pointSets.size(), 1.0);
	unsigned int i(0);
	for(map<unsigned int, unsigned int>::iterator it(pointSets.begin()); it != pointSets.end(); it++){
		v[7*i] = extentFactors[0]*(double(B.x(it->first))/double(B.size[0]) - 0.5);
		v[7*i + 1] = extentFactors[1]*(double(B.y(it->first))/double(B.size[1]) - 0.5);
		v[7*i + 2] = extentFactors[2]*(double(B.z(it->first))/double(B.size[2]) - 0.5);
		for(unsigned int j(3); j < 7; j++)
			v[7*i + j] = defaultPointColor[j - 3];
		pm[it->first] = i;
		if(it->second == sets.size())
			v[7*i + 6] = 0.2;
		else if(4*sets.size() == setCols.size()){
			v[7*i + 3] = setCols[4*it->second];
			v[7*i + 4] = setCols[4*it->second + 1];
			v[7*i + 5] = setCols[4*it->second + 2];
			v[7*i + 6] = setCols[4*it->second + 3];
		}else{
			v[7*i + 3] = 0.2;
			v[7*i + 4] = 0.6;
			v[7*i + 5] = 0.4;
			v[7*i + 6] = 0.4;
		}
		i++;
	}
	return v;
}

vector<double> pointAndLineCols(const Lumens &L, const BinaryVolume &B, const double voxdims[], const vector<vector<vector<unsigned int> > > &backbones,
		const vector<vector<vector<unsigned int> > > &branchpoints, vector<double> &pc, map<unsigned int, unsigned int> &pm){
	pc = pointsWithColors(B, voxdims, vector<vector<unsigned int> >(), vector<double>(), pm);
	for(unsigned int i(0); 7*i < pc.size(); i++)
		pc[7*i + 4] = pc[7*i + 5] = 0.0; // make points red
	return lineCols(B, voxdims, backbones, branchpoints);
}

/**
 * Sets colors to default based on set of indices, v, and mapped to indices in the set of all points by pm
 * @param v vector of indices
 * @param pm map of points
 */
void setDefaultColor(const vector<unsigned int> &v, const map<unsigned int, unsigned int> &pm){
	map<unsigned int, vector<double> > uc;
	for(unsigned int i(0); i < v.size(); i++){
		unsigned int pmIndex(pm.at(v[i]));
		for(unsigned int j(0); j < 4; j++)
			uc[pmIndex].push_back(defaultPointColor[j]);
	}
	wxGetApp().myFrame->m_canvas->updateCols(uc);
}

/**
 * Sets colors based on set of indices, v, and mapped to indices in the set of all points by pm
 * @param v vector of indices
 * @param pm map of points
 * @param r
 * @param g
 * @param b
 * @param a
 */
void setPointSetColor(const vector<unsigned int> &v, const map<unsigned int, unsigned int> &pm,
		double r, double g, double b, double a){
	double c[4];
	c[0] = r;
	c[1] = g;
	c[2] = b;
	c[3] = a;
	map<unsigned int, vector<double> > uc;
	for(unsigned int i(0); i < v.size(); i++){
		if(pm.find(v[i]) == pm.end())
			continue;
		unsigned int pmIndex(pm.at(v[i]));
		for(unsigned int j(0); j < 4; j++)
			uc[pmIndex].push_back(c[j]);
	}
	wxGetApp().myFrame->m_canvas->updateCols(uc);
}

/**
 * Sets color at certain point, i
 * @param i index of point
 * @param pm map of points
 * @param r
 * @param g
 * @param b
 * @param a
 */
void setPointColor(unsigned int i, const map<unsigned int, unsigned int> &pm,
		double r, double g, double b, double a){
	if(pm.find(i) == pm.end())
		return;
	double c[4];
	c[0] = r;
	c[1] = g;
	c[2] = b;
	c[3] = a;
	map<unsigned int, vector<double> > uc;
	unsigned int pmIndex(pm.at(i));
	for(unsigned int j(0); j < 4; j++)
		uc[pmIndex].push_back(c[j]);
	wxGetApp().myFrame->m_canvas->updateCols(uc);
}

/**
 * Assumes setCols is the correct size
 * @param pwc
 * @sets
 */
map<unsigned int, vector<double> > updateSubsetPointsWithColors(vector<double> pwc, const vector<vector<unsigned int> > &sets,
		const vector<double> &setCols, map<unsigned int, unsigned int> &pm){
	map<unsigned int, vector<double> > uc;
	for(unsigned int i(0); i < sets.size(); i++){
		for(unsigned int j(0); j < sets.size(); j++){
			unsigned int b(pm[sets[i][j]]);
			for(unsigned int k(0); k < 4; k++){
				uc[b].push_back(setCols[4*i + k]);
				pwc[7*b + 3 + k] = setCols[4*i + k];
			}
		}
	}
	return uc;
}

vector<double> surfaceTriangle(){
	double surface[18] = {	-0.5, -0.5, 0.0,
							-0.5, -0.5, 0.0,
							0.0, 0.5, 0.0,
							0.0, 0.5, 0.0,
							0.5, -0.5, 0.0,
							0.5, 0.5, 0.0};
	vector<double> s;
	for(unsigned int i(0); i < 18; i++)
		s.push_back(surface[i]);
	return s;
}

vector<double> surfaceTetrahedron(){
	double surface[24] = {	-0.5, -0.5, 0.5,
							-0.5, -0.5, 0.5,
							0.0, 0.5, 0.5,
							0.0, 0.5, 0.5,
							0.5, -0.5, 0.5,
							0.5, 0.5, 0.5,
							0.0, 0.0, -0.5,
							0.0, 0.0, -0.5};
	vector<double> s;
	for(unsigned int i(0); i < 24; i++)
		s.push_back(surface[i]);
	return s;
}

vector<double> surfaceSquare(){
	double surface[24] = {	-0.5, -0.5, 0.0,
							-0.5, -0.5, 0.0,
							-0.5, 0.5, 0.0,
							-0.5, 0.5, 0.0,
							0.5, -0.5, 0.0,
							0.5, -0.5, 0.0,
							0.5, 0.5, 0.0,
							0.5, 0.5, 0.0};
	vector<double> s;
	for(unsigned int i(0); i < 24; i++)
		s.push_back(surface[i]);
	return s;
}

void wxTest(){
	system("del /Q .\\outputs\\*.png");
	wxGetApp().myFrame->updateStatus("Initializing...");
	wxGetApp().myFrame->m_canvas->newSurface(surfaceTriangle());
	double thresh(0.22);

#if defined(__WXMSW__) || defined(__WINDOWS__)
	string pathToImages("."); // Windows
#else
    string pathToImages("/Users/andersonju/Desktop/minimalSurfaces_share"); // Mac
#endif

    //Lumens L(simpleLumensCube(5));
    double voxdims[] = {1.0, 1.0, 1.0};
	//Lumens L(readPNGImages(pathToImages + "/T_test", 0, 2));//0-7
	//Lumens L(readPNGImages(pathToImages + "/small_easy_test", 0, 0));
	//Lumens L(readPNGImages(pathToImages + "/small_network_test", 0, 7));
	//Lumens L(readPNGImages(pathToImages + "/small_diag_test", 0, 7));
	//Lumens L(readPNGImages(pathToImages + "/small_loop_test", 0, 7));
	Lumens L(readPNGImages(pathToImages + "/small_circle_test/small/smaller", 0, 9));
	//Lumens L(readPNGImages(pathToImages + "/FITC-MCA0_N12_NIH001_s1_4", 0, 66)); double voxdims[] = {1.648, 1.648, 0.492};
	//Lumens L(readPNGImages(pathToImages + "/MacCL_168", 0, 167));
	//Lumens L(readPNGImages(pathToImages + "/FITC-MCA0_N12_PI001_s1", 0, 61));
	//writePNGBackbonesThree(L, vector<vector<unsigned int> >(), pathToImages + "/outputs/test_Lumens.png");
	BinaryVolume B(threshThrash(L, thresh));
	writePNGBinaryVolume(B, pathToImages + "/outputs/test_BinaryVolume.png");
	wxGetApp().myFrame->m_canvas->newSurface(surfaceSquare());
	//map<unsigned int, unsigned int> extraEdges;
	//vector<unsigned int> ep(EulerianPath(B, B.findFirstAtOrAfter(B.totalSize()/2), extraEdges));
	//vector<unsigned int> epd(duplicateExtras(ep, extraEdges));
	wxGetApp().myFrame->m_canvas->newSurface(surfaceTetrahedron());
	//vector<double> s(scaffoldWithColorsOLD(B, ep, extraEdges, vector<vector<unsigned int> >(), vector<double>()));
	//vector<double> s(scaffoldWithColors(B, epd, vector<vector<unsigned int> >(), vector<double>()));
	//wxGetApp().myFrame->m_canvas->newScaffold(s);
	map<unsigned int, unsigned int> pm;
	vector<double> pwc(pointsWithColors(B, voxdims, vector<vector<unsigned int> >(), vector<double>(), pm));
	wxGetApp().myFrame->m_canvas->newPoints(pwc);
	wxGetApp().myFrame->updateStatus("Waiting...");

//	wxGetApp().myFrame->m_canvas->newSurface(surfaceSquare());
#if defined(__WXMSW__) || defined(__WINDOWS__)
	Sleep(1000*3);
#else
	sleep(3);
#endif
//	wxGetApp().myFrame->m_canvas->newSurface(surfaceTetrahedron());
	map<unsigned int, vector<double> > uc(updateSubsetPointsWithColors(pwc, vector<vector<unsigned int> >(1, vector<unsigned int>(1, B.findFirstAtOrAfter(0))), vector<double>(4, 1.0), pm));
	wxGetApp().myFrame->m_canvas->updateCols(uc);
	wxGetApp().myFrame->updateStatus("");
}
#endif

/**
 * Returns true if sphere is disjoint
 * @param B binary volume
 * @param uB binary volume
 * @param seed
 * @param critFrac
 * @param r
 * @param falsifiedPoints
 */
bool disjointCriticalSphere(const BinaryVolume &B, BinaryVolume &uB, unsigned int seed, double critFrac, double &r, vector<unsigned int> &falsifiedPoints
#if TRY_WX == 1
		, const map<unsigned int, unsigned int> &pm
#endif
		){
	r = -1.0;
	falsifiedPoints.clear();
	vector<unsigned int> f(1, seed);
	vector<double> d(1, 0.0);
	BinaryVolume fB(B.size, true);
	if(uB.is(seed)){ // still unvisited
		fB.f(f[0]);
		map<unsigned int, vector<double> > uc;
#if TRY_WX == 1
		unsigned int pmIndex(pm.at(f[0]));
		uc[pmIndex] = vector<double>(4, 1.0); uc[pmIndex][0] = uc[pmIndex][1] = 0.0; uc[pmIndex][3] = 0.1;
		wxGetApp().myFrame->m_canvas->updateCols(uc);
#endif
	}else // already visited
		return false;
	double frac(1.0);
	unsigned int trueCount(1), count(1);
	while(frac > critFrac && count > 0){
		r = ceil(r + 1.0);
		trueCount = count = 0;

		while(d[0] - r < 1.0e-9){
			count++;
			if(B.is(f[0]))
				trueCount++;
			vector<unsigned int> nh(vesselBlocksInNeighborhood(fB, f[0]));
			for(unsigned int i(0); i < nh.size(); i++){
				if(B.is(nh[i]) && !uB.is(nh[i])){ // has been visited before (not unvisited)
					vector<unsigned int> trueFrontiers;
					for(unsigned int i(0); i < f.size(); i++){
						if(B.is(f[i]))
							trueFrontiers.push_back(f[i]);
					}
#if TRY_WX == 1
					setDefaultColor(trueFrontiers, pm);
#endif
					return false;
				}// in what follows, nh[i] has not been visited before in any sphere
				double c(EuclideanDistance(B, seed, nh[i]));
				unsigned int hi((unsigned int)d.size()), lo(0);
				unsigned int mid(hi/2);
				while(hi - lo > 1){
					if(d[mid] < c)
						lo = mid;
					else if(d[mid] == c)
						hi = lo = mid;
					else // d[mid] > c
						hi = mid;
					mid = lo + (hi - lo)/2;
				}
				unsigned int j(lo);
				if(d[lo] < c)
					j = hi;
				f.insert(f.begin() + j, nh[i]);
				d.insert(d.begin() + j, c);
				fB.f(nh[i]);
#if TRY_WX == 1
				if(uB.is(nh[i])){
					map<unsigned int, vector<double> > uc;
					unsigned int pmIndex(pm.at(nh[i]));
					uc[pmIndex] = vector<double>(4, 1.0); uc[pmIndex][0] = uc[pmIndex][1] = 0.0; uc[pmIndex][3] = 0.7;
					wxGetApp().myFrame->m_canvas->updateCols(uc);
				}
#endif
			}
			if(uB.is(f[0])){
				uB.f(f[0]);
				falsifiedPoints.push_back(f[0]);
#if TRY_WX == 1
				map<unsigned int, vector<double> > uc;
				unsigned int pmIndex(pm.at(f[0]));
				uc[pmIndex] = vector<double>(4, 1.0); uc[pmIndex][0] = uc[pmIndex][2] = 0.0; uc[pmIndex][3] = 0.7;
				wxGetApp().myFrame->m_canvas->updateCols(uc);
#endif
			}
			f.erase(f.begin());
			d.erase(d.begin());
			if(d.empty()){
				vector<unsigned int> trueFrontiers;
				for(unsigned int i(0); i < f.size(); i++){
					if(B.is(f[i]))
						trueFrontiers.push_back(f[i]);
				}
#if TRY_WX == 1
				setDefaultColor(trueFrontiers, pm);
#endif
				return false;
			}
		}
		frac = double(trueCount)/double(count);
	}
	vector<unsigned int> trueFrontiers;
	for(unsigned int i(0); i < f.size(); i++){
		if(B.is(f[i]))
			trueFrontiers.push_back(f[i]);
	}
	//fB.t(trueFrontiers);
#if TRY_WX == 1
	setDefaultColor(trueFrontiers, pm);
#endif
	return true;
}

map<unsigned int, double> sphereCoarsen(const BinaryVolume &B, BinaryVolume &uB, double critFrac
#if TRY_WX == 1
		, const map<unsigned int, unsigned int> &pm
#endif
		, double rSeed = 0.0, double gSeed = 1.0, double bSeed = 1.0, double aSeed = 1.0){
	map<unsigned int, double> rv;
	unsigned int consecutiveFailures(0), maxSeedFailures(100000), maxConsecutiveFailures(200), reachedConsecutiveFailures(0);
	double minSeedMovement(1.0);
	BinaryVolume sB(uB); // for untried seeds
	while(consecutiveFailures < maxConsecutiveFailures && sB.findFirstAtOrAfter(0) < sB.totalSize()){
		if(reachedConsecutiveFailures < consecutiveFailures)
			reachedConsecutiveFailures = consecutiveFailures;
		int paddedDigits(1 + (int)floor(log10(maxConsecutiveFailures)));
		wxGetApp().myFrame->updateStatus("Vectorizing roughly... "
			+ paddedInt(consecutiveFailures, paddedDigits) + " up to "
			+ paddedInt(reachedConsecutiveFailures, paddedDigits) +" of "
			+ makeString(maxConsecutiveFailures) + "."
			+ "  Found " + makeString(rv.size()) + " points so far..."
			);
		unsigned int seed(sB.findFirstAtOrAfter(kiss()%sB.totalSize())), consecutiveSeedFailures(0);
		while(seed == sB.totalSize() && consecutiveSeedFailures < maxSeedFailures){
			consecutiveSeedFailures++;
			seed = sB.findFirstAtOrAfter(kiss()%sB.totalSize());
		}
		if(seed == sB.totalSize()){
			consecutiveFailures++;
			continue;
		}
		double seedMovement(minSeedMovement);
		bool seedFailed(false);
		double r(-1.0), rPrev(-1.0);
		vector<unsigned int> falsifiedPoints;
		bool centering(true);
		while(centering && !seedFailed){
			sB.f(seed);
			r = -1.0;
			seedFailed = !disjointCriticalSphere(B, uB, seed, critFrac, r, falsifiedPoints
#if TRY_WX == 1
				, pm
#endif
				);
			
			if(seedFailed){
				uB.t(falsifiedPoints);
#if TRY_WX == 1
				setDefaultColor(falsifiedPoints, pm);
#endif
			}else{
				unsigned int newSeed(centerOfMassVessel(B, falsifiedPoints));
				seedMovement = EuclideanDistance(B, seed, newSeed);
				centering = seedMovement >= minSeedMovement && r > 1.0*rPrev;
				if(centering){
					uB.t(falsifiedPoints);
#if TRY_WX == 1
					setDefaultColor(falsifiedPoints, pm);
#endif
					seed = newSeed;
				}
			}
			rPrev = r;
		}
		if(seedFailed){
			uB.t(falsifiedPoints);
#if TRY_WX == 1
			setDefaultColor(falsifiedPoints, pm);
#endif
			consecutiveFailures++;
		}else{
			rv[seed] = r;
			sB.f(falsifiedPoints);
			consecutiveFailures = 0;
#if TRY_WX == 1
			setPointSetColor(falsifiedPoints, pm, 1.0, 0.0, 0.0, 0.7);
			setPointSetColor(vector<unsigned int>(1, seed), pm, rSeed, gSeed, bSeed, aSeed);
			double x(0.0), y(0.0), z(0.0);
			normalizedCoord(seed, x, y, z);
			wxGetApp().myFrame->m_canvas->addNumber(seed, x, y, z);
#endif
		}
	}
	return rv;
}

/**
 * Returns a map with a key of seed and value of vector of sphere seeds
 * @param B binary volume
 * @param uB binary volume
 * @param rv
 */
map<unsigned int, vector<unsigned int> > adjacentSpheres(const BinaryVolume &B, const BinaryVolume &uB, map<unsigned int, double> &rv
#if TRY_WX == 1
		, const map<unsigned int, unsigned int> &pm
#endif
		){
	map<unsigned int, vector<unsigned int> > adj;
	BinaryVolume cB(uB), // unvisited during exploration of intersphere space
		sB(B); // unvisited during exploration of intrasphere space
	unsigned int seed(cB.findFirstAtOrAfter(0));
	while(seed < cB.totalSize()){
		//cout << "\n adjacentSpheres(): seed = " << seed << endl;
		vector<unsigned int> F, f(1, seed), centers; // F is for intraspheres, f for unexplored structure
		while(!f.empty()){
			//Sleep(100);
			vector<unsigned int> nh(vesselBlocksInNeighborhood(cB, f[0]));
			f.insert(f.end(), nh.begin(), nh.end());
			cB.f(nh);
#if TRY_WX == 1
			setPointColor(f[0], pm, 0.0, 1.0, 0.0, 0.7);
#endif
			nh = vesselBlocksInNeighborhood(B, f[0]);
			for(unsigned int i(0); i < nh.size(); i++){
				if(!uB.is(nh[i])){
					F.push_back(nh[i]);
					sB.f(nh[i]);
					if(rv.find(nh[i]) != rv.end()) // is center
						pushUnique(nh[i], centers);//centers.push_back(nh[i]);
#if TRY_WX == 1
					else // is not center
						setPointColor(nh[i], pm, 1.0, 0.0, 0.0, 0.7);
#endif
				}
			}
			f.erase(f.begin());
		}
		//cout << "\n adjacentSpheres(): F.size() = " << F.size() << endl;
		vector<unsigned int> intrasphereVisited(F);
		while(!F.empty()){
			//Sleep(100);
			vector<unsigned int> nh(vesselBlocksInNeighborhood(sB, F[0])); // not visited in intrasphere (starts as all of B)
			for(unsigned int i(0); i < nh.size(); i++){
				if(!uB.is(nh[i])){ // is intrasphere (not unvisited -> visited)
					if(rv.find(nh[i]) != rv.end()) // is center
						pushUnique(nh[i], centers);//centers.push_back(nh[i]);
#if TRY_WX == 1
					else // is not center
						setPointColor(nh[i], pm, 1.0, 0.0, 0.0, 0.7);
#endif
					F.push_back(nh[i]);
					sB.f(nh[i]);
					intrasphereVisited.push_back(nh[i]);
				}
			}
			F.erase(F.begin());
		}
		sB.t(intrasphereVisited);
		if(centers.size() > 1)
			adj[seed] = centers;
		seed = cB.findFirstAtOrAfter(seed + 1);
	}
	return adj;
}

/**
 * May return more than keepAtLeastThisManyLargest if there are multiple components with the smallest amount
 */
 BinaryVolume removeSmallestConnectedComponents(const BinaryVolume &B, unsigned int keepAtLeastThisManyLargest
#if TRY_WX == 1
		, map<unsigned int, unsigned int> &pm
#endif
		){
	if(keepAtLeastThisManyLargest == 0)
		keepAtLeastThisManyLargest = 1; // pointless to be empty
	BinaryVolume uB(B);
	priority_queue<unsigned int> cc_sizes;
	unsigned int seed(uB.findFirstAtOrAfter(0));
	while(seed < uB.totalSize()){
		uB.f(seed);
		vector<unsigned int> f(1, seed);
#if TRY_WX == 1
		setPointSetColor(f, pm, 1.0, 0.0, 0.0, 0.3);
#endif
		unsigned int cc_size(1);
		while(!f.empty()){
			vector<unsigned int> nh(vesselBlocksInNeighborhood(uB, f[0]));
			uB.f(nh);
			f.insert(f.end(), nh.begin(), nh.end());
#if TRY_WX == 1
			setPointSetColor(nh, pm, 1.0, 0.0, 0.0, 0.3);
#endif
			cc_size += nh.size();
			f.erase(f.begin());
		}
		cc_sizes.push(cc_size);
		seed = uB.findFirstAtOrAfter(seed + 1);
	}
	for(unsigned int i(1); i < keepAtLeastThisManyLargest && cc_sizes.size() > 1; i++)
		cc_sizes.pop();
	unsigned int minimumSize(cc_sizes.top());
	uB = B;
	BinaryVolume lccsB(B), eB(B);
	seed = uB.findFirstAtOrAfter(0);
	while(seed < uB.totalSize()){
		vector<unsigned int> f(1, seed);
		uB.f(seed);
#if TRY_WX == 1
		setDefaultColor(f, pm);
#endif
		unsigned int cc_size(1);
		while(!f.empty()){ // CANNOT stop early, must continue to explore full cc
			vector<unsigned int> nh(vesselBlocksInNeighborhood(uB, f[0]));
			uB.f(nh);
			f.insert(f.end(), nh.begin(), nh.end());
#if TRY_WX == 1
			setDefaultColor(nh, pm);
#endif
			cc_size += nh.size();
			f.erase(f.begin());
		}
		if(cc_size < minimumSize){
			f = vector<unsigned int>(1, seed);
			lccsB.f(seed);
			eB.f(seed);
			while(!f.empty()){
				vector<unsigned int> nh(vesselBlocksInNeighborhood(eB, f[0]));
				lccsB.f(nh);
				eB.f(nh);
				f.insert(f.end(), nh.begin(), nh.end());
				f.erase(f.begin());
			}
		}
		seed = uB.findFirstAtOrAfter(seed + 1);
	}
	return lccsB;
}

/**
 * Returns frontiers that are interspheres
 * @param B binary volume
 * @param uB binary volume
 * @param seed
 */
vector<unsigned int> unvisitedFrontier(const BinaryVolume &B, const BinaryVolume &uB, unsigned int seed){
	if(uB.is(seed))
		return vector<unsigned int>();
	BinaryVolume cB(B); // unvisited during exploration of intrasphere space
	vector<unsigned int> f(1, seed), F;
	cB.f(f);
	while(!f.empty()){
		vector<unsigned int> nh(vesselBlocksInNeighborhood(cB, f[0]));
		for(unsigned int i(0); i < nh.size(); i++){
			if(!uB.is(nh[i]))
				f.push_back(nh[i]);
			else
				F.push_back(nh[i]);
			cB.f(nh[i]);
		}
		f.erase(f.begin());
	}
	return F;
}

/**
 * Returns frontier that is intrasphere
 * @param B binary volume
 * @param uB binary volume
 * @param seed
 */
vector<unsigned int> visitedFrontier(const BinaryVolume &B, const BinaryVolume &uB, unsigned int seed){
	if(!uB.is(seed))
		return vector<unsigned int>();
	BinaryVolume cB(B); // unvisited during exploration of intersphere space
	vector<unsigned int> f(1, seed), F;
	cB.f(f);
	while(!f.empty()){
		vector<unsigned int> nh(vesselBlocksInNeighborhood(cB, f[0]));
		for(unsigned int i(0); i < nh.size(); i++){
			if(uB.is(nh[i]))
				f.push_back(nh[i]);
			else
				F.push_back(nh[i]);
			cB.f(nh[i]);
		}
		f.erase(f.begin());
	}
	return F;
}

/**
 * Returns the center of the sphere that includes the seed
 * @param B binary volume
 * @param uB binary volume
 * @param RV
 * @param seed
 */
unsigned int findVisitedCenter(const BinaryVolume &B, const BinaryVolume &uB, const map<unsigned int, double> &rv, unsigned int seed){
	if(!B.is(seed) || uB.is(seed))
		return B.totalSize();
	BinaryVolume sB(B);
	vector<unsigned int> f(1, seed);
	sB.f(f);
	while(!f.empty()){
		vector<unsigned int> nh(vesselBlocksInNeighborhood(sB, f[0]));
		for(unsigned int i(0); i < nh.size(); i++){
			if(rv.find(nh[i]) != rv.end())
				return nh[i];
			if(!uB.is(nh[i]))
				f.push_back(nh[i]);
			sB.f(nh[i]);
		}
		f.erase(f.begin());
	}
	return B.totalSize();
}

/**
 *backbones[x].size() might be greater than adj[x].size() because of extra segments required for branchpoint...
 */
map<unsigned int, vector<vector<unsigned int> > > connectionBackbones(const BinaryVolume &B, const BinaryVolume &uB, map<unsigned int, double> &rv, map<unsigned int, vector<unsigned int> > &adj, map<unsigned int, unsigned int> &pm){
	map<unsigned int, vector<vector<unsigned int> > > backbones;
	BinaryVolume cB(B);
	for(map<unsigned int, vector<unsigned int> >::iterator it(adj.begin()); it != adj.end(); it++){
		vector<vector<unsigned int> > vfcc(segregateConnectedComponents(visitedFrontier(B, uB, it->first), B)), cBackbone;
		vector<unsigned int> centers;
		for(unsigned int i(0); i < vfcc.size(); i++){
			centers.push_back(findVisitedCenter(B, uB, rv, vfcc[i][0]));
			cBackbone.push_back(vector<unsigned int>(1, centerOfMassVessel(B, vfcc[i])));
			setPointColor(backbones[it->first].back()[0], pm, 1.0, 0.0, 0.0, 0.7);
		}
		vector<bool> advanceable(centers.size(), true);
		while(!allFalse(advanceable)){
			for(unsigned int i(0); i < centers.size(); i++){
				//advance until collision or split...
			}
		}
		//organize collisions...
		//treat disjoint collisions and splits...
	}
	return backbones;
}

/**
 * Assumes at least half of the voxels are outside the vasculature
 */
BinaryVolume findOutside(const BinaryVolume &B){
	if(B.size[0] < 2 || B.size[1] < 2 || B.size[2] < 2){ // 2D slice: all nonvascular voxels are the outside
		BinaryVolume O(B.size, true);
		unsigned int seed(B.findFirstAtOrAfter(0));
		while(seed < B.totalSize()){
			O.f(seed);
			seed = B.findFirstAtOrAfter(seed + 1);
		}
		return O;
	}
	unsigned int maxSeeds(1000);
	BinaryVolume uB(B.size, true); // unvisited in volume
	unsigned int seed(B.findFirstFalseAtOrAfter(0)), numSeeds(0);
	while(seed < B.totalSize() && numSeeds < maxSeeds){
		BinaryVolume O(B.size, false);
		while(!uB.is(seed))
			seed = B.findFirstFalseAtOrAfter(seed + 1);
		vector<unsigned int> f(1, seed);
		unsigned int ccSize(1);
		while(!f.empty()){
			vector<unsigned int> nh(vesselBlocksInNeighborhood(uB, f[0]));
			f.insert(f.end(), nh.begin(), nh.end());
			uB.f(nh);
			for(unsigned int i(0); i < nh.size(); i++){
				if(!B.is(nh[i])){
					O.t(nh[i]);
					ccSize++;
				}
			}
			f.erase(f.begin());
		}
		if(ccSize > B.totalSize()/2)
			return O;
		O.clear();
		seed = B.findFirstFalseAtOrAfter(seed + 1);
        numSeeds++;
	}
	// did not find obvious contiguous outside: return blank
	return BinaryVolume(B.size, false);
}

/**
 * Key is sphere seed, value is vector of connection seeds
 */
map<unsigned int, vector<unsigned int> > sphereConnectionSeeds(map<unsigned int, vector<unsigned int> > &adj){
	map<unsigned int, vector<unsigned int> > scs;
    //cout << "\n sphereConnectionSeeds(): adj.size() = " << adj.size() << endl;
	for(map<unsigned int, vector<unsigned int> >::iterator it(adj.begin()); it != adj.end(); it++){
		for(unsigned int i(0); i < it->second.size(); i++)
			pushUnique(it->first, scs[it->second[i]]);
	}
	return scs;
}

void removeSphereVisits(const BinaryVolume &B, BinaryVolume &uB, unsigned int seed, map<unsigned int, unsigned int> &pm){
	if(uB.is(seed))
		return;
	BinaryVolume sB(B); // unvisited for intrasphere; could also just set visited back to true and only expand to unvisited (this would not require an extra BinaryVolume)
	vector<unsigned int> f(1, seed);
	uB.t(seed);
	setDefaultColor(vector<unsigned int>(1, seed), pm);
	sB.f(seed);
	while(!f.empty()){
		vector<unsigned int> nh(vesselBlocksInNeighborhood(sB, f[0]));
		sB.f(nh);
		for(unsigned int i(0); i < nh.size(); i++){
			if(!uB.is(nh[i])){
				f.push_back(nh[i]);
				uB.t(nh[i]);
				setDefaultColor(vector<unsigned int>(1, nh[i]), pm);
			}
		}
		f.erase(f.begin());
	}
}

/** 
 * Finds spheres with only a single connection and only a single cc frontier; updates uB, rv, adj, scs
 * @param B binary volume
 * @param uB binary volume
 * @param rv
 * @param adj
 */
bool removeFalseTipsByContextPass(const BinaryVolume &B, BinaryVolume &uB, map<unsigned int, double> &rv, map<unsigned int, vector<unsigned int> > &adj,
		map<unsigned int, vector<unsigned int> > &scs
#if TRY_WX == 1
		, map<unsigned int, unsigned int> &pm
#endif
		){
	bool changed(false);
	map<unsigned int, double>::iterator it(rv.begin());
	while(it != rv.begin()){
		if(scs[it->first].size() < 2){ // only a single connection
			vector<vector<unsigned int> > vfcc(segregateConnectedComponents(unvisitedFrontier(B, uB, it->first), B));
			if(vfcc.size() < 2){ // only a single cc frontier
				changed = true;
				for(unsigned int i(0); i < scs[it->first].size(); i++)
					removeFrom(it->first, adj[scs[it->first][i]]);
				scs.erase(it->first);
#if TRY_WX == 1
				removeSphereVisits(B, uB, it->first, pm);
#endif
				rv.erase(it);
				it = rv.begin(); // may have changed earlier connections
			}else
				it++;
		}
	}
	// remove elements from adj that now are only a single component (unnecessary tips)
	map<unsigned int, vector<unsigned int> >::iterator itA(adj.begin());
	while(itA != adj.end()){
		if(itA->second.size() < 2){ // too few spheres in connection (one or zero); remove connection
			changed = true;
			if(itA->second.size() == 1){ // exactly one sphere for this connection
				unsigned int eraseSeed(itA->second[0]); // itA->second[0] is the only sphere seed for that connection
				removeFrom(itA->first, scs[eraseSeed]); // in case something was handled incorrectly elsewhere
				if(scs[itA->second[0]].size() == 0){// if no connections remain for the sphere seed, remove the effects of the sphere seed from uB and rv
#if TRY_WX == 1
					removeSphereVisits(B, uB, eraseSeed, pm);
#endif
					rv.erase(eraseSeed);
				}
				scs.erase(eraseSeed);
			} // otherwise there is no other sphere in the connection, so just erase it
			adj.erase(itA);
			itA = adj.begin(); // may not be efficient, but is thorough
		}else
			itA++;
	}
	return changed;
}

void removeFalseTipsByContext(const BinaryVolume &B, BinaryVolume &uB, map<unsigned int, double> &rv, map<unsigned int, vector<unsigned int> > &adj,
		map<unsigned int, vector<unsigned int> > &scs
#if TRY_WX == 1
		, map<unsigned int, unsigned int> &pm
#endif
		){
	bool changed(true);
	while(changed){
		changed = removeFalseTipsByContextPass(B, uB, rv, adj, scs
#if TRY_WX == 1
			, pm
#endif
			);
	}
}

unsigned int fillInternalFalses(BinaryVolume &B, const BinaryVolume &O){
	unsigned int seed(B.findFirstFalseAtOrAfter(0)), fillings(0);
	while(seed < B.totalSize()){
		if(!O.is(seed)){ // not outside
			B.t(seed);
			fillings++;
		}
		seed = B.findFirstFalseAtOrAfter(seed + 1);
	}
	return fillings;
}

/**
 * Returns the point in f that is farthest from any point in O (taking a path through f)
 * @param O binary volume
 * @param f
 */
unsigned int findFarthestInside(const BinaryVolume &O, const vector<unsigned int> &f){
	// initialize distances
	vector<double> d(f.size(), -1.0);
	for(unsigned int i(0); i < f.size(); i++){
		vector<unsigned int> nh(vesselBlocksInNeighborhood(O, f[i]));
		if(nh.empty())
			continue;
		d[i] = separation3D(nh[0], f[i]);
		for(unsigned int j(1); j < nh.size(); j++){
			double sep(separation3D(nh[j], f[i]));
			if(d[i] > sep)
				d[i] = sep;
		}
	}
	// adjacencies and/or distances in f
	vector<vector<unsigned int> > adjf(f.size(), vector<unsigned int>());
	vector<vector<double> > dadjf(f.size(), vector<double>());
	for(unsigned int i(0); i < f.size(); i++){
		for(unsigned int j(i + 1); j < f.size(); j++){
			if(inNeighborhood(f[i], f[j], O)){
				double sep(separation3D(f[i], f[j]));
				if(pushUnique(j, adjf[i]))//adjf[i].push_back(j);
					dadjf[i].push_back(sep);
				if(pushUnique(i, adjf[j]))//adjf[j].push_back(i);
					dadjf[j].push_back(sep);
			}
		}
	}
	// find shortest distances (somewhat inefficiently)
	bool changed = true;
	while(changed){
		changed = false;
		for(unsigned int i(0); i < f.size(); i++){
			for(unsigned int j(0); j < adjf[i].size(); j++){
				unsigned int k(adjf[i][j]);
				if(d[k] >= 0.0){ // some path to outside is known
					double dist(d[k] + dadjf[i][j]);
					if(d[i] < 0.0 || d[i] > dist){
						d[i] = dist;
						changed = true;
					}
				}
			}
		}
	}
	// choose smallest (favoring center of tied regions)
	double farthest(d[0]);
	for(unsigned int i(0); i < f.size(); i++){
		if(farthest < d[i])
			farthest = d[i];
	}
	vector<unsigned int> farthests;
	for(unsigned int i(0); i < f.size(); i++){
		if(d[i] == farthest)
			farthests.push_back(f[i]);
	}
	return centerOfMassFromSet(O, farthests);
}

/** 
 * Returns a map with key with the seed from adj as key and a vector of critical points that match the ordering of the sphere seed in adj
 * @param B binary volume
 * @param uB binary volume
 * @param rv
 * @param adj
 */
map<unsigned int, vector<unsigned int> > findCriticalVertebrae(const BinaryVolume &B, const BinaryVolume &uB, const BinaryVolume &O, map<unsigned int, double> &rv, map<unsigned int, vector<unsigned int> > &adj){
	map<unsigned int, vector<unsigned int> > critVert(adj);
	for(map<unsigned int, vector<unsigned int> >::iterator it(adj.begin()); it != adj.end(); it++){
		vector<vector<unsigned int> > vfcc(segregateConnectedComponents(visitedFrontier(B, uB, it->first), B));
		map<unsigned int, vector<unsigned int> > vfcc_matched;
		for(unsigned int i(0); i < vfcc.size(); i++){
			unsigned int centerLabel(findVisitedCenter(B, uB, rv, vfcc[i][0]));
			if(centerLabel < B.totalSize())
				vfcc_matched[centerLabel].insert(vfcc_matched[centerLabel].end(), vfcc[i].begin(), vfcc[i].end());
		}
		for(map<unsigned int, vector<unsigned int> >::iterator itf(vfcc_matched.begin()); itf != vfcc_matched.end(); itf++){
			unsigned int i(0);
			bool foundIndex(false);
			while(i < it->second.size() && !foundIndex){ // finding index in adj, so looking at it (NOT itf)
				foundIndex = it->second[i] == itf->first;
				if(!foundIndex)
					i++;
			}
			if(!foundIndex) // this should never happen, but just in case, leave it as the seed
				cout << "\n Error findCriticalVertebrae(): sphere center " << itf->first << " not found in " << makeString(it->second) << endl;
			else
				critVert[it->first][i] = findFarthestInside(O, itf->second);
		}
	}
	return critVert;
}

/** 
 * Returns a vector of the points that make up the meat of the sphere defined by seed
 * @param B binary volume
 * @param uB binary volume
 * @param seed
 */
vector<unsigned int> intraspherePoints(const BinaryVolume &B, const BinaryVolume &uB, unsigned int seed){
	if(uB.is(seed)) // unvisited: not a sphere
		return vector<unsigned int>();
	vector<unsigned int> s(1, seed), f(1, seed);
	BinaryVolume sB(B);
	sB.f(seed);
	while(!f.empty()){
		vector<unsigned int> nh(vesselBlocksInNeighborhood(sB, f[0]));
		sB.f(nh);
		for(unsigned int i(0); i < nh.size(); i++){
			if(!uB.is(nh[i])){ // not unvisited = visited: is in the sphere
				s.push_back(nh[i]);
				f.push_back(nh[i]);
			}
		}
		f.erase(f.begin());
	}
	return s;
}

/** 
 * Returns a vector of points that make up the meat of the intersphere connection defined by seed
 * @param B binary volume
 * @param uB binary volume
 * @param seed
 */
vector<unsigned int> interspherePoints(const BinaryVolume &B, const BinaryVolume &uB, unsigned int seed){
	if(!uB.is(seed)) // visited inside a sphere
		return vector<unsigned int>();
	vector<unsigned int> s(1, seed), f(1, seed);
	BinaryVolume sB(B);
	sB.f(seed);
	while(!f.empty()){
		vector<unsigned int> nh(vesselBlocksInNeighborhood(sB, f[0]));
		sB.f(nh);
		for(unsigned int i(0); i < nh.size(); i++){
			if(uB.is(nh[i])){ // unvisited: is not in a sphere
				s.push_back(nh[i]);
				f.push_back(nh[i]);
			}
		}
		f.erase(f.begin());
	}
	return s;
}

/**
 * Uses erosion to determine backbone, includes critical vertices from cv in the returned backbone (not ordered in spatial sequence)
 * @param B binary volume
 * @param O binary volume
 * @param s
 */
vector<unsigned int> erodeForBackbone(const BinaryVolume &B, const BinaryVolume &O, vector<unsigned int> s, const vector<unsigned int> &cv){\
	for(unsigned int i(0); i < cv.size(); i++)
		pushUnique(cv[i], s);
	// initialize distances
	vector<double> d(s.size(), -1.0);
	for(unsigned int i(0); i < s.size(); i++){
		vector<unsigned int> nh(vesselBlocksInNeighborhood(O, s[i]));
		if(nh.empty())
			continue;
		d[i] = separation3D(nh[0], s[i]);
		for(unsigned int j(1); j < nh.size(); j++){
			double sep(separation3D(nh[j], s[i]));
			if(d[i] > sep)
				d[i] = sep;
		}
	}
	// adjacencies and/or distances in s
	vector<vector<unsigned int> > adjf(s.size(), vector<unsigned int>());
	map<unsigned int, vector<unsigned int> > nhs; // stored indices in B of the neighborhood of the key, which itself is an index in B
	vector<vector<double> > dadjf(s.size(), vector<double>());
	for(unsigned int i(0); i < s.size(); i++){
		for(unsigned int j(i + 1); j < s.size(); j++){
			if(inNeighborhood(s[i], s[j], O)){
				double sep(separation3D(s[i], s[j]));
				if(pushUnique(j, adjf[i])){//adjf[i].push_back(j);
					dadjf[i].push_back(sep);
					nhs[s[i]].push_back(s[j]);
				}
				if(pushUnique(i, adjf[j])){//adjf[j].push_back(i);
					dadjf[j].push_back(sep);
					nhs[s[j]].push_back(s[i]);
				}
			}
		}
	}
	// find shortest distances (somewhat inefficiently)
	bool changed = true;
	while(changed){
		changed = false;
		for(unsigned int i(0); i < s.size(); i++){
			for(unsigned int j(0); j < adjf[i].size(); j++){
				unsigned int k(adjf[i][j]);
				if(d[k] >= 0.0){ // some path to outside is known
					double dist(d[k] + dadjf[i][j]);
					if(d[i] < 0.0 || d[i] > dist){
						d[i] = dist;
						changed = true;
					}
				}
			}
		}
	}
	map<double, vector<unsigned int> > toErode;
	for(unsigned int i(0); i < s.size(); i++)
		toErode[d[i]].push_back(s[i]);
	// erode locally
	bool eroded(true);
	while(eroded){
		eroded = false;
		for(map<double, vector<unsigned int> >::iterator it(toErode.begin()); it != toErode.end(); it++){
			for(int i(0); i < (int)it->second.size(); i++){
				unsigned int potRem(it->second[i]);
				if(isIn(potRem, cv))
					continue;
				vector<vector<unsigned int> > ccs(segregateConnectedComponents(nhs[potRem], B)); // could simply look and return boolean for increased speed
				if(ccs.size() < 2){ // remove all trace (both sides of connection) from toErode and nhs
					eroded = true;
					for(unsigned int j(0); j < nhs[it->second[i]].size(); j++){
						unsigned int neighborIndex(nhs[potRem][j]);
						removeFrom(it->second[i], nhs[neighborIndex]);
					}
					nhs.erase(potRem);
					removeFrom(potRem, it->second);
					i--;
				}
			}
		}
	}
	// translate into vector and erode globally
	vector<unsigned int> backbone;
	for(map<double, vector<unsigned int> >::iterator it(toErode.begin()); it != toErode.end(); it++)
		backbone.insert(backbone.end(), it->second.begin(), it->second.end());
	bool erased(true);
	while(erased){
		erased = false;
		for(unsigned int i(0); i < backbone.size(); i++){
			unsigned int potRem(backbone[i]);
			if(isIn(potRem, cv))
				continue;
			backbone.erase(backbone.begin() + i);
			vector<vector<unsigned int> > ccs(segregateConnectedComponents(backbone, B)); // could simply look and return boolean for increased speed
			if(ccs.size() > 1)
				backbone.insert(backbone.begin() + i, potRem);
			else
				erased = true;
		}
	}
    // explore possible reroutings for a more accurate backbone...
	return backbone;
}

/** 
 * Returns a map with the intersphere and intrasphere seeds as keys as the backbone for each component as value
 * The backbones connect to but do not include critVerts
 */
map<unsigned int, vector<unsigned int> > erosionBackbones(const BinaryVolume &B, const BinaryVolume &uB, const BinaryVolume &O,
		map<unsigned int, double> &rv, map<unsigned int, vector<unsigned int> > &adj, map<unsigned int, vector<unsigned int> > &scs,
		map<unsigned int, vector<unsigned int> > &critVert){
	map<unsigned int, vector<unsigned int> > backbones;
	for(map<unsigned int, double>::iterator it(rv.begin()); it != rv.end(); it++){
		vector<unsigned int> s(intraspherePoints(B, uB, it->first));
		vector<unsigned int> cv;
		for(unsigned int i(0); i < scs[it->first].size(); i++){
			unsigned int intersphereSeed(scs[it->first][i]);
			unsigned int critIndex(0);
			while(critIndex < adj[intersphereSeed].size() && adj[intersphereSeed][critIndex] != it->first)
				critIndex++;
			if(critIndex >= adj[intersphereSeed].size())
				cout << "\n Error erosionBackbones(): could not find " << it->first << " in " << makeString(adj[intersphereSeed]) << endl;
			else
				cv.push_back(critVert[intersphereSeed][critIndex]);
		}
		if(cv.empty()){
			cout << "\n erosionBackbones(): cv for sphere seed is empty!" << endl;
		}
		backbones[it->first] = erodeForBackbone(B, O, s, cv);
	}
	for(map<unsigned int, vector<unsigned int> >::iterator it(critVert.begin()); it != critVert.end(); it++){
		if(it->second.empty()){
			cout << "\n erosionBackbones(): cv for intersphere seed is empty!" << endl;
		}
		vector<unsigned int> s(interspherePoints(B, uB, it->first));
		backbones[it->first] = erodeForBackbone(B, O, s, it->second);
	}
	// erase empty backbones
	for(map<unsigned int, vector<unsigned int> >::iterator it(backbones.begin()); it != backbones.end(); it++){
		if(backbones[it->first].empty())
			backbones.erase(it->first);
	}
	return backbones;
}

/** 
 * Assumes b is a minimal backbone skeletonization
 */
void orderSegmentedBackbone(const BinaryVolume &B, vector<unsigned int> &b){
	vector<unsigned int> ub(b);
	b.clear();
	// find a tip
	unsigned int tipIndex(ub[0]);
	for(unsigned int i(0); i < ub.size(); i++){
		vector<unsigned int> nh(findNeighbors(B, ub[i], ub));
		if(nh.size() == 2){ // ub[i] and one neighbor
			tipIndex = ub[i];
			i = (unsigned int)ub.size();
		}
	}
	b.push_back(tipIndex);
	removeFrom(tipIndex, ub);
	while(!ub.empty()){
		vector<unsigned int> nh(findNeighbors(B, b.back(), ub));
		removeFrom(b.back(), nh);
		if(nh.size() == 1){
			b.push_back(nh[0]);
			removeFrom(nh[0], ub);
		}else{
			cout << "\n Error orderSegmentedBackbone(): Did not finish ordering backbone before all points visited" << endl;
			return;
		}
	}
}

/** 
 * Returns vector of segmented, ordered backbones; Also constructs adjacencies for backbones based on the indices in the segmented, ordered backbones vector: branchpoints
 * Assumes rawBackbones contains no duplicate points and that critVerts are tips in each individual backbone
 */
vector<vector<unsigned int> > segmentBackbones(const BinaryVolume &B, map<unsigned int, vector<unsigned int> > &rawBackbones,
		map<unsigned int, vector<unsigned int> > &critVert, vector<vector<unsigned int> > &branchpoints){
    if(rawBackbones.size() == 0)
        return vector<vector<unsigned int> >();
    if(rawBackbones.size() == 1){
        vector<unsigned int> onlyBackbone(rawBackbones.begin()->second);
        orderSegmentedBackbone(B, onlyBackbone);
        return vector<vector<unsigned int> >(1, onlyBackbone);
    }
    // organize each seed individually
	map<unsigned int, vector<vector<unsigned int> > > individualBranchPoints, individualOrganizedBackbones;
	map<unsigned int, vector<unsigned int> > tipSeeds; // keeps track of tips so that they can easily be found for stitching
    for(map<unsigned int, vector<unsigned int> >::iterator it(rawBackbones.begin()); it != rawBackbones.end(); it++){ // each unsorted backbone segment should be strictly arborescent (i.e., no reticulations)
		vector<unsigned int> unorganizedBackbone(it->second);
		unsigned int tipIndex(0); // need to find a tip to start; could find from critVert, but critVert is not conveniently organized for spheres
		vector<unsigned int> nh(findNeighbors(B, unorganizedBackbone[tipIndex], unorganizedBackbone));
		while(nh.size() > 2 && tipIndex < unorganizedBackbone.size()){
			tipIndex++;
			nh = findNeighbors(B, unorganizedBackbone[tipIndex], unorganizedBackbone);
		}
		if(tipIndex >= unorganizedBackbone.size()){
			cout << "\n Error segmentBackbones(): could not find a tip for seed " << it->first << endl;
			continue;
		}
		// explore segment backbone starting from a tip (unorganizedBackbone[tipIndex])
		vector<unsigned int> f(1, unorganizedBackbone[tipIndex]), bi(1, 0);
		individualOrganizedBackbones[it->first].push_back(vector<unsigned int>(1, f[0]));
		tipSeeds[f[0]].push_back(it->first);
		unorganizedBackbone.erase(unorganizedBackbone.begin() + tipIndex);
        while(!f.empty()){
			nh = findNeighbors(B, f[0], unorganizedBackbone);
            if(nh.size() > 1){ // branchpoint
				individualBranchPoints[it->first].push_back(vector<unsigned int>(1, bi[0]));
				f.erase(f.begin());
				bi.erase(bi.begin());
				for(unsigned int i(0); i < nh.size(); i++){
					f.push_back(nh[i]);
					bi.push_back((unsigned int)individualOrganizedBackbones[it->first].size());
					individualBranchPoints[it->first].back().push_back(bi.back());
					individualOrganizedBackbones[it->first].push_back(vector<unsigned int>(1, nh[i]));
					removeFrom(nh[i], unorganizedBackbone);
				}
			}else if(nh.size() == 1){ // skeleton segment might continue
				if(f[0] != individualOrganizedBackbones[it->first][0][0] && isIn(f[0], critVert)){ // inline branchpoint
					individualBranchPoints[it->first].push_back(vector<unsigned int>(1, bi[0])); // add previous segment to current branch point
					removeFrom(f[0], individualOrganizedBackbones[it->first][bi[0]]); // remove f[0] from previous segment
					individualBranchPoints[it->first].back().push_back((unsigned int)individualOrganizedBackbones[it->first].size()); // add f[0]'s new segment to new branch point (early because it will be added next)
					individualOrganizedBackbones[it->first].push_back(vector<unsigned int>(1, f[0])); // create new segment for f[0]
					bi[0] = (unsigned int)individualOrganizedBackbones[it->first].size(); // neighbor will be in new segment
					individualBranchPoints[it->first].back().push_back(bi[0]); // add neighbor's segment to branch point
					individualOrganizedBackbones[it->first].push_back(vector<unsigned int>(1, nh[0]));
					pushUnique(it->first, tipSeeds[f[0]]);
				}else // skeleton segment continues
					individualOrganizedBackbones[it->first][bi[0]].push_back(nh[0]);
				f[0] = nh[0];
				removeFrom(nh[0], unorganizedBackbone);
			}else{ // tip
				pushUnique(it->first, tipSeeds[f[0]]);//tipSeeds[f[0]].push_back(it->first);
				f.erase(f.begin());
				bi.erase(bi.begin());
			}
		}
    }
    // create vector of backbones into one big vector of individual backbones
	vector<vector<unsigned int> > backbones;
	branchpoints.clear();
	map<unsigned int, unsigned int> offsets; // the number of backbones already entered ahead of this segment: for seed x, individualOrganizedBackbones[x][i] is backbones[offsets[x] + i]
	for(map<unsigned int, vector<vector<unsigned int> > >::iterator it(individualOrganizedBackbones.begin()); it != individualOrganizedBackbones.end(); it++){
		offsets[it->first] = (unsigned int)backbones.size();
		backbones.insert(backbones.end(), it->second.begin(), it->second.end());
		for(unsigned int i(0); i < individualBranchPoints[it->first].size(); i++){
			branchpoints.push_back(vector<unsigned int>());
			for(unsigned int j(0); j < individualBranchPoints[it->first][i].size(); j++)
				branchpoints.back().push_back(offsets[it->first] + individualBranchPoints[it->first][i][j]);
		}
	}
	// stitch tips that have at least two segments by grouping into a branchpoint
	for(map<unsigned int, vector<unsigned int> >::iterator it(tipSeeds.begin()); it != tipSeeds.end(); it++){
		if(it->second.size() < 2) // actual tip or stranded component
			continue;
		branchpoints.push_back(vector<unsigned int>());
		for(unsigned int i(0); i < it->second.size(); i++){
			unsigned int seed(it->second[i]);
			unsigned int tipIndividualBackboneIndex((unsigned int)individualOrganizedBackbones[seed].size());
			for(unsigned int j(0); j < individualOrganizedBackbones[seed].size(); j++){ // over each of the individual seed's backbones
				unsigned int kmax((unsigned int)individualOrganizedBackbones[seed][j].size());
				for(unsigned int k(0); k < kmax; k++){ // over each point in the backbone
					if(individualOrganizedBackbones[seed][j][k] == it->first){
						tipIndividualBackboneIndex = j;
						k = kmax; // skip the rest of the loop
						j = (unsigned int)individualOrganizedBackbones[seed].size(); // skip the rest of the loop
					}
				}
			}
			if(tipIndividualBackboneIndex >= individualOrganizedBackbones[seed].size()){
				cout << "\n Error segmentBackbones(): tip point " << it->first << " not found in backbones from seed " << seed << endl;
				continue;
			}
			branchpoints.back().push_back(offsets[seed] + tipIndividualBackboneIndex);
		}
	}
	// combine backbones in branch points with only two backbone segments for final version
	// these backbones should always share the critical vertex that defined them
	for(int i(0); i < (int)branchpoints.size(); i++){
		if(branchpoints[i].size() == 2){
			unsigned int h(branchpoints[i][0]), t(branchpoints[i][1]);
			vector<unsigned int> head, tail;
			if(backbones[h][0] == backbones[t][0]){
				head = reversed(backbones[h]);
				tail = backbones[t];
			}else if(backbones[h].back() == backbones[t][0]){
				head = backbones[h];
				tail = backbones[t];
			}else if(backbones[h][0] == backbones[t].back()){
				head = backbones[t];
				tail = backbones[h];
			}else if(backbones[h].back() == backbones[t].back()){
				head = backbones[h];
				tail = reversed(backbones[t]);
			}else{
				cout << "\n Error segmentBackbones(): backbones at branchpoint " << i
					<< " share no critical point" << "\n\t backbones are:"
					<< "\n\t\t" << makeString(backbones[h])
					<< "\n\t\t" << makeString(backbones[t])
					<< endl;
				continue;
			}
			// perform stitch
			bool overlapping(true);
			while(overlapping && tail.size() > 1){
				tail.erase(tail.begin());
				vector<unsigned int> nh(findNeighbors(B, tail[1], head));
				overlapping = nh.size() > 0;
			}
			head.insert(head.end(), tail.begin(), tail.end());
			backbones[h] = head;
			backbones[t].clear();
			// rewrite instances of t in branchpoints to h
			for(unsigned int j(0); j < branchpoints.size(); j++){
				for(unsigned int k(0); k < branchpoints[j].size(); k++){
					if(branchpoints[j][k] == t)
						branchpoints[j][k] = h;
				}
			}
			branchpoints.erase(branchpoints.begin() + i);
			i--;
		}
	}
	// condense backbones so that it is compact (no empty backbones) and adjust branchPoints accordingly
	for(int i(0); i < (int)backbones.size(); i++){
		if(backbones[i].size() == 0){
			backbones.erase(backbones.begin() + i);
			for(unsigned int j(0); j < branchpoints.size(); j++){
				for(unsigned int k(0); k < branchpoints[j].size(); k++){
					if((int)branchpoints[j][k] > i)
						branchpoints[j][k]--;
				}
			}
			i--;
		}
	}
	return backbones;
}

vector<vector<vector<unsigned int> > > segregateBackbones(const vector<vector<unsigned int> > &backbones, const vector<vector<unsigned int> > &branchpoints, vector<vector<vector<unsigned int> > > &segregatedBranchpoints){
	map<unsigned int, vector<unsigned int> > adj; // local list of adjacencies
	for(unsigned int i(0); i < branchpoints.size(); i++){
		for(unsigned int j(0); j < branchpoints[i].size(); j++){
			for(unsigned int k(j + 1); k < branchpoints[i].size(); k++){
				pushUnique(branchpoints[i][k], adj[branchpoints[i][j]]); // shouldn't need to require uniqueness, but I'll include it to be safe
				pushUnique(branchpoints[i][j], adj[branchpoints[i][k]]);
			}
		}
	}
	vector<vector<vector<unsigned int> > > segregatedBackbones;
	vector<unsigned int> backbonesIndexToSegregatedBackbonesIndex(backbones.size(), 0), backbonesIndexToConnectedComponentIndex(backbones.size(), 0);
	vector<bool> included(backbones.size(), false);
	while(!allTrue(included)){
		unsigned int seed(findFirstFalse(included));
		vector<unsigned int> toCheck(1, seed);
		backbonesIndexToConnectedComponentIndex[seed] = (unsigned int)segregatedBackbones.size();
		backbonesIndexToSegregatedBackbonesIndex[seed] = 0;
		segregatedBackbones.push_back(vector<vector<unsigned int> >(1, backbones[seed]));
		included[seed] = true;
		while(!toCheck.empty()){
			for(unsigned int i(0); i < adj[toCheck[0]].size(); i++){
				unsigned int neighborIndex(adj[toCheck[0]][i]);
				if(!included[neighborIndex]){
					included[neighborIndex] = true;
					backbonesIndexToConnectedComponentIndex[neighborIndex] = (unsigned int)segregatedBackbones.size() - 1;
					backbonesIndexToSegregatedBackbonesIndex[neighborIndex] = (unsigned int)segregatedBackbones.back().size(); // not "- 1" because it hasn't been added yet
					segregatedBackbones.back().push_back(backbones[neighborIndex]);
					toCheck.push_back(neighborIndex);
				}
			}
			toCheck.erase(toCheck.begin());
		}
	}
	segregatedBranchpoints = vector<vector<vector<unsigned int> > >(segregatedBackbones.size(), vector<vector<unsigned int> >());
	for(unsigned int i(0); i < branchpoints.size(); i++){
		segregatedBranchpoints[backbonesIndexToConnectedComponentIndex[branchpoints[i][0]]].push_back(vector<unsigned int>(1, backbonesIndexToSegregatedBackbonesIndex[branchpoints[i][0]]));
		for(unsigned int j(1); j < branchpoints[i].size(); j++)
			segregatedBranchpoints[backbonesIndexToConnectedComponentIndex[branchpoints[i][j]]].back().push_back(backbonesIndexToSegregatedBackbonesIndex[branchpoints[i][j]]);
	}
	// reorganize segregatedBackbones and segregatedBranchpoints by number of segments
	bool changed(true);
	while(changed){
		changed = false;
		for(unsigned int i(0); i < segregatedBackbones.size() - 1; i++){
			if(segregatedBackbones[i].size() < segregatedBackbones[i + 1].size()){
				changed = true;
				vector<vector<unsigned int> > temp(segregatedBackbones[i]);
				segregatedBackbones[i] = segregatedBackbones[i + 1];
				segregatedBackbones[i + 1] = temp;
				temp = segregatedBranchpoints[i];
				segregatedBranchpoints[i] = segregatedBranchpoints[i + 1];
				segregatedBranchpoints[i + 1] = temp;
			}
		}
	}
	return segregatedBackbones;
}

/**
 * Returns the meat associated with backbone and not otherBackbones
 * The val of the map is the fraction of that voxel that belongs to the segment
 * @param B binary volume
 * @param backbone
 * @param otherBackbones
 */
map<unsigned int, double> segmentMeat(const BinaryVolume &B, const vector<unsigned int> &backbone, const vector<vector<unsigned int> > &otherBackbones){
	map<unsigned int, double> fseg;
	vector<map<unsigned int, double> > fother(otherBackbones.size(), map<unsigned int, double>());
	map<unsigned int, double> s;
	BinaryVolume uB(B);
	for(unsigned int i(0); i < backbone.size(); i++)
		fseg[backbone[i]] = 0.0;
	for(unsigned int i(0); i < otherBackbones.size(); i++){
		for(unsigned int j(0); j < otherBackbones[i].size(); j++)
			fother[i][otherBackbones[i][j]] = 0.0;
	}
	while(!fseg.empty()){
		// find minimim distance
		double minDist(fseg.begin()->second);
		for(map<unsigned int, double>::iterator it(fseg.begin()); it != fseg.end(); it++){
			if(minDist > it->second)
				minDist = it->second;
		}
		for(unsigned int i(0); i < fother.size(); i++){
			for(map<unsigned int, double>::iterator it(fother[i].begin()); it != fother[i].end(); it++){
				if(minDist > it->second)
					minDist = it->second;
			}
		}
		// expand frontiers
		vector<unsigned int> toErase;
		for(map<unsigned int, double>::iterator it(fseg.begin()); it != fseg.end(); it++){
			if(minDist == it->second){
				toErase.push_back(it->first);
				unsigned int shareCount(0);
				for(unsigned int i(0); i < fother.size(); i++){
					if(fother[i].find(it->first) != fother[i].end()){ // exists in another frontier
						if(fother[i][it->first] == minDist) // tie for closest backbone
							shareCount++;
					}
				}
				s[it->first] = 1.0/(1.0 + shareCount);
				uB.f(it->first);
				// check neighborhood
				vector<unsigned int> nh(vesselBlocksInNeighborhood(uB, it->first));
				for(unsigned int i(0); i < nh.size(); i++){
					double sepOffset(separation3D(it->first, nh[i]));
					double sep(it->second + sepOffset);
					bool betterSep(fseg.find(nh[i]) == fseg.end());
					if(!betterSep)
						betterSep = fseg[nh[i]] > sep;
					if(betterSep)
						fseg[nh[i]] = sep;
				}
			}
		}
		while(!toErase.empty()){
			fseg.erase(toErase[0]);
			for(unsigned int i(0); i < fother.size(); i++)
				fother[i].erase(toErase[0]);
			toErase.erase(toErase.begin());
		}
		for(unsigned int k(0); k < fother.size(); k++){
			for(map<unsigned int, double>::iterator it(fother[k].begin()); it != fother[k].end(); it++){
				if(minDist == it->second){
					toErase.push_back(it->first);
					uB.f(it->first);
					// check neighborhood
					vector<unsigned int> nh(vesselBlocksInNeighborhood(uB, it->first));
					for(unsigned int i(0); i < nh.size(); i++){
						double sepOffset(separation3D(it->first, nh[i]));
						double sep(it->second + sepOffset);
						bool betterSep(fother[k].find(nh[i]) == fother[k].end()); // already checked fseg and fother[x] with x <= k and it->first is not along the shortest path
						if(!betterSep)
							betterSep = fother[k][nh[i]] > sep;
						if(betterSep)
							fother[k][nh[i]] = sep;
					}
				}
			}
			while(!toErase.empty()){
				fseg.erase(toErase[0]);
				for(unsigned int i(0); i < fother.size(); i++)
					fother[i].erase(toErase[0]);
				toErase.erase(toErase.begin());
			}
		}
	}
	return s;
}

/**
 * Returns the index included in s that is farthest from the other segments in B
 * @param s
 * @param B
 */
unsigned int farthestFromOtherSegments(vector<unsigned int> &s, const BinaryVolume &B){
	// initialize distances
	vector<double> d(s.size(), -1.0);
	for(unsigned int i(0); i < s.size(); i++){
		vector<unsigned int> nh(vesselBlocksInNeighborhood(B, s[i]));
		for(unsigned int j(0); j < nh.size(); j++){
			if(!isIn(nh[j], s)) // is on boundary with other segments
				d[i] = 0.0;
		}
	}
	// adjacencies and/or distances in s
	vector<vector<unsigned int> > adjf(s.size(), vector<unsigned int>());
	map<unsigned int, vector<unsigned int> > nhs; // stored indices in B of the neighborhood of the key, which itself is an index in B
	vector<vector<double> > dadjf(s.size(), vector<double>());
	for(unsigned int i(0); i < s.size(); i++){
		for(unsigned int j(i + 1); j < s.size(); j++){
			if(inNeighborhood(s[i], s[j], B)){
				double sep(separation3D(s[i], s[j]));
				if(pushUnique(j, adjf[i])){//adjf[i].push_back(j);
					dadjf[i].push_back(sep);
					nhs[s[i]].push_back(s[j]);
				}
				if(pushUnique(i, adjf[j])){//adjf[j].push_back(i);
					dadjf[j].push_back(sep);
					nhs[s[j]].push_back(s[i]);
				}
			}
		}
	}
	// find shortest distances (somewhat inefficiently)
	bool changed = true;
	while(changed){
		changed = false;
		for(unsigned int i(0); i < s.size(); i++){
			for(unsigned int j(0); j < adjf[i].size(); j++){
				unsigned int k(adjf[i][j]);
				if(d[k] >= 0.0){ // some path to other segments is known
					double dist(d[k] + dadjf[i][j]);
					if(d[i] < 0.0 || d[i] > dist){
						d[i] = dist;
						changed = true;
					}
				}
			}
		}
	}
	double maxDist(0.0);
	for(unsigned int i(0); i < s.size(); i++){
		if(maxDist < d[i])
			maxDist = d[i];
	}
	for(unsigned int i(0); i < s.size(); i++){
		if(maxDist == d[i])
			return s[i];
	}
	return B.totalSize();
}

/** 
 * Collects and returns the segment backbones (from the variable backbones) such that the length of each path is at least critLength
 * @param comingFrom the direction of the parent in the search, so it is stipped in adding more backbones
 * @param exclude backbones that will not be visisted (this is necessary for local reticulations and for the search through branchpoints)
 */
vector<vector<unsigned int> > extendForLengthOf(const BinaryVolume &B, const vector<vector<unsigned int> > &backbones, const vector<vector<unsigned int> > &branchpoints,
		unsigned int comingFrom, double critLength, double currentLength, vector<unsigned int> &exclude){
	if(currentLength > critLength)
		return vector<vector<unsigned int> >();
	vector<unsigned int> toExtend;
	// find branch points that are shared with i
	// if i is a tip, there is only one
	// if i is not a tip, the parents and siblings have already been excluded to avoid duplicates or inacccurate lengths
	// if comingFrom is a terminal tip (possibly distinct from the initial tip), the nothing can be extended, so nothing is extended
	for(unsigned int i(0); i < branchpoints.size(); i++){
		if(isIn(comingFrom, branchpoints[i])){
			for(unsigned int j(0); j < branchpoints[i].size(); j++){
				if(!isIn(branchpoints[i][j], exclude)){
					exclude.push_back(branchpoints[i][j]);
					toExtend.push_back(branchpoints[i][j]);
				}
			}
		}
	}
	// perform extensions
	vector<vector<unsigned int> > extendedPaths;
	for(unsigned int i(0); i < toExtend.size(); i++){
		extendedPaths.push_back(backbones[toExtend[i]]);
		vector<vector<unsigned int> > childPaths(extendForLengthOf(B, backbones, branchpoints, toExtend[i], critLength, currentLength + backboneLength(backbones[toExtend[i]]), exclude));
		extendedPaths.insert(extendedPaths.end(), childPaths.begin(), childPaths.end());
	}
	return extendedPaths;
}

/** 
 * Assumes that there is at least one branch point
 */
void extendTips(const BinaryVolume &B, const BinaryVolume &O, vector<vector<unsigned int> > &backbones,
		const vector<vector<unsigned int> > &branchpoints
#if TRY_WX == 1
		, map<unsigned int, unsigned int> &pm
#endif
		){
	// find tips
	vector<bool> isTip(backbones.size(), false);
	vector<vector<unsigned int> > lastBranchpoints(backbones.size(), vector<unsigned int>()); // tips only have one; will list the adjacent backbones
	for(unsigned int i(0); i < branchpoints.size(); i++){
		for(unsigned int j(0); j < branchpoints[i].size(); j++){
			isTip[branchpoints[i][j]] = !isTip[branchpoints[i][j]]; // tips are listed exactly once; nontips are listed exactly twice
			lastBranchpoints[branchpoints[i][j]] = branchpoints[i];
		}
	}
	vector<unsigned int> tips;
	for(unsigned int i(0); i < backbones.size(); i++){
		if(isTip[i]){
            // get segment belonging to tip
            vector<unsigned int> exclude(1, i);
			vector<vector<unsigned int> > otherBackbones(extendForLengthOf(B, backbones, branchpoints, i, 2.0*backboneLength(backbones[i]) + 1.0, 0.0, exclude));
			//removeFrom(i, lastBranchpoints[i]);
			//for(unsigned int j(0); j < lastBranchpoints[i].size(); j++)
			//	otherBackbones.push_back(backbones[lastBranchpoints[i][j]]);
			map<unsigned int, double> sFrac(segmentMeat(B, backbones[i], otherBackbones));
			vector<unsigned int> s;
			s.reserve(sFrac.size());
			for(map<unsigned int, double>::iterator it(sFrac.begin()); it != sFrac.end(); it++)
				s.push_back(it->first);
#if TRY_WX == 1
			setPointSetColor(s, pm, 0.1, 0.1, 1.0, 0.5);
			setPointSetColor(backbones[i], pm, 0.0, 1.0, 0.0, 1.0);
			for(unsigned int j(0); j < otherBackbones.size(); j++)
				setPointSetColor(otherBackbones[j], pm, 1.0, 0.0, 1.0, 1.0);
#endif
			// find critical vertebra farthest from branchpoint
			unsigned int critTip(farthestFromOtherSegments(s, B));
			//cout << "\nextendTips(): critTip = " << critTip << endl;
			// erode using two critical vertebrae
			unsigned int tipTip(backbones[i][0]);
			if(inNeighborhood(tipTip, otherBackbones[0][0], B) || inNeighborhood(tipTip, otherBackbones[0].back(), B))
				tipTip = backbones[i].back();
			vector<unsigned int> cv(1, tipTip);
			cv.push_back(critTip);
			vector<unsigned int> missingTip(erodeForBackbone(B, O, s, cv));
			orderSegmentedBackbone(B, missingTip);
			//cout << "\nextendTips(): tipTip = " << tipTip << endl;
			// stitch with existing backbone;
			if(missingTip.size() > 1){// includes more than just tipTip
				vector<unsigned int> head, tail;
				if(missingTip[0] == tipTip && backbones[i][0] == tipTip){ // branchpoint at back; missingTip must be reversed
					head = reversed(missingTip);
					tail = backbones[i];
				}else if(missingTip[0] == tipTip && backbones[i].back() == tipTip){ // branchpoint at front
					head = backbones[i];
					tail = missingTip;
				}else if(missingTip.back() == tipTip && backbones[i][0] == tipTip){ // branchpoint at back
					head = missingTip;
					tail = backbones[i];
				}else if(missingTip.back() == tipTip && backbones[i].back() == tipTip){ // branchpoint at front; missingTip must be reversed
					head = backbones[i];
					tail = reversed(missingTip);
				}else{
					cout << "\n Error extendTips(): Could not properly match endpoints for backbone[" << i << "] = "
						<< makeString(backbones[i]) << endl;
					continue;
				}
				head.insert(head.end(), tail.begin() + 1, tail.end());
				backbones[i] = head;
#if TRY_WX == 1
				setDefaultColor(s, pm);
				setDefaultColor(backbones[i], pm);
				for(unsigned int j(0); j < otherBackbones.size(); j++)
					setDefaultColor(otherBackbones[j], pm);
#endif
			}
		}
	}
}

void generalStatusUpdate(string newStatus){
	cout << endl << newStatus << endl;
#if TRY_WX == 1
	wxGetApp().myFrame->updateStatus(newStatus);
#endif
}

/** 
 * Analyzes backbone i in backbones given branchpoints
 * Finds numVox (number of voxels, i.e., the volume), aveRadSolid (average radius of the solid meat)
 * @param aveRadSurf average radius of the outside of the meat
 */
void meatAnalysis(const BinaryVolume &B, const BinaryVolume &O,
		const vector<vector<unsigned int> > &backbones, const vector<vector<unsigned int> > &branchpoints,
		unsigned int i, double &numVox, double &aveRadSolid, double &aveRadSurf){
	// determine set of points that belong to segment i
	vector<unsigned int> exclude(1, i);
	vector<vector<unsigned int> > otherBackbones(extendForLengthOf(B, backbones, branchpoints, i, 2.0*backboneLength(backbones[i]) + 1.0, 0.0, exclude));
	map<unsigned int, double> sMap(segmentMeat(B, backbones[i], otherBackbones));
	numVox = 0.0;
	vector<unsigned int> s;
	s.reserve(sMap.size());
	for(map<unsigned int, double>::iterator it(sMap.begin()); it != sMap.end(); it++){
		s.push_back(it->first);
		numVox += it->second;
	}
	//rewrite averageRadius(...) so that creating another BinaryVolume is not necessary
	BinaryVolume sB(B.size, false);
	sB.t(s);
	averageRadius(sB, backbones[i], aveRadSolid, aveRadSurf);
}

inline double backboneNumberName(unsigned int backboneSetIndex, unsigned int backboneIndex, unsigned int numBackboneSets){
	return backboneIndex + (backboneSetIndex + 1)/(double)pow(10.0, ceil(log10(numBackboneSets + 1)));
}

/** 
 * @param i the index of the connected component in backbonesGlobal
 * @param j the index in backbone that is the child of parent
 */
string toStringSubtreeWithParent(double voxelVolume, unsigned int parent, unsigned int i, unsigned int j, vector<unsigned int> &exclude,
		const vector<vector<unsigned int> > &backbones,
		const vector<vector<unsigned int> > &branchpoints,
		const vector<double> &volumes, const vector<double> &aveRad){
	vector<unsigned int> adj;
	for(unsigned int k(0); k < branchpoints.size(); k++){
		if(isIn(j, branchpoints[k]))
			pushUniqueSet(branchpoints[k], adj);
	}
	for(unsigned int k(0); k < exclude.size(); k++)
		removeFrom(exclude[k], adj);
	stringstream strstr;
	double vol(voxelVolume*volumes[j]),
		len(backboneLength(backbones[j]));
	strstr << backboneNumberName(i, j, (unsigned int)backbones.size())
		<< "\t" << vol << "\t" << len
		<< "\t" << radFromVolLen(vol, len) << "\t" << aveRad[j];
	if(parent < backbones.size())
		strstr << "\t" << parent;
	else
		strstr << "\tN/A";
	strstr << "\t" << adj.size();
	for(unsigned int k(0); k < adj.size(); k++){
		strstr << "\t" << backboneNumberName(i, adj[k], (unsigned int)backbones.size());
		exclude.push_back(adj[k]);
	}
	strstr << endl;
	for(unsigned int k(0); k < adj.size(); k++) // add child strings
		strstr << toStringSubtreeWithParent(voxelVolume, j, i, adj[k], exclude, backbones, branchpoints, volumes, aveRad);
	return strstr.str();
}

/** 
 * Writes a file with parent-child hierarchy; some segments may have only one child because of reticulations
 */
void writeTSVwithRoots(string outputTSVwithRootsFn, string lengthUnits, const vector<unsigned int> &roots,
		const vector<vector<vector<unsigned int> > > &backbones,
		const vector<vector<vector<unsigned int> > > &branchpoints,
		const vector<vector<double> > &volumes, const vector<vector<double> > &aveRad){

	double voxelVolume(voxdimsGlobal[0]*voxdimsGlobal[1]*voxdimsGlobal[2]);
	ofstream outFile(outputTSVwithRootsFn.c_str());
	outFile << " name"
		<< "\t vol(cu." << lengthUnits << ")\t len(" << lengthUnits << ")"
		<< "\t <r>_vl(" << lengthUnits << ")\t <r>_obs(" << lengthUnits <<")"
		<< "\t par\t num_child\t children..."
		<< endl;
	for(unsigned int i(0); i < backbones.size(); i++){
		vector<unsigned int> exclude(1, roots[i]);
		outFile << toStringSubtreeWithParent(voxelVolume, (unsigned int)backbones[i].size(), i, roots[i], exclude, backbones[i], branchpoints[i], volumes[i], aveRad[i]);
	}
	outFile.close();
}

void writeTSV(string outputTSVfn, string lengthUnits,
		const vector<vector<vector<unsigned int> > > &backbones,
		const vector<vector<vector<unsigned int> > > &branchpoints,
		const vector<vector<double> > &volumes, const vector<vector<double> > &aveRad){
	double voxelVolume(voxdimsGlobal[0]*voxdimsGlobal[1]*voxdimsGlobal[2]);
	ofstream outFile(outputTSVfn.c_str());
	outFile << " name"
		<< "\t vol(cu." << lengthUnits << ")\t len(" << lengthUnits << ")"
		<< "\t <r>_vl(" << lengthUnits << ")\t <r>_obs(" << lengthUnits <<")"
		<< "\t num_adj\t adj..."
		<< endl;
	for(unsigned int i(0); i < backbones.size(); i++){
		for(unsigned int j(0); j < backbones[i].size(); j++){
			vector<unsigned int> adj;
			for(unsigned int k(0); k < branchpoints[i].size(); k++){
				if(isIn(j, branchpoints[i][k]))
					pushUniqueSet(branchpoints[i][k], adj);
			}
			removeFrom(j, adj);
			double vol(voxelVolume*volumes[i][j]),
				len(backboneLength(backbones[i][j]));
			outFile << backboneNumberName(i, j, (unsigned int)backbones.size())
				<< "\t" << vol << "\t" << len
				<< "\t" << radFromVolLen(vol, len) << "\t" << aveRad[i][j]
				<< "\t" << adj.size();
			for(unsigned int k(0); k < adj.size(); k++)
				outFile << "\t" << backboneNumberName(i, adj[k], (unsigned int)backbones.size());
			outFile << endl;
		}
	}
	outFile.close();
}

/** 
 * Chooses the root segment (indices in backbones) for each connected component and returns the size of backbones to indicate an error
 */
unsigned int assignRoot(const vector<vector<unsigned int> > &branchpoints, const vector<double> &aveRad){
	double maxRad(0.0);
	for(unsigned int i(0); i < aveRad.size(); i++){
		unsigned int branchpointCount(0);
		for(unsigned int j(0); j < branchpoints.size(); j++){
			if(isIn(i, branchpoints[j]))
				branchpointCount++;
		}
		if(maxRad < aveRad[i] && branchpointCount == 1)
			maxRad = aveRad[i];
	}
	for(unsigned int i(0); i < aveRad.size(); i++){
		if(aveRad[i] == maxRad)
			return i;
	}
	return (unsigned int)aveRad.size();
}

/** 
 * Chooses the root segments (indices in each vector of backbones) for each connected component
 */
vector<unsigned int> assignRoots(const vector<vector<vector<unsigned int> > > &branchpoints, const vector<vector<double> > &aveRad){
	vector<unsigned int> roots;
	for(unsigned int i(0); i < aveRad.size(); i++)
		roots.push_back(assignRoot(branchpoints[i], aveRad[i]));
	return roots;
}

/** 
 * Analyzes the vascular structure from images imStart to imEnd in imageDir (filename XXXXX.png) and writes output to TSVfnBase.tsv and/or TSVfnBase_withRoots.tsv as desired (see near the end of this method)
 * @param outputTSVfnBase outputted tsv file where analysis is written to
 * @param voxdims are the dimensions of a voxel
 * @param lengthUnit astring that specifies the unit of length for voxdims (e.g. "m" or "mm" or "um" [micrometers] or "nm" or simply "units")
 * @param thresh is the intensity threshold to identify vascular voxels
 * @param critFrac is the critical fraction of vascular voxels within a given sphere surface frontier that is required to continue growing the sphere
 */
void analyzeVascularStructure(string imageDir, int imStart, int imEnd, string outputTSVfnBase, double voxdims[], string lengthUnit,double thresh, double critFrac){
	startTime = time(NULL);
	kisset(11, 222, 3333);
	Lumens L(readPNGImages(imageDir, imStart, imEnd));
#if TRY_WX == 1
	wxGetApp().myFrame->updateStatus("Loading data...");
#endif
	BinaryVolume B(threshThrash(L, thresh));
	lengthUnitGlobal = lengthUnit;
	for(unsigned int i(0); i < 3; i++){
		voxdimsGlobal[i] = voxdims[i];
		voldimsGlobal[i] = B.size[i];
	}
	generalStatusUpdate("Initializing points...");
#if TRY_WX == 1
	map<unsigned int, unsigned int > pm;
	vector<double> pc(pointsWithColors(B, voxdims, vector<vector<unsigned int> >(), vector<double>(), pm));
#endif
#if TRY_WX == 1
	wxGetApp().myFrame->m_canvas->newPoints(pc);
#endif
	generalStatusUpdate("Showing all points.  Finding largest ccs...");
	BinaryVolume lccsB = removeSmallestConnectedComponents(B, 9
#if TRY_WX == 1
		, pm
#endif
		);
#if TRY_WX == 1
	generalStatusUpdate("Showing all points. Initializing points for largest ccs... (expect no visualization interaction)");
	pc = pointsWithColors(lccsB, voxdims, vector<vector<unsigned int> >(), vector<double>(), pm);
	wxGetApp().myFrame->m_canvas->newPoints(pc);
#endif
	generalStatusUpdate("Showing largest ccs.");
	B = lccsB;
	BinaryVolume O(findOutside(B));
	unsigned int fillings(fillInternalFalses(B, O));
	cout << "\n Filled " << fillings << " hole(s)." << endl;
	generalStatusUpdate("Coarsening...");
	BinaryVolume uB(B);
	map<unsigned int, double> rv(sphereCoarsen(B, uB, critFrac
#if TRY_WX == 1
		, pm
#endif
		));
	cout << endl << rv.size() << " spheres for critFrac = " << critFrac << endl;
    cout << endl << rv.size() << " total spheres for critFrac = " << critFrac << endl;

	generalStatusUpdate("Coarsened (" + makeString(rv.size()) + " points).  Finding adjacent spheres...");
	map<unsigned int, vector<unsigned int> > adj(adjacentSpheres(B, uB, rv
#if TRY_WX == 1
		, pm
#endif
		));
	generalStatusUpdate("Coarsened (" + makeString(rv.size()) + " points).  "
		+ makeString(adj.size()) + " connections between spheres. Removing false tips...");
	//unsigned int prevSphereSeeds(rv.size()), prevConnectionSeeds(adj.size());
	map<unsigned int, vector<unsigned int> > scs(sphereConnectionSeeds(adj));
	removeFalseTipsByContext(B, uB, rv, adj, scs
#if TRY_WX == 1
		, pm
#endif
		);
#if TRY_WX == 1
	for(map<unsigned int, vector<unsigned int> >::iterator it(adj.begin()); it != adj.end(); it++){
		double x(0.0), y(0.0), z(0.0);
		normalizedCoord(it->first, x, y, z);
		wxGetApp().myFrame->m_canvas->addNumber(it->first, x, y, z, 0.0, 0.5, 0.0, 1.0);
	}
#endif
	generalStatusUpdate("Coarsened (" + makeString(rv.size()) + " points).  "
		+ makeString(adj.size()) + " connections between spheres. Finding critical vertebrae...");
	map<unsigned int, vector<unsigned int> > critVert(findCriticalVertebrae(B, uB, O, rv, adj));
#if TRY_WX == 1
	for(map<unsigned int, vector<unsigned int> >::iterator it(critVert.begin()); it != critVert.end(); it++)
		setPointSetColor(it->second, pm, 0.0, 0.0, 0.0, 1.0);
#endif
	generalStatusUpdate("Coarsened (" + makeString(rv.size()) + " points).  "
		+ makeString(adj.size()) + " connections between spheres. Finding backbones...");
	map<unsigned int, vector<unsigned int> > rawBackbones(erosionBackbones(B, uB, O, rv, adj, scs, critVert));
#if TRY_WX == 1
	for(map<unsigned int, vector<unsigned int> >::iterator it(rawBackbones.begin()); it != rawBackbones.end(); it++)
		setPointSetColor(it->second, pm, 0.0, 0.0, 0.0, 0.5);
	for(map<unsigned int, vector<unsigned int> >::iterator it(critVert.begin()); it != critVert.end(); it++)
		setPointSetColor(it->second, pm, 0.0, 0.0, 0.0, 1.0);
#endif
	cout << "\n Found " << rawBackbones.size() << " raw (not segmented or segregated) backbones" << endl;
	generalStatusUpdate("Coarsened (" + makeString(rv.size()) + " points).  "
		+ makeString(adj.size()) + " connections between spheres. Segmenting backbones...");
	vector<vector<unsigned int> > branchpoints;
	vector<vector<unsigned int> > backbones(segmentBackbones(B, rawBackbones, critVert, branchpoints));
	cout << "\n Found " << backbones.size() << " backbones (after segmenting, before segregating)" << endl;
	cout << "\n Found " << branchpoints.size() << " branch points" << endl;
	cout << "\n B.totalSize() = " << B.totalSize() << "\n B.totalTrue() = " << B.totalTrue() << endl;
	generalStatusUpdate("Coarsened (" + makeString(rv.size()) + " points).  "
		+ makeString(adj.size()) + " connections between spheres. Resetting colors...");
#if TRY_WX == 1
	// write a new method that does not require making a list of all the points to start out...
	vector<unsigned int> allPoints;
	allPoints.reserve(pm.size()); 
	for(map<unsigned int, unsigned int>::iterator it(pm.begin()); it != pm.end(); it++)
		allPoints.push_back(it->first);
	setDefaultColor(allPoints, pm);
#endif
	generalStatusUpdate("Coarsened (" + makeString(rv.size()) + " points).  "
		+ makeString(adj.size()) + " connections between spheres. Extending tips...");
	extendTips(B, O, backbones, branchpoints
#if TRY_WX
		, pm
#endif
		);
	generalStatusUpdate("Coarsened (" + makeString(rv.size()) + " points).  "
		+ makeString(adj.size()) + " connections between spheres. Segregating backbones...");
	vector<vector<vector<unsigned int> > > segregatedBranchpoints;
	vector<vector<vector<unsigned int> > > segregatedBackbones(segregateBackbones(backbones, branchpoints, segregatedBranchpoints));
	backbonesGlobal = segregatedBackbones;
	branchpointsGlobal = segregatedBranchpoints;
	cout << "\n cc\t #backbones\t #branch points" << endl;
	for(unsigned int i(0); i < segregatedBackbones.size(); i++)
		cout << i << "\t" << segregatedBackbones[i].size() << "\t" << segregatedBranchpoints[i].size() << endl;
	vector<vector<vector<bool> > > branchAtFront(vector<vector<vector<bool> > >(segregatedBranchpoints.size(), vector<vector<bool> >()));
	for(unsigned int i(0); i < segregatedBranchpoints.size(); i++){ // over each cc
		branchAtFront[i] = vector<vector<bool> >(segregatedBranchpoints[i].size(), vector<bool>());
		for(unsigned int j(0); j < segregatedBranchpoints[i].size(); j++){ // over each branch point
			branchAtFront[i][j] = vector<bool>(segregatedBranchpoints[i][j].size(), false);
			unsigned int nearBranchpoint(segregatedBackbones[i][segregatedBranchpoints[i][j][0]][0]),
				nh(segregatedBackbones[i][segregatedBranchpoints[i][j][1]][0]),
				nt(segregatedBackbones[i][segregatedBranchpoints[i][j][1]].back());
			if(!inNeighborhood(nearBranchpoint, nh, B) && !inNeighborhood(nearBranchpoint, nt, B))
				nearBranchpoint = segregatedBackbones[i][segregatedBranchpoints[i][j][0]].back();
			for(unsigned int k(0); k < segregatedBranchpoints[i][j].size(); k++){
				unsigned int f(segregatedBackbones[i][segregatedBranchpoints[i][j][k]][0]), b(segregatedBackbones[i][segregatedBranchpoints[i][j][k]].back());
				double distFront(separation3D(nearBranchpoint, f)), distBack(separation3D(nearBranchpoint, b));
				branchAtFront[i][j][k] = distFront <= distBack;
			}
		}
	}
	branchAtFrontGlobal = branchAtFront;
	vector<vector<double> > volumes(segregatedBackbones.size(), vector<double>()),
		aveRadSolid(segregatedBackbones.size(), vector<double>()),
		aveRadSurf(segregatedBackbones.size(), vector<double>());
	for(unsigned int i(0); i < segregatedBackbones.size(); i++){
		volumes[i] = vector<double>(segregatedBackbones[i].size(), 0.0);
		aveRadSolid[i] = vector<double>(segregatedBackbones[i].size(), 0.0);
		aveRadSurf[i] = vector<double>(segregatedBackbones[i].size(), 0.0);
	}
	for(unsigned int i(0); i < segregatedBackbones.size(); i++){
		for(unsigned int j(0); j < segregatedBackbones[i].size(); j++)
			meatAnalysis(B, O, segregatedBackbones[i], segregatedBranchpoints[i], j, volumes[i][j], aveRadSolid[i][j], aveRadSurf[i][j]);
	}
	volumesGlobal = volumes;
	aveRadSolidGlobal = aveRadSolid;
	aveRadSurfGlobal = aveRadSurf;
	generalStatusUpdate("Coarsened (" + makeString(rv.size()) + " points).  "
		+ makeString(adj.size()) + " connections between spheres. Displaying backbones...");
#if TRY_WX == 1
	vector<double> bc(pointAndLineCols(L, B, voxdims, vector<vector<vector<unsigned int> > >(1, backbones), vector<vector<vector<unsigned int> > >(1, branchpoints), pc, pm));
	wxGetApp().myFrame->m_canvas->newPoints(pc);
	wxGetApp().myFrame->m_canvas->newLines(bc);
	vector<double> newNumbers;
	for(unsigned int i(0); i < segregatedBackbones.size(); i++){
		for(unsigned int j(0); j < segregatedBackbones[i].size(); j++){
			unsigned int com(centerOfMassFromSet(B, segregatedBackbones[i][j]));
			double x(0.0), y(0.0), z(0.0);
			normalizedCoord(com, x, y, z);
			newNumbers.push_back(x);
			newNumbers.push_back(y);
			newNumbers.push_back(z);
			newNumbers.push_back(0.0); // r
			newNumbers.push_back(0.0); // g
			newNumbers.push_back(0.0); // b
			newNumbers.push_back(1.0); // a
			newNumbers.push_back(backboneNumberName(i, j, (unsigned int)segregatedBackbones.size()));
		}
	}
	wxGetApp().myFrame->m_canvas->newNumbers(newNumbers);
#endif
	time_t endTime = time(NULL);
	cout << "\n\nRuntime: " << niceTime(difftime(endTime, startTime));
	cout << "\nProgram complete.\n";
	generalStatusUpdate("Coarsened (" + makeString(rv.size()) + " points).  "
		+ makeString(adj.size()) + " connections between spheres. [" + niceTime(difftime(endTime, startTime)) + "]");
	writeTSV(outputTSVfnBase + ".tsv", lengthUnit, segregatedBackbones, segregatedBranchpoints, volumes, aveRadSurf);
	vector<unsigned int> roots(assignRoots(segregatedBranchpoints, aveRadSurf));
	writeTSVwithRoots(outputTSVfnBase + "_withRoots.tsv", lengthUnit, roots, segregatedBackbones, segregatedBranchpoints, volumes, aveRadSurf);
}

void sphereCoarsenTest(){
#if defined(__WXMSW__) || defined(__WINDOWS__)
    string pathToImages("."); // Windows
#else
    string pathToImages("/Users/vascularuser1/Documents/AngicartCPlusPlus"); // Mac
    //string pathToImages("/Users/andersonju/Desktop/minimalSurfaces_share"); // Mac desktop
#endif
	double thresh(0.22), critFrac(0.16); // critFrac of 0.16 \approx 1/6 for 1 out of the six faces having a vessel neighbor
	//string imageDir(pathToImages + "/T_test"); int imStart(0), imEnd(2); double voxdims[] = {1.0, 1.0, 1.0}; string lengthUnit("units");//0-7
	string imageDir(pathToImages + "/small_easy_test"); int imStart(0), imEnd(0); double voxdims[] = {1.0, 1.0, 1.0}; string lengthUnit("units");
	//string imageDir(pathToImages + "/small_network_test"); int imStart(0), imEnd(7); double voxdims[] = {1.0, 1.0, 1.0}; string lengthUnit("units");
	//string imageDir(pathToImages + "/small_diag_test"); int imStart(0), imEnd(7); double voxdims[] = {1.0, 1.0, 1.0}; string lengthUnit("units");
	//string imageDir(pathToImages + "/small_loop_test"); int imStart(0), imEnd(7); double voxdims[] = {1.0, 1.0, 1.0}; string lengthUnit("units");
	//string imageDir(pathToImages + "/small_circle_test/small/smaller"); int imStart(0), imEnd(9); double voxdims[] = {1.0, 1.0, 1.0}; string lengthUnit("units");
	//string imageDir(pathToImages + "/FITC-MCA0_N12_NIH001_s1_4"); int imStart(0), imEnd(66); double voxdims[] = {1.648, 1.648, 0.492};  thresh = 0.22; string lengthUnit("um");
	//string imageDir(pathToImages + "/MacCL_168"); int imStart(0), imEnd(167); double voxdims[] = {1.0, 1.0, 1.0}; string lengthUnit("units");
	//string imageDir(pathToImages + "/FITC-MCA0_N12_PI001_s1"); int imStart(0), imEnd(61); double voxdims[] = {0.412, 0.412, 0.492}; thresh = 0.07; string lengthUnit("um");
	//string imageDir(pathToImages + "/Lung Images (Processed) - Study 2310 L2"); int imStart(1), imEnd(337); double voxdims[] = {1.0, 1.0, 1.0}; string lengthUnit("um"); // um units not to scale
	string outputTSVfnBase(pathToImages + "/summary");
	analyzeVascularStructure(imageDir, imStart, imEnd, outputTSVfnBase, voxdims, lengthUnit, thresh, critFrac);
}

#if TRY_WX != 1

int main(int argc, char *argv[]){
	startTime = time(NULL);
	srand((int)startTime);
	kisset(rand(), rand(), rand());
	cout.precision(16);
	if(argc == 1){
		cout << "\n Running sphereCoarsenTest()..." << endl;
		sphereCoarsenTest();
		cout << "\n sphereCoarsenTest() complete." << endl;
	}else if(argc == 10){
		string imageDir(argv[1]);
		int imStart(atoi(argv[2])), imEnd(atoi(argv[3]));
		string outputTSVfnBase(argv[4]);
		double voxdims[] = {atof(argv[5]), atof(argv[6]), atof(argv[7])};
		string lengthUnit(argv[8]);
		double thresh(atof(argv[9])), critFrac(0.16);
		analyzeVascularStructure(imageDir, imStart, imEnd, outputTSVfnBase, voxdims, lengthUnit, thresh, critFrac);
	}else{
		cout << "\n\nExpected arguments:\n\t string image_directory,\n\t int image_start_inclusive\n\t int image_end_inclusive"
			<< "\n\t string output_filename_base\n\t double voxel_dimension_x\n\t double voxel_dimension_y\n\t double voxel_dimension_z"
			<< "\n\t string length_unit_name\n\t double normalized_intensity_threshold" << endl;
	}
	time_t endTime = time(NULL);
	cout << "\n\nRuntime: " << niceTime(difftime(endTime, startTime));
	cout << "\nProgram complete.\n";

	return 0;
}

#endif