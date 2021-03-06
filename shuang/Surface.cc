#include "Surface.h"
#include "Instance.h"
#include "Log.h"
#include "Window.h"

Surface::Surface(const std::shared_ptr<Instance> &instance, const std::shared_ptr<Window> &window)
    : mInstance{instance} {
  mHandle = window->createSurface(instance->getHandle());
  mExtent = window->getExtent();
}

Surface::~Surface() {
  log_func;
  vkDestroySurfaceKHR(mInstance->getHandle(), mHandle, nullptr);
}
