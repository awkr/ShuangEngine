#pragma once

#include "Instance.h"

class PhysicalDevice {
public:
  explicit PhysicalDevice(const std::shared_ptr<Instance> &instance);
  ~PhysicalDevice();

  const VkPhysicalDevice                 &getHandle() const { return mHandle; }
  const VkPhysicalDeviceProperties       &getProperties() const { return mProperties; }
  const VkPhysicalDeviceFeatures         &getFeatures() const { return mFeatures; }
  const VkPhysicalDeviceMemoryProperties &getMemoryProperties() const { return mMemoryProperties; }
  VkFormat getSuitableDepthFormat(bool depthOnly = true, const std::vector<VkFormat> &formats = {
                                                             VK_FORMAT_D32_SFLOAT,
                                                             VK_FORMAT_D24_UNORM_S8_UINT,
                                                             VK_FORMAT_D16_UNORM,
                                                         }) const;

  /**
   * @brief Checks that a given memory type is supported by the GPU
   * @param bits The memory requirement type bits
   * @param properties The memory property to search for
   * @param found True if found, false if not found
   * @returns The memory type index of the found memory type
   */
  uint32_t getMemoryType(uint32_t bits, VkMemoryPropertyFlags properties,
                         VkBool32 *found = nullptr);

private:
  [[nodiscard]] static bool isDepthOnlyFormat(VkFormat format);

  VkPhysicalDevice                 mHandle;
  VkPhysicalDeviceProperties       mProperties;
  VkPhysicalDeviceFeatures         mFeatures;
  VkPhysicalDeviceMemoryProperties mMemoryProperties;
};
