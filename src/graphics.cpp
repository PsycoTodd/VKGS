#include <precomp.h>
#include <glfw_window.h>
#include <graphics.h>
#include <GLFW/glfw3.h>
#include <iostream>

namespace todd {

  static VKAPI_ATTR VkBool32 VKAPI_CALL ValidationCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT severity,
    VkDebugUtilsMessageTypeFlagsEXT type,
    const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
    void* user_data)
  {
    if (severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
      std::cerr << "Validation Error: " << callback_data->pMessage << std::endl;
    }
    else {
      std::cout << "Validation Message: " << callback_data->pMessage << std::endl;
    }
    return VK_FALSE;
  }

  static VkDebugUtilsMessengerCreateInfoEXT GetCreateMessagerInfo() {
    VkDebugUtilsMessengerCreateInfoEXT creation_info = {};
    creation_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    creation_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                    VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
                                    VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;
    creation_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;

    creation_info.pfnUserCallback = ValidationCallback;
    creation_info.pUserData = nullptr;
    return creation_info;
  }

  Graphics::Graphics(gsl::not_null<Window*> window):_window(window)
  {
#if !defined(NDEBUG)
    _validation_enabled = true;
#endif

    InitializeVulkan();
  }
  Graphics::~Graphics()
  {
    if (_instance) {
      vkDestroyInstance(_instance, nullptr);
    }
  }

  void Graphics::InitializeVulkan()
  {
    CreateInstance();
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
    std::array<gsl::czstring, 2> validation_layers = { "VK_LAYER_KHRONOS_validation", "VK_EXT_debug_utils"};
    if (!AreAllLayersSupported(validation_layers)) {
      _validation_enabled = false;
    }


    gsl::span<gsl::czstring> suggested_extensions = GetSuggestedInstanceExtensions();

    if (!AreAllExtensionsSupported(suggested_extensions)) {
      std::exit(EXIT_FAILURE);
    }

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
    instance_creation_info.enabledExtensionCount = suggested_extensions.size();
    instance_creation_info.ppEnabledExtensionNames = suggested_extensions.data();

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
}
