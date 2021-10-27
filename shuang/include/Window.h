#pragma once

#include <vulkan/vulkan.hpp>

struct GLFWwindow;
class Application;

class Window {
public:
  Window(const Application *exampleBase, uint32_t width, uint32_t height, const char *title);
  ~Window();

  bool shouldClose();
  void close();
  void pollEvents();

  VkSurfaceKHR      createSurface(VkInstance instance) const;
  const VkExtent2D &getExtent() const { return mExtent; }

private:
  VkExtent2D  mExtent;
  GLFWwindow *mHandle = nullptr;
};
