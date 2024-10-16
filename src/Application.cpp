#include "HolaMundo.h"
#include "preInitFunctions.h"
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include "implot.h"

#include "audiocapture.h"

#include <fstream>

#define file_path "../output/amplitudes.txt"


int main()
{
    GLFWwindow* miVentana = initContextAndCreateWindow();

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); //(void) io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
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
    checkErr(err);

    int numDevices = Pa_GetDeviceCount();
    const PaDeviceInfo* deviceInfo[numDevices];
    for(int i = 0; i < numDevices; i++)
        deviceInfo[i] = Pa_GetDeviceInfo(i);



    // ARCHIVOS
    std::ifstream amplitudes(file_path);
    if(!amplitudes)
    {
        std::cout << "ERROR AL LEER ARCHIVO\n";
        return 0;
    }
    std::string valor;



    //considera 48000 Hz
    int i = 0;
    float data_y[1000];
    while(std::getline(amplitudes, valor))
    {
        if (i%218 == 0) data_y[i/218] = stof(valor);
        i++;
    }


    bool show_demo_window = false;
    bool show_another_window = false;

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

        // 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
        {
            static float f = 0.0f;
            static int counter = 0;

            ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.
            //implot
            if (ImPlot::BeginPlot("My Plot"))
            {
                ImPlot::PlotLine("My Line Plot", data_y, 1000);
                ImPlot::EndPlot();
            }
            ImGui::End();

            ImGui::Begin("Device Selection");
            static int device_selected_idx = -1;
            if (ImGui::BeginListBox("Devices"))
            {
                for (int i = 0; i < numDevices; i++)
                {
                    //deviceInfo = Pa_GetDeviceInfo(i);
                    if (ImGui::Selectable(deviceInfo[i]->name, device_selected_idx == i))
                        device_selected_idx = i;
                }
                ImGui::EndListBox();
                if (device_selected_idx > -1)
                {
                    ImGui::Text("Nombre:               %s", deviceInfo[device_selected_idx]->name);
                    ImGui::Text("Max Inputs Channels:  %i", deviceInfo[device_selected_idx]->maxInputChannels);
                    ImGui::Text("Max Outputs Channels: %i", deviceInfo[device_selected_idx]->maxOutputChannels);
                    ImGui::Text("Default Smaple Rate:  %d", deviceInfo[device_selected_idx]->defaultSampleRate);
                }
                
            }
            ImGui::End();






/*
            ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
            ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
            ImGui::Checkbox("Another Window", &show_another_window);

            ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
            ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

            if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
                counter++;
            ImGui::SameLine();
            ImGui::Text("counter = %d", counter);

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
            */
            
            
        }

        // 3. Show another simple window.
        if (show_another_window)
        {
            ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
            ImGui::Text("Hello from another window!");
            if (ImGui::Button("Close Me"))
                show_another_window = false;
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

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    //ImPlot::DestroyContext();
    ImGui::DestroyContext();

    glfwDestroyWindow(miVentana);
    glfwTerminate();

    print("Hola Mundo!");
    return 0;
}