#ifndef _STRUCTS
#define _STRUCTS

#include <iostream>

struct DataStruct 
{
    virtual ~DataStruct() = default;
};

struct StereoVolumeStruct: DataStruct
{
    float volume1;
    float volume2;
    float dif;

    void calculeDif()
    {
        dif = volume1 - volume2;
        dif = dif < 0? -dif : dif;
    }
    void setVolume1(float vol)
    {
        volume1 = vol;
        calculeDif();
    }
    void setVolume2(float vol)
    {
        volume2 = vol;
        calculeDif();
    }
    void setVolumes(float vol1, float vol2)
    {
        setVolume1(vol1);
        setVolume2(vol2);
        calculeDif();
    }

};

struct SimpleStereoBuffer: DataStruct
{
public:
    float* l_data;
    float* r_data;
    int lenght;
    SimpleStereoBuffer(int n) : lenght(n)
    {
        l_data = new float[n];
        r_data = new float[n];
        for (int i = 0; i < n; i++) l_data[i] = r_data[i] = 0.0f;
    }
    ~SimpleStereoBuffer()
    {
        delete[] l_data;
        delete[] r_data;
    }
    void copyBufferData(float* in, unsigned int bufferLen)
    {
        for (unsigned int i = 0; i < lenght || 2*i < bufferLen; i++)
        {
            l_data[i] = in[2*i];
            r_data[i] = in[2*i + 1];
        }
    }
};

template<typename T>
struct CircularBuffer: public DataStruct
{
public:
    T* data;
    int lenght;
    int offset;

    //constructor, rellena con elementos por defecto
    //WARNING: si el template no posee un constructor por defecto no rellenará nada
    CircularBuffer(int n)
    :lenght(n), offset(0)
    {
        data = new T[n];
        if constexpr (std::is_default_constructible<T>::value)
            for (int i = 0; i < n; i++) data[i] = T();
    }
    ~CircularBuffer()
    {
        delete[] data;
    }

    void pushData(const T& value)
    {
        data[offset] = value;
        nextIdx(offset);
    }
    void printData()
    {
        int aux = offset;
        for (int i = 0; i < lenght; i++)
        {
            std::cout << data[aux] << " ";
            nextIdx(aux);
        }
        std::cout << std::endl;
    }

 private:
    void nextIdx(int& idx)
    {
        idx = (idx + 1) % lenght;
    }
    
};


#endif