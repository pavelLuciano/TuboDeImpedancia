#pragma once
#include "portaudio.h"
#include <fftw3.h>
#include <vector>
#include <cmath>
#include <fstream>

namespace tdi
{
    //Normalizar
    void normalize(float **signal, int size)
    {
        
    }
    //////////////////////////
    //NIVELACION DE DECIBELIOS
    //////////////////////////
    static inline float max(double a, double b) {
        return a > b ? a : b;
    }
    // Función para calcular el valor RMS
    float calcularRMS(const float* samples, int size) {
        float sumOfSquares = 0.0;
        for (int i = 0; i < size; i++)
            sumOfSquares += samples[i] * samples[i];
        
        return std::sqrt(sumOfSquares / size); // Calcular RMS
    }

    // Función para convertir RMS a dB SPL
    double rmsADecibelios(double rms/*, double pres_ref = 20e-6*/) { //quitamos la presion de referencia
        return 10 * std::log10(max(rms,  1e-3)/1e-3); // Convertir RMS a dB mV
        //return 20 * std::log10(rms / pres_ref);
    }
    /////////////////////////////////
    //CAPTURA DE AUDIO
    /////////////////////////////////
    struct CalibrationData
    {
        int n; //cantidad de promedios
        int n_count; //cantidad de promedios actuales
        
        std::vector<float> paraWAV;

        std::ofstream* l_signal;
        std::ofstream* r_signal;

        float* signal_a;
        float* signal_b;
        int fs;
        int fs_offset; //estos valores ayudaran a saber cuando están llenos los arreglos
        //ADVERTENCIA: COMO AMBOS ARREGLOS USAN EL MISMO OFFSET, DEBEN LLENARSE AL MISMO TIEMPO.

        CalibrationData(int n, int fs)
        {
            //nota: los tamaños deberias ser fs*t pero como estamos tomando muestras de un segundo no hace falta multiplicarlo.
            this->n = n;
            this->fs = fs;
            this->fs_offset = 0;
            this->n_count = 0;
            signal_a = new float[fs];
            signal_b = new float[fs];
            for(int i = 0; i < fs; i++)
            {
                signal_a[i] = 0;
                signal_b[i] = 0;
            }
        }
        ~CalibrationData()
        {
            delete[] signal_a;
            delete[] signal_b;
            l_signal->close();
            r_signal->close();
        }

        //ya no sirve si se calcula la media inmediatamente (ver abajo)
        void calculateAvg()
        {
            for (int i = 0; i < fs; i++)
            {
                signal_a[i] /= n;
                signal_b[i] /= n;
            }
        }
        void resetCount()
        {
            n_count++;
            fs_offset = 0;

        }
        bool addData(float amplitude_a, float amplitude_b)
        {
            if (n == n_count) return false;
            //con esto irá calculando la media inmediantamente
            this->signal_a[fs_offset] = (this->signal_a[fs_offset]*n_count + amplitude_a)/(n_count+1);
            this->signal_b[fs_offset] = (this->signal_b[fs_offset]*n_count + amplitude_b)/(n_count+1); 
            fs_offset++;
            if (fs == fs_offset) resetCount();
            //if (n == n_count) calculateAvg();
            return true;
        }
    };

    static int paCalibrationCallback(
        const void* inputBuffer, void* outputBuffer, unsigned long framesPerBuffer,
        const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags,
        void* userData
    ){
        CalibrationData* calibrationData = static_cast<CalibrationData*>(userData);
        float* in = (float*)inputBuffer;
        (void)outputBuffer;
        (void)timeInfo; //el tiempo no es relevante ya que lo mediremos en base a la frecuencia de muestreo fs



         tdi::CalibrationData* data = (tdi::CalibrationData*) userData;
        

        for (unsigned long i = 0; i < framesPerBuffer*2 ; i += 2) 
        {
            (*calibrationData).paraWAV.push_back(in[i]);
            *(data->l_signal) << in[i] << "\n";
            *(data->r_signal) << in[i+1] << "\n";
            if (!calibrationData->addData(in[i], in[i+1])) return paComplete;
        }
        return paContinue;
    }

    static inline void checkErr(PaError err)
    {
        if (err != paNoError)
        {
            std::cout << "ERROR: " << Pa_GetErrorText(err) << std::endl;
            exit(EXIT_FAILURE);
        }      
    }

    void initAudioCaptureForCalibration(PaStream** stream, CalibrationData* userData, unsigned int device)
    {
        PaError err;
        std::cout << "abriendo Stream\n";
        const unsigned int SAMPLE_RATE = Pa_GetDeviceInfo(device)->defaultSampleRate;
        const unsigned int FRAMES_PER_BUFFER = 512;
        PaStreamParameters inputParameters;
        PaStreamParameters outputParameters;
        memset(&inputParameters, 0, sizeof(inputParameters));
        inputParameters.device = device;
        inputParameters.channelCount = 2;
        inputParameters.hostApiSpecificStreamInfo = NULL;
        inputParameters.sampleFormat = paFloat32;
        inputParameters.suggestedLatency = Pa_GetDeviceInfo(device)->defaultLowInputLatency;
        
        memset(&outputParameters, 0, sizeof(outputParameters));
        outputParameters.device = device;
        outputParameters.channelCount = 4;
        outputParameters.hostApiSpecificStreamInfo = NULL;
        outputParameters.sampleFormat = paFloat32;
        outputParameters.suggestedLatency = Pa_GetDeviceInfo(device)->defaultLowOutputLatency;
        err = Pa_OpenStream(
            stream,
            &inputParameters,
            &outputParameters,
            SAMPLE_RATE,
            FRAMES_PER_BUFFER,
            paNoFlag,
            paCalibrationCallback,
            (void*)userData
        );
        checkErr(err);
        err = Pa_StartStream(*stream);
        checkErr(err);
    }

    void closeAudioCaptureForCalibration(PaStream** stream)
    {
        PaError err;
        err = Pa_StopStream(*stream);
        checkErr(err);
        err = Pa_CloseStream(*stream);
        checkErr(err);
    }
    //////////////////////////
    //TRANSFORMADA DE FOURIER
    ///////////////////////////
    void fftFrecuencySpectrum(float** magn, const float* signal, int size)
    {
        fftwf_complex* in = new fftwf_complex[size];
        fftwf_complex* out = new fftwf_complex[size];
        float* arr = new float[size/2];
        //frec = new float[size/2];

        for (int i = 0; i < size; i++)
        {float* arr = new float[size/2];
            in[i][0] = signal[i];
            in[i][1] = 0.0f;
        }

        // Crear el plan para la FFT
        fftwf_plan plan = fftwf_plan_dft_1d(size, in, out, FFTW_FORWARD, FFTW_ESTIMATE);

        // Ejecutar la FFT
        fftwf_execute(plan);

        for (int i = 0; i<size/2; i++)
        {
            arr[i] = std::sqrt(
                pow(out[i][0], 2) + 
                pow(out[i][1], 2)
            );
            //frec[i] = i
            //habia guardado otro array para frecuencias pero su formula era redundante ya que  la frecuencia esta en el indice
            //pues la razon entre el tamaño de los arreglos y el sample rate es 1 (en este caso particular)
        }
        (*magn) = arr;
        delete[] in;
        delete[] out;
        fftwf_destroy_plan(plan);
        //fftwf_free(in);
        //fftwf_free(out);
    }

    //esta funcion esta construida de tal manera que no se ejecuta mas de una vez sobre un array
    bool HFunction(float* signal_1, float* signal_2, float **H_12, int size)
    {
        if (*H_12 != nullptr) return false;
        float* ft_1;
        float* ft_2;
        tdi::fftFrecuencySpectrum(&ft_1, signal_1, size);
        tdi::fftFrecuencySpectrum(&ft_2, signal_2, size);

        float *arr = new float[size/2];
        std::cout << "Se calculará H_12\n";
        for (int i = 0; i<size/2 ; i++)
        {
            arr[i] =  ft_2[i] / ft_1[i];
        }
        *H_12 = arr;
        std::cout  << "Se calculó H_12\n";
        delete[] ft_1;
        delete[] ft_2;
        return true;
    }
    bool HcFunction(const float *HI, const float *HII, float **Hc, int size)
    {
        if (*Hc != nullptr) return false;
        float *arr = new float[size];
        for (int i = 0; i < size; i++)
        {
            arr[i] =  std::sqrt(HI[i]*HII[i]);
        }
        *Hc = arr;
        return true;
    }
}

