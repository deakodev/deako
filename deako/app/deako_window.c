#include "deako_pch.h"
#include "deako_app.h"

#include <GLFW/glfw3.h>

static void _dk_glfw_error_callback(int error, const char* description)
{
    DK_ERROR("DK_ERRNO_GLFW (%d): %s", error, description);
}

static void _dk_app_window_on_close(GLFWwindow* window)
{
    dk_app_t* app   = (dk_app_t*)glfwGetWindowUserPointer(window);
    app->is_running = false;
}

int _dk_app_window_init(dk_app_t* app, int width, int height, const char* title)
{
    DK_CHECK(glfwInit(), DK_ERRNO_UNKNOWN);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // for vulkan

    glfwSetErrorCallback(_dk_glfw_error_callback);

    GLFWwindow* glfw_window =
    glfwCreateWindow((width >= 0 ? width : 800), (height >= 0 ? height : 600), (title ? title : "Deako"), NULL, NULL);
    DK_CHECK(glfw_window, DK_ERRNO_UNKNOWN);
    app->glfw_window = glfw_window;

    glfwSetWindowUserPointer(glfw_window, (void*)app); /* easy access to the windows' data in glfw callbacks */

    glfwSetWindowCloseCallback(glfw_window, _dk_app_window_on_close);

    return DK_STATUS_OK;
}

void _dk_app_window_poll(void)
{
    glfwPollEvents();
}