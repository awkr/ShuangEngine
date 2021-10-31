#include "Window.h"
#include "Application.h"
#include "Logger.h"
#include "Macros.h"

#include <stdexcept>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

namespace {

void errorCallback(int error, const char *description) {
  log_error("GLFW error: code {}: {}", error, description);
}

void windowCloseCallback(GLFWwindow *window) { glfwSetWindowShouldClose(window, GLFW_TRUE); }

void windowSizeCallback(GLFWwindow *window, int width, int height) {
  if (auto application = reinterpret_cast<Application *>(glfwGetWindowUserPointer(window))) {
    application->resize(width, height);
  }
}

void windowFocusCallback(GLFWwindow *window, int focused) {}

void keyCallback(GLFWwindow *window, int key, int /* scancode */, int action, int /* mods */) {
  auto application = reinterpret_cast<Application *>(glfwGetWindowUserPointer(window));
  if (!application) {
    return;
  }

  KeyCode keyCode;
  switch (key) {
  case GLFW_KEY_ESCAPE:
    keyCode = KeyCode::ESCAPE;
    break;
  case GLFW_KEY_UP:
    keyCode = KeyCode::UP;
    break;
  case GLFW_KEY_DOWN:
    keyCode = KeyCode::DOWN;
    break;
  case GLFW_KEY_LEFT:
    keyCode = KeyCode::LEFT;
    break;
  case GLFW_KEY_RIGHT:
    keyCode = KeyCode::RIGHT;
    break;
  case GLFW_KEY_H:
    keyCode = KeyCode::H;
    break;
  case GLFW_KEY_L:
    keyCode = KeyCode::L;
    break;
  case GLFW_KEY_J:
    keyCode = KeyCode::J;
    break;
  case GLFW_KEY_K:
    keyCode = KeyCode::K;
    break;
  case GLFW_KEY_W:
    keyCode = KeyCode::W;
    break;
  case GLFW_KEY_S:
    keyCode = KeyCode::S;
    break;
  case GLFW_KEY_A:
    keyCode = KeyCode::A;
    break;
  case GLFW_KEY_D:
    keyCode = KeyCode::D;
    break;
  default:
    keyCode = KeyCode::NONE;
  }

  KeyAction keyAction;
  switch (action) {
  case GLFW_PRESS:
    keyAction = KeyAction::DOWN;
    break;
  case GLFW_RELEASE:
    keyAction = KeyAction::UP;
    break;
  default:
    keyAction = KeyAction::NONE;
  }

  application->handleEvent(KeyInputEvent{keyCode, keyAction});
}

void cursorPosCallback(GLFWwindow *window, double xPos, double yPos) {}

void mouseButtonCallback(GLFWwindow *window, int button, int action, int /* mods */) {}

} // namespace

Window::Window(const Application *application, uint32_t width, uint32_t height, const char *title) {
  if (!glfwInit()) {
    throw std::runtime_error("Failed to init GLFW");
  }

  glfwSetErrorCallback(errorCallback);
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

  if (mHandle = glfwCreateWindow(width, height, title, nullptr, nullptr); !mHandle) {
    throw std::runtime_error("Failed to create GLFW");
  }

  glfwSetWindowUserPointer(mHandle,
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
  log_func;
  glfwTerminate();
}

bool Window::shouldClose() { return glfwWindowShouldClose(mHandle); }

void Window::close() { glfwSetWindowShouldClose(mHandle, GLFW_TRUE); }

void Window::pollEvents() { glfwPollEvents(); }

VkSurfaceKHR Window::createSurface(const VkInstance instance) const {
  VkSurfaceKHR surface;
  vkAssert(glfwCreateWindowSurface(instance, mHandle, nullptr, &surface));
  return surface;
}

double Window::getTime() const { return glfwGetTime(); }
