#ifndef LUMENS_H
#define LUMENS_H 1

struct Lumens{
	unsigned int size[3];
	double ***lumens;
	Lumens(){ size[0] = size[1] = size[2] = 0; lumens = new double**[1]; lumens[0] = new double*[1]; lumens[0][0] = new double[1]; }
	~Lumens(){
		if(size[0] == 0)
			size[0] = size[1] = size[2] = 1;
		for(unsigned int i(0); i < size[0]; i++){
			for(unsigned int j(0); j < size[1]; j++)
				delete[] lumens[i][j];
			delete[] lumens[i];
		}
		size[0] = size[1] = size[2] = 0;
	}
	Lumens(unsigned int x, unsigned int y, unsigned int z){
		size[0] = x; size[1] = y, size[2] = z;
		lumens = new double**[x];
		for(unsigned int i(0); i < x; i++){
			lumens[i] = new double*[y];
			for(unsigned int j(0); j < y; j++){
				lumens[i][j] = new double[z];
				for(unsigned int k(0); k < z; k++)
					lumens[i][j][k] = 0.0;
			}
		}
	}
	Lumens(const Lumens &L){
		unsigned int x(L.size[0]), y(L.size[1]), z(L.size[2]);
		size[0] = x; size[1] = y, size[2] = z;
		lumens = new double**[x];
		for(unsigned int i(0); i < x; i++){
			lumens[i] = new double*[y];
			for(unsigned int j(0); j < y; j++){
				lumens[i][j] = new double[z];
				for(unsigned int k(0); k < z; k++)
					lumens[i][j][k] = L.lumens[i][j][k];
			}
		}
	}
	Lumens& operator=(const Lumens &L){
		unsigned int x(L.size[0]), y(L.size[1]), z(L.size[2]);
		size[0] = x; size[1] = y, size[2] = z;
		lumens = new double**[x];
		for(unsigned int i(0); i < x; i++){
			lumens[i] = new double*[y];
			for(unsigned int j(0); j < y; j++){
				lumens[i][j] = new double[z];
				for(unsigned int k(0); k < z; k++)
					lumens[i][j][k] = L.lumens[i][j][k];
			}
		}
		return *this;
	}
	
	unsigned int totalSize() const ;
	unsigned int indexOf(unsigned int x, unsigned int y, unsigned int z) const ;
	unsigned int x(unsigned int i) const ;
	unsigned int y(unsigned int i) const ;
	unsigned int z(unsigned int i) const ;
	double minLumen() const ;
	double maxLumen() const ;
};

void normalizeLumens(Lumens &L);
Lumens simpleLumensCube(unsigned int cubeSide = 2);

#endif