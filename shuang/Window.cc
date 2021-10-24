#include "Window.h"
#include "Application.h"
#include "Logger.h"
#include "Macros.h"

#include <stdexcept>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

namespace {

void errorCallback(int error, const char *description) {
  logError("GLFW error: code {}: {}", error, description);
}

void windowCloseCallback(GLFWwindow *window) {
  glfwSetWindowShouldClose(window, GLFW_TRUE);
}

void windowSizeCallback(GLFWwindow *window, int width, int height) {
  if (auto application =
          reinterpret_cast<Application *>(glfwGetWindowUserPointer(window))) {
    application->resize(width, height);
  }
}

void windowFocusCallback(GLFWwindow *window, int focused) {}

void keyCallback(GLFWwindow *window, int key, int /* scancode */, int action,
                 int /* mods */) {
  auto application =
      reinterpret_cast<Application *>(glfwGetWindowUserPointer(window));
  if (!application) {
    return;
  }

  KeyCode keyCode;
  switch (key) {
  case GLFW_KEY_ESCAPE:
    keyCode = KeyCode::ESCAPE;
    break;
  }

  KeyAction keyAction;
  switch (action) {
  case GLFW_RELEASE:
    keyAction = KeyAction::UP;
    break;
  }

  application->handleEvent(KeyInputEvent{keyCode, keyAction});
}

void cursorPosCallback(GLFWwindow *window, double xPos, double yPos) {}

void mouseButtonCallback(GLFWwindow *window, int button, int action,
                         int /* mods */) {}

} // namespace

Window::Window(const Application *application, uint32_t width, uint32_t height,
               const char *title) {
  if (!glfwInit()) {
    throw std::runtime_error("Failed to init GLFW");
  }

  glfwSetErrorCallback(errorCallback);
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

  if (mHandle = glfwCreateWindow(width, height, title, nullptr, nullptr);
      !mHandle) {
    throw std::runtime_error("Failed to create GLFW");
  }

  glfwSetWindowUserPointer(
      mHandle,
      reinterpret_cast<void *>(const_cast<Application *>(application)));

  glfwSetWindowCloseCallback(mHandle, windowCloseCallback);
  glfwSetWindowSizeCallback(mHandle, windowSizeCallback);
  glfwSetWindowFocusCallback(mHandle, windowFocusCallback);
  glfwSetKeyCallback(mHandle, keyCallback);
  glfwSetCursorPosCallback(mHandle, cursorPosCallback);
  glfwSetMouseButtonCallback(mHandle, mouseButtonCallback);

  mExtent.width  = width;
  mExtent.height = height;
}

Window::~Window() {
  logInfo(__func__);
  glfwTerminate();
}

bool Window::shouldClose() { return glfwWindowShouldClose(mHandle); }

void Window::close() { glfwSetWindowShouldClose(mHandle, GLFW_TRUE); }

void Window::pollEvents() { glfwPollEvents(); }

VkSurfaceKHR Window::createSurface(const VkInstance instance) const {
  VkSurfaceKHR surface;
  ASSERT(glfwCreateWindowSurface(instance, mHandle, nullptr, &surface));
  return surface;
}
