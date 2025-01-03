#ifndef _SIGNAL_CALIBRADOR_H
#define _SIGNAL_CALIBRADOR_H

#include <portaudio.h>
#include "AudioHandler.h"

class SignalCalibrator: public AudioHandler
{
public:
    SignalCalibrator();
    SignalCalibrator(int device);
    ~SignalCalibrator();

    bool setParameters() override;
    bool initStream() override;
    bool initStream(int device) override;
    bool closeStream() override;
    bool isStreamActive() const override;
    bool isStreamOpen() const override;
    bool checkErr(PaError err) const override;
};
#endif