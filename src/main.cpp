#include "Dispatcher.h"

#include <GLFW/glfw3.h>

#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtc/matrix_transform.hpp"

Dispatcher *_dispatcher;
int _buttonState = 0;
glm::vec2 _cursor;

#ifdef _WIN32
#define WAITMS(x) Sleep(x)
#else

/* Portable sleep for platforms other than Windows. */
#define WAITMS(x)                               \
  struct timeval wait = { 0, (x) * 1000 };      \
  (void)select(0, NULL, NULL, NULL, &wait);
#endif

#define KEY_W 119
#define KEY_S 115
#define KEY_A 97
#define KEY_D 100
#define KEY_Q 113
#define KEY_Z 122

static void character(GLFWwindow* window, unsigned int k)
{
        float xDiff = 0.0;
        float yDiff = 0.0;
        float zDiff = 0.0;
        float delta = 0.01;

        switch (k) {
            case KEY_Q:
                zDiff = delta;
                break;
            case KEY_Z:
                zDiff = -delta;
                break;
            case KEY_W:
                xDiff = delta;
                break;
            case KEY_S:
                xDiff = -delta;
                break;
            case KEY_A:
                yDiff = delta;
                break;
            case KEY_D:
                yDiff = -delta;
                break;
            default:;
        }

        _dispatcher->rotateScene(glm::vec3(xDiff, yDiff, zDiff));
}

static void cursor(GLFWwindow* window, double xpos, double ypos)
{
        if (_buttonState != GLFW_PRESS)
                return;

        glm::vec2 newCursor = glm::vec2(xpos, ypos);

        _dispatcher->translateScene(_cursor, newCursor);

        _cursor = newCursor;
}

static void mouse(GLFWwindow* window, int button, int state, int mod)
{
        if (button == 0) {
                _buttonState = state;
                if (state == GLFW_PRESS) {
                        double cursor_x, cursor_y;
                        glfwGetCursorPos(window, &cursor_x, &cursor_y);
                        _cursor = glm::vec2(cursor_x, cursor_y);
                }
        }
}

static void scroll(GLFWwindow* window, double xScroll, double yScroll)
{
        if (yScroll == 1.0f)
                _dispatcher->zoomSceneUp();
        else if (yScroll == -1.0f)
                _dispatcher->zoomSceneDown();
}

// new window size
static void reshape( GLFWwindow* window, int width, int height )
{
        _dispatcher->windowSizeChanged(width, height);
}

static void error(int error, const char* description)
{
        std::cout << "[GLFW] error: " << description << std::endl;
}

// entry point
int main(int argc, char *argv[])
{
    GLFWwindow* window;
    int width, height;

    if (!glfwInit()) {
        std::cout << "[GLFW] error: Failed to initialize GLFW" << std::endl;
        exit(EXIT_FAILURE);
    }

    glfwWindowHint(GLFW_DEPTH_BITS, 16);

    window = glfwCreateWindow(960, 540, "Test", NULL, NULL);
    if (!window) {
        std::cout << "[GLFW] error: Failed to open GLFW window" << std::endl;
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    // Set callback functions
    glfwSetFramebufferSizeCallback(window, reshape);
    glfwSetCharCallback(window, character);
    glfwSetMouseButtonCallback(window, mouse);
    glfwSetScrollCallback(window, scroll);
    glfwSetCursorPosCallback(window, cursor);
    glfwSetErrorCallback(error);

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    GLenum err = glewInit();
    if (GLEW_OK != err) {
            // Problem: glewInit failed, something is seriously wrong
            std::cout << "[GLEW] error: glewInit failed: " << glewGetErrorString(err) << std::endl;
            exit(EXIT_FAILURE);
    }

    glfwGetFramebufferSize(window, &width, &height);

    _dispatcher = new Dispatcher(width, height);

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        _dispatcher->renderScene();

        // Swap buffers
        glfwSwapBuffers(window);
        glfwPollEvents();

        _dispatcher->idle();
    }

    // Terminate GLFW
    glfwTerminate();

    delete _dispatcher;

    // Exit program
    exit(EXIT_SUCCESS);
}

