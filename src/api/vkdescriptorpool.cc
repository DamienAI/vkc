#include <vk/api/vkdescriptorpool.h>
#include <vk/api/vkutils.h>

namespace Vk {
  namespace api {
    DescriptorPool::DescriptorPool(VkDevice device, VkDescriptorPool descriptorPool)
    : device(device)
    , descriptorPool(descriptorPool)
    {
    }

    DescriptorPool::~DescriptorPool() {
      if (descriptorPool) {
        vkDestroyDescriptorPool(device, descriptorPool, nullptr);
        descriptorPool = nullptr;
      }
    }

    std::unique_ptr<DescriptorPool> DescriptorPool::create(VkDevice device, VkDescriptorPoolSize* poolSizes, uint32_t poolSizeCount, uint32_t maxSets) {
      VkDescriptorPoolCreateInfo poolInfo = {
        VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        nullptr,
        0,
        maxSets,
        poolSizeCount,
        poolSizes,
      };

      VkDescriptorPool descriptorPool;
      utils::validateResult(vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool), "vkCreateDescriptorPool");
      return std::make_unique<DescriptorPool>(device, descriptorPool);
    }

    std::unique_ptr<DescriptorSet> DescriptorPool::createDescriptorSet(VkDescriptorSetLayout descriptorSetLayout) const {
      return DescriptorSet::create(device, descriptorPool, descriptorSetLayout);
    }
  }
}
