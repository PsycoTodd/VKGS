#include "glfw_window.h"
#include "glfw_monitor.h"
#include <GLFW/glfw3.h>

namespace todd {

  Window::Window(const gsl::czstring& name, const glm::ivec2& size):_window(glfwCreateWindow(size.x, size.y, name, nullptr, nullptr), [](GLFWwindow* window) {glfwDestroyWindow(window);})
  {
    if (_window == nullptr) {
      std::exit(EXIT_FAILURE);
    }
  }

  glm::vec2 Window::GetWindowSize() const {
    glm::ivec2 window_size;
    glfwGetWindowSize(_window.get(), &window_size.x, &window_size.y);
    return std::move(window_size);
  }

  bool Window::ShouldClose() const {
    return glfwWindowShouldClose(_window.get());
  }

  std::shared_ptr<GLFWwindow> Window::GetHandler() const {
    return _window;
  }

  bool Window::TryMoveToMonitor(std::uint16_t monitor_number) {
    gsl::span<GLFWmonitor*> monitors = todd::GetMonitors();

    if (monitors.size() > monitor_number) {
      todd::MoveWindowToMonitor(monitors[monitor_number], _window.get());
      return true;
    }
    return false;
  }

}
