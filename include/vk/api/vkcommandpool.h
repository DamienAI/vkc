#pragma once

#include <vk/api/vkcommandbuffer.h>

#include <vulkan/vulkan.h>

#include <memory>

namespace Vk {
  namespace api {
    class CommandPool {
      public:
        CommandPool(VkDevice device, VkCommandPool commandPool);
        ~CommandPool();

        static std::unique_ptr<CommandPool> create(VkDevice device, uint32_t queueFamilyIndex);

        std::unique_ptr<CommandBuffer> createCommandBuffer() const;

      private:
        VkDevice device;
        VkCommandPool commandPool;
    };
  }
}
