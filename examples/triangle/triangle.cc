#include <VulkanExampleBase.h>

class VulkanExample : public VulkanExampleBase {
public:
  VulkanExample() : VulkanExampleBase() {}
  ~VulkanExample() {}

  void init() {
    initWindow();
    initVulkan();
  }
  void prepare() {}
  void mainLoop() {}

private:
  void initWindow() {}
  void initVulkan() {}
};

int main() {
  VulkanExample *example = new VulkanExample();
  example->init();
  example->prepare();
  example->mainLoop();
  delete example;
}
