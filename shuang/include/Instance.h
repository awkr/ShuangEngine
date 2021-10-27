#pragma once

#include <vulkan/vulkan.hpp>

class Instance {
public:
  Instance(const char *applicationName, const std::vector<const char *> &requiredExtensions = {},
           bool enableValidation = true);
  ~Instance();

  const VkInstance &getHandle() const { return mHandle; }

private:
  bool        isExtensionSupport(const char *extension);
  bool        isLayerSupport(const char *name);
  static bool equals(const char *a, const char *b);

  bool                           mEnableValidation;
  std::vector<std::string>       mSupportedExtensions;
  std::vector<VkLayerProperties> mSupportedLayers;
  VkInstance                     mHandle;
};
