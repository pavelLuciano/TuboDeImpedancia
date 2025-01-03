#ifndef _BASIC_AUDIO_HANDLER_H
#define _BASIC_AUDIO_HANDLER_H

#include "AudioHandler.h"



class BasicAudioHandler : public AudioHandler
{
public:
    BasicAudioHandler();
    BasicAudioHandler(int device);
    ~BasicAudioHandler();

    bool setParameters() override;
    bool initStream() override;
    bool initStream(int device) override;
    bool closeStream() override;
    bool isStreamActive() const override;
    bool isStreamOpen() const override;
    bool checkErr(PaError err) const override;

    float* lData();
    float* rData();
    int dataLen();
};

#endif