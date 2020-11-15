#include <vk/api/vkdescriptorset.h>
#include <vk/api/vkutils.h>

namespace Vk {
  namespace api {
    DescriptorSet::DescriptorSet(VkDevice device, VkDescriptorPool descriptorPool, VkDescriptorSet descriptorSet, VkDescriptorSetLayout descriptorSetLayout)
    : device(device)
    , descriptorPool(descriptorPool)
    , descriptorSet(descriptorSet)
    , descriptorSetLayout(descriptorSetLayout)
    {
    }
    
    DescriptorSet::~DescriptorSet() {
      if (descriptorSet) {
        // TODO HANDLE THIS
        // vkFreeDescriptorSets(device, descriptorPool, 1, &descriptorSet);
        descriptorSet = nullptr;
      }
    }
    
    std::unique_ptr<DescriptorSet> DescriptorSet::create(VkDevice device, VkDescriptorPool descriptorPool, VkDescriptorSetLayout descriptorSetLayout) {
      VkDescriptorSetAllocateInfo allocateInfo = {
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        nullptr,
        descriptorPool,
        1,
        &descriptorSetLayout
      };

      VkDescriptorSet descriptorSet;
      utils::validateResult(vkAllocateDescriptorSets(device, &allocateInfo, &descriptorSet), "vkAllocateCommandBuffers");
      return std::make_unique<DescriptorSet>(device, descriptorPool, descriptorSet, descriptorSetLayout);
    }
  }
}