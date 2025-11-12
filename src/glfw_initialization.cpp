#include "glfw_initialization.h"
#include <GLFW/glfw3.h>
#include <cstdlib>
#include <precomp.h>

namespace todd {

  GlfwInitialization::GlfwInitialization() {
    if (glfwInit() != GLFW_TRUE) {
      std::exit(EXIT_FAILURE);
    }
  }

  GlfwInitialization::~GlfwInitialization() {
    glfwTerminate();
  }



}
