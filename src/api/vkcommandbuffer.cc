#include <vk/api/vkcommandbuffer.h>
#include <vk/api/vkutils.h>

namespace Vk {
  namespace api {
    const uint64_t defaultFenceTimeout = 100000000000; // in ns

    CommandBuffer::CommandBuffer(VkDevice device, VkCommandPool commandPool, VkCommandBuffer commandBuffer)
    : device(device)
    , commandPool(commandPool)
    , commandBuffer(commandBuffer)
    {
    }
    
    CommandBuffer::~CommandBuffer() {
      if (commandBuffer) {
        vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
        commandBuffer = nullptr;
      }
    }
    
    std::unique_ptr<CommandBuffer> CommandBuffer::create(VkDevice device, VkCommandPool commandPool) {
      VkCommandBufferAllocateInfo cmdBufAllocateInfo = {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        nullptr,
        commandPool,
        VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        1
      };

      VkCommandBuffer commandBuffer;
      utils::validateResult(vkAllocateCommandBuffers(device, &cmdBufAllocateInfo, &commandBuffer), "vkAllocateCommandBuffers");
      return std::make_unique<CommandBuffer>(device, commandPool, commandBuffer);
    }

    void CommandBuffer::bindPipeline(VkPipeline pipeline, VkPipelineBindPoint bindingPoint) const {
      vkCmdBindPipeline(commandBuffer, bindingPoint, pipeline);
    }

    void CommandBuffer::bindDescriptorSets(VkPipelineLayout pipelineLayout, const VkDescriptorSet* descriptorSets, uint32_t setsCount, VkPipelineBindPoint bindingPoint) const {
      vkCmdBindDescriptorSets(commandBuffer, bindingPoint, pipelineLayout, 0, setsCount, descriptorSets, 0, nullptr);
    }

    void CommandBuffer::bindDescriptorSets(VkPipelineLayout pipelineLayout, const DescriptorSet& descriptorSet, VkPipelineBindPoint bindingPoint) const {
      VkDescriptorSet descSet = descriptorSet.getHandle();
      vkCmdBindDescriptorSets(commandBuffer, bindingPoint, pipelineLayout, 0, 1, &descSet, 0, nullptr);
    }

    void CommandBuffer::pushConstants(VkPipelineLayout pipelineLayout, VkShaderStageFlagBits stage, const void* data, uint32_t dataSize) const {
      vkCmdPushConstants(commandBuffer, pipelineLayout, stage, 0, dataSize, data);
    }

    void CommandBuffer::dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) const {
      vkCmdDispatch(commandBuffer, groupCountX, groupCountY, groupCountZ);
    }

    void CommandBuffer::begin() const {
      VkCommandBufferBeginInfo beginInfo = {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        nullptr,
        VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT,
        nullptr
      };

      utils::validateResult(vkBeginCommandBuffer(commandBuffer, &beginInfo), "vkBeginCommandBuffer");
    }

    void CommandBuffer::end() const {
      utils::validateResult(vkEndCommandBuffer(commandBuffer), "vkEndCommandBuffer");
    }

    void CommandBuffer::reset() const {
      vkResetCommandBuffer(commandBuffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
    }

    void CommandBuffer::submit(VkQueue submitQueue) const
    {
      VkSubmitInfo submitInfo = {
        VK_STRUCTURE_TYPE_SUBMIT_INFO,
        nullptr,
        0,
        nullptr,
        nullptr,
        1,
        &commandBuffer,
        0,
        nullptr
      };

      // Create fence to ensure that the command buffer has finished executing
      VkFenceCreateInfo fenceCreateInfo = {
        VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        nullptr,
        0
      };

      VkFence fence;
      utils::validateResult(vkCreateFence(device, &fenceCreateInfo, nullptr, &fence), "vkCreateFence");

      // Submit to the queue
      utils::validateResult(vkQueueSubmit(submitQueue, 1, &submitInfo, fence), "vkSubmitQueue");

      // Wait for the fence to signal that command buffer has finished executing
      utils::validateResult(vkWaitForFences(device, 1, &fence, VK_TRUE, defaultFenceTimeout), "vkWaitForFences");

      vkDestroyFence(device, fence, nullptr);
    }
  }
}