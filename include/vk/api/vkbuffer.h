#pragma once

#include <vulkan/vulkan.h>

#include <memory>

namespace Vk {
  namespace api {
    class Buffer {
      private:

        uint32_t findMemoryType(uint32_t memoryTypeBits, VkMemoryPropertyFlags properties) const;

        void release();
      
      private:
        VkDeviceSize size;

        VkDevice device;
        // We need physical device to search the right memory type.
        VkPhysicalDevice physicalDevice;
        VkBuffer buffer;
        VkDeviceMemory memory;

        // set when map for persistent mapping
        void* mappedPtr;
        VkDeviceSize mappedOffset;
        VkDeviceSize mappedSize;

      public:
        static std::unique_ptr<Buffer> create(VkPhysicalDevice physicalDevice, VkDevice device, VkDeviceSize size, VkBufferUsageFlags usage, bool allocate = true);

        ~Buffer();

        VkDeviceSize getSize() const { return size; }
        VkBuffer getHandle() const { return buffer; }
        void* getMappedPointer() const { return mappedPtr; }

        VkDescriptorBufferInfo getBufferInfo(VkDeviceSize offset = 0) const { 
          VkDescriptorBufferInfo bufferInfo {
            getHandle(),
            offset,
            getSize(),
          };

          return bufferInfo;
        };

        // Default properties to be able to map the buffer
        void allocateMemory(VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, bool bind = true);

        // TODO
        // void stagedCopy(VkQueue submitQueue, VkCommandPool pool, const void* data, VkDeviceSize offset, VkDeviceSize size);

        VkDescriptorBufferInfo getDescriptor(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0) const
        {
          VkDescriptorBufferInfo descriptor{
            buffer,
            offset,
            size
          };
          return descriptor;
        }
        
        void* map(size_t regionOffset = 0, size_t regionSize = VK_WHOLE_SIZE);
        void unmap();
    };
  }
}
