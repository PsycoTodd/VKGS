#include <precomp.h>
#include <glfw_window.h>
#include <graphics.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <spdlog/spdlog.h>

#pragma region VK_FUNCTION_EXT_IMPL

VKAPI_ATTR VkResult VKAPI_CALL vkCreateDebugUtilsMessengerEXT(
  VkInstance instance,
  const VkDebugUtilsMessengerCreateInfoEXT* info,
  const VkAllocationCallbacks* allocator,
  VkDebugUtilsMessengerEXT* debug_messenger
) {
  PFN_vkCreateDebugUtilsMessengerEXT function =
    reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));
  if (function != nullptr) {
    function(instance, info, allocator, debug_messenger);
  }
  else {
    return VK_ERROR_EXTENSION_NOT_PRESENT;
  }
}

VKAPI_ATTR void VKAPI_CALL vkDestroyDebugUtilsMessengerEXT(
  VkInstance instance,
  VkDebugUtilsMessengerEXT debug_messenger,
  const VkAllocationCallbacks* allocator
) {
  PFN_vkDestroyDebugUtilsMessengerEXT function =
    reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"));
  if (function != nullptr) {
    function(instance, debug_messenger, allocator);
  }
}

#pragma endregion

namespace todd {
#pragma region VALIDATION_LAYERS
  static VKAPI_ATTR VkBool32 VKAPI_CALL ValidationCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT severity,
    VkDebugUtilsMessageTypeFlagsEXT type,
    const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
    void* user_data)
  {
    if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
      spdlog::warn("Vulkan Validation: {}", callback_data->pMessage);
    }
    else {
      spdlog::error("Vulkan Error: {}", callback_data->pMessage);
    }
    return VK_FALSE;
  }

  static VkDebugUtilsMessengerCreateInfoEXT GetCreateMessagerInfo() {
    VkDebugUtilsMessengerCreateInfoEXT creation_info = {};
    creation_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    creation_info.pNext = nullptr;
    creation_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                    VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    creation_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;

    creation_info.pfnUserCallback = ValidationCallback;
    creation_info.pUserData = nullptr;
    return creation_info;
  }

  void Graphics::SetupDebugMessenger() {
    if (!_validation_enabled) {
      return;
    }
    VkDebugUtilsMessengerCreateInfoEXT info = GetCreateMessagerInfo();
    VkResult result = vkCreateDebugUtilsMessengerEXT(_instance, &info, nullptr, &_debug_messenger);
    if (result != VK_SUCCESS) {
      spdlog::error("Cannot create debug messenger.");
      return;
    }
  }

#pragma endregion

  Graphics::Graphics(gsl::not_null<Window*> window):_window(window)
  {
#if !defined(NDEBUG)
    _validation_enabled = true;
#endif

    InitializeVulkan();
  }
  Graphics::~Graphics()
  {
    if (_logical_deivce != nullptr) {
      vkDestroyDevice(_logical_deivce, nullptr);
    }

    if (_instance) {
      if (_surface != VK_NULL_HANDLE) {
        vkDestroySurfaceKHR(_instance, _surface, nullptr);
      }
      if (_debug_messenger) {
        vkDestroyDebugUtilsMessengerEXT(_instance, _debug_messenger, nullptr);
      }
      vkDestroyInstance(_instance, nullptr);
    }
  }

  void Graphics::InitializeVulkan()
  {
    CreateInstance();
    SetupDebugMessenger();
    CreateSurface();
    PickPhysicalDevice();
    CreateLogicalDeviceAndQueues();
  }

  gsl::span<gsl::czstring> Graphics::GetSuggestedInstanceExtensions() {
    std::uint32_t glfw_extensionCount = 0;
    gsl::czstring* glfw_extensions;
    glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extensionCount);
    return { glfw_extensions, glfw_extensionCount };
  }

  std::vector<VkExtensionProperties> Graphics::GetSupportedInstanceExtensions() {
    std::uint32_t count;
    vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr);

    if (count == 0) {
      return {};
    }

    std::vector<VkExtensionProperties> properties(count);
    vkEnumerateInstanceExtensionProperties(nullptr, &count, properties.data());

    return properties;
  }

  bool ExtensionMatchesName(const gsl::czstring& name, const VkExtensionProperties& property) {
    return streq(property.extensionName, name);
  }

  bool IsExtensionSupported(const gsl::span<VkExtensionProperties>& extensions,
                            const gsl::czstring& name)
  {
    return std::any_of(extensions.begin(), extensions.end(), std::bind_front(ExtensionMatchesName, name));
  }

  bool Graphics::AreAllExtensionsSupported(gsl::span<gsl::czstring> extensions)
  {
    std::vector<VkExtensionProperties> supported_extensions = GetSupportedInstanceExtensions();
    return std::all_of(extensions.begin(), extensions.end(), std::bind_front(IsExtensionSupported, supported_extensions));
  }

  std::vector<gsl::czstring> Graphics::GetRequiredInstanceExtensions()
  {
    gsl::span<gsl::czstring> suggested_extensions = GetSuggestedInstanceExtensions();
    std::vector<gsl::czstring> required_extensions(suggested_extensions.size());
    std::copy(suggested_extensions.begin(), suggested_extensions.end(), required_extensions.begin());

    if (_validation_enabled) {
      required_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    if (!AreAllExtensionsSupported(required_extensions)) {
      std::exit(EXIT_FAILURE);
    }
    return required_extensions;
  }

  bool LayerMatchesName(const gsl::czstring& name, const VkLayerProperties& property) {
    return streq(property.layerName, name);
  }

  bool IsLayerSupported(const gsl::span<VkLayerProperties>& layers,
                        const gsl::czstring& name)
  {
    return std::any_of(layers.begin(), layers.end(), std::bind_front(LayerMatchesName, name));
  }

  bool Graphics::AreAllLayersSupported(gsl::span<gsl::czstring> layers)
  {
    std::vector<VkLayerProperties> supported_layers = GetSupportedValidationLayers();
    return std::all_of(layers.begin(), layers.end(), std::bind_front(IsLayerSupported, supported_layers));
  }

  std::vector<VkLayerProperties> Graphics::GetSupportedValidationLayers()
  {
    std::uint32_t count;
    vkEnumerateInstanceLayerProperties(&count, nullptr);

    if (count == 0) {
      return {};
    }

    std::vector<VkLayerProperties> properties(count);
    vkEnumerateInstanceLayerProperties(&count, properties.data());

    return properties;
  }

  void Graphics::CreateInstance()
  {
    std::array<gsl::czstring, 1> validation_layers = {"VK_LAYER_KHRONOS_validation"};
    if (!AreAllLayersSupported(validation_layers)) {
      _validation_enabled = false;
    }
    std::vector<gsl::czstring> required_extensions = GetRequiredInstanceExtensions();

    VkApplicationInfo app_info = {};
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pNext = nullptr;
    app_info.pApplicationName = "VK GS";
    app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.pEngineName = "vEng";
    app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo instance_creation_info = {};
    instance_creation_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instance_creation_info.pNext = nullptr;
    instance_creation_info.pApplicationInfo = &app_info;
    instance_creation_info.enabledExtensionCount = required_extensions.size();
    instance_creation_info.ppEnabledExtensionNames = required_extensions.data();

    VkDebugUtilsMessengerCreateInfoEXT messager_creation_info = GetCreateMessagerInfo();
    if (_validation_enabled) {
      instance_creation_info.pNext = &messager_creation_info;
      instance_creation_info.enabledLayerCount = validation_layers.size();
      instance_creation_info.ppEnabledLayerNames = validation_layers.data();
    }
    else {
      instance_creation_info.enabledLayerCount = 0;
      instance_creation_info.ppEnabledLayerNames = nullptr;
    }

    VkResult result = vkCreateInstance(&instance_creation_info, nullptr, &_instance);
    if (result != VK_SUCCESS) {
      std::exit(EXIT_FAILURE);
    }
  }

#pragma region DEVCIES_AND_QUEUES

  bool Graphics::IsDeviceSuitable(VkPhysicalDevice device) {

    QueueFamilyIndices families = FindQueueFamilies(device);
    return families.IsValid();
  }

  void Graphics::PickPhysicalDevice() {
    std::vector<VkPhysicalDevice> devices = GetAvailableDevices();

    std::erase_if(devices, std::not_fn(std::bind_front(&Graphics::IsDeviceSuitable, this)));
    if (devices.empty()) {
      spdlog::error("No physical devices that match the criteria.");
      std::exit(EXIT_FAILURE);
    }


    
    _physical_device = devices[0];
  }

  std::vector<VkPhysicalDevice> Graphics::GetAvailableDevices() {
    std::uint32_t device_count;
    vkEnumeratePhysicalDevices(_instance, &device_count, nullptr);
    if (!device_count) {
      return {};
    }
    std::vector<VkPhysicalDevice> devices(device_count);
    vkEnumeratePhysicalDevices(_instance, &device_count, devices.data());
    return devices;
  }

  Graphics::QueueFamilyIndices Graphics::FindQueueFamilies(VkPhysicalDevice device) {
    std::uint32_t queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, nullptr);
    std::vector<VkQueueFamilyProperties> families(queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, families.data());

    auto graphics_family_it = std::find_if(families.begin(), families.end(), [](const VkQueueFamilyProperties& props) {
      return props.queueFlags & (VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_TRANSFER_BIT);
    });

    QueueFamilyIndices result;
    result.graphics_family = graphics_family_it - families.begin();

    for (std::uint32_t i = 0; i < families.size(); ++i) {
      VkBool32 has_presentation_support = false;
      vkGetPhysicalDeviceSurfaceSupportKHR(device, i, _surface, &has_presentation_support);
      if (has_presentation_support) {
        result.presentation_family = i;
        break;
      }
    }

    return result;
  }

  void Graphics::CreateLogicalDeviceAndQueues() {
    QueueFamilyIndices picked_device_families = FindQueueFamilies(_physical_device);
    if (!picked_device_families.IsValid()) {
      std::exit(EXIT_FAILURE);
    }

    std::set<std::uint32_t> unique_queue_faimilies = {
      picked_device_families.graphics_family.value(),
    picked_device_families.presentation_family.value()};

    std::float_t queue_priority = 1.0f;

    std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
    for (std::uint32_t unique_queue_family : unique_queue_faimilies) {
      VkDeviceQueueCreateInfo queue_info = {};
      queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
      queue_info.queueFamilyIndex = unique_queue_family;
      queue_info.queueCount = 1;
      queue_info.pQueuePriorities = &queue_priority;
      queue_create_infos.push_back(std::move(queue_info));
    }

    VkPhysicalDeviceFeatures required_features = {};
    VkDeviceCreateInfo device_info = {};
    device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_info.queueCreateInfoCount = queue_create_infos.size();
    device_info.pQueueCreateInfos = queue_create_infos.data();
    device_info.pEnabledFeatures = &required_features;
    device_info.enabledExtensionCount = 0;
    device_info.enabledLayerCount = 0; // deprecated

    VkResult res = vkCreateDevice(_physical_device, &device_info, nullptr, &_logical_deivce);
    if (res != VK_SUCCESS) {
      std::exit(EXIT_FAILURE);
    }
    vkGetDeviceQueue(_logical_deivce, picked_device_families.graphics_family.value(), 0, &_graphics_queue);
    vkGetDeviceQueue(_logical_deivce, picked_device_families.presentation_family.value(), 0, &_present_queue);
  }

#pragma endregion

#pragma region PRESENTATION
  void Graphics::CreateSurface() {
    VkResult result = glfwCreateWindowSurface(_instance, _window->GetHandler().get(), nullptr, &_surface);
    if (result != VK_SUCCESS) {
      std::exit(EXIT_FAILURE);
    }
  }
#pragma endregion

}
