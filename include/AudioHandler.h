#ifndef _AUDIO_HANDLER_H
#define _AUDIO_HANDLER_H
#include <portaudio.h>
#include "Structs.h"
#include "callback_func.h"
#include "string.h" //memset

#define NO_DEVICE -1
#define BUFFER_SIZE 512
#define FRAMES_PER_BUFFER 512

class AudioHandler
{
public:
    char* deviceName;
    int framesPerBuffer;
    int device_idx = NO_DEVICE;
    PaStream* stream = NULL;
    DataStruct* data;
    PaStreamParameters inputParameters;
    PaStreamParameters outputParameters;

public:
    
    AudioHandler(){};
    AudioHandler(int device)
    {
        setDevice(device);
    }
    virtual ~AudioHandler()
    {
        std::cout << "AudioHandler destructor ejecutado" << std::endl;
        delete data;
    }

    virtual bool setParameters()         = 0;
    virtual bool initStream()            = 0;
    virtual bool initStream(int device)  = 0;
    virtual bool closeStream()           = 0;
    virtual bool isStreamActive() const  = 0;
    virtual bool isStreamOpen() const    = 0;
    virtual bool checkErr(PaError) const = 0;
    bool setDevice(int device)
    {
        if (device < 0) return false;
        if (device >= Pa_GetDeviceCount()) return false;
        device_idx = device;
        return true;
    }
    int getDevice()
    {
        return device_idx;
    }
//public?
//esto no funciona por la manera en que los metodos funcionan
//esta funciona tecinamente es "int (AudioHandler::*)" y eso no es aceptable por portaudio
//ya que las funciones de callback deben ser "int (*)"
//implementaremos las funciones de callback en un archivo aparte 
    /*int paCallbackFunction(
        const void* inputBuffer, void* outputBuffer, unsigned long framesPerBuffer,
        const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags,
        void* userData
    );*/
    
};
#endif