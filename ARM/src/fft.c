#include <stdio.h>
#include <math.h>
#include <complex.h>
#include "fft.h"

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
    cplx out[n];
    for (int i = 0; i < n; i++) out[i] = buf[i];

    _fft(buf, out, n, 1);
}

void *fft_mag(cplx *buf, float *out, int n)
{
    cplx temp[n];
    for (int i = 0; i < n; i++) temp[i] = buf[i];

    _fft(buf, temp, n, 1);
    _mag(temp, out, n);

    return out;
}
