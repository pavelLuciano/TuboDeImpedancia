//PORTAUDIO
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <stdlib.h> //libreria de c
#include <stdio.h> //libreria de c
#include <cstring>
#include <vector>
#include <cmath>
#include "portaudio.h"

namespace AudioCapture{
    
    struct CircularBuffer
    {
        int max_size;
        int offset;
        std::vector<float> data;

        CircularBuffer(int size) : max_size(size), offset(0) {
        data.resize(size);
        }
        void AddData(float value) {
        data[offset] = value;
        offset = (offset + 1) % max_size;
        }

        const float* GetData() const {
            return data.data();
        }

        int Size() const {
            return max_size;
        }
    };
    
    struct CallbackData
    {
        std::ofstream* l_file;
        std::ofstream* r_file;
        std::ofstream* d_file;
        unsigned int file_lenght = 0;
        std::ofstream* binFile;
        CircularBuffer cBuffer = CircularBuffer(4096);
    };

    
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

        float vol_l = 0;
        float vol_r = 0;

        for (unsigned long i = 0; i < framesPerBuffer*2 ; i += 2) {
            vol_l = max(vol_l, std::abs(in[i]));
            *(data->l_file) << in[i] << "\n";
            vol_r = max(vol_r, std::abs(in[i+1]));
            *(data->r_file) << in[i+1] << "\n";

            *(data->d_file) << std::abs(in[i]- in[i+1]) << "\n";
            (*data).file_lenght++;
        }

        for (int i = 0; i < dispSize; i++) {
            float barProportion = i / (float)dispSize;
            if (barProportion <= vol_l && barProportion <= vol_r) {
                printf("█");
            } else if (barProportion <= vol_l) {
                printf("▀");
            } else if (barProportion <= vol_r) {
                printf("▄");
            } else {
                printf(" ");
            }
        }
/*
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
        }*/

        fflush(stdout);
        return 0;
    }

    bool captureAudio(PaStream* stream, CallbackData* userData, unsigned int device)
    {
        
        PaError err;
        const unsigned int SAMPLE_RATE = Pa_GetDeviceInfo(device)->defaultSampleRate;
        const unsigned int FRAMES_PER_BUFFER = 512;
        PaStreamParameters inputParameters;
        PaStreamParameters outputParameters;
        memset(&inputParameters, 0, sizeof(inputParameters));
        inputParameters.device = device;
        inputParameters.channelCount = 2;
        inputParameters.hostApiSpecificStreamInfo = NULL;
        inputParameters.sampleFormat = paFloat32;
        inputParameters.suggestedLatency = Pa_GetDeviceInfo(device)->defaultLowInputLatency;
        
        memset(&outputParameters, 0, sizeof(outputParameters));
        outputParameters.device = device;
        outputParameters.channelCount = 4;
        outputParameters.hostApiSpecificStreamInfo = NULL;
        outputParameters.sampleFormat = paFloat32;
        outputParameters.suggestedLatency = Pa_GetDeviceInfo(device)->defaultLowOutputLatency;

        std::ofstream l_file("../output/l_amplitudes.txt");
        std::ofstream r_file("../output/r_amplitudes.txt");
        std::ofstream d_file("../output/d_amplitudes.txt");
        std::ofstream binFile("../output/waveform.wav", std::ios::binary);
        if (!l_file.is_open() || !r_file.is_open()) {
            std::cerr << "Error al abrir los archivos\n";
            return false;
        }
        (*userData).l_file = &l_file;
        (*userData).r_file = &r_file;
        (*userData).d_file = &d_file;
        (*userData).binFile = &binFile;

        //prepara el .wav file
        //HEADER CHUNK
        binFile << "RIFF";
        binFile << "----";
        binFile << "WAVE";
        //FORMAT CHUNK
        binFile << "fmt ";
        writeToFile(binFile, 16, 4);
        writeToFile(binFile, 1, 2);
        writeToFile(binFile, 1, 2);
        writeToFile(binFile, SAMPLE_RATE, 4);
        writeToFile(binFile, 16*SAMPLE_RATE/8, 4);
        writeToFile(binFile, 2, 2);
        writeToFile(binFile, 16, 2);
        //DATA CHUNK
        binFile << "data";
        binFile << "----";
        int preAudioPos = binFile.tellp();
        err = Pa_OpenStream(
            &stream,
            &inputParameters,
            &outputParameters,
            SAMPLE_RATE,
            FRAMES_PER_BUFFER,
            paNoFlag,
            paTestCallback,
            (void*)userData
        );
        checkErr(err);
        err = Pa_StartStream(stream);
        checkErr(err);
        Pa_Sleep(5000);
        //std::cout << "Presione ENTER para cerrar...\n";
        //std::cin.get();
        err = Pa_StopStream(stream);
        checkErr(err);
        err = Pa_CloseStream(stream);
        checkErr(err);

        int postAudioPos = binFile.tellp();
        binFile.seekp(preAudioPos - 4);
        writeToFile(binFile, postAudioPos - preAudioPos, 4);
        binFile.seekp(4, std::ios::beg);
        writeToFile(binFile, postAudioPos - 8, 4);
        l_file.close();
        r_file.close();
        d_file.close();
        binFile.close();

        std::cout << "Hola Mundo!\n";
        return true;
    }
    

    bool initStream(PaStream* stream, CallbackData* userData, unsigned int device)
    {
        PaError err;
        const unsigned int SAMPLE_RATE = Pa_GetDeviceInfo(device)->defaultSampleRate;
        const unsigned int FRAMES_PER_BUFFER = 512;
        PaStreamParameters inputParameters;
        PaStreamParameters outputParameters;
        memset(&inputParameters, 0, sizeof(inputParameters));
        inputParameters.device = device;
        inputParameters.channelCount = 1;
        inputParameters.hostApiSpecificStreamInfo = NULL;
        inputParameters.sampleFormat = paFloat32;
        inputParameters.suggestedLatency = Pa_GetDeviceInfo(device)->defaultLowInputLatency;
        
        memset(&outputParameters, 0, sizeof(outputParameters));
        outputParameters.device = device;
        outputParameters.channelCount = 4;
        outputParameters.hostApiSpecificStreamInfo = NULL;
        outputParameters.sampleFormat = paFloat32;
        outputParameters.suggestedLatency = Pa_GetDeviceInfo(device)->defaultLowOutputLatency;

        err = Pa_OpenStream(
            &stream,
            &inputParameters,
            &outputParameters,
            SAMPLE_RATE,
            FRAMES_PER_BUFFER,
            paNoFlag,
            paTestCallback,
            (void*)userData
        );
        checkErr(err);

        return true;
    }

    void createWAV(const std::vector<float> &vector, int sample_rate)
    {
        //funcion aparte (boorar esto eventualmente)
        std::ofstream binFile("../output/wave.wav", std::ios::binary);
        binFile << "RIFF";
        binFile << "----";
        binFile << "WAVE";
        //FORMAT CHUNK
        binFile << "fmt ";
        writeToFile(binFile, 16, 4);
        writeToFile(binFile, 1, 2);
        writeToFile(binFile, 1, 2);
        writeToFile(binFile, sample_rate, 4);
        writeToFile(binFile, 16*sample_rate/8, 4);
        writeToFile(binFile, 2, 2);
        writeToFile(binFile, 16, 2);
        //DATA CHUNK
        binFile << "data";
        binFile << "----";
        int preAudioPos = binFile.tellp();

        int sample = 0;
        const int maxAmplitude = pow(2, 16 - 1) - 1; 
        for (auto amplitude: vector)
        {
            sample = static_cast<int> (maxAmplitude * amplitude);
            binFile.write(reinterpret_cast<char*> (&sample), 2);
        }

        int postAudioPos = binFile.tellp();
        binFile.seekp(preAudioPos - 4);
        writeToFile(binFile, postAudioPos - preAudioPos, 4);
        binFile.seekp(4, std::ios::beg);
        writeToFile(binFile, postAudioPos - 8, 4);
        binFile.close();
    }
}



