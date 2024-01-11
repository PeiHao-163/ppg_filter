/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/

#include <stdio.h>
#include <math.h>
#include <assert.h>
#define PI 3.14159265358979323846264338327950288
#define LENGTH 1024

typedef struct
 {
        float real;
        float imag;
 } complex;
    
static complex _complex_v[LENGTH];
static complex _complex_dummy[LENGTH];

float getreal(complex w){
        return w.real;
}

float getimag(complex w){
        return w.imag;
    }


void _fft(complex *v, size_t n, complex *dummy) {
      size_t k;
      complex *ve = 0;
      complex *vo = 0;

      size_t m;
      complex w;
      complex z;

      if (n == 1) {
        return;
      }

      ve = dummy;
      vo = dummy + (n / 2);

      for (k = 0; k < n / 2  ; k++) {
        ve[k].real = v[2 * k].real;
        ve[k].imag = v[2*k].imag;
        vo[k].real = v[2 * k + 1].real;
        vo[k].imag = v[2 * k + 1].imag;
      }

      _fft(ve, n / 2, v);
      _fft(vo, n / 2, v);

      

      for (m = 0; m < n / 2; m++) {
        w.real = cos(2 * PI * m / (float)n);
        w.imag = -sin(2 * PI * m / (float)n);
          
        z.real = (getreal(w) * getreal(vo[m]) - getimag(w) * getimag(vo[m]));
        z.imag = (getreal(w) * getimag(vo[m]) + getimag(w) * getreal(vo[m]));

          
        v[m].real = (getreal(ve[m]) + getreal(z) );
        v[m].imag = (getimag(ve[m]) + getimag(z));
        v[m + n / 2].real  = (getreal(ve[m]) - getreal(z));
        v[m + n/2].imag = (getimag(ve[m]) - getimag(z));
      }
    }

void fft(complex *v, size_t n) {
      assert(n <= LENGTH);

      _fft(v, n, _complex_dummy);
}


    void fft_abs(float *v, size_t n) {
      size_t i;

      for (i = 0; i < n; i++) {
          _complex_v[i].real = v[i] ;
          _complex_v[i].imag = 0;
      }

      fft(_complex_v, n);

      for (i = 0; i < n; i++) {
        v[i] = sqrt(getreal(_complex_v[i]) * getreal(_complex_v[i]) +
               getimag(_complex_v[i]) * getimag(_complex_v[i]));
          
      }
    }

    void _ifft(complex *v, size_t n, complex *dummy) {
      size_t k;
      complex *ve = dummy;
      complex *vo = dummy + (n / 2);

      size_t m;
      complex w;
      complex ww;
      complex z;

      if (n == 0) {
        return;
      }

      

      for (k = 0; k < n / 2; k++) {
        ve[k].imag = v[2 * k].imag;
        ve[k].real = v[2 * k].imag;
        vo[k].imag = v[2 * k + 1].imag;
        vo[k].real = v[2 * k + 1].real;
      }

      _ifft(ve, n / 2, v);
      _ifft(vo, n / 2, v);

      

      for (m = 0; m < n / 2; m++) {
        w.real = (cos(2 * PI * m / (float)n));
        w.imag = 0.0;
        ww.imag = (sin(2 * PI * m / (float)n));
        ww.real = 0.0;
        z.real = (getreal(w) * getreal(vo[m]) - getimag(ww) * getimag(vo[m]));
        z.imag =  (getreal(ww) * getimag(vo[m]) + getimag(w) * getreal(vo[m]));
        v[m].real = (getreal(ve[m]) + getreal(z));
        v[m].imag = (getimag(ve[m]) + getimag(z));
        v[m + n / 2].real = (getreal(ve[m]) - getreal(z));
        v[m + n/2].imag = (getimag(ve[m]) - getimag(z));
      }
    }

    void ifft(complex *v, size_t n) {
      size_t i;
      _ifft(v, n, _complex_dummy);
      for (i = 0; i < n; i++) {
        v[i].real /= n;
        v[i].imag /= n;
      }
    }

    void ifft_abs(float *v, size_t n) {
      size_t i;
      for (i = 0; i < n; i++) {
          _complex_v[i].real = v[i];
          _complex_v[i].imag = 0.0;
      }

      ifft(_complex_v, n);

      for (i = 0; i < n; i++) {
          v[i] = sqrt(getreal(_complex_v[i]) * getreal(_complex_v[i]));
          v[i] = v[i]*2 - sqrt(getimag(_complex_v[i]) * getimag(_complex_v[i]));
      }
    }

    void fftfreq(float *output, size_t n, float d) {
      size_t i;
      size_t m = (n - 1) / 2 + 1;
      float val = 1.0 / ((float)n * d);

      for (i = 0; i < m; i++) {
        output[i] = (float)i * val;
      }

      for (i = m; i < n; i++) {
        output[i] = -((float)n - (float)i) * val;
      }
    }


/* [] END OF FILE */
