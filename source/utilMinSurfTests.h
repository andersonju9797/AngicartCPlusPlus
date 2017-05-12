#ifndef _UTILMINSURFTESTS
#define _UTILMINSURFTESTS 1

#include <map>
#include <string>
#include <sstream>
#include <vector>

using namespace std;

template <class T>
string makeString(const T x, streamsize prec = 8){
	stringstream strstr;
	strstr.precision(prec);
	strstr << x;
	return strstr.str();
}

template <class T>
string makeString(const vector<T> &x, streamsize prec = 8){
	if(x.size() == 0)
		return "()";
	stringstream strstr;
	strstr.precision(prec);
	strstr << "(" << x[0];
	for(unsigned int i(1); i < x.size(); i++)
		strstr << ", " << x[i];
	strstr << ")";
	return strstr.str();
}

// returns true if x is found in v
template <class T>
bool isIn(const T &x, const vector<T> &v){
	for(unsigned int i(0); i < v.size(); i++){
		if(x == v[i])
			return true;
	}
	return false;
}

// returns true if added
template <class T>
bool pushUnique(T x, vector<T> &v){
	for(unsigned int i(0); i < v.size(); i++){
		if(x == v[i])
			return false;
	}
	v.push_back(x);
	return true;
}

// returns true if something from x added to v
template <class T>
bool pushUniqueSet(const vector<T> &x, vector<T> &v){
	bool pushed(false);
	for(unsigned int i(0); i < x.size(); i++)
		pushed = pushUnique(x[i], v) || pushed;
	return pushed;
}

// removes the first x from v that it finds, if any
template <class T>
void removeFrom(T x, vector<T> &v){
	for(unsigned int i(0); i < v.size(); i++){
		if(x == v[i]){
			v.erase(v.begin() + i);
			return;
		}
	}
}

template <class T>
vector<T> reversed(const vector<T> &v){
	vector<T> r(v);
	for(unsigned int i(0); i < v.size(); i++)
		r[i] = v[v.size() - 1 - i];
	return r;
}

bool isIn(unsigned int x, map<unsigned int, vector<unsigned int> > &m);
string niceTime(double t, bool roundSeconds = false);
string paddedInt(int x, int digits);
void rainbowColor(unsigned int i, unsigned int iMax, unsigned char &r, unsigned char &g, unsigned char &b);
unsigned int absDiff(unsigned int x, unsigned int y);
string makeStringPos(const vector<unsigned int> &x, const vector<unsigned int> &y, const vector<unsigned int> &z);
string makeStringOneLine(map<unsigned int, unsigned int> &m);

#endif