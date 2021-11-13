#include <memory>

#include <Application.h>

class Example : public Application {
public:
  Example() : Application() {
    mTitle = "Example: Basic triangle";

    // Values not set here are initialized in the base class constructor
  }

  ~Example() override { log_debug(__func__); }
};

int main() {
  try {
    auto example = std::make_unique<Example>();
    example->setup();
    example->mainLoop();
  } catch (const std::exception &e) {
    log_error("exception caught: {}", e.what());
  }
}
