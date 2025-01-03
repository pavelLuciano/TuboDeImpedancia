#ifndef _CALLBACK_FUNC
#define _CALLBACK_FUNC

#include "Structs.h"
#include "aumi_math.h"
#include <portaudio.h>

//usaremos esta principalmente para el reconocimiento de dispositivos
int simpleSetereoCallback(
    const void* inputBuffer, void* outputBuffer, unsigned long framesPerBuffer,
    const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags,
    void* userData
);

int stereoVolumeCallback(
    const void* inputBuffer, void* outputBuffer, unsigned long framesPerBuffer,
    const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags,
    void* userData
);


#endif