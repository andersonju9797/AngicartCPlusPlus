/**
 BinaryVolume.cpp
 Purpose: Creates BinaryVolume object storing locations corresponding to vasculature
 
 @author David Hunt
 @version 1.1 7/3/17
 */

#include "BinaryVolume.h"

bool BinaryVolume::is(unsigned int x, unsigned int y, unsigned int z) const {
	if(x >= size[0] || y >= size[1] || z >= size[2])
		return false;
	return bits[indexOf(x, y, z)];
}
bool BinaryVolume::is(unsigned int i) const {
	return is(x(i), y(i), z(i));
}
void BinaryVolume::set(unsigned int x, unsigned int y, unsigned int z, bool tf){
	if(x < size[0] && y < size[1] && z < size[2])
		bits[indexOf(x, y, z)] = tf;
}
void BinaryVolume::t(unsigned int x, unsigned int y, unsigned int z){
	set(x, y, z, true);
}
void BinaryVolume::t(unsigned int i){
    t(x(i), y(i), z(i));
}
void BinaryVolume::t(const vector<unsigned int> &v){
	for(unsigned int i(0); i < v.size(); i++)
		t(x(v[i]), y(v[i]), z(v[i]));
}
void BinaryVolume::f(unsigned int x, unsigned int y, unsigned int z){
	set(x, y, z, false);
}
void BinaryVolume::f(unsigned int i){
	f(x(i), y(i), z(i));
}
void BinaryVolume::f(const vector<unsigned int> &v){
	for(unsigned int i(0); i < v.size(); i++)
		f(x(v[i]), y(v[i]), z(v[i]));
}
void BinaryVolume::clear(){
	bits = vector<bool>(totalSize(), false);
}
unsigned int BinaryVolume::findFirstAtOrAfter(unsigned int i) const {
	unsigned int iMax(totalSize());
	while(i < iMax){
		if(bits[i])
			return i;
		i++;
	}
	return i;
}
unsigned int BinaryVolume::findFirstFalseAtOrAfter(unsigned int i) const {
	unsigned int iMax(totalSize());
	while(i < iMax){
		if(!bits[i])
			return i;
		i++;
	}
	return i;
}
unsigned int BinaryVolume::totalSize() const {
	return size[0]*size[1]*size[2];
}
unsigned int BinaryVolume::totalTrue() const {
	unsigned int totSize(totalSize()), totTrue(0);
	for(unsigned int i(0); i < totSize; i++){
		if(bits[i])
			totTrue++;
	}
	return totTrue;
}
unsigned int BinaryVolume::indexOf(unsigned int x, unsigned int y, unsigned int z) const {
	return x*size[1]*size[2] + y*size[2] + z;
}
unsigned int BinaryVolume::x(unsigned int i) const {
	return i/(size[1]*size[2]);
}
unsigned int BinaryVolume::y(unsigned int i) const {
	return (i%(size[1]*size[2]))/size[2];
}
unsigned int BinaryVolume::z(unsigned int i) const {
	return i%size[2];
}
string BinaryVolume::makeStringPos(unsigned int i) const {
	stringstream strstr;
	strstr << "(" << x(i) << ", " << y(i) << ", " << z(i) << ")";
	return strstr.str();
}