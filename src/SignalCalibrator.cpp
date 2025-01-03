#include "SignalCalibrator.h"
SignalCalibrator::SignalCalibrator()
{
    //modifivcar los buffersize
    data = new StereoVolumeStruct();
    framesPerBuffer = 1024;
    //habra otra forma?
    memset(&inputParameters, 0, sizeof(inputParameters));
    memset(&outputParameters, 0, sizeof(outputParameters));
}
SignalCalibrator::SignalCalibrator(int device): AudioHandler(device)
{
    SignalCalibrator();
}
SignalCalibrator::~SignalCalibrator()
{
    std::cout << "CALIBRADOR DE SEÑAL: destruyendo... \n"; 
    closeStream();
}
bool SignalCalibrator::setParameters()
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
bool SignalCalibrator::initStream()
{
    if (isStreamActive() || device_idx == NO_DEVICE) return false;
    if (!isStreamOpen())
    {
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
bool SignalCalibrator::initStream(int device)
{
    if (device == device_idx) return initStream();
    setDevice(device);
    closeStream();
    return initStream();
}
bool SignalCalibrator::closeStream()
{
    if(isStreamActive()) 
        checkErr(Pa_StopStream(stream));

    if (isStreamOpen())
    {
        checkErr(Pa_CloseStream(stream));
        stream = NULL; // == nullptr; ??
        return true;
    }
    return false;
}
bool SignalCalibrator::isStreamActive() const
{
    if (stream == NULL) return false;
    return Pa_IsStreamActive(stream) == 1;
}
bool SignalCalibrator::isStreamOpen() const
{
    return stream != NULL;
}
bool SignalCalibrator::checkErr(PaError err) const
{
    if (err != paNoError)
    {
        std::cout << "ERROR: " << Pa_GetErrorText(err) << std::endl;
        return false;
    }
    return true;
}
