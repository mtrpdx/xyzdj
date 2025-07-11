#ifndef _fft_h
#define _fft_h

#include <stdio.h>
#include <math.h>
#include <complex.h>

typedef double complex cplx;
const double PI = atan2(1, 1) * 4;

void _fft(cplx buf[], cplx out[], int n, int step);
void _mag(cplx buf[], float out[], int n);
void *fft(cplx *buf, int n);
void *fft_mag(cplx *buf, float *out, int n);

#endif
