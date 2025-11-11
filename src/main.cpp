#include "precomp.h"
#include "glfw_initialization.h"
#include "glfw_monitor.h"
#include "glfw_window.h"
#include <GLFW/glfw3.h>


std::int32_t main(std::int32_t argc, gsl::zstring* argv)
{
  const todd::GlfwInitialization _glfw;

  todd::Window window("Todd Vulkan Engine", { 800, 600 });
  window.TryMoveToMonitor(0);

  glm::ivec2 window_size = window.GetWindowSize();

  while (!window.ShouldClose()) {
    glfwPollEvents();
  }

  return EXIT_SUCCESS;
}
