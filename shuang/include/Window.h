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

  VkSurfaceKHR                    createSurface(VkInstance instance) const;
  [[nodiscard]] const VkExtent2D &getExtent() const { return mExtent; }
  [[nodiscard]] double            getTime() const;
  void                            setFocused(bool focused) { mFocused = focused; }
  [[nodiscard]] bool              getFocused() const { return mFocused; }

private:
  VkExtent2D  mExtent{};
  GLFWwindow *mHandle  = nullptr;
  bool        mFocused = false;
};
