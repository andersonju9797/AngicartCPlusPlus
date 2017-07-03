/**
 utilMinSurfTests.cpp
 Purpose:
 
 @author David Hunt
 @version 1.1 7/3/17
 */

#include "utilMinSurfTests.h"
#include <cmath>
#include <sstream>

bool isIn(unsigned int x, map<unsigned int, vector<unsigned int> > &m){
	for(map<unsigned int, vector<unsigned int> >::iterator it(m.begin()); it != m.end(); it++){
		for(unsigned int i(0); i < it->second.size(); i++){
			if(it->second[i] == x)
				return true;
		}
	}
	return false;
}

string niceTime(double t, bool roundSeconds){
	if(t >= 0.0){
		stringstream nT;
		int m(60);
		int h(60*m);
		int d(24*h);
		int w(7*d);
		int wt((int)floor(t/w));
		t -= w*wt;
		int dt((int)floor(t/d));
		t -= d*dt;
		int ht((int)floor(t/h));
		t -= h*ht;
		int mt((int)floor(t/m));
		t -= m*mt;
		if(wt > 0)
			nT << wt << "w ";
		if(wt > 0 || dt > 0)
			nT << dt << "d ";
		if(wt > 0 || dt > 0 || ht > 0)
			nT << ht << "h ";
		if(wt > 0 || dt > 0 || ht > 0 || mt > 0)
			nT << mt << "m ";
		if(roundSeconds)
			nT << ceil(t) << "s";
		else
			nT << t << "s";
		return nT.str();
	} else return "-(" + niceTime(-t) + ")";
}

string paddedInt(int x, int digits){
	string s(makeString(x));
	int thresh(10);
	for(int i(1); i < digits; i++){
		if(x < thresh)
			s = "0" + s;
		thresh *= 10;
	}
	return s;
}

void rainbowColor(unsigned int i, unsigned int iMax, unsigned char &r, unsigned char &g, unsigned char &b){
	unsigned int x(5);
	unsigned int cycle(x*(1 + (iMax/x)));
	double arg(2.0*acos(-1.0)*double(i/x + (cycle/x)*(i%x))/double(cycle));
	r = (unsigned char)floor(cos(arg)*127 + 128);
	g = (unsigned char)floor(cos(arg + 2.0*acos(-1.0)/3.0)*127 + 128);
	b = (unsigned char)floor(cos(arg + 4.0*acos(-1.0)/3.0)*127 + 128);
}

unsigned int absDiff(unsigned int x, unsigned int y){
	if(x > y)
		return x - y;
	return y - x;
}

string makeStringPos(const vector<unsigned int> &x, const vector<unsigned int> &y, const vector<unsigned int> &z){
	if(x.size() == 0)
		return "[]";
	stringstream strstr;
	strstr << "[(" << x[0] << "," << y[0] << "," << z[0] << ")";
	for(unsigned int i(1); i < x.size(); i++){
		strstr << "; (" << x[i] << "," << y[i] << "," << z[i] << ")";
	}
	strstr << "]";
	return strstr.str();
}

string makeStringOneLine(map<unsigned int, unsigned int> &m){
	stringstream strstr;
	for(map<unsigned int, unsigned int>::iterator it(m.begin()); it != m.end(); it++)
		strstr << makeString(it->first) << "(" << makeString(it->second) << ") ";
	return strstr.str();
}