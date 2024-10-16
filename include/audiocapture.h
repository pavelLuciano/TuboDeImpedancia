//PORTAUDIO
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <stdlib.h> //libreria de c
#include <stdio.h> //libreria de c
#include <cstring>
#include <cmath>
#include "portaudio.h"

typedef struct
{
    std::ofstream* file;
    std::ofstream* binFile;
} CallbackData;

static inline float max(float a, float b) {
    return a > b ? a : b;
}

static inline void checkErr(PaError err)
{
    if (err != paNoError)
    {
        std::cout << "ERROR: " << Pa_GetErrorText(err) << std::endl;
        exit(EXIT_FAILURE);
    }      
}


void writeToFile(std::ofstream &file, int value, int size)
{
    file.write(reinterpret_cast<const char*>(&value), size);
}


static int paTestCallback(
    const void* inputBuffer, void* outputBuffer, unsigned long framesPerBuffer,
    const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags,
    void* userData
){
    float* in = (float*)inputBuffer;
    (void)outputBuffer;

    int dispSize = 100;
    printf("\r");

    //un canal
    CallbackData *data = (CallbackData*) userData;
    float vol = 0.0f;
    int sample = 0;
    const int maxAmplitude = pow(2, 16 - 1) - 1; 
    for (unsigned long i = 0; i < framesPerBuffer; i++)
    {
        vol = max(vol,std::abs(in[i]));
        *(data->file) << in[i] << "\n";

        sample = static_cast<int> (maxAmplitude * in[i]);
        (*(data->binFile)).write(reinterpret_cast<char*> (&sample), 2); 
    }

    for (int i = 0; i < dispSize; i++) {
        float barProportion = i / (float)dispSize;
        if (barProportion <= vol) printf("█");
        else printf(" ");  
    }

    fflush(stdout);
    return 0;
}
