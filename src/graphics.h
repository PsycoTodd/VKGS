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
  struct QueueFamilyIndices {
    std::optional<std::uint32_t> graphics_family = std::nullopt;
    std::optional<std::uint32_t> presentation_family = std::nullopt;

    bool IsValid() const { return graphics_family.has_value()  && presentation_family.has_value(); };
  };
private:
  void InitializeVulkan();
  void CreateInstance();
  void SetupDebugMessenger();
  void PickPhysicalDevice();
  void CreateLogicalDeviceAndQueues();
  std::vector<gsl::czstring> GetRequiredInstanceExtensions();

  std::vector<VkPhysicalDevice> GetAvailableDevices();
  bool IsDeviceSuitable(VkPhysicalDevice device);
  void CreateSurface();

  static gsl::span<gsl::czstring> GetSuggestedInstanceExtensions();
  static std::vector<VkExtensionProperties> GetSupportedInstanceExtensions();
  static std::vector<VkLayerProperties> GetSupportedValidationLayers();
  static bool AreAllExtensionsSupported(gsl::span<gsl::czstring> extensions);
  static bool AreAllLayersSupported(gsl::span<gsl::czstring> extensions);

  QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);

private:
  VkInstance _instance = VK_NULL_HANDLE;
  VkPhysicalDevice _physical_device = VK_NULL_HANDLE;
  VkDevice _logical_deivce = VK_NULL_HANDLE;
  VkQueue _graphics_queue = VK_NULL_HANDLE;
  VkQueue _present_queue = VK_NULL_HANDLE;
  VkDebugUtilsMessengerEXT _debug_messenger;
  VkSurfaceKHR _surface;

  gsl::not_null<Window*> _window;
  bool _validation_enabled = false;

};
}
