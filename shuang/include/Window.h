#pragma once

#include <vulkan/vulkan.hpp>

struct GLFWwindow;
class Application;

class Window {
public:
  Window(const Application *exampleBase, int width, int height, const char *title);
  ~Window();

  bool shouldClose();
  void close();
  void pollEvents();

  VkSurfaceKHR      createSurface(VkInstance instance) const;
  const VkExtent2D &getExtent() const { return mExtent; }
  double            getTime() const;

private:
  VkExtent2D  mExtent;
  GLFWwindow *mHandle = nullptr;
};
