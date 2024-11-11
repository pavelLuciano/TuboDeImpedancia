#include <GL/glew.h>
#include <GLFW/glfw3.h>

//GLFW Y GLEW
const int HEIGHT = 600;
const int WIDTH = 1200;
const char* WINDOW_NAME = "IMGUI";

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
GLFWwindow* initContextAndCreateWindow()
{
    if (!glfwInit()) return NULL;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    GLFWwindow* window;
    window = glfwCreateWindow(WIDTH, HEIGHT, WINDOW_NAME, NULL, NULL);
    if (!window)
    {
        //std::cout << "Fall� en crear la ventana GLFW" << "\n";
        glfwTerminate();
        return NULL;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);


    if (glewInit() != GLEW_OK) 
    {
        glfwTerminate();
        return NULL;
    }
    //std::cout << "OpenGL iniciado correctamente\nVersion: " << glGetString(GL_VERSION) << "\n";
    //glEnable(GL_DEPTH_TEST);

    return window;
}
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

//IMGUI

