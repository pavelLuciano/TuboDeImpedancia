#include "BasicAudioHandler.h"

BasicAudioHandler::BasicAudioHandler()
{
    framesPerBuffer = 512;
    data = new SimpleStereoBuffer(512);
    //habra otra forma?
    memset(&inputParameters, 0, sizeof(inputParameters));
    memset(&outputParameters, 0, sizeof(outputParameters));
}
BasicAudioHandler::BasicAudioHandler(int device): AudioHandler(device)
{
    BasicAudioHandler();
}
BasicAudioHandler::~BasicAudioHandler()
{
    closeStream();
    std::cout<<"SELECTOR DE DISPOSITIVO: destruyendo..."<<std::endl;
}

bool BasicAudioHandler::setParameters()
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
//Inicia el stream 
bool BasicAudioHandler::initStream()
{
    if (isStreamActive()) return false;
    if (!isStreamOpen())
    {
        std::cout<<"abrimos stream"<< std::endl;
        setParameters();
        checkErr(
            Pa_OpenStream(
                &stream, 
                &inputParameters, 
                &outputParameters, 
                Pa_GetDeviceInfo(device_idx) -> defaultSampleRate,
                framesPerBuffer,
                paNoFlag,
                simpleSetereoCallback,
                (void*)data
        ));   
    }
    checkErr(Pa_StartStream(stream));
    return true;
}

bool BasicAudioHandler::initStream(int device)
{
    if (device == device_idx) return initStream();
    setDevice(device);
    closeStream();
    return initStream();  
}
bool BasicAudioHandler::closeStream()
{
    std::cout << "Cerramos Stream" << std::endl;
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
bool BasicAudioHandler::isStreamActive() const
{
    if (stream == NULL) return false;
    return Pa_IsStreamActive(stream) == 1;
}
bool BasicAudioHandler::isStreamOpen() const
{
    return stream != NULL;
}
bool BasicAudioHandler::checkErr(PaError err) const
{
    if (err != paNoError)
    {
        std::cout << "ERROR: " << Pa_GetErrorText(err) << std::endl;
        return true;
    }
    return false;
}

//no estoy muy seguro de esto pq estoy forzando a ser una instancia de SimpleStereoBuffer
//aunque solo por el constructor sabemos que esto siempre deberia ser cierto
float* BasicAudioHandler::lData()
{
    return dynamic_cast<SimpleStereoBuffer*>(data)->l_data;
}
float* BasicAudioHandler::rData()
{
    return dynamic_cast<SimpleStereoBuffer*>(data)->r_data;
}
int BasicAudioHandler::dataLen()
{
    return dynamic_cast<SimpleStereoBuffer*>(data)->lenght;
}
