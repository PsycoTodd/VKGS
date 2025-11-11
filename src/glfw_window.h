#pragma once
#include <memory>

struct GLFWwindow;

namespace todd {
  class Window {
  public:
    Window(const gsl::czstring& name, const glm::ivec2& size);
    ~Window() = default;
    glm::vec2 GetWindowSize() const;
    bool ShouldClose() const;
    std::shared_ptr<GLFWwindow> GetHandler() const;
    bool TryMoveToMonitor(std::uint16_t monitor_number);

  private:
    std::shared_ptr<GLFWwindow> _window;
  };

}
