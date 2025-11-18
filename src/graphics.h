#pragma once

#include <vulkan/vulkan.h>
#include <vector>

namespace todd {

class Window;

class Graphics final {
public:
  Graphics(gsl::not_null<Window*> window);
  ~Graphics();
private:
  void InitializeVulkan();
  void CreateInstance();
  void SetupDebugMessenger();
  std::vector<gsl::czstring> GetRequiredInstanceExtensions();

  static gsl::span<gsl::czstring> GetSuggestedInstanceExtensions();
  static std::vector<VkExtensionProperties> GetSupportedInstanceExtensions();
  static std::vector<VkLayerProperties> GetSupportedValidationLayers();
  static bool AreAllExtensionsSupported(gsl::span<gsl::czstring> extensions);
  static bool AreAllLayersSupported(gsl::span<gsl::czstring> extensions);

private:
  VkInstance _instance = nullptr;
  VkDebugUtilsMessengerEXT _debug_messenger;
  gsl::not_null<Window*> _window;
  bool _validation_enabled = false;

};
}
