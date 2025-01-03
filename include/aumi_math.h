#ifndef _AUMI_MATH_H
#define _AUMI_MATH_H
#include <fftw3.h>
#include <math.h>
#include "Structs.h"
namespace aumimath
{
    static inline float max(double a, double b) {
        return a > b ? a : b;
    }
    //calclua la fft discreta y devuelve el vector de magnitud
    inline float* dft_mag_f(float sample[], int len)
    {

        fftwf_complex* out = fftwf_alloc_complex(len / 2 + 1);
        fftwf_plan plan = fftwf_plan_dft_r2c_1d(len, sample, out, FFTW_ESTIMATE);
        fftwf_execute(plan);

        float* mag = new float[len / 2 + 1];

        for (int i = 0; i < len / 2 + 1; i++)
            mag[i] = std::sqrt(out[i][0] * out[i][0] + out[i][1] * out[i][1]);

        fftwf_destroy_plan(plan);
        fftwf_free(out);

        return mag;
    }

    inline float* dft_complex_f(float sample[], int len)
    {
        int complex_len = len % 2 == 0 ? len : len + 1;
        fftwf_complex* out = fftwf_alloc_complex(complex_len / 2);
        fftwf_plan plan = fftwf_plan_dft_r2c_1d(len, sample, out, FFTW_ESTIMATE);
        fftwf_execute(plan);

        //revisar
        
        float* complex = new float[complex_len];

        for (int i = 0; i < complex_len / 2; i++)
        {
            complex[2*i] = out[i][0];
            complex[2*i + 1] = out[i][1];
        }

        fftwf_destroy_plan(plan);
        fftwf_free(out);
        return complex;
    }

    inline float rms(float* arr, int len)
    {
        float rms = 0.0f;
        for(int i = 0; i < len; i++)
        {
            rms += arr[i] * arr[i];
        }
        return std::sqrt(rms / len);
    }

    inline void stereoDB(float* buff, int fpb, StereoVolumeStruct* data)
    {
        float rms1 = 0.0f, rms2 = 0.0f;
        for(int i = 0; i < fpb*2; i+=2)
        {
            rms1 += buff[i] * buff[i];
            rms2 += buff[i+1] * buff[i+1];
        }
        rms1 = std::sqrt(rms1 / fpb);
        rms2 = std::sqrt(rms2 / fpb);

        
        data->setVolumes(
            20 * std::log10(max(rms1, 1e-6)/1e-3),
            20 * std::log10(max(rms2, 1e-6)/1e-3)
            /*
            rms1 > 0.0f? 20.0f * std::log10(rms1) : -100.0f,
            rms2 > 0.0f? 20.0f * std::log10(rms2) : -100.0f
            */
        );
    }
}

#endif