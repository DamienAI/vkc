#pragma once

#include <vk/api/vkdescriptorset.h>

#include <vulkan/vulkan.h>

#include <memory>

namespace Vk {
  namespace api {
    class CommandBuffer {
      public:
        CommandBuffer(VkDevice device, VkCommandPool commandPool, VkCommandBuffer commandBuffer);
        ~CommandBuffer();

        void bindPipeline(VkPipeline pipeline, VkPipelineBindPoint bindingPoint = VK_PIPELINE_BIND_POINT_COMPUTE) const;
        void bindDescriptorSets(VkPipelineLayout pipelineLayout, const VkDescriptorSet* descriptorSets, uint32_t setsCount = 1, VkPipelineBindPoint bindingPoint = VK_PIPELINE_BIND_POINT_COMPUTE) const;
        void bindDescriptorSets(VkPipelineLayout pipelineLayout, const DescriptorSet& descriptorSet, VkPipelineBindPoint bindingPoint = VK_PIPELINE_BIND_POINT_COMPUTE) const;
        void pushConstants(VkPipelineLayout pipelineLayout, VkShaderStageFlagBits stage, const void* data, uint32_t dataSize) const;

        void dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) const;

        void begin() const;
        void end() const;
        void reset() const;
        void submit(VkQueue submitQueue) const;

        static std::unique_ptr<CommandBuffer> create(VkDevice device, VkCommandPool commandPool);

      private:
        VkDevice device;
        VkCommandPool commandPool;
        VkCommandBuffer commandBuffer;
    };
  }
}
