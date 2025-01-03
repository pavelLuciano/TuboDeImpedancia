#ifndef _AUDIO_MASTER_H
#define _AUDIO_MASTER_H
#include "BasicAudioHandler.h"
#include "SignalCalibrator.h"
#include <vector>
#include <string>

typedef int AudioMaster_Comands;
typedef int AudioMaster_States;
typedef int AudioMaster_Warnings;

//Protocolo de uso de AudioMaster_Comands:
// 1.Apagar un dispositivo SIEMPRE tiene prioridad por sobre encenderlo
//   Si llega un comando con la instruccion de apagar y encender un dispositivo, este quedará apagado
// 2.Por agregar...
enum AudioMaster_Commands_
{
    AMC_None                   = 0,
    AMC_DeviceSelector_TurnOn  = 1 << 0,
    AMC_DeviceSelector_TurnOff = 1 << 1,
    AMC_SignalCalibrator_TurnOn      = 1 << 2,
    AMC_SignalCalibrator_TurnOff     = 1 << 3,

//aun sin implementar
    AMC_H12_TurnOn             = 1 << 4,
    AMC_H12_TurnOff            = 1 << 5,
    AMC_H21_TurnOn             = 1 << 6,
    AMC_H21_TurnOff            = 1 << 7,
    AMC_Speaker_TurnOn         = 1 << 8,
    AMC_Speaker_TurnOff        = 1 << 9,

    AMC_Update_DeviceSelector_Device = 1 << 10,
    AMC_UpdateAll_Devices      = 1 << 11

};
inline const char* COMMANDS_LIST[] = {
    "None",
    "DeviceSelector_TurnOn",
    "DeviceSelector_TurnOff",
    "Calibrator_TurnOn",
    "Calibrator_TurnOff"
};
//serias dudas acerca del uso de esto
enum AudioMaster_Comand_Warning_
{
    AMC_W_NO_WARNING              = 0,
    AMC_W_DEVICESELECTOR_CONFLICT = AMC_DeviceSelector_TurnOff | AMC_DeviceSelector_TurnOn,
    AMC_W_CALIBRATOR_CONFLICT     = AMC_SignalCalibrator_TurnOff | AMC_SignalCalibrator_TurnOn
};
#define DETECT_CONFLICT_(x,y) ((x) & (y) == (y))
const int WARNINGS_IDs[] = {0,3,5};
inline const char* WARNINGS_LIST[] = {
    "No_Warning",
    "DeviceSelector_Conflict",
    "Calibrator_Conflict"
    };

//esto es importante
enum AudioMaster_States_
{
    AMS_All_Off           = 0,
    AMS_DeviceSelector_On = 1 << 0,
    AMS_SignalCalibrator_On     = 1 << 1
};

//estos se usan para modificar un estado con el operador binario XOR '^'
//Hay que tener cuidado al trabajar con estos
enum AudioMaster_State_Switch
{
    AMSwitch_Nothing        = 0,
    AMSwitch_DeviceSelector = 1 << 0,
    AMSwitch_SignalCalibrator     = 1 << 1
};

class AudioMaster
{
private:
    AudioMaster_States currentState;
    AudioMaster_States newState;
    AudioMaster_Comands lastCommand;

    std::vector<std::string> warnings;

    int device_idx = NO_DEVICE;
    
public:
    BasicAudioHandler* deviceSelector;
    SignalCalibrator* signalCalibrator;

    AudioMaster();
    ~AudioMaster();

    //Funciones para manejar errores y logs
    void logStates();
    void logComands(AudioMaster_Comands);
    const std::string stateAndComandsInfo();
    const std::string deviceSelectorInfo();
    const std::string calibratorInfo();

//funciones que modifican el estado del AudioMaster
    bool setDevice(int); //usar esta para modificar desde fuera
    void computeCommands(AudioMaster_Comands);

private:

    bool updateState();
    bool setNewState(AudioMaster_States);
    bool checkStateConsistency();
    bool autoSetState();
    bool setDeviceForAll();
    bool setDeviceForDeviceSelector();

//funciones que encienden o apagan los Audio Handlers
    bool deviceSelectorOn();
    bool deviceSelectorOff();
    bool calibratorOn();
    bool calibratorOff();


};



#endif