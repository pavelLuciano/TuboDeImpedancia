
#include "HolaMundo.h"
#include "init_deps.h"
#include "aumi_math.h"
#include "AudioMaster.h"
#include "BasicAudioHandler.h"
#include "SignalCalibrator.h"

#define LOG(x) std::cout<< x <<std::endl

#include <portaudio.h>
#include <fstream>
#include <sstream>
#include <iostream>


void showMenuBar();//Barra de Menu
void showDeviceSelector(int* device_idx, int numDevices, const PaDeviceInfo* deviceInfo[], AudioMaster_Comands* commands, AudioMaster* audioMaster);
void showCalibration(int device_idx, AudioMaster* audioMaster);
void showDebugWindow(AudioMaster* am);

int main()
{
    GLFWwindow* mi_ventana = initDepsAndCreateWin();
    if(!mi_ventana) return EXIT_FAILURE;

    //dejar esto en una funcion
    PaError err;
    err = Pa_Initialize();
    int numDevices = Pa_GetDeviceCount();
    const PaDeviceInfo* deviceInfo[numDevices];
    for(int i = 0; i < numDevices; i++)
        deviceInfo[i] = Pa_GetDeviceInfo(i);
    int device;

    //AUDIO HANDLERS
    AudioMaster am;
    AudioMaster_Comands am_comands;
    while(!glfwWindowShouldClose(mi_ventana))
    {
        am_comands = AMC_None;
        glfwPollEvents();
        if (glfwGetWindowAttrib(mi_ventana, GLFW_ICONIFIED) != 0)
        {
            ImGui_ImplGlfw_Sleep(10);
            continue;
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        // We demonstrate using the full viewport area or the work area (without menu-bars, task-bars etc.)
        // Based on your use case you may want one or the other.
        static bool use_work_area = true;
        static ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings;
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(use_work_area ? viewport->WorkPos : viewport->Pos);
        ImGui::SetNextWindowSize(use_work_area ? viewport->WorkSize : viewport->Size);
        if (ImGui::Begin("Example: Fullscreen window", nullptr, flags))
        {
            if(ImGui::BeginTabBar("Etapas"))
            {
                if(ImGui::BeginTabItem("Device"))
                {
                    am_comands |= AMC_SignalCalibrator_TurnOff;
                    ImGui::Text("Acá Colocaremos el Selector de Distositivo");
                    showDeviceSelector(&device, numDevices, deviceInfo, &am_comands , &am);
                    ImGui::EndTabItem();
                }
                if(ImGui::BeginTabItem("Calibracion de Nivel"))
                {
                    am_comands |= AMC_DeviceSelector_TurnOff;
                    ImGui::Text("Acá Colocaremos el Calibrador");
                    showCalibration(device, &am);
                    ImGui::EndTabItem();
                }
                if(ImGui::BeginTabItem("Factor de Transferencia"))
                {
                    am_comands |= AMC_DeviceSelector_TurnOff | AMC_SignalCalibrator_TurnOff;
                    ImGui::Text("Acá Colocaremos el Calibrador");
                    ImGui::EndTabItem();
                }
                ImGui::EndTabBar();
            }
             
        }
        ImGui::End();

        //showDebugWindow(&audioMaster);

        //RENDER
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(mi_ventana, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        //glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(mi_ventana);
    }

    err = Pa_Terminate();
    return EXIT_SUCCESS;
}

void showMenuBar()
{
    if (ImGui::BeginMainMenuBar())
    {
        // Menú "Archivo"
        if (ImGui::BeginMenu("Archivo"))
        {
            if (ImGui::MenuItem("Abrir")) { /* Acción para "Abrir" */ }
            if (ImGui::MenuItem("Guardar")) { /* Acción para "Guardar" */ }
            if (ImGui::MenuItem("Salir")) { /* Acción para "Salir" */ }
            ImGui::EndMenu();
        }

        // Menú "Editar"
        if (ImGui::BeginMenu("Editar"))
        {
            if (ImGui::MenuItem("Copiar")) { /* Acción para "Copiar" */ }
            if (ImGui::MenuItem("Pegar")) { /* Acción para "Pegar" */ }
            ImGui::EndMenu();
        }

        // Menú "Ayuda"
        if (ImGui::BeginMenu("Ayuda"))
        {
            if (ImGui::MenuItem("Acerca de")) { /* Acción para "Acerca de" */ }
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }

}

void showDeviceSelector(int* device_idx, int numDevices, const PaDeviceInfo* deviceInfo[], AudioMaster_Comands* commands, AudioMaster* audioMaster)
{
    static bool change_device = false; //¿funcionaria sin el static? ¿que seria mejor practica?
    float item_height = ImGui::GetTextLineHeightWithSpacing();
    if (ImGui::BeginListBox("", ImVec2(400, item_height * numDevices + 10)))
    {
        for (int i = 0; i < numDevices; i++)
        {
            std::ostringstream oss;
            oss << i << " " << (deviceInfo[i]->name);
            if (ImGui::Selectable(oss.str().c_str(), (*device_idx) == i))
            {
                *device_idx = i; //hay que borrarlo
                audioMaster->setDevice(i);
                (*commands) |= AMC_Update_DeviceSelector_Device | AMC_DeviceSelector_TurnOn;
            }
        }
    }ImGui::EndListBox();

    BasicAudioHandler* audioHandler = audioMaster->deviceSelector;
    ImGui::PlotLines("Micrófono 1",
                      aumimath::dft_mag_f(audioHandler->lData(),audioHandler->dataLen()), 
                      audioHandler->dataLen() / 2,
                      0,
                      "",
                      0.0f,
                      10.0f,
                      ImVec2(700.0f, 80.0f));
    
    ImGui::PlotLines("Micrófono 2", 
                      aumimath::dft_mag_f(audioHandler->rData(), audioHandler->dataLen()),
                      audioHandler->dataLen() / 2,
                      0,
                      "",
                      0.0f,
                      10.0f,
                      ImVec2(700.0f, 80.0f));
}

void showCalibration(int device_idx, AudioMaster* audioMaster)
{
    SignalCalibrator* sigCal = audioMaster -> signalCalibrator;
    if(ImGui::Button("Calibrar"))
    {
        if (!sigCal->isStreamActive()) sigCal->initStream(device_idx);
        else sigCal->closeStream();
    }

    ImGui::Text("volumen 1:  %f\n", dynamic_cast<StereoVolumeStruct*>(sigCal->data)->volume1);
    ImGui::Text("volumen 2:  %f\n", dynamic_cast<StereoVolumeStruct*>(sigCal->data)->volume2);
    ImGui::Text("diferencia: %f\n", dynamic_cast<StereoVolumeStruct*>(sigCal->data)->dif);
}
void showDebugWindow(AudioMaster* am)
{
    std::string info = "Audio Master: \n";
    info += " - State: ";

    ImGui::Begin("Debug");
    ImGui::Text("%s", info.c_str());
    ImGui::Text("%s", am->deviceSelectorInfo().c_str());
    ImGui::Text("%s", am->calibratorInfo().c_str());
    ImGui::End();
}