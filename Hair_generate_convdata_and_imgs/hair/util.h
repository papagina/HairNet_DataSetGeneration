
#ifndef UTIL_H
#define UTIL_H

#include <algorithm>
#include <vector>
#include <cmath>
#include <iostream>

#ifdef WIN32
#undef min
#undef max
#endif

using std::min;
using std::max;
using std::swap;

template<class T>
inline T sqr(const T &x)
{ return x*x; }

template<class T>
inline T min(T a1, T a2, T a3)
{ return min(a1, min(a2, a3)); }

template<class T>
inline T min(T a1, T a2, T a3, T a4)
{ return min(min(a1, a2), min(a3, a4)); }

template<class T>
inline T min(T a1, T a2, T a3, T a4, T a5)
{ return min(min(a1, a2), min(a3, a4), a5); }

template<class T>
inline T min(T a1, T a2, T a3, T a4, T a5, T a6)
{ return min(min(a1, a2), min(a3, a4), min(a5, a6)); }

template<class T>
inline T max(T a1, T a2, T a3)
{ return max(a1, max(a2, a3)); }

template<class T>
inline T max(T a1, T a2, T a3, T a4)
{ return max(max(a1, a2), max(a3, a4)); }

template<class T>
inline T max(T a1, T a2, T a3, T a4, T a5)
{ return max(max(a1, a2), max(a3, a4),  a5); }

template<class T>
inline T max(T a1, T a2, T a3, T a4, T a5, T a6)
{ return max(max(a1, a2), max(a3, a4),  max(a5, a6)); }

template<class T>
inline T clamp(T a, T lower, T upper)
{
	if(a<lower) return lower;
	else if(a>upper) return upper;
	else return a;
}

#ifdef WIN32
// there may be some fancy bit-trickery that's faster...
inline long lround(double x)
{
	if(x>0)
		return (x-floor(x)<0.5) ? (long)floor(x) : (long)ceil(x);
	else
		return (x-floor(x)<=0.5) ? (long)floor(x) : (long)ceil(x);
}
#endif //WIN32

inline unsigned int round_up_to_power_of_two(unsigned int n)
{
	int exponent=0;
	--n;
	while(n){
		++exponent;
		n>>=1;
	}
	return 1<<exponent;
}

inline unsigned int round_down_to_power_of_two(unsigned int n)
{
	int exponent=0;
	while(n>1){
		++exponent;
		n>>=1;
	}
	return 1<<exponent;
}

inline int intlog2(int x)
{
	int exp=-1;
	while(x){
		x>>=1;
		++exp;
	}
	return exp;
}

template<class T>
void set_zero(std::vector<T> &v)
{ for(int i=(int)v.size()-1; i>=0; --i) v[i]=0; }

template<class T>
T abs_max(const std::vector<T> &v)
{
	T m=0;
	for(int i=(int)v.size()-1; i>=0; --i){
		if(std::fabs(v[i])>m)
			m=std::fabs(v[i]);
	}
	return m;
}

template<class T>
bool contains(const std::vector<T> &a, T e)
{
	for(unsigned int i=0; i<a.size(); ++i)
		if(a[i]==e) return true;
	return false;
}

#endif // UTIL_H
