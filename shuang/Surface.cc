#include "Surface.h"
#include "Instance.h"
#include "Logger.h"
#include "Window.h"

Surface::Surface(const Instance                *instance,
                 const std::shared_ptr<Window> &window)
    : mInstance{instance} {
  mHandle = window->createSurface(instance->getHandle());
  mExtent = window->getExtent();
}

Surface::~Surface() {
  logInfo(__func__);
  vkDestroySurfaceKHR(mInstance->getHandle(), mHandle, nullptr);
}
