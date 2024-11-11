#include "HolaMundo.h"
#include "preInitFunctions.h"
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include "implot.h"
#include "audiocapture.h"
#include "calibracion.h"

#include <fstream>
#include <sstream>
#include <iostream>

#define file_path   "../output/amplitudes.txt"
#define l_file_path "../output/l_amplitudes.txt"
#define r_file_path "../output/r_amplitudes.txt"
#define d_file_path "../output/d_amplitudes.txt"

#define C0_SOUND_SPEED 343.2
#define LAMBDA         1
#define p_DENSITY      1.186

using namespace AudioCapture;

int main()
{
    GLFWwindow* miVentana = initContextAndCreateWindow();

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); //(void) io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    //io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // IF using Docking Branch

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(miVentana, true);          // Second param install_callback=true will install GLFW callbacks and chain to existing ones.
    ImGui_ImplOpenGL3_Init();
    int display_w, display_h;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    ///////////////////////////////////////////////////////
    //              Eleccion de Dispositivos             //
    ///////////////////////////////////////////////////////
    PaError err;
    err = Pa_Initialize();
    tdi::checkErr(err);

    int numDevices = Pa_GetDeviceCount();
    const PaDeviceInfo* deviceInfo[numDevices];
    for(int i = 0; i < numDevices; i++)
        deviceInfo[i] = Pa_GetDeviceInfo(i);


    //IMGUI EXAMPLE
    static float f = 0.0f;
    static int counter = 0;
    bool show_demo_window = true;
    bool show_another_window = false;

    //TUBO DE IMPEDANCIA
    bool isInputDeviceSelected = false,
         isRecordStarted = false,
         isRecordFinish = false,
         calibration_factor = false;
    float rms1, rms2, dB1, dB2;
    float *frec_spec;
    float *H_12_I = NULL;
    float *H_12_II = NULL;
    float *Hc = NULL;
    int device_selected_idx = -1;
    tdi::CalibrationData* data;
    tdi::CalibrationData* HI_data;
    tdi::CalibrationData* HII_data;
    PaStream* stream;

    while(!glfwWindowShouldClose(miVentana))
    {
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        glfwPollEvents();
        if (glfwGetWindowAttrib(miVentana, GLFW_ICONIFIED) != 0)
        {
            ImGui_ImplGlfw_Sleep(10);
            continue;
        }

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);

        //Flujo del Tubo De Impedancia
        if (!isInputDeviceSelected)
        {
            ImGui::Begin("Device Selection");
            if (ImGui::BeginListBox("Devices"))
            {
                for (int i = 0; i < numDevices; i++)
                {

                    std::ostringstream oss;
                    oss << i << " # " << (deviceInfo[i]->name);

                    if (ImGui::Selectable(oss.str().c_str(), device_selected_idx == i))
                        device_selected_idx = i;
                }
            }ImGui::EndListBox();
            if (device_selected_idx > -1)
            {
                ImGui::Text("Nombre:               %s", deviceInfo[device_selected_idx]->name);
                ImGui::Text("Max Inputs Channels:  %i", deviceInfo[device_selected_idx]->maxInputChannels);
                ImGui::Text("Max Outputs Channels: %i", deviceInfo[device_selected_idx]->maxOutputChannels);
                ImGui::Text("Default Smaple Rate:  %f", deviceInfo[device_selected_idx]->defaultSampleRate);

                if(ImGui::Button("Cargar dispositivo"))
                {
                    isInputDeviceSelected = true;
                    data     = new tdi::CalibrationData(10,(int)deviceInfo[device_selected_idx]->defaultSampleRate);
                    HI_data  = new tdi::CalibrationData(10,(int)deviceInfo[device_selected_idx]->defaultSampleRate);
                    HII_data = new tdi::CalibrationData(10,(int)deviceInfo[device_selected_idx]->defaultSampleRate);
                } 
            }
            ImGui::End();
        }
        else
        {
            if (!isRecordStarted && Pa_IsStreamActive(stream) != 1)
            {
                std::cout << "Hola! ya iniciamos stream\n";
                tdi::initAudioCaptureForCalibration(&stream, data, device_selected_idx);
                Pa_Sleep(500);// me sicoseo
                isRecordStarted = true;
            }
            if(!isRecordFinish && Pa_IsStreamActive(stream) == 0)
            {
                Pa_Sleep(500);// me sicoseo
                tdi::closeAudioCaptureForCalibration(&stream);
                
                createWAV((*data).paraWAV,deviceInfo[device_selected_idx]->defaultSampleRate);

                rms1 = tdi::calcularRMS((*data).signal_a, (*data).fs);
                rms2 = tdi::calcularRMS((*data).signal_b, (*data).fs);
                dB1 = tdi::rmsADecibelios(rms1);
                dB2 = tdi::rmsADecibelios(rms2);
                isRecordFinish = true;
                calibration_factor = true;

                tdi::fftFrecuencySpectrum(&frec_spec, (*data).signal_a, (*data).fs);
            }
    
            ImGui::Begin("Audio Capture");
            if (ImPlot::BeginPlot("My Plot"))
            {
                ImPlot::PlotLine("Microfono 1", data->signal_a, 44000);
                ImPlot::PlotLine("Microfono 2", data->signal_b, 44000);
                //ImPlot::PlotLine("vector", (data->paraWAV).data(), 20000);
                ImPlot::EndPlot();
            }
            ImGui::Text("n_count: %i", data->n_count);
            if (isRecordFinish)
            {
                ImGui::Text("rms1: %f", rms1);
                ImGui::Text("rms2: %f", rms2);
                ImGui::Text("dB1: %f",  dB1);
                ImGui::Text("dB2: %f",  dB2);
                ImGui::Text("|dB2 - dB1| = %f", std::abs(dB2 - dB1));
                if (ImPlot::BeginPlot("Plot"))
                {
                    ImPlot::PlotLine("Transformada", frec_spec, 22000);
                    ImPlot::EndPlot();
                }
            }
            ImGui::End();

        }

        //CALCULO DEL FACTOR DE CALIBRACION

        if(calibration_factor)
        {
            static bool is_a_stream_active = false;
            static bool HI_started = false;
            static bool HII_started = false;
            ImGui::Begin("Calibration");
            if(!HI_started && ImGui::Button("Grabar H_12_I"))
            {
                std::cout << "Hola! ya iniciamos stream HI\n";
                tdi::initAudioCaptureForCalibration(&stream, HI_data, device_selected_idx);
                Pa_Sleep(500);// me sicoseo
                HI_started = true;
                is_a_stream_active = true;
            }
            if(!HII_started && ImGui::Button("Grabar H_12_II"))
            {
                std::cout << "Hola! ya iniciamos stream HII\n";
                tdi::initAudioCaptureForCalibration(&stream, HII_data, device_selected_idx);
                Pa_Sleep(500);// me sicoseo
                HII_started = true;
                is_a_stream_active = true;
            }
            if(is_a_stream_active && Pa_IsStreamActive(stream) == 0)
            {
                Pa_Sleep(500);// me sicoseo
                tdi::closeAudioCaptureForCalibration(&stream);
                is_a_stream_active = false;
            }
            if (HI_started && ImPlot::BeginPlot("H_12_I"))
            {
                ImPlot::PlotLine("Microfono 1", HI_data->signal_a, 44000);
                ImPlot::PlotLine("Microfono 2", HI_data->signal_b, 44000);
                ImPlot::EndPlot();
            }
            if (HII_started && ImPlot::BeginPlot("H_12_II") )
            {
                ImPlot::PlotLine("Microfono 1", HII_data->signal_a, 44000);
                ImPlot::PlotLine("Microfono 2", HII_data->signal_b, 44000);
                ImPlot::EndPlot();
            }
            if(HI_started && HII_started && !is_a_stream_active && ImPlot::BeginPlot("Hc"))
            {
                //std::cout << "Alo?\n";
                tdi::HFunction(HI_data->signal_a, HI_data->signal_b,  &H_12_I,  HI_data->fs);
                tdi::HFunction(HII_data->signal_b,HII_data->signal_a, &H_12_II, HII_data->fs);
                tdi::HcFunction(H_12_I,           H_12_II,            &Hc,      HI_data->fs/2);

                ImPlot::PlotLine("H_12_I",  H_12_I,  22000);
                ImPlot::PlotLine("H_12_II", H_12_II, 22000);
                ImPlot::PlotLine("Hc",      Hc,      22000);
                ImPlot::EndPlot();
            }
            ImGui::End();
        }
    
        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(miVentana, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(miVentana);
    }
    if (Hc != nullptr)
    {
        delete data;
        delete HI_data;
        delete HII_data;
        delete Hc;
        delete H_12_I;
        delete H_12_II;
        std::cout << "ESTOY LIMPIO!\n";
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();

    glfwDestroyWindow(miVentana);
    glfwTerminate();

    err = Pa_Terminate();
    tdi::checkErr(err);

    print("Chao Mundo!");
    return 0;
}