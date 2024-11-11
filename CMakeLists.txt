# Versión mínima requerida de CMake
cmake_minimum_required(VERSION 3.10)

# Nombre del proyecto
project(MyProgram)

# Establecer el estándar de C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Directorios
set(SRC_DIR "${CMAKE_SOURCE_DIR}/src")
set(INCLUDE_DIR "${CMAKE_SOURCE_DIR}/include")
set(DEPS_DIR "${CMAKE_SOURCE_DIR}/dependencies")
set(IMGUI_DIR "${DEPS_DIR}/imgui")
set(PORTAUDIO_DIR "${DEPS_DIR}/portaudio")
set(IMPLOT_DIR "${DEPS_DIR}/implot")

# Incluir los directorios de cabeceras
include_directories("/usr/local/include")
include_directories(${INCLUDE_DIR})
include_directories(${DEPS_DIR})
include_directories(${IMGUI_DIR})
include_directories(${IMPLOT_DIR})
include_directories(${PORTAUDIO_DIR}/include)


link_directories("/usr/local/lib")

# Buscar los archivos fuente en src/
file(GLOB SOURCES "${SRC_DIR}/*.cpp")

# Añadir fuentes específicas de ImGui
list(APPEND SOURCES
    "${IMGUI_DIR}/imgui.cpp"
    "${IMGUI_DIR}/imgui_demo.cpp"
    "${IMGUI_DIR}/imgui_draw.cpp"
    "${IMGUI_DIR}/imgui_tables.cpp"
    "${IMGUI_DIR}/imgui_widgets.cpp"
    "${IMGUI_DIR}/backends/imgui_impl_glfw.cpp"
    "${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp"
)
#añadir fuentes especificas de ImPlot
list(APPEND SOURCES
    "${IMPLOT_DIR}/implot.cpp"
    "${IMPLOT_DIR}/implot_items.cpp"
    "${IMPLOT_DIR}/implot_demo.cpp"
)

# Definir el ejecutable
add_executable(MyProgram ${SOURCES})

# Librerías y banderas
find_package(OpenGL REQUIRED)
find_package(Threads REQUIRED)

target_link_libraries(MyProgram
    ${CMAKE_THREAD_LIBS_INIT}
    ${OPENGL_LIBRARIES}
    fftw3
    fftw3f
    glfw         # Librería GLFW
    #rt           # Librería tiempo real
    asound       # ALSA para audio
    jack         # Soporte para JACK
    pthread      # Hilos
    portaudio    # Librería de PortAudio
    GLEW
)
