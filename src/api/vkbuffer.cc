#include <vk/api/vkbuffer.h>
#include <vk/api/vkutils.h>

#include <stdexcept>
#include <cstring>

namespace Vk {
  namespace api {
    void* Buffer::map(size_t regionOffset, size_t regionSize)
    {
      mappedOffset = regionOffset;
      mappedSize = regionSize;
      utils::validateResult(vkMapMemory(device, memory, mappedOffset, mappedSize, 0, &mappedPtr), "vkMapMemory");
      return mappedPtr;
    }

    void Buffer::unmap()
    {
      if (mappedPtr)
      {
        vkUnmapMemory(device, memory);
        mappedPtr = nullptr;
      }
    }

    void Buffer::release() {
      unmap();
      if (memory) {
        vkFreeMemory(device, memory, nullptr);
        memory = nullptr;
      }
      if (buffer) {
        vkDestroyBuffer(device, buffer, nullptr);
        buffer = nullptr;
      }
    }

    Buffer::~Buffer() {
      release();
    }

    std::unique_ptr<Buffer> Buffer::create(VkPhysicalDevice physicalDevice, VkDevice device, VkDeviceSize size, VkBufferUsageFlags usage, bool allocate)
    {
      std::unique_ptr<Buffer> buffer = std::make_unique<Buffer>();

      VkBufferCreateInfo bufferInfo = {
        VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        nullptr,
        0,
        size,
        usage,
        VK_SHARING_MODE_EXCLUSIVE,
        0,
        nullptr
      };

      utils::validateResult(vkCreateBuffer(device, &bufferInfo, nullptr, &buffer->buffer), "vkCreateBuffer");
      buffer->size = size;
      buffer->device = device;
      buffer->physicalDevice = physicalDevice;

      if (allocate) {
        buffer->allocateMemory();
      }

      return buffer;
    }

    uint32_t Buffer::findMemoryType(uint32_t memoryTypeBits, VkMemoryPropertyFlags properties) const {
      VkPhysicalDeviceMemoryProperties memoryProperties;
      vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);

      for (uint32_t index = 0; index < memoryProperties.memoryTypeCount; ++index) {
        if ((memoryTypeBits & (1 << index)) &&
            ((memoryProperties.memoryTypes[index].propertyFlags & properties) == properties))
          return index;
      }
      return -1;
    }
    
    void Buffer::allocateMemory(VkMemoryPropertyFlags properties, bool bind)
    {
      VkMemoryRequirements memRequirements;
      vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

      VkMemoryAllocateInfo allocInfo = {
        VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        nullptr,
        memRequirements.size,
        findMemoryType(memRequirements.memoryTypeBits, properties)
      };

      utils::validateResult(vkAllocateMemory(device, &allocInfo, nullptr, &memory), "vkAllocateMemory");

      if (bind) {
        utils::validateResult(vkBindBufferMemory(device, buffer, memory, 0), "vkBindBufferMemory");
      }
    }

    /*
    void Buffer::stagedCopy(VkQueue submitQueue, VkCommandPool pool, const void* data, VkDeviceSize offset, VkDeviceSize size)
    {
      // Create staging buffer
      std::unique_ptr<Buffer> stagingBuffer = Buffer::create(physicalDevice, device, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
      stagingBuffer->allocateMemory(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, true);
      stagingBuffer->map();

      // Copy memory
      memcpy(stagingBuffer->mappedPtr, data, size);
      stagingBuffer->unmap();

      // Perform the copy
      VkCommandBuffer copyCmd = createCommandBuffer(device, pool);
      
      beginCommandBuffer(copyCmd);

      VkBufferCopy copyRegion = {
          .size = size
      };

      vkCmdCopyBuffer(copyCmd, stagingBuffer->buffer, buffer, 1, &copyRegion);

      // Execute commands
      endCommandBuffer(copyCmd);
      flushCommandBuffer(device, submitQueue, pool, copyCmd);
      destroyCommandBuffer(device, pool, copyCmd);
    }
    */
  }
}