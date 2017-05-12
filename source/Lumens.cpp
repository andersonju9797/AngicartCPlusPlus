#include "Lumens.h"

unsigned int Lumens::totalSize() const {
	return size[0]*size[1]*size[2];
}

unsigned int Lumens::indexOf(unsigned int x, unsigned int y, unsigned int z) const {
	return x*size[1]*size[2] + y*size[2] + z;
}
unsigned int Lumens::x(unsigned int i) const {
	return i/(size[1]*size[2]);
}
unsigned int Lumens::y(unsigned int i) const {
	return (i%(size[1]*size[2]))/size[2];
}
unsigned int Lumens::z(unsigned int i) const {
	return i%size[2];
}
double Lumens::minLumen() const {
	double m(lumens[0][0][0]);
	for(unsigned int i(0); i < size[0]; i++){
		for(unsigned int j(0); j < size[1]; j++){
			for(unsigned int k(0); k < size[2]; k++){
				if(m > lumens[i][j][k])
					m = lumens[i][j][k];
			}
		}
	}
	return m;
}
double Lumens::maxLumen() const {
	double m(lumens[0][0][0]);
	for(unsigned int i(0); i < size[0]; i++){
		for(unsigned int j(0); j < size[1]; j++){
			for(unsigned int k(0); k < size[2]; k++){
				if(m < lumens[i][j][k])
					m = lumens[i][j][k];
			}
		}
	}
	return m;
}

void normalizeLumens(Lumens &L){
	if(L.size[0] < 1 || L.size[1] < 1 || L.size[2] < 1)
		return;
	double maxL(L.lumens[0][0][0]), minL(L.lumens[0][0][0]);
	for(unsigned int x(0); x < L.size[0]; x++){
		for(unsigned int y(0); y < L.size[1]; y++){
			for(unsigned int z(0); z < L.size[2]; z++){
				if(maxL < L.lumens[x][y][z])
					maxL = L.lumens[x][y][z];
				if(minL > L.lumens[x][y][z])
					minL = L.lumens[x][y][z];
			}
		}
	}
	if(maxL == minL){
		for(unsigned int x(0); x < L.size[0]; x++){
			for(unsigned int y(0); y < L.size[1]; y++){
				for(unsigned int z(0); z < L.size[2]; z++)
					L.lumens[x][y][z] = 0.0;
			}
		}
		return;
	}
	for(unsigned int x(0); x < L.size[0]; x++){
		for(unsigned int y(0); y < L.size[1]; y++){
			for(unsigned int z(0); z < L.size[2]; z++)
				L.lumens[x][y][z] = (L.lumens[x][y][z] - minL)/(maxL - minL);
		}
	}
}

Lumens simpleLumensCube(unsigned int cubeSide){
	Lumens L(cubeSide, cubeSide, cubeSide);
	for(unsigned int x(0); x < L.size[0]; x++){
		for(unsigned int y(0); y < L.size[1]; y++){
			for(unsigned int z(0); z < L.size[2]; z++)
				L.lumens[x][y][z] = 1.0;
		}
	}
	L.lumens[0][0][0] = 0.0;
	return L;
}
