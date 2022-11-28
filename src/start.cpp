#include "glcommon.h"

#include <iostream>
#include <cstdlib>

#include <cstdio>

#include <vector>

#include "text/textrender.h"
#include "text/textprocess.h"

#include <windows.h>

using namespace std;

// Dimensions of our window
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

//#define PROFILE_PERF

void APIENTRY glDebugOutput(GLenum source,
                            GLenum type,
                            unsigned int id,
                            GLenum severity,
                            GLsizei length,
                            const char *message,
                            const void *userParam)
{
    // ignore non-significant error/warning codes
    if(id == 131169 || id == 131185 || id == 131218 || id == 131204) return;

    std::cout << "---------------" << std::endl;
    std::cout << "Debug message (" << id << "): " <<  message << std::endl;

    switch (source)
    {
        case GL_DEBUG_SOURCE_API:             std::cout << "Source: API"; break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   std::cout << "Source: Window System"; break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER: std::cout << "Source: Shader Compiler"; break;
        case GL_DEBUG_SOURCE_THIRD_PARTY:     std::cout << "Source: Third Party"; break;
        case GL_DEBUG_SOURCE_APPLICATION:     std::cout << "Source: Application"; break;
        case GL_DEBUG_SOURCE_OTHER:           std::cout << "Source: Other"; break;
    } std::cout << std::endl;

    switch (type)
    {
        case GL_DEBUG_TYPE_ERROR:               std::cout << "Type: Error"; break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cout << "Type: Deprecated Behaviour"; break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  std::cout << "Type: Undefined Behaviour"; break;
        case GL_DEBUG_TYPE_PORTABILITY:         std::cout << "Type: Portability"; break;
        case GL_DEBUG_TYPE_PERFORMANCE:         std::cout << "Type: Performance"; break;
        case GL_DEBUG_TYPE_MARKER:              std::cout << "Type: Marker"; break;
        case GL_DEBUG_TYPE_PUSH_GROUP:          std::cout << "Type: Push Group"; break;
        case GL_DEBUG_TYPE_POP_GROUP:           std::cout << "Type: Pop Group"; break;
        case GL_DEBUG_TYPE_OTHER:               std::cout << "Type: Other"; break;
    } std::cout << std::endl;

    switch (severity)
    {
        case GL_DEBUG_SEVERITY_HIGH:         std::cout << "Severity: high"; break;
        case GL_DEBUG_SEVERITY_MEDIUM:       std::cout << "Severity: medium"; break;
        case GL_DEBUG_SEVERITY_LOW:          std::cout << "Severity: low"; break;
        case GL_DEBUG_SEVERITY_NOTIFICATION: std::cout << "Severity: notification"; break;
    } std::cout << std::endl;
    std::cout << std::endl;
}

int main() {
	FreeConsole();
	string stringpath = "C:/fasttexteditor";
	CreateDirectory(stringpath.c_str(), NULL);

    // Our future window
    GLFWwindow* window;

    // Initialize GLFW
    if ( ! glfwInit() )
    {
    	cerr << "Unable to initialize GLFW" << endl;
        return -1;
    }

    // I'll be using OpenGL Core 3.3. So we provide these hints to GLFW
    // BEFORE creating the window with glfwCreateWindow.
    // These hints specify some options for our window and framebuffer.
    glfwDefaultWindowHints();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4); //TODO: lower OpenGL version requirements
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    glfwWindowHint(GLFW_DOUBLEBUFFER, GL_FALSE);

    // Create the window and its OpenGL context
    window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Super Fast Text Editor", NULL, NULL);

    // If window creation failed, stop. Don't forget to release
    // resources with glfwTerminate.
    if ( ! window )
    {
    	cout << "Stopping" << endl;
        glfwTerminate();
        return -1;
    }

    // Before we can use the newly created OpenGL context, we need
    // to make it current. Use glfwMakeContextCurrent for that.
    glfwMakeContextCurrent(window);

    //Set callbcaks -- this will dramatically reduce CPU versus polling
    glfwSetCharCallback(window, char_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Initialize GLAD. If it succeeds, we're ready to invoke
    // OpenGL functions.
    if ( ! gladLoadGLLoader((GLADloadproc)glfwGetProcAddress) )
    {
        glfwTerminate();
        return -1;
    }

    initRenderer();

    glfwShowWindow(window);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

#ifdef DEBUG
    glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glDebugMessageCallback(glDebugOutput, nullptr);
	glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
#endif

	init();

	renderCaret(0, 0);
	glFlush();

    // The loop.
    while ( ! glfwWindowShouldClose(window) ) {
        // Waits until something happens
        glfwWaitEvents();

        //Flush changes to screen, no need to swap buffers since we're single buffering
        glFlush();
    }

    // Always release resources allocated by GLFW
    glfwTerminate();
    cleanup();

    return 0;
}
