#pragma once

#include <vulkan/vulkan.hpp>

class Instance;
class Window;

class Surface {
public:
  Surface(const std::shared_ptr<Instance> &instance, const std::shared_ptr<Window> &window);
  ~Surface();

  const VkSurfaceKHR &getHandle() const { return mHandle; }
  const VkExtent2D   &getExtent() const { return mExtent; }

private:
  const std::shared_ptr<Instance> &mInstance = nullptr;
  VkSurfaceKHR                     mHandle;
  VkExtent2D                       mExtent;
};
