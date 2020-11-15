#pragma once

#include <vk/api/vkdescriptorset.h>

#include <vulkan/vulkan.h>

#include <memory>

namespace Vk {
  namespace api {
    class DescriptorPool {
      public:
        DescriptorPool(VkDevice device, VkDescriptorPool commandPool);
        ~DescriptorPool();

        static std::unique_ptr<DescriptorPool> create(VkDevice device, VkDescriptorPoolSize* poolSizes, uint32_t poolSizeCount, uint32_t maxSets);

        std::unique_ptr<DescriptorSet> createDescriptorSet(VkDescriptorSetLayout descriptorSetLayout) const;

      private:
        VkDevice device;
        VkDescriptorPool descriptorPool;
    };
  }
}
