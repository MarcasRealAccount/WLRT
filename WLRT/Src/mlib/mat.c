#include "mat.h"

char               mmaxc(char a, char b) { return a > b ? a : b; }
signed char        mmaxsc(signed char a, signed char b) { return a > b ? a : b; }
unsigned char      mmaxuc(unsigned char a, unsigned char b) { return a > b ? a : b; }
short              mmaxs(short a, short b) { return a > b ? a : b; }
unsigned short     mmaxus(unsigned short a, unsigned short b) { return a > b ? a : b; }
int                mmaxi(int a, int b) { return a > b ? a : b; }
unsigned int       mmaxui(unsigned int a, unsigned int b) { return a > b ? a : b; }
long               mmaxl(long a, long b) { return a > b ? a : b; }
unsigned long      mmaxul(unsigned long a, unsigned long b) { return a > b ? a : b; }
long long          mmaxll(long long a, long long b) { return a > b ? a : b; }
unsigned long long mmaxull(unsigned long long a, unsigned long long b) { return a > b ? a : b; }
float              mmaxf(float a, float b) { return a > b ? a : b; }
double             mmaxd(double a, double b) { return a > b ? a : b; }
long double        mmaxld(long double a, long double b) { return a > b ? a : b; }

char               mminc(char a, char b) { return a < b ? a : b; }
signed char        mminsc(signed char a, signed char b) { return a < b ? a : b; }
unsigned char      mminuc(unsigned char a, unsigned char b) { return a < b ? a : b; }
short              mmins(short a, short b) { return a < b ? a : b; }
unsigned short     mminus(unsigned short a, unsigned short b) { return a < b ? a : b; }
int                mmini(int a, int b) { return a < b ? a : b; }
unsigned int       mminui(unsigned int a, unsigned int b) { return a < b ? a : b; }
long               mminl(long a, long b) { return a < b ? a : b; }
unsigned long      mminul(unsigned long a, unsigned long b) { return a < b ? a : b; }
long long          mminll(long long a, long long b) { return a < b ? a : b; }
unsigned long long mminull(unsigned long long a, unsigned long long b) { return a < b ? a : b; }
float              mminf(float a, float b) { return a < b ? a : b; }
double             mmind(double a, double b) { return a < b ? a : b; }
long double        mminld(long double a, long double b) { return a < b ? a : b; }
