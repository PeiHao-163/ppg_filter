#pragma once
#ifndef _FFTNEW_H_
#define _FFTNEW_H_

#define PI 3.14159265358979323846264338327950288
#define LENGTH 1024

typedef struct
{
    float real;
    float imag;
} complex;

static complex _complex_v[LENGTH];
static complex _complex_dummy[LENGTH];

float getreal(complex w);
float getimag(complex w);

void _fft(complex* v, size_t n, complex* dummy);
void fft(complex* v, size_t n);
void fft_abs(float* v, size_t n);

void _ifft(complex* v, size_t n, complex* dummy);
void ifft(complex* v, size_t n);
void ifft_abs(float* v, size_t n);

void fftfreq(float* output, size_t n, float d);

#endif
