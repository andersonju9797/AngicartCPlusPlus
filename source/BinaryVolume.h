/**
 BinaryVolume.h
 Purpose: Creates BinaryVolume object storing locations corresponding to vasculature
 
 @author David Hunt
 @version 1.1 7/3/17
 */

#ifndef BINARYVOLUME_H

#define BINARYVOLUME_H 1
#include <vector>
#include <sstream>
#include <string>

using namespace std;

struct BinaryVolume{
	unsigned int size[3];
	BinaryVolume(unsigned int x, unsigned int y, unsigned int z){
		size[0] = x;
		size[1] = y;
		size[2] = z;
		bits = vector<bool>(x*y*z, false);
	}
	BinaryVolume(const BinaryVolume &B){
		size[0] = B.size[0];
		size[1] = B.size[1];
		size[2] = B.size[2];
		bits = B.bits;
	}
	BinaryVolume(const unsigned int givenSize[], bool tf){
		size[0] = givenSize[0];
		size[1] = givenSize[1];
		size[2] = givenSize[2];
		bits = vector<bool>(size[0]*size[1]*size[2], tf);
	}
	bool is(unsigned int x, unsigned int y, unsigned int z) const ;
	bool is(unsigned int i) const ;
	void set(unsigned int x, unsigned int y, unsigned int z, bool tf);
	void t(unsigned int x, unsigned int y, unsigned int z);
	void t(unsigned int i);
	void t(const vector<unsigned int> &v);
	void f(unsigned int x, unsigned int y, unsigned int z);
	void f(unsigned int i);
	void f(const vector<unsigned int> &v);
	void clear();
	unsigned int findFirstAtOrAfter(unsigned int i = 0) const ;
	unsigned int findFirstFalseAtOrAfter(unsigned int i = 0) const ;
	unsigned int totalSize() const ;
	unsigned int totalTrue() const ;
	unsigned int indexOf(unsigned int x, unsigned int y, unsigned int z) const ;
	unsigned int x(unsigned int i) const ;
	unsigned int y(unsigned int i) const ;
	unsigned int z(unsigned int i) const ;
	string makeStringPos(unsigned int i) const ;
private:
	vector<bool> bits;
};
#endif