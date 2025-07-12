#include <stdio.h>
#include <math.h>
#include <complex.h>
#include <stdlib.h>
#include "fft.h"

const double PI = atan2(1, 1) * 4;

void _fft(cplx buf[], cplx out[], int n, int step)
{
	if (step < n) {
		_fft(out, buf, n, step * 2);
		_fft(out + step, buf + step, n, step * 2);
 
		for (int i = 0; i < n; i += 2 * step) {
			cplx t = cexp(-I * PI * i / n) * out[i + step];
			buf[i / 2]     = out[i] + t;
			buf[(i + n)/2] = out[i] - t;
		}
	}
}

void _mag(cplx buf[], float out[], int n) {
    float re;
    float im;
    float mag;

    for (int i = 0; i < n; i++) {
        re = creal(buf[i]);
        im = cimag(buf[i]);
        mag = sqrt((re * re) + (im * im));
        out[i] = mag;
    }
}

void *fft(cplx buf[], int n)
{
	cplx *out = malloc(n * sizeof(cplx));
    if (!out) {
        return NULL;
    }
	for (int i = 0; i < n; i++) out[i] = buf[i];
 
	_fft(buf, out, n, 1);

    // determine if log2(n) is even or odd to determine the output buffer
    int levels = 0;
    for (int i = n; i > 1; i >>= 1) {
        levels++;
    }

    // if log2(n) is even, the result is in the out buffer, so we need to copy it
    if ((levels % 2) == 0) {
        for (int i = 0; i < n; i++) {
            buf[i] = out[i];
        }
    }

    free(out);
	return buf;
}

void *fft_mag(cplx *buf, float *out, int n)
{
	cplx *temp = malloc(n * sizeof(cplx));
    if (!temp) {
        return NULL;
    }
	for (int i = 0; i < n; i++) temp[i] = buf[i];
 
	_fft(buf, temp, n, 1);

    // determine if log2(n) is even or odd to determine the output buffer
    int levels = 0;
    for (int i = n; i > 1; i >>= 1) {
        levels++;
    }

    cplx *result_buf;
    if (levels % 2 != 0) { // odd
        result_buf = buf;
    } else { // even
        result_buf = temp;
    }
 
	_mag(result_buf, out, n);
 
    free(temp);
	return out;
}