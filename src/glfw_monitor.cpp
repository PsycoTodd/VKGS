#include <glfw_monitor.h>
#include <GLFW/glfw3.h>

namespace todd {

  gsl::span<GLFWmonitor*> GetMonitors() {
    std::int32_t monitor_count = 0;
    GLFWmonitor** monitor_pointers = glfwGetMonitors(&monitor_count);
    return gsl::span<GLFWmonitor*>(monitor_pointers, monitor_count);
  }

  glm::ivec2 GetMonitorPosition(gsl::not_null<GLFWmonitor*> monitor) {
    glm::ivec2 monitor_logical_position;
    glfwGetMonitorPos(monitor, &monitor_logical_position.x, &monitor_logical_position.y);
    return monitor_logical_position;
  }

  glm::ivec2 GetMonitorWorkareaSize(gsl::not_null<GLFWmonitor*> monitor) {
    glm::ivec2 monitorSize;
    glfwGetMonitorWorkarea(monitor, nullptr, nullptr, &monitorSize.x, &monitorSize.y);
    return monitorSize;
  }

  void MoveWindowToMonitor(gsl::not_null<GLFWmonitor*> monitor, gsl::not_null<GLFWwindow*> window) {
    glm::ivec2 windowSize;
    glfwGetWindowSize(window, &windowSize.x, &windowSize.y);

    auto monitorSize = GetMonitorWorkareaSize(monitor);

    glm::ivec2 window_new_pos = GetMonitorPosition(monitor) + monitorSize / 2 - windowSize / 2;

    glfwSetWindowPos(window, window_new_pos.x, window_new_pos.y);

  }

}
