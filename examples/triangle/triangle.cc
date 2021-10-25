#include <memory>

#include <Application.h>

class Example : public Application {
public:
  Example() : Application() {
    mTitle = "Example: Basic triangle";
    // Setup camera
    mCamera.setPosition(glm::vec3(0, 0, -2.5f));
    mCamera.setPerspective(60.0f, (float)mWidth / (float)mHeight, 1.0f, 256.0f);
    // Values not set here are initialized in the base class constructor
  }

  ~Example() override { log_debug(__func__); }
};

int main() {
  auto example = std::make_unique<Example>();
  example->setup();
  example->mainLoop();
}
