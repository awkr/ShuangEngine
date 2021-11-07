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
  if (auto platform = reinterpret_cast<Application *>(glfwGetWindowUserPointer(window))) {
    platform->resize(width, height);
  }
}

void windowFocusCallback(GLFWwindow *window, int focused) {
  if (auto platform = reinterpret_cast<Application *>(glfwGetWindowUserPointer(window))) {
    platform->setFocus(focused);
  }
}

KeyCode mapKeyCode(int key) {
  static const std::unordered_map<int, KeyCode> lookup = {
      {GLFW_KEY_ESCAPE, KeyCode::ESCAPE}, {GLFW_KEY_UP, KeyCode::UP},
      {GLFW_KEY_DOWN, KeyCode::DOWN},     {GLFW_KEY_LEFT, KeyCode::LEFT},
      {GLFW_KEY_RIGHT, KeyCode::RIGHT},   {GLFW_KEY_H, KeyCode::H},
      {GLFW_KEY_L, KeyCode::L},           {GLFW_KEY_J, KeyCode::J},
      {GLFW_KEY_K, KeyCode::K},           {GLFW_KEY_W, KeyCode::W},
      {GLFW_KEY_R, KeyCode::R},           {GLFW_KEY_S, KeyCode::S},
      {GLFW_KEY_A, KeyCode::A},           {GLFW_KEY_D, KeyCode::D},
      {GLFW_KEY_Q, KeyCode::Q},           {GLFW_KEY_E, KeyCode::E},
      {GLFW_KEY_F, KeyCode::F},           {GLFW_KEY_I, KeyCode::I},
      {GLFW_KEY_U, KeyCode::U},           {GLFW_KEY_O, KeyCode::O},
      {GLFW_KEY_F5, KeyCode::F5},
  };
  auto iter = lookup.find(key);
  if (iter == lookup.end()) {
    return KeyCode::UNKNOWN;
  }
  return iter->second;
}

KeyAction mapKeyAction(int action) {
  static const std::unordered_map<int, KeyAction> lookup = {
      {GLFW_PRESS, KeyAction::DOWN},
      {GLFW_RELEASE, KeyAction::UP},
      {GLFW_REPEAT, KeyAction::REPEAT},
  };
  auto iter = lookup.find(action);
  if (iter == lookup.end()) {
    return KeyAction::UNKNOWN;
  }
  return iter->second;
}

MouseAction mapMouseAction(int action) {
  static const std::unordered_map<int, MouseAction> lookup = {
      {GLFW_PRESS, MouseAction::DOWN},
      {GLFW_RELEASE, MouseAction::UP},
  };
  auto iter = lookup.find(action);
  if (iter == lookup.end()) {
    return MouseAction::UNKNOWN;
  }
  return iter->second;
}

MouseButton mapMouseButton(int button) {
  static const std::unordered_map<int, MouseButton> lookup = {
      {GLFW_MOUSE_BUTTON_1, MouseButton::LEFT},    {GLFW_MOUSE_BUTTON_2, MouseButton::RIGHT},
      {GLFW_MOUSE_BUTTON_3, MouseButton::MIDDLE},  {GLFW_MOUSE_BUTTON_4, MouseButton::BACK},
      {GLFW_MOUSE_BUTTON_5, MouseButton::FORWARD},
  };
  auto iter = lookup.find(button);
  if (iter == lookup.end()) {
    return MouseButton::UNKNOWN;
  }
  return iter->second;
}

void keyCallback(GLFWwindow *window, int key, int /* scancode */, int action, int /* mods */) {
  if (auto platform = reinterpret_cast<Application *>(glfwGetWindowUserPointer(window))) {
    auto keyCode   = mapKeyCode(key);
    auto keyAction = mapKeyAction(action);

    platform->handleEvent(KeyInputEvent{keyCode, keyAction});
  }
}

void cursorPosCallback(GLFWwindow *window, double xPos, double yPos) {
  if (auto platform = reinterpret_cast<Application *>(glfwGetWindowUserPointer(window))) {
    platform->handleEvent(MouseButtonInputEvent{MouseButton::UNKNOWN, MouseAction::MOVE,
                                                static_cast<float>(xPos),
                                                static_cast<float>(yPos)});
  }
}

void mouseButtonCallback(GLFWwindow *window, int button, int action, int mods) {
  if (auto platform = reinterpret_cast<Application *>(glfwGetWindowUserPointer(window))) {
    double x, y;
    glfwGetCursorPos(window, &x, &y);

    auto mouseButton = mapMouseButton(button);
    auto mouseAction = mapMouseAction(action);

    platform->handleEvent(MouseButtonInputEvent{mouseButton, mouseAction, static_cast<float>(x),
                                                static_cast<float>(y)});
  }
}

void onScroll(GLFWwindow *window, double xOffset, double yOffset) {
  if (auto platform = reinterpret_cast<Application *>(glfwGetWindowUserPointer(window))) {
    platform->handleEvent(MouseButtonInputEvent{MouseButton::MIDDLE, MouseAction::SCROLL,
                                                static_cast<float>(xOffset),
                                                static_cast<float>(yOffset)});
  }
}

} // namespace

Window::Window(const Application *application, int width, int height, const char *title) {
  if (!glfwInit()) {
    throw std::runtime_error("Failed to init GLFW");
  }

  glfwSetErrorCallback(errorCallback);
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

  //  auto       *monitor = glfwGetPrimaryMonitor();
  //  const auto *mode    = glfwGetVideoMode(monitor);
  //  glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);

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
  glfwSetScrollCallback(mHandle, onScroll);

  mExtent.width  = width;
  mExtent.height = height;

  mFocused = true;
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
