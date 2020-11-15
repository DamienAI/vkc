#pragma once

#include <vk/api/vkdevice.h>
#include <vk/api/vkbuffer.h>

#include <cstring>
#include <vector>

// TODO allocator support

namespace Vk {
  template<class DataType> class ArrayBuffer
  {
    public:
      static constexpr auto descriptor_type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;

      ArrayBuffer(Vk::api::Device& device, const std::vector<DataType>& initial)
      : device(device)
      , elementsCount(initial.size())
      , buffer(device.createBuffer(initial.size() * sizeof(DataType), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT))
      {
        buffer->map();
        fromVector(initial);
      }

      ArrayBuffer(Vk::api::Device& device, const uint64_t elementsCount)
      : device(device)
      , elementsCount(elementsCount)
      , buffer(device.createBuffer(elementsCount * sizeof(DataType), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT))
      {
        buffer->map();
      }

      // TODO: Staged copy using the command buffer and async support

      auto fromVector(const std::vector<DataType>& data) -> void {
        auto bufferSize = data.size() * sizeof(DataType);
        if (bufferSize > buffer->getSize()) {
          throw std::runtime_error("Cannot load " + std::to_string(bufferSize) + " bytes buffer in a " + std::to_string(buffer->getSize()) + " bytes device buffer");
        }
        std::memcpy(buffer->getMappedPointer(), data.data(), bufferSize);
      }

      auto fromMemory(const DataType* data) -> void {
        auto bufferSize = elementsCount * sizeof(DataType);
        std::memcpy(buffer->getMappedPointer(), data, bufferSize);
      }

      auto toVector() const -> std::vector<DataType> {
        auto elementsCount = buffer->getSize() / sizeof(DataType);
        std::vector<DataType> data(elementsCount);
        std::memcpy(data.data(), buffer->getMappedPointer(), elementsCount * sizeof(DataType));
        return data;
      }

      auto getApiBuffer() const -> Vk::api::Buffer& {
        return *buffer;
      }

      auto getElementsCount() const -> size_t {
        return elementsCount;
      }

    private:
      Vk::api::Device& device;
      size_t elementsCount;
      const std::unique_ptr<Vk::api::Buffer> buffer;
  };
}
