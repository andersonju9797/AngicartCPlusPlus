#include <cmath>

#include "pngMinSurf.h"

//The following method has been altered from the original example_decode.cpp of LodePNG
//Example 1
//Decode from disk to raw pixels with a single function call
imagePNG decodeOneStep(const char* filename)
{
  vector<unsigned char> image; //the raw pixels
  unsigned int width, height;

  //decode
  unsigned error = lodepng::decode(image, width, height, filename);

  //if there's an error, display it
  if(error) cout << "decoder error " << error << ": " << lodepng_error_text(error) << endl;

  //the pixels are now in the vector "image", 4 bytes per pixel, ordered RGBARGBA..., use it as texture, draw it, ...
  return imagePNG(width, height, image);
}

// dirName does not need a final slash; expects image filenames of 5 padded integers
Lumens readPNGImages(string dirName, int start, int end){
	string s(dirName + "/" + paddedInt(start, 5) + ".png");
	imagePNG im(decodeOneStep(s.c_str())); // can move to z loop for speed
	if(im.image.size() == 0)
		abort();
	Lumens L(im.width, im.height, end - start + 1);
	for(int z(start); z <= end; z++){
		string s(dirName + "/" + paddedInt(z, 5) + ".png");
		imagePNG im(decodeOneStep(s.c_str()));
		for(unsigned int x(0); x < im.width; x++){
			for(unsigned int y(0); y < im.height; y++){
				int i(4*(y*im.width + x));
				L.lumens[x][y][z - start] = (double(im.image[i]) + double(im.image[i + 1]) + double(im.image[i + 2]))/3.0;
			}
		}
	}
	normalizeLumens(L);
	return L;
}

//The following method has been altered from the original example_decode.cpp of LodePNG
//Example 1
//Encode from raw pixels to disk with a single function call
//The image argument has width * height RGBA pixels or width * height * 4 bytes
void encodeOneStep(const char* filename, std::vector<unsigned char>& image, unsigned int width, unsigned int height)
{
  //Encode the image
  unsigned error = lodepng::encode(filename, image, width, height);

  //if there's an error, display it
  if(error) std::cout << "encoder error " << error << ": "<< lodepng_error_text(error) << std::endl;
}

void writePNGLumens(const Lumens &L, string fn){
	vector<unsigned char> im;
	for(unsigned int x(0); x < L.size[0]; x++){
		for(unsigned int y(0); y < L.size[1]; y++){
			double zSum(0.0);
			for(unsigned int z(0); z < L.size[2]; z++)
				zSum += L.lumens[x][y][z];
			//int i(4*(x*L.size[0] + y));
			char v((unsigned char)floor(255*zSum/L.size[2]));
			im.push_back(v); im.push_back(v); im.push_back(v); im.push_back(255);
		}
	}
	encodeOneStep(fn.c_str(), im, L.size[0], L.size[1]);
}

void writePNGHighlights(const Lumens &L, const vector<unsigned int> &xH, const vector<unsigned int> &yH, const vector<unsigned int> &zH, string fn){
	vector<unsigned char> im;
	for(unsigned int x(0); x < L.size[0]; x++){
		for(unsigned int y(0); y < L.size[1]; y++){
			double zSum(0.0);
			for(unsigned int z(0); z < L.size[2]; z++)
				zSum += L.lumens[x][y][z];
			char v((unsigned char)floor(255*zSum/L.size[2]));
			im.push_back(v); im.push_back(v); im.push_back(v); im.push_back(255);
		}
	}
	for(unsigned int i(0); i < xH.size(); i++){
		int base(4*(xH[i]*L.size[0] + yH[i]));
		im[base] = 0;
		im[base + 1] = 255;
		im[base + 2] = 0;
		im[base + 3] = 255;
	}
	encodeOneStep(fn.c_str(), im, L.size[0], L.size[1]);
}

void writePNGHighlights(const Lumens &L, const vector<unsigned int> &H, string fn){
	vector<unsigned char> im;
	for(unsigned int x(0); x < L.size[0]; x++){
		for(unsigned int y(0); y < L.size[1]; y++){
			double zSum(0.0);
			for(unsigned int z(0); z < L.size[2]; z++)
				zSum += L.lumens[x][y][z];
			char v((unsigned char)floor(255*zSum/L.size[2]));
			im.push_back(v); im.push_back(v); im.push_back(v); im.push_back(255);
		}
	}
	for(unsigned int i(0); i < H.size(); i++){
		unsigned int xH(L.x(H[i])), yH(L.y(H[i]));
		int base(4*(xH*L.size[0] + yH));
		im[base] = 0;
		im[base + 1] = 255;
		im[base + 2] = 0;
		im[base + 3] = 255;
	}
	encodeOneStep(fn.c_str(), im, L.size[0], L.size[1]);
}

void writePNGHighlights(const Lumens &L, const vector<unsigned int> &Hsub, const vector<unsigned int> &Hdom, string fn){
	vector<unsigned char> im;
	for(unsigned int x(0); x < L.size[0]; x++){
		for(unsigned int y(0); y < L.size[1]; y++){
			double zSum(0.0);
			for(unsigned int z(0); z < L.size[2]; z++)
				zSum += L.lumens[x][y][z];
			char v((unsigned char)floor(255*zSum/L.size[2]));
			im.push_back(v); im.push_back(v); im.push_back(v); im.push_back(255);
		}
	}
	for(unsigned int i(0); i < Hsub.size(); i++){
		unsigned int xH(L.x(Hsub[i])), yH(L.y(Hsub[i]));
		int base(4*(xH*L.size[0] + yH));
		im[base] = 0;
		im[base + 1] = 255;
		im[base + 2] = 0;
		im[base + 3] = 255;
	}
	for(unsigned int i(0); i < Hdom.size(); i++){
		unsigned int xH(L.x(Hdom[i])), yH(L.y(Hdom[i]));
		int base(4*(xH*L.size[0] + yH));
		im[base] = 0;
		im[base + 1] = 0;
		im[base + 2] = 255;
		im[base + 3] = 255;
	}
	encodeOneStep(fn.c_str(), im, L.size[0], L.size[1]);
}

void writePNGHighlightsThreeOLD(const Lumens &L, const vector<unsigned int> &Hsub, const vector<unsigned int> &Hdom, string fn){
	unsigned int border(10), imwidth(L.size[0] + 3*border + L.size[1]), imheight(L.size[1] + 3*border + L.size[2]);
	vector<unsigned char> im(4*imwidth*imheight, 0);
	unsigned int rDom(0), gDom(255), bDom(0), rSub(0), gSub(0), bSub(255);
	for(unsigned int x(0); x < L.size[0]; x++){
		for(unsigned int y(0); y < L.size[1]; y++){
			double zSum(0.0);
			for(unsigned int z(0); z < L.size[2]; z++)
				zSum += L.lumens[x][y][z];
			char v((unsigned char)floor(255*zSum/L.size[2]));
			int base(4*((border + x)*imheight + y + border));
			im[base] = im[base + 1] = im[base + 2] = v;
			im[base + 3] = 255;
		}
	}
	for(unsigned int i(0); i < Hsub.size(); i++){
        unsigned int xBB(L.x(Hsub[i])), yBB(L.y(Hsub[i]));//, zBB(L.z(Hsub[i]));
		int base(4*((border + xBB)*imheight + yBB + border));
		im[base] = rSub;
		im[base + 1] = gSub;
		im[base + 2] = bSub;
		im[base + 3] = 255;
	}
	for(unsigned int i(0); i < Hdom.size(); i++){
        unsigned int xBB(L.x(Hdom[i])), yBB(L.y(Hdom[i]));//, zBB(L.z(Hdom[i]));
		int base(4*((border + xBB)*imheight + yBB + border));
		im[base] = rDom;
		im[base + 1] = gDom;
		im[base + 2] = bDom;
		im[base + 3] = 255;
	}

	for(unsigned int z(0); z < L.size[2]; z++){
		for(unsigned int y(0); y < L.size[1]; y++){
			double xSum(0.0);
			for(unsigned int x(0); x < L.size[0]; x++)
				xSum += L.lumens[x][y][z];
			char v((unsigned char)floor(255*xSum/L.size[2]));
			int base(4*((2*border + z + L.size[0])*imheight + y + border));
			im[base] = im[base + 1] = im[base + 2] = v;
			im[base + 3] = 255;
		}
	}
	for(unsigned int i(0); i < Hsub.size(); i++){
		unsigned int yBB(L.y(Hsub[i])), zBB(L.z(Hsub[i]));// xBB(L.x(Hsub[i])),
		int base(4*((2*border + zBB + L.size[0])*imheight + yBB + border));
		im[base] = rSub;
		im[base + 1] = gSub;
		im[base + 2] = bSub;
		im[base + 3] = 255;
	}
	for(unsigned int i(0); i < Hdom.size(); i++){
		unsigned int yBB(L.y(Hdom[i])), zBB(L.z(Hdom[i]));//xBB(L.x(Hdom[i])),
		int base(4*((2*border + zBB + L.size[0])*imheight + yBB + border));
		im[base] = rDom;
		im[base + 1] = gDom;
		im[base + 2] = bDom;
		im[base + 3] = 255;
	}

	for(unsigned int x(0); x < L.size[0]; x++){
		for(unsigned int z(0); z < L.size[2]; z++){
			double ySum(0.0);
			for(unsigned int y(0); y < L.size[1]; y++)
				ySum += L.lumens[x][y][z];
			char v((unsigned char)floor(255*ySum/L.size[1]));
			int base(4*((border + x)*imheight + z + 2*border + L.size[1]));
			im[base] = im[base + 1] = im[base + 2] = v;
			im[base + 3] = 255;
		}
	}
	for(unsigned int i(0); i < Hsub.size(); i++){
		unsigned int xBB(L.x(Hsub[i])), zBB(L.z(Hsub[i]));//, yBB(L.y(Hsub[i]))
		int base(4*((border + xBB)*imheight + zBB + 2*border + L.size[1]));
		im[base] = rSub;
		im[base + 1] = gSub;
		im[base + 2] = bSub;
		im[base + 3] = 255;
	}
	for(unsigned int i(0); i < Hdom.size(); i++){
		unsigned int xBB(L.x(Hdom[i])), zBB(L.z(Hdom[i]));//, yBB(L.y(Hdom[i]))
		int base(4*((border + xBB)*imheight + zBB + 2*border + L.size[1]));
		im[base] = rDom;
		im[base + 1] = gDom;
		im[base + 2] = bDom;
		im[base + 3] = 255;
	}

	encodeOneStep(fn.c_str(), im, imwidth, imheight);
}

void writePNGHighlightsThree(const Lumens &L, const vector<unsigned int> &Hsub, const vector<unsigned int> &Hdom, string fn,
	unsigned int splitCount, unsigned int modulus){
	unsigned int border(10), imwidth(L.size[0] + 3*border + L.size[1]), imheight(L.size[1] + 3*border + L.size[2]);
	vector<unsigned char> im(4*imwidth*imheight, 0);
	unsigned char rDom(0), gDom(255), bDom(0), rSub(0), gSub(0), bSub(255);
	if(modulus > 0)
		rainbowColor(splitCount%modulus, modulus, rSub, gSub, bSub);
	for(unsigned int x(0); x < L.size[0]; x++){
		for(unsigned int y(0); y < L.size[1]; y++){
			double zSum(0.0);
			for(unsigned int z(0); z < L.size[2]; z++)
				zSum += L.lumens[x][y][z];
			char v((unsigned char)floor(255.0*zSum/L.size[2]));
			int base(4*((y + border)*imwidth + border + x));
			im[base] = im[base + 1] = im[base + 2] = v;
			im[base + 3] = 255;
		}
	}
	for(unsigned int i(0); i < Hsub.size(); i++){
		unsigned int xBB(L.x(Hsub[i])), yBB(L.y(Hsub[i]));//, zBB(L.z(Hsub[i]))
		int base(4*((border + yBB)*imwidth + xBB + border));
		im[base] = rSub;
		im[base + 1] = gSub;
		im[base + 2] = bSub;
		im[base + 3] = 255;
	}
	for(unsigned int i(0); i < Hdom.size(); i++){
		unsigned int xBB(L.x(Hdom[i])), yBB(L.y(Hdom[i]));//, zBB(L.z(Hdom[i]))
		int base(4*((border + yBB)*imwidth + xBB + border));
		im[base] = rDom;
		im[base + 1] = gDom;
		im[base + 2] = bDom;
		im[base + 3] = 255;
	}

	for(unsigned int z(0); z < L.size[2]; z++){
		for(unsigned int y(0); y < L.size[1]; y++){
			double xSum(0.0);
			for(unsigned int x(0); x < L.size[0]; x++)
				xSum += L.lumens[x][y][z];
			char v((unsigned char)floor(255*xSum/L.size[0]));
			int base(4*((y + border)*imwidth + 2*border + z + L.size[0]));
			im[base] = im[base + 1] = im[base + 2] = v;
			im[base + 3] = 255;
		}
	}
	for(unsigned int i(0); i < Hsub.size(); i++){
		unsigned int yBB(L.y(Hsub[i])), zBB(L.z(Hsub[i]));//xBB(L.x(Hsub[i])),
		int base(4*((yBB + border)*imwidth + 2*border + zBB + L.size[0]));
		im[base] = rSub;
		im[base + 1] = gSub;
		im[base + 2] = bSub;
		im[base + 3] = 255;
	}
	for(unsigned int i(0); i < Hdom.size(); i++){
		unsigned int yBB(L.y(Hdom[i])), zBB(L.z(Hdom[i]));//xBB(L.x(Hdom[i])),
		int base(4*((yBB + border)*imwidth + 2*border + zBB + L.size[0]));
		im[base] = rDom;
		im[base + 1] = gDom;
		im[base + 2] = bDom;
		im[base + 3] = 255;
	}

	for(unsigned int x(0); x < L.size[0]; x++){
		for(unsigned int z(0); z < L.size[2]; z++){
			double ySum(0.0);
			for(unsigned int y(0); y < L.size[1]; y++)
				ySum += L.lumens[x][y][z];
			char v((unsigned char)floor(255*ySum/L.size[1]));
			int base(4*((z + 2*border + L.size[1])*imwidth + border + x));
			im[base] = im[base + 1] = im[base + 2] = v;
			im[base + 3] = 255;
		}
	}
	for(unsigned int i(0); i < Hsub.size(); i++){
		unsigned int xBB(L.x(Hsub[i])), zBB(L.z(Hsub[i]));// , yBB(L.y(Hsub[i]))
		int base(4*((zBB + 2*border + L.size[1])*imwidth + border + xBB));
		im[base] = rSub;
		im[base + 1] = gSub;
		im[base + 2] = bSub;
		im[base + 3] = 255;
	}
	for(unsigned int i(0); i < Hdom.size(); i++){
		unsigned int xBB(L.x(Hdom[i])), zBB(L.z(Hdom[i])); // , yBB(L.y(Hdom[i]))
		int base(4*((zBB + 2*border + L.size[1])*imwidth + border + xBB));
		im[base] = rDom;
		im[base + 1] = gDom;
		im[base + 2] = bDom;
		im[base + 3] = 255;
	}

	encodeOneStep(fn.c_str(), im, imwidth, imheight);
}

void writePNGBackbones(const Lumens &L, const vector<vector<unsigned int> > &backbones, string fn){
	vector<unsigned char> im;
	for(unsigned int x(0); x < L.size[0]; x++){
		for(unsigned int y(0); y < L.size[1]; y++){
			double zSum(0.0);
			for(unsigned int z(0); z < L.size[2]; z++)
				zSum += L.lumens[x][y][z];
			char v((unsigned char)floor(255*zSum/L.size[2]));
			im.push_back(v); im.push_back(v); im.push_back(v); im.push_back(255);
		}
	}
	for(unsigned int i(0); i < backbones.size(); i++){
		for(unsigned int j(0); j < backbones[i].size(); j++){
			unsigned int xBB(L.x(backbones[i][j])), yBB(L.y(backbones[i][j]));
			int base(4*(xBB*L.size[0] + yBB));
			rainbowColor(i, (unsigned int)backbones.size(), im[base], im[base + 1], im[base + 2]);
			//im[base] = 0;
			//im[base + 1] = 255;
			//im[base + 2] = 0;
			im[base + 3] = 255;
		}
	}	
	encodeOneStep(fn.c_str(), im, L.size[0], L.size[1]);
}

void writePNGBackbonesThreeOLD(const Lumens &L, const vector<vector<unsigned int> > &backbones, string fn){
	unsigned int border(10), imwidth(L.size[0] + 3*border + L.size[1]), imheight(L.size[1] + 3*border + L.size[2]);
	vector<unsigned char> im(4*imwidth*imheight, 0);
	for(unsigned int x(0); x < L.size[0]; x++){
		for(unsigned int y(0); y < L.size[1]; y++){
			double zSum(0.0);
			for(unsigned int z(0); z < L.size[2]; z++)
				zSum += L.lumens[x][y][z];
			char v((unsigned char)floor(255*zSum/L.size[2]));
			int base(4*((border + x)*imheight + y + border));
			im[base] = im[base + 1] = im[base + 2] = v;
			im[base + 3] = 255;
		}
	}
	for(unsigned int i(0); i < backbones.size(); i++){
		for(unsigned int j(0); j < backbones[i].size(); j++){
			unsigned int xBB(L.x(backbones[i][j])), yBB(L.y(backbones[i][j])), zBB(L.z(backbones[i][j]));
			bool highestZ(true);
			for(unsigned int m(0); m < backbones.size() && highestZ; m++){
				if(m == i)
					continue;
				for(unsigned int n(0); n < backbones[m].size() && highestZ; n++){
					highestZ = !(xBB == L.x(backbones[m][n])
						&& yBB == L.y(backbones[m][n])
						&& zBB < L.z(backbones[m][n]));
				}
			}
			if(highestZ){
				int base(4*((border + xBB)*imheight + yBB + border));
				rainbowColor(i, (unsigned int)backbones.size(), im[base], im[base + 1], im[base + 2]);
				im[base + 3] = 255;
			}
		}
	}

	for(unsigned int z(0); z < L.size[2]; z++){
		for(unsigned int y(0); y < L.size[1]; y++){
			double xSum(0.0);
			for(unsigned int x(0); x < L.size[0]; x++)
				xSum += L.lumens[x][y][z];
			char v((unsigned char)floor(255*xSum/L.size[0]));
			int base(4*((2*border + L.size[0] + z)*imheight + y + border));
			im[base] = im[base + 1] = im[base + 2] = v;
			im[base + 3] = 255;
		}
	}
	for(unsigned int i(0); i < backbones.size(); i++){
		for(unsigned int j(0); j < backbones[i].size(); j++){
			unsigned int xBB(L.x(backbones[i][j])), yBB(L.y(backbones[i][j])), zBB(L.z(backbones[i][j]));
			bool highestX(true);
			for(unsigned int m(0); m < backbones.size() && highestX; m++){
				if(m == i)
					continue;
				for(unsigned int n(0); n < backbones[m].size() && highestX; n++){
					highestX = !(xBB < L.x(backbones[m][n])
						&& yBB == L.y(backbones[m][n])
						&& zBB == L.z(backbones[m][n]));
				}
			}
			if(highestX){
				int base(4*((2*border + L.size[0] + zBB)*imheight + yBB + border));
				rainbowColor(i, (unsigned int)backbones.size(), im[base], im[base + 1], im[base + 2]);
				im[base + 3] = 255;
			}
		}
	}

	for(unsigned int x(0); x < L.size[0]; x++){
		for(unsigned int z(0); z < L.size[2]; z++){
			double ySum(0.0);
			for(unsigned int y(0); y < L.size[1]; y++)
				ySum += L.lumens[x][y][z];
			char v((unsigned char)floor(255*ySum/L.size[1]));
			int base(4*((border + x)*imheight + z + 2*border + L.size[1]));
			im[base] = im[base + 1] = im[base + 2] = v;
			im[base + 3] = 255;
		}
	}
	for(unsigned int i(0); i < backbones.size(); i++){
		for(unsigned int j(0); j < backbones[i].size(); j++){
			unsigned int xBB(L.x(backbones[i][j])), yBB(L.y(backbones[i][j])), zBB(L.z(backbones[i][j]));
			bool highestY(true);
			for(unsigned int m(0); m < backbones.size() && highestY; m++){
				if(m == i)
					continue;
				for(unsigned int n(0); n < backbones[m].size() && highestY; n++){
					highestY = !(xBB == L.x(backbones[m][n])
						&& yBB < L.y(backbones[m][n])
						&& zBB == L.z(backbones[m][n]));
				}
			}
			if(highestY){
				int base(4*((border + xBB)*imheight + zBB + 2*border + L.size[1]));
				rainbowColor(i, (unsigned int)backbones.size(), im[base], im[base + 1], im[base + 2]);
				im[base + 3] = 255;
			}
		}
	}

	encodeOneStep(fn.c_str(), im, imwidth, imheight);
}

void writePNGBranchingJunctions(unsigned int numBackbones, const vector<vector<unsigned int> > &branchingJunctions, string fn){
	unsigned int imwidth(numBackbones), imheight(2*((unsigned int)branchingJunctions.size() + 1) + 1);
	vector<unsigned char> im(4*imwidth*imheight, 255);
	vector<unsigned char> r(numBackbones, 0), g(numBackbones, 0), b(numBackbones, 0);
	for(unsigned int i(0); i < numBackbones; i++){
		rainbowColor(i, numBackbones, r[i], g[i], b[i]);
		im[4*(numBackbones + i)] = r[i];
		im[4*(numBackbones + i) + 1] = g[i];
		im[4*(numBackbones + i) + 2] = b[i];
	}
	for(unsigned int i(0); i < branchingJunctions.size(); i++){
		for(unsigned int j(0); j < branchingJunctions[i].size(); j++){
			unsigned int base(4*(imwidth*(2*i + 3) + j));
			if(branchingJunctions[i][j] < numBackbones){
				im[base] = r[branchingJunctions[i][j]];
				im[base + 1] = g[branchingJunctions[i][j]];
				im[base + 2] = b[branchingJunctions[i][j]];
			}else{
				im[base] = im[base + 1] = im[base + 2] = 0;
			}
		}
	}
	encodeOneStep(fn.c_str(), im, imwidth, imheight);
}

void writePNGLegend(const vector<vector<unsigned int> > &backbones, string fn){
	vector<unsigned char> im(4*backbones.size(), 255);
	for(unsigned int i(0); i < backbones.size(); i++)
		rainbowColor(i, (unsigned int)backbones.size(), im[4*i], im[4*i + 1], im[4*i + 2]);
	encodeOneStep(fn.c_str(), im, (unsigned int)backbones.size(), 1);
}

void writePNGBackbonesThree(const Lumens &L, const vector<vector<unsigned int> > &backbones, string fn){
	unsigned int border(10), imwidth(L.size[0] + 3*border + L.size[1]), imheight(L.size[1] + 3*border + L.size[2]);
	vector<unsigned char> im(4*imwidth*imheight, 0);
	for(unsigned int x(0); x < L.size[0]; x++){
		for(unsigned int y(0); y < L.size[1]; y++){
			double zSum(0.0);
			for(unsigned int z(0); z < L.size[2]; z++)
				zSum += L.lumens[x][y][z];
			char v((unsigned char)floor(255*zSum/L.size[2]));
			int base(4*((border + y)*imwidth + x + border));
			im[base] = im[base + 1] = im[base + 2] = v;
			im[base + 3] = 255;
		}
	}
	for(unsigned int i(0); i < backbones.size(); i++){
		for(unsigned int j(0); j < backbones[i].size(); j++){
			unsigned int xBB(L.x(backbones[i][j])), yBB(L.y(backbones[i][j])), zBB(L.z(backbones[i][j]));
			bool highestZ(true);
			for(unsigned int m(0); m < backbones.size() && highestZ; m++){
				if(m == i)
					continue;
				for(unsigned int n(0); n < backbones[m].size() && highestZ; n++){
					highestZ = !(xBB == L.x(backbones[m][n])
						&& yBB == L.y(backbones[m][n])
						&& zBB < L.z(backbones[m][n]));
				}
			}
			if(highestZ){
				int base(4*((border + yBB)*imwidth + xBB + border));
				rainbowColor(i, (unsigned int)backbones.size(), im[base], im[base + 1], im[base + 2]);
				im[base + 3] = 255;
			}
		}
	}

	for(unsigned int z(0); z < L.size[2]; z++){
		for(unsigned int y(0); y < L.size[1]; y++){
			double xSum(0.0);
			for(unsigned int x(0); x < L.size[0]; x++)
				xSum += L.lumens[x][y][z];
			char v((unsigned char)floor(255*xSum/L.size[0]));
			int base(4*((border + y)*imwidth + 2*border + L.size[0] + z + border));
			im[base] = im[base + 1] = im[base + 2] = v;
			im[base + 3] = 255;
		}
	}
	for(unsigned int i(0); i < backbones.size(); i++){
		for(unsigned int j(0); j < backbones[i].size(); j++){
			unsigned int xBB(L.x(backbones[i][j])), yBB(L.y(backbones[i][j])), zBB(L.z(backbones[i][j]));
			bool highestX(true);
			for(unsigned int m(0); m < backbones.size() && highestX; m++){
				if(m == i)
					continue;
				for(unsigned int n(0); n < backbones[m].size() && highestX; n++){
					highestX = !(xBB < L.x(backbones[m][n])
						&& yBB == L.y(backbones[m][n])
						&& zBB == L.z(backbones[m][n]));
				}
			}
			if(highestX){
				int base(4*((border + yBB)*imwidth + 2*border + L.size[0] + zBB + border));
				rainbowColor(i, (unsigned int)backbones.size(), im[base], im[base + 1], im[base + 2]);
				im[base + 3] = 255;
			}
		}
	}

	for(unsigned int x(0); x < L.size[0]; x++){
		for(unsigned int z(0); z < L.size[2]; z++){
			double ySum(0.0);
			for(unsigned int y(0); y < L.size[1]; y++)
				ySum += L.lumens[x][y][z];
			char v((unsigned char)floor(255*ySum/L.size[1]));
			int base(4*((z + 2*border + L.size[1])*imwidth + border + x));
			im[base] = im[base + 1] = im[base + 2] = v;
			im[base + 3] = 255;
		}
	}
	for(unsigned int i(0); i < backbones.size(); i++){
		for(unsigned int j(0); j < backbones[i].size(); j++){
			unsigned int xBB(L.x(backbones[i][j])), yBB(L.y(backbones[i][j])), zBB(L.z(backbones[i][j]));
			bool highestY(true);
			for(unsigned int m(0); m < backbones.size() && highestY; m++){
				if(m == i)
					continue;
				for(unsigned int n(0); n < backbones[m].size() && highestY; n++){
					highestY = !(xBB == L.x(backbones[m][n])
						&& yBB < L.y(backbones[m][n])
						&& zBB == L.z(backbones[m][n]));
				}
			}
			if(highestY){
				int base(4*((zBB + 2*border + L.size[1])*imwidth + border + xBB));
				rainbowColor(i, (unsigned int)backbones.size(), im[base], im[base + 1], im[base + 2]);
				im[base + 3] = 255;
			}
		}
	}

	encodeOneStep(fn.c_str(), im, imwidth, imheight);
}

void writePNGBinaryVolume(const BinaryVolume &B, string fn){
	vector<unsigned char> im(4*B.size[0]*B.size[1], 0);
	for(unsigned int x(0); x < B.size[0]; x++){
		for(unsigned int y(0); y < B.size[1]; y++){
			double zSum(0.0);
			for(unsigned int z(0); z < B.size[2]; z++){
				if(B.is(x, y, z))
					zSum += 1.0;
			}
			int i(4*(y*B.size[0] + x));
			char v((unsigned char)floor(255*zSum/B.size[2]));
			im[i] = im[i + 1] = im[i + 2] = v;
			im[i + 3] = 255;
		}
	}
	encodeOneStep(fn.c_str(), im, B.size[0], B.size[1]);
}