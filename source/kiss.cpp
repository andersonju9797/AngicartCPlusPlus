/**
 Kiss.cpp
 Purpose: Generating random seeds
 
 @author David Hunt
 @version 1.1 7/3/17
 */

#include "stdlib.h"
#include "limits.h"
#include "kiss.h"
#include <iostream>

using namespace std;

seed_type seed;

/**
 kisset sets the seeds of RNG
 @param ii
 @param jj
 @param kk
 @param exclaim
 */
void kisset(unsigned long ii, unsigned long jj, unsigned long kk, bool exclaim)
{
  seed.i = ii;     /* Initiate the RNG using */
  seed.j = jj;     /* user-input seeds */
  seed.k = kk;
  if(exclaim)
	cout << "\n kisset (" << ii << ", " << jj << ", " << kk << ")" << endl;
}

void kisseed(unsigned long &ii, unsigned long &jj, unsigned long &kk){
	ii = seed.i;
	jj = seed.j;
	kk = seed.k;
}

void kissprint(){
	cout << "\n kissprint (" << seed.i << ", " << seed.j << ", " << seed.k << ")" << endl;
}

/**
 kiss returns the real part of the RNG
 @return long representing real part of RNG
 */
long kiss()
{
  seed.j = seed.j ^ (seed.j<<17);
  seed.k = (seed.k ^(seed.k<<18)) & 0x7fffffff;
  return (seed.i = 69069*seed.i + 23606797) +
    (seed.j ^= (seed.j>>15)) + (seed.k ^= (seed.k>>13));
}

/**
 rkiss generates random number in [0,1)
 @return random double between [0,1)
 */
double rkiss()
{
 double r;
 r=((double) kiss())/(2*(LONG_MAX+1.0)) + 0.5;
 return(r);
}

/**
 r2kiss generates random number in (0,1)
 @return random double between (0,1)
 */
double r2kiss()
{
 double r;
 r=((double) kiss())/(2*(LONG_MAX+1.0)) + 0.5;
 while(r<=0.0)
       {
       r=((double) kiss())/(2*(LONG_MAX+1.0)) + 0.5;
       }
 return(r);
}