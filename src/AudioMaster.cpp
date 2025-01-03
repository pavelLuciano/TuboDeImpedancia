#include "AudioMaster.h"

AudioMaster::AudioMaster()
{
    deviceSelector = new BasicAudioHandler();
    signalCalibrator = new SignalCalibrator();
    currentState = newState = AMS_All_Off;
}
AudioMaster::~AudioMaster()
{
    delete deviceSelector;
    delete signalCalibrator;
}

//Aplica los cambios de newState para convertirlo a currentState
bool AudioMaster::updateState()
{
    if (currentState == newState) return true;
    if(newState & AMS_DeviceSelector_On) deviceSelectorOn();
    else deviceSelectorOff();
    if(newState & AMS_SignalCalibrator_On) calibratorOn();
    else calibratorOff();
    return true;
}
bool AudioMaster::setDevice(int device)
{
    device_idx = device;
    return true;
}
bool AudioMaster::setDeviceForAll()
{
    deviceSelector->setDevice(device_idx);
    signalCalibrator->setDevice(device_idx);
    return true;
}
bool AudioMaster::setDeviceForDeviceSelector()
{
    deviceSelector->setDevice(device_idx);
    return true;
}
//Aplica Comandos para generar un nuevo estado AudioMaster::newState
void AudioMaster::computeCommands(AudioMaster_Comands commands)
{
    //es necesario el modificar esta funcion cuando el programa se complejice        
    // 0. DeviceSelector_TurnOn  
    // 1. DeviceSelector_TurnOff 
    // 2. Calibrator_TurnOn      
    // 3. Calibrator_TurnOff
    lastCommand = commands;  
    AudioMaster_States computedState = AMS_All_Off;
    if (commands & AMC_DeviceSelector_TurnOn)  computedState |= AMS_DeviceSelector_On;
    if (commands & AMC_DeviceSelector_TurnOff) computedState &= ~AMS_DeviceSelector_On;
    if (commands & AMC_SignalCalibrator_TurnOn)      computedState |= AMS_SignalCalibrator_On;
    if (commands & AMC_SignalCalibrator_TurnOff)     computedState &= ~AMS_SignalCalibrator_On;

    if (commands & AMC_Update_DeviceSelector_Device)
    {
        if (currentState & AMS_DeviceSelector_On) 
            deviceSelector->initStream(device_idx);
        else
            deviceSelector->setDevice(device_idx);
    }

}

//cambia AudioMaster::newState al estado que se le entrega
bool AudioMaster::setNewState(AudioMaster_States state)
{
    newState = state;
    return true;
}

bool AudioMaster::deviceSelectorOn()
{

}
bool AudioMaster::deviceSelectorOff()
{

}
bool AudioMaster::calibratorOn()
{

}
bool AudioMaster::calibratorOff()
{
    
}

void AudioMaster::logStates()
{

}
void AudioMaster::logComands(AudioMaster_Comands)
{

}
const std::string AudioMaster::stateAndComandsInfo()
{
    
}
const std::string AudioMaster::deviceSelectorInfo()
{
    std::string info = "Device Selector: \n";
    int device = deviceSelector->getDevice(); 
    info += " - Device: " + (device==NO_DEVICE? "NO DEVICE" : std::to_string(device) +" "+ Pa_GetDeviceInfo(device)->name) + "\n";
    info += " - Estado: " + deviceSelector->isStreamActive()? "Activo\n" : "Inactivo\n";
    return info.c_str();

}
const std::string AudioMaster::calibratorInfo()
{
    std::string info = "Calibrador:\n";
    int device = signalCalibrator->getDevice(); 
    info += " - Device: " + (device==NO_DEVICE? "NO DEVICE" : std::to_string(device) +" "+ Pa_GetDeviceInfo(device)->name) + "\n";
    info += " - Estado: " + signalCalibrator->isStreamActive()? "Activo\n" : "Inactivo\n";
    return info.c_str();
}
