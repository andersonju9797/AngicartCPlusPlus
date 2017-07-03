/**
 pngMinSurf.h
 Purpose:
 
 @author David Hunt
 @version 1.1 7/3/17
 */

#ifndef PNGMINSURF_H
#define PNGMINSURF_H 1
#include <iostream>
#include <string>
#include <vector>
#include "lodepng.h"
#include "BinaryVolume.h"
#include "Lumens.h"
#include "utilMinSurfTests.h"

using namespace std;

struct imagePNG{
	unsigned int width, height;
	vector<unsigned char> image;
	imagePNG(unsigned int w, unsigned int h, vector<unsigned char> &i){
		width = w;
		height = h;
		image = i;
	}
};
imagePNG decodeOneStep(const char* filename);
Lumens readPNGImages(string dirName, int start, int end);
void encodeOneStep(const char* filename, std::vector<unsigned char>& image, unsigned int width, unsigned int height);
void writePNGLumens(const Lumens &L, string fn);
void writePNGHighlights(const Lumens &L, const vector<unsigned int> &xH, const vector<unsigned int> &yH, const vector<unsigned int> &zH, string fn);
void writePNGHighlights(const Lumens &L, const vector<unsigned int> &H, string fn);
void writePNGHighlights(const Lumens &L, const vector<unsigned int> &Hsub, const vector<unsigned int> &Hdom, string fn);
void writePNGHighlightsThreeOLD(const Lumens &L, const vector<unsigned int> &Hsub, const vector<unsigned int> &Hdom, string fn);
void writePNGHighlightsThree(const Lumens &L, const vector<unsigned int> &Hsub, const vector<unsigned int> &Hdom, string fn,
	unsigned int splitCount = 0, unsigned int modulus = 0);
void writePNGBackbones(const Lumens &L, const vector<vector<unsigned int> > &backbones, string fn);
void writePNGBackbonesThreeOLD(const Lumens &L, const vector<vector<unsigned int> > &backbones, string fn);
void writePNGBranchingJunctions(unsigned int numBackbones, const vector<vector<unsigned int> > &branchingJunctions, string fn);
void writePNGLegend(const vector<vector<unsigned int> > &backbones, string fn);
void writePNGBackbonesThree(const Lumens &L, const vector<vector<unsigned int> > &backbones, string fn);
void writePNGBinaryVolume(const BinaryVolume &B, string fn);
#endif