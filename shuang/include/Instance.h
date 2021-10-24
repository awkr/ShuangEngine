#pragma once

#include <vulkan/vulkan.hpp>

class Instance {
public:
  explicit Instance(const char *applicationName, bool enableValidation = true);
  ~Instance();

  const VkInstance &getHandle() const { return mHandle; }

private:
  std::vector<std::string>  mSupportedInstanceExtensions;
  std::vector<const char *> mRequestedInstanceExtensions;
  VkInstance                mHandle;
  bool                      mEnableValidation;
};
