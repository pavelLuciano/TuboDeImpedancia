#include "HcCalibrator.h"
#include "aumi_math.h"

HcCalibrator::HcCalibrator()
{
    bufferSize = 512; //Tamano de la nuestra recibida
    complexBufferSize = bufferSize % 2 == 0? bufferSize : bufferSize + 1;
    data = NULL;
    Hc = new float[complexBufferSize];

    memset(&inputParameters, 0, sizeof(inputParameters));
    memset(&outputParameters, 0, sizeof(outputParameters));
}
HcCalibrator::HcCalibrator(int device): AudioHandler(device)
{
    HcCalibrator();
}
HcCalibrator::~HcCalibrator()
{
    delete[] H_21;
    delete[] H_12;
    delete[] Hc;
}
float* HcCalibrator::crossSpectrum_ab()
{
    SimpleStereoBuffer* _data = (SimpleStereoBuffer*) data;
    float* Sa = aumimath::dft_complex_f(_data->l_data, _data->lenght);
    float* Sb = aumimath::dft_complex_f(_data->r_data, _data->lenght);
    float* S_ab = new float[complexBufferSize];

    float mag;
    for (int i = 0; i < complexBufferSize; i+=2)
    {
        mag = std::sqrt(Sb[i]*Sb[i] + Sb[i+1] + Sb[i+1]);
        S_ab[i] = Sa[i] * mag;
        S_ab[i+1] = Sa[i+1] * mag;
    }
    delete[] Sa;
    delete[] Sb;
    return S_ab;
}
float* HcCalibrator::autoSpectrum()
{
    SimpleStereoBuffer* _data = (SimpleStereoBuffer*) data;
    float* Sa = aumimath::dft_complex_f(_data->l_data, _data->lenght);
    float* S_aa = new float[complexBufferSize];
    for (int i = 0; i < complexBufferSize; i+=2)
    {
        S_aa[i] =std::sqrt(Sa[i] * Sa[i] + Sa[i+1] * Sa[i+1]);
    }
    delete[] Sa;
    return S_aa;
}
bool HcCalibrator::calculate_Hab(float* Hab)
{
    if (data == nullptr) return false;
    float* S_ab = crossSpectrum_ab();
    float* S_aa = autoSpectrum();

    for (int i = 0; i < complexBufferSize/2; i++)
    {
        Hab[2*i] = S_ab[2*i]/S_aa[i];
        Hab[2*i+1] = S_ab[2*i+1]/S_aa[i];
    }
    delete[] S_aa;
    delete[] S_ab;
    return true;
}
bool HcCalibrator::calculate_Hc()
{
    if (H_12 == nullptr || H_21 == nullptr) return false;
    for(int i = 0; i < bufferSize/2 + 1; i++)
    {
        Hc[i] = std::sqrt(H_12[i] * H_21[i]);
    }
    return true;
}
bool HcCalibrator::setParameters()
{
    inputParameters.device = device_idx;
    inputParameters.channelCount = Pa_GetDeviceInfo(device_idx)->maxInputChannels > 2? 2 : Pa_GetDeviceInfo(device_idx)->maxInputChannels;
    inputParameters.hostApiSpecificStreamInfo = NULL;
    inputParameters.sampleFormat = paFloat32;
    inputParameters.suggestedLatency = Pa_GetDeviceInfo(device_idx)->defaultLowInputLatency;

    outputParameters.device = device_idx;
    outputParameters.channelCount = Pa_GetDeviceInfo(device_idx)->maxOutputChannels;
    outputParameters.hostApiSpecificStreamInfo = NULL;
    outputParameters.sampleFormat = paFloat32;
    outputParameters.suggestedLatency = Pa_GetDeviceInfo(device_idx)->defaultLowOutputLatency;

    return true;
}
bool HcCalibrator::initStream()
{
    if (isStreamActive() || device_idx == NO_DEVICE) return false;
    if (!isStreamOpen())
    {
        data = new SimpleStereoBuffer(bufferSize);
        setParameters();
        checkErr(
            Pa_OpenStream(
                &stream, 
                &inputParameters, 
                &outputParameters, 
                Pa_GetDeviceInfo(device_idx) -> defaultSampleRate,
                framesPerBuffer,
                paNoFlag,
                stereoVolumeCallback,
                (void*)data
        ));   
    }
    checkErr(Pa_StartStream(stream));
    return true;
}
bool HcCalibrator::initStream(int device)
{
    if (device == device_idx) return initStream();
    setDevice(device);
    closeStream();
    return initStream();
}
bool HcCalibrator::closeStream()
{
    if(isStreamActive()) 
        checkErr(Pa_StopStream(stream));

    if (isStreamOpen())
    {
        checkErr(Pa_CloseStream(stream));
        stream = NULL; // == nullptr; ??
        delete[] data;
        return true;
    }
    return false;
}
bool HcCalibrator::isStreamActive() const
{
    if (stream == NULL) return false;
    return Pa_IsStreamActive(stream) == 1;
}
bool HcCalibrator::isStreamOpen() const
{
    return stream != NULL;
}
bool HcCalibrator::checkErr(PaError err) const
{
    if (err != paNoError)
    {
        std::cout << "ERROR: " << Pa_GetErrorText(err) << std::endl;
        return false;
    }
    return true;
}
