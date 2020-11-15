#pragma once

#include <vk/api/vkbuffer.h>

#include <vulkan/vulkan.h>

#include <string>

namespace Vk {
  namespace api {
    namespace utils {
      void validateResult(VkResult result, const std::string& function = "unknown");

      inline VkDescriptorPoolSize descriptorPoolSize(VkDescriptorType type, uint32_t descriptorCount)
      {
        VkDescriptorPoolSize poolSize {
          type,
          descriptorCount
        };
        return poolSize;
      }

      inline VkWriteDescriptorSet writeDescriptorSet(
        VkDescriptorSet dstSet,
        VkDescriptorType type,
        uint32_t binding,
        const VkDescriptorBufferInfo* bufferInfo = nullptr,
        const VkDescriptorImageInfo* imageInfo = nullptr,
        const VkBufferView* texelBufferView = nullptr,
        uint32_t descriptorCount = 1)
      {
        VkWriteDescriptorSet writeDescriptorSet {
          VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
          nullptr,
          dstSet,
          binding,
          0,
          descriptorCount,
          type,
          imageInfo,
          bufferInfo,
          texelBufferView
        };
        return writeDescriptorSet;
      }

      inline VkWriteDescriptorSet writeDescriptorSet(
        VkDescriptorSet dstSet,
        uint32_t binding,
        const VkDescriptorBufferInfo* bufferInfo)
      {
        VkWriteDescriptorSet writeDescriptorSet {
          VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
          nullptr,
          dstSet,
          binding,
          0,
          1,
          VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
          nullptr,
          bufferInfo,
          nullptr
        };
        return writeDescriptorSet;
      }

      inline VkPushConstantRange pushConstantRange(
        VkShaderStageFlags stageFlags,
        uint32_t size,
        uint32_t offset = 0
      ) {
        VkPushConstantRange range {
          stageFlags,
          offset,
          size
        };

        return range;
      }
    }
  }
}
