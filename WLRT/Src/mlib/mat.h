#pragma once

#include <stdint.h>
#include <stddef.h>

#define mmax(A, B) _Generic((A),                                  \
		char:               mmaxc(A,   (char) (B)),               \
		signed char:        mmaxsc(A,  (signed char) (B)),        \
		unsigned char:      mmaxuc(A,  (unsigned char) (B)),      \
		short:              mmaxs(A,   (short) (B)),              \
		unsigned short:     mmaxus(A,  (unsigned short) (B)),     \
		int:                mmaxi(A,   (int) (B)),                \
		unsigned int:       mmaxui(A,  (unsigned int) (B)),       \
		long:               mmaxl(A,   (long) (B)),               \
		unsigned long:      mmaxul(A,  (unsigned long) (B)),      \
		long long:          mmaxll(A,  (long long) (B)),          \
		unsigned long long: mmaxull(A, (unsigned long long) (B)), \
		float:              mmaxf(A,   (float) (B)),              \
		double:             mmaxd(A,   (double) (B)),             \
		long double:        mmaxld(A,  (long double) (B)))
#define mmin(A, B) _Generic((A),                                  \
		char:               mminc(A,   (char) (B)),               \
		signed char:        mminsc(A,  (signed char) (B)),        \
		unsigned char:      mminuc(A,  (unsigned char) (B)),      \
		short:              mmins(A,   (short) (B)),              \
		unsigned short:     mminus(A,  (unsigned short) (B)),     \
		int:                mmini(A,   (int) (B)),                \
		unsigned int:       mminui(A,  (unsigned int) (B)),       \
		long:               mminl(A,   (long) (B)),               \
		unsigned long:      mminul(A,  (unsigned long) (B)),      \
		long long:          mminll(A,  (long long) (B)),          \
		unsigned long long: mminull(A, (unsigned long long) (B)), \
		float:              mminf(A,   (float) (B)),              \
		double:             mmind(A,   (double) (B)),             \
		long double:        mminld(A,  (long double) (B)))

char               mmaxc(char a, char b);
signed char        mmaxsc(signed char a, signed char b);
unsigned char      mmaxuc(unsigned char a, unsigned char b);
short              mmaxs(short a, short b);
unsigned short     mmaxus(unsigned short a, unsigned short b);
int                mmaxi(int a, int b);
unsigned int       mmaxui(unsigned int a, unsigned int b);
long               mmaxl(long a, long b);
unsigned long      mmaxul(unsigned long a, unsigned long b);
long long          mmaxll(long long a, long long b);
unsigned long long mmaxull(unsigned long long a, unsigned long long b);
float              mmaxf(float a, float b);
double             mmaxd(double a, double b);
long double        mmaxld(long double a, long double b);

char               mminc(char a, char b);
signed char        mminsc(signed char a, signed char b);
unsigned char      mminuc(unsigned char a, unsigned char b);
short              mmins(short a, short b);
unsigned short     mminus(unsigned short a, unsigned short b);
int                mmini(int a, int b);
unsigned int       mminui(unsigned int a, unsigned int b);
long               mminl(long a, long b);
unsigned long      mminul(unsigned long a, unsigned long b);
long long          mminll(long long a, long long b);
unsigned long long mminull(unsigned long long a, unsigned long long b);
float              mminf(float a, float b);
double             mmind(double a, double b);
long double        mminld(long double a, long double b);
