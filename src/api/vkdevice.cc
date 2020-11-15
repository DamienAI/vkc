#include <vk/api/vkdevice.h>
#include <vk/api/vkutils.h>

#include <vulkan/vulkan.h>

#include <algorithm>
#include <vector>
#include <iostream>
#include <stdexcept>
#include <cstring>
#include <set>

namespace Vk {
  namespace api {
    const char* validationLayerName = "VK_LAYER_LUNARG_standard_validation";

    /**
     * 
     * 
     */
    std::vector<const char*> getExtensionsList(bool needCudaInterop)
    {
      // No rendering so nothing to add here.
      // For rendering we would use at least the swapchain support.
      const std::vector<const char*> deviceExtensions = {};

      const std::vector<const char*> deviceExtensionsForCudaInterop = {
        VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME,
        VK_KHR_EXTERNAL_SEMAPHORE_EXTENSION_NAME,
        VK_KHR_EXTERNAL_MEMORY_FD_EXTENSION_NAME,
        VK_KHR_EXTERNAL_SEMAPHORE_FD_EXTENSION_NAME,
      };

      std::vector<const char*> extensions = deviceExtensions;
      if (needCudaInterop) {
        extensions.insert(extensions.end(), deviceExtensionsForCudaInterop.begin(), deviceExtensionsForCudaInterop.end());
      }

      return extensions;
    }

    bool hasNeededExtensions(VkPhysicalDevice device, bool needCudaInterop)
    {
      uint32_t extensionCount;
      vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

      std::vector<VkExtensionProperties> availableExtensions(extensionCount);
      vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

      auto deviceExtensions = getExtensionsList(needCudaInterop);
      std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

      for (const auto& extension : availableExtensions) {
        requiredExtensions.erase(extension.extensionName);
      }

      return requiredExtensions.empty();
    }

    std::vector<std::string> getInstanceLayersNames()
    {
      uint32_t layersCount = 0;
      std::vector<std::string> layers;

      vkEnumerateInstanceLayerProperties(&layersCount, nullptr);

      std::vector<VkLayerProperties> availableLayers(layersCount);
      vkEnumerateInstanceLayerProperties(&layersCount, availableLayers.data());

      std::for_each(availableLayers.begin(), availableLayers.end(), [&layers](VkLayerProperties& p) {
        layers.push_back(std::string(p.layerName));
      });

      return layers;
    }

    VkInstance createInstance(bool enableValidationLayers)
    {
      VkInstance instance;

      VkApplicationInfo appInfo = {};
      appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
      appInfo.pApplicationName = "Vulkan Compute interface";
      appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
      appInfo.pEngineName = "TheBestOne";
      appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
      appInfo.apiVersion = VK_API_VERSION_1_0;

      VkInstanceCreateInfo createInfo = {};
      createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
      createInfo.pApplicationInfo = &appInfo;
      createInfo.enabledLayerCount = 0;

      std::vector<const char*> enabledExtensionNameList;
      enabledExtensionNameList.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
      enabledExtensionNameList.push_back(VK_KHR_EXTERNAL_MEMORY_CAPABILITIES_EXTENSION_NAME);
      enabledExtensionNameList.push_back(VK_KHR_EXTERNAL_SEMAPHORE_CAPABILITIES_EXTENSION_NAME);

      if (enableValidationLayers) {
        const auto layers = getInstanceLayersNames();
        auto layer = std::find(layers.begin(), layers.end(), validationLayerName);
        if (layer == layers.end()) {
          throw std::runtime_error("Validation layer (VK_LAYER_LUNARG_standard_validation) not found");
        }
        enabledExtensionNameList.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
        createInfo.enabledLayerCount = 1;
        createInfo.ppEnabledLayerNames = &validationLayerName;
      }

      createInfo.enabledExtensionCount = enabledExtensionNameList.size();
      createInfo.ppEnabledExtensionNames = enabledExtensionNameList.data();

      utils::validateResult(vkCreateInstance(&createInfo, nullptr, &instance), "vkCreateInstance");

      return instance;
    }

    std::tuple<VkPhysicalDevice, VkPhysicalDeviceMemoryProperties, VkPhysicalDeviceProperties> findDevice(VkInstance instance) {
      uint32_t devicesCount = 0;
      utils::validateResult(vkEnumeratePhysicalDevices(instance, &devicesCount, nullptr), "vkEnumeratePhysicalDevices");
      if (devicesCount == 0) {
        throw std::runtime_error("No device available, aborting");
      }

      std::vector<VkPhysicalDevice> devices(devicesCount);
      utils::validateResult(vkEnumeratePhysicalDevices(instance, &devicesCount, devices.data()), "vkEnumeratePhysicalDevices");

      for (const auto& device : devices) {
        if (!hasNeededExtensions(device, false)) {
          continue;
        }

        // TODO we do not check for rendering caps here we just create a compute device
        VkPhysicalDeviceMemoryProperties memoryProperties = {};
        vkGetPhysicalDeviceMemoryProperties(device, &memoryProperties);

        VkPhysicalDeviceProperties physicalDeviceProperties;
        vkGetPhysicalDeviceProperties(device, &physicalDeviceProperties);

        return std::make_tuple(device, memoryProperties, physicalDeviceProperties);
      }

      throw std::runtime_error("No suitable device found, aborting");
    }

    uint32_t getComputeQueueFamilyIndex(VkPhysicalDevice device) {
      uint32_t count = 0;
      vkGetPhysicalDeviceQueueFamilyProperties(device, &count, nullptr);

      std::vector<VkQueueFamilyProperties> queues(count);
      vkGetPhysicalDeviceQueueFamilyProperties(device, &count, queues.data());

      uint32_t index = 0;
      for (; index < queues.size(); ++index) {
        auto props = queues[index];
        if (props.queueCount > 0 && props.queueFlags & VK_QUEUE_COMPUTE_BIT) {
          break;
        }
      }
      if (index == queues.size()) {
        throw std::runtime_error("Cannot find a compute queue for the selected device");
      }

      return index;
    }

    std::pair<VkDevice, VkQueue> createDevice(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex, bool enableValidationLayers) {
      float queuePriorities = 1.0;
      
      VkDeviceQueueCreateInfo queueCreateInfo = {
        VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        nullptr,
        0,
        queueFamilyIndex,
        1,
        &queuePriorities
      };

      std::vector<const char*> enabledLayers = {};
      if (enableValidationLayers) {
        enabledLayers.push_back(validationLayerName);
      }

      VkPhysicalDeviceFeatures deviceFeatures = {};
      VkDeviceCreateInfo deviceCreateInfo = {
        VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        nullptr,
        0,
        1,
        &queueCreateInfo,
        static_cast<uint32_t>(enabledLayers.size()),
        enabledLayers.data(),
        0,
        nullptr,
        &deviceFeatures,
      };

      VkDevice device;
      utils::validateResult(vkCreateDevice(physicalDevice, &deviceCreateInfo, NULL, &device), "vkCreateDevice");

      VkQueue queue;
      vkGetDeviceQueue(device, queueFamilyIndex, 0, &queue);

      return std::pair<VkDevice, VkQueue>(device, queue);
    }

    struct DeviceData {
      VkInstance instance;
      VkDevice device;
      uint32_t computeQueueFamilyIndex;
      VkQueue computeQueue;
      VkPhysicalDevice physicalDevice;
      VkPhysicalDeviceMemoryProperties memoryProperties;
      VkPhysicalDeviceProperties physicalDeviceProperties;
    };

    Device::Device(std::unique_ptr<DeviceData> deviceData)
    :data(std::move(deviceData))
    {}

    Device::Device() = default;
    Device::Device(Device&&) = default;
    Device::~Device() = default;
    Device& Device::operator=(Device&&) = default;

    auto Device::findFirstAvailable(bool enableValidationLayers) -> Device {
      auto data = std::make_unique<DeviceData>();

      data->instance = createInstance(enableValidationLayers);
      auto physicialDeviceInfo = findDevice(data->instance);
      data->physicalDevice = std::get<0>(physicialDeviceInfo);
      data->memoryProperties = std::get<1>(physicialDeviceInfo);
      data->physicalDeviceProperties = std::get<2>(physicialDeviceInfo);

      data->computeQueueFamilyIndex = getComputeQueueFamilyIndex(data->physicalDevice);
      auto deviceInfo = createDevice(data->physicalDevice, data->computeQueueFamilyIndex, enableValidationLayers);
      data->device = deviceInfo.first;
      data->computeQueue = deviceInfo.second;

      return Device(std::move(data));
    }

    std::unique_ptr<CommandPool> Device::createCommandPool() const {
      return CommandPool::create(data->device, data->computeQueueFamilyIndex);
    }

    std::unique_ptr<Shader> Device::createShader(const std::string& filename, VkShaderStageFlagBits stage, const std::string& entrypoint) const {
      return Shader::create(data->device, filename, stage, entrypoint);
    }

    std::unique_ptr<Buffer> Device::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, bool allocate) const {
      return Buffer::create(data->physicalDevice, data->device, size, usage, allocate);
    }

    std::unique_ptr<DescriptorPool> Device::createDescriptorPool(VkDescriptorPoolSize* poolSizes, uint32_t poolSizeCount, uint32_t maxSets) const {
      return DescriptorPool::create(data->device, poolSizes, poolSizeCount, maxSets);
    }

    VkPipelineCache Device::createPipelineCache() const {
      VkPipelineCacheCreateInfo pipelineCacheCreateInfo = {
        VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO,
        nullptr,
        0,
        0,
        nullptr
      };

      VkPipelineCache pipelineCache;
      utils::validateResult(vkCreatePipelineCache(data->device, &pipelineCacheCreateInfo, nullptr, &pipelineCache), "vkCreatePipelineCache");
      return pipelineCache;
    }

    void Device::releasePipelineCache(VkPipelineCache pipelineCache) const {
      vkDestroyPipelineCache(data->device, pipelineCache, nullptr);
    }

    VkPipeline Device::createComputePipeline(VkPipelineCache pipelineCache, VkPipelineLayout pipelineLayout, const VkPipelineShaderStageCreateInfo& shaderStageCI) const {
      VkComputePipelineCreateInfo computePipelineCreateInfo = {
        VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
        nullptr,
        0,
        shaderStageCI,
        pipelineLayout,
        nullptr,
        0
      };
      VkPipeline pipeline;
      utils::validateResult(vkCreateComputePipelines(data->device, pipelineCache, 1, &computePipelineCreateInfo, nullptr, &pipeline), "vkCreateComputePipelines");
      return pipeline;
    }

    void Device::releasePipeline(VkPipeline pipeline) const {
      vkDestroyPipeline(data->device, pipeline, nullptr);
    }

    VkDescriptorSetLayout Device::createDescriptorSetLayout(const VkDescriptorSetLayoutCreateInfo* createInfo) const {
      VkDescriptorSetLayout descriptorSetLayout;
      utils::validateResult(vkCreateDescriptorSetLayout(data->device, createInfo, nullptr, &descriptorSetLayout), "vkCreateDescriptorSetLayout");
      return descriptorSetLayout;
    }
    
    void Device::releaseDescriptorSetLayout(VkDescriptorSetLayout layout) const
    {
      vkDestroyDescriptorSetLayout(data->device, layout, nullptr);
    }

    VkPipelineLayout Device::createPipelineLayout(const VkPipelineLayoutCreateInfo* createInfo) const
    {
      VkPipelineLayout pipelineLayout;
      utils::validateResult(vkCreatePipelineLayout(data->device, createInfo, nullptr, &pipelineLayout), "vkCreateDescriptorSetLayout");
      return pipelineLayout;
    }
    
    void Device::releasePipelineLayout(VkPipelineLayout layout) const
    {
      vkDestroyPipelineLayout(data->device, layout, nullptr);
    }
    

    void Device::submit(const CommandBuffer& commandBuffer) const {
      commandBuffer.submit(data->computeQueue);
    }

    void Device::updateDescriptorSets(const VkWriteDescriptorSet* writes, uint32_t writesCounts) const {
      vkUpdateDescriptorSets(data->device, writesCounts, writes, 0, nullptr);
    }

    VkPhysicalDeviceProperties Device::getProperties() const {
      return data->physicalDeviceProperties;
    }

    void Device::summary() const {
      auto properties = getProperties();
      std::cout << " ========= Device summary =========" << std::endl;
      std::cout << "Device name: " << properties.deviceName<< std::endl;
      std::cout << "Vendor id: " << properties.vendorID<< std::endl;
      std::cout << "Device id: " << properties.deviceID<< std::endl;
      std::cout << "Device type: " << properties.deviceType<< std::endl;
      std::cout << "--- Compute --- " << std::endl;
      std::cout << "Max threads per group: " << getMaxThreadsPerWorkgroup() << std::endl;
      std::cout << "Max work group size: " 
        << properties.limits.maxComputeWorkGroupSize[0]
        << " x "
        << properties.limits.maxComputeWorkGroupSize[1]
        << " x "
        << properties.limits.maxComputeWorkGroupSize[2]
        << std::endl;

      std::cout << "Max work group count: " 
        << properties.limits.maxComputeWorkGroupCount[0]
        << " x "
        << properties.limits.maxComputeWorkGroupCount[1]
        << " x "
        << properties.limits.maxComputeWorkGroupCount[2]
        << std::endl;
      std::cout << " =========+++++++++++++++=========" << std::endl;
    }
  }
}
