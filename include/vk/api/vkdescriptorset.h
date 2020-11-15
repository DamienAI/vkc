#pragma once

#include <vulkan/vulkan.h>

#include <memory>

namespace Vk {
  namespace api {
    class DescriptorSet {
      public:
        DescriptorSet(VkDevice device, VkDescriptorPool descriptorPool, VkDescriptorSet descriptorSet, VkDescriptorSetLayout descriptorSetLayout);
        ~DescriptorSet();

        VkDescriptorSet getHandle() const { return descriptorSet; }

        static std::unique_ptr<DescriptorSet> create(VkDevice device, VkDescriptorPool descriptorPool, VkDescriptorSetLayout descriptorSetLayout);

      private:
        VkDevice device;
        VkDescriptorPool descriptorPool;
        VkDescriptorSet descriptorSet;
        VkDescriptorSetLayout descriptorSetLayout;
    };
  }
}
