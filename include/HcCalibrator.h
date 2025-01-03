#ifndef _HC_CALIBRATOR_H
#define _HC_CALIBRATOR_H

#include "AudioHandler.h"
#include <math.h>

class HcCalibrator: public AudioHandler
{
private:
    int promedios;
    int bufferSize;
    int complexBufferSize;
    float* H_12;
    float* H_21;
    float* Hc;
    //auxiliares
    float* buffer;
    float* complexBuffer;
    
public:
    HcCalibrator();
    HcCalibrator(int);
    ~HcCalibrator();

    void setPromedios();
    float* crossSpectrum_ab();
    float* autoSpectrum();
    bool calculate_Hab(float* Hab);
    bool calculate_Hc();

    bool setParameters() override;
    bool initStream() override;
    bool initStream(int device) override;
    bool closeStream() override;
    bool isStreamActive() const override;
    bool isStreamOpen() const override;
    bool checkErr(PaError err) const override;
};

#endif