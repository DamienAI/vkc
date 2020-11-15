#include <vk/api/vkcommandpool.h>
#include <vk/api/vkutils.h>

namespace Vk {
  namespace api {
    CommandPool::CommandPool(VkDevice device, VkCommandPool commandPool)
    : device(device)
    , commandPool(commandPool)
    {
    }

    CommandPool::~CommandPool() {
      if (commandPool) {
        vkDestroyCommandPool(device, commandPool, nullptr);
        commandPool = nullptr;
      }
    }

    std::unique_ptr<CommandPool> CommandPool::create(VkDevice device, uint32_t queueFamilyIndex) {
      VkCommandPoolCreateInfo poolInfo = {
        VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        nullptr,
        VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        queueFamilyIndex
      };

      VkCommandPool commandPool;
      utils::validateResult(vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool), "vkCreateCommandPool");
      return std::make_unique<CommandPool>(device, commandPool);
    }

    std::unique_ptr<CommandBuffer> CommandPool::createCommandBuffer() const {
      return CommandBuffer::create(device, commandPool);
    }
  }
}
