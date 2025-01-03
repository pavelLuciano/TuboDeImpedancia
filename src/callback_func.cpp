#include "callback_func.h"

//usaremos esta principalmente para el reconocimiento de dispositivos
int simpleSetereoCallback(
    const void* inputBuffer, void* outputBuffer, unsigned long framesPerBuffer,
    const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags,
    void* userData
)
{
    float* in = (float*)inputBuffer;
    (void)outputBuffer;

    SimpleStereoBuffer *data = (SimpleStereoBuffer*) userData;

    data->copyBufferData(in, framesPerBuffer);
    for(int i = 0; i < framesPerBuffer*2; i+=2)
    {
        printf("\r%f , %f", in[i], in[i+1]);
    }
    return paContinue;
}

int stereoVolumeCallback(
    const void* inputBuffer, void* outputBuffer, unsigned long framesPerBuffer,
    const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags,
    void* userData
){
    float* in = (float*)inputBuffer;
    (void)outputBuffer;
    StereoVolumeStruct* data = (StereoVolumeStruct*) userData;

    aumimath::stereoDB(in, framesPerBuffer, data);

    return paContinue;
}

