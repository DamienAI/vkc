#pragma once

#include <vk/api/vkbuffer.h>
#include <vk/api/vkcommandpool.h>
#include <vk/api/vkdescriptorpool.h>
#include <vk/api/vkshader.h>

#include <memory>

namespace Vk {
  namespace api {
    class DeviceData;

    class Device {

      public:
        Device();

        // TODO we might find another way but for now just delete it
        Device(const Device&) = delete;

        Device(Device&&);

        Device(std::unique_ptr<DeviceData> deviceData);

        ~Device();

        Device& operator=(const Device&) = delete;
        Device& operator=(Device&&);

        static Device findFirstAvailable(bool enableValidationLayers = false);

        std::unique_ptr<CommandPool> createCommandPool() const;
        std::unique_ptr<DescriptorPool> createDescriptorPool(VkDescriptorPoolSize* poolSizes, uint32_t poolSizeCount, uint32_t maxSets = 64) const;

        std::unique_ptr<Buffer> createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, bool allocate = true) const;
        std::unique_ptr<Shader> createShader(const std::string& filename, VkShaderStageFlagBits stage = VkShaderStageFlagBits::VK_SHADER_STAGE_COMPUTE_BIT, const std::string& entrypoint = "main") const;

        VkPipelineCache createPipelineCache() const;
        void releasePipelineCache(VkPipelineCache pipelineCache) const;

        VkPipeline createComputePipeline(VkPipelineCache pipelineCache, VkPipelineLayout pipelineLayout, const VkPipelineShaderStageCreateInfo& shaderStageCI) const;
        void releasePipeline(VkPipeline pipeline) const;

        VkDescriptorSetLayout createDescriptorSetLayout(const VkDescriptorSetLayoutCreateInfo* createInfo) const;
        void releaseDescriptorSetLayout(VkDescriptorSetLayout layout) const;

        VkPipelineLayout createPipelineLayout(const VkPipelineLayoutCreateInfo* createInfo) const;
        void releasePipelineLayout(VkPipelineLayout layout) const;

        void submit(const CommandBuffer& commandBuffer) const;

        void updateDescriptorSets(const VkWriteDescriptorSet* writes, uint32_t writesCounts) const;

        VkPhysicalDeviceProperties getProperties() const;
        auto getMaxThreadsPerWorkgroup() const -> uint32_t { return getProperties().limits.maxComputeWorkGroupInvocations; }
        auto getMaxWorkGroupSize() const -> std::array<uint32_t, 3>
        {
          return
          {
            getProperties().limits.maxComputeWorkGroupSize[0],
            getProperties().limits.maxComputeWorkGroupSize[1],
            getProperties().limits.maxComputeWorkGroupSize[2],
          };
        }
        auto getMaxWorkGroupCount() const -> std::array<uint32_t, 3>
        {
          return
          {
            getProperties().limits.maxComputeWorkGroupCount[0],
            getProperties().limits.maxComputeWorkGroupCount[1],
            getProperties().limits.maxComputeWorkGroupCount[2],
          };
        }
        auto getMaxSharedMemorySize() const -> uint32_t { return getProperties().limits.maxComputeSharedMemorySize; }
        
        void summary() const;

      private:
        std::unique_ptr<DeviceData> data;
    };
  }
}
