#ifndef _KISS_H
#define _KISS_H 1

//using namespace std;

struct seed_type{
  seed_type() : i(314159265), j(362436069), k(521288629){} /* default seeds of RNG */
  unsigned long i, j, k;
};

void kisset(unsigned long ii,
	    unsigned long jj,
	    unsigned long kk,
		bool exclaim = true);
void kisseed(unsigned long &ii, unsigned long &jj, unsigned long &kk);
void kissprint();
long kiss();
double rkiss();
double r2kiss();

#endif