#pragma once

#include <vk/api/vkdevice.h>
#include <vk/vkutils.hpp>

#include <iostream>

namespace Vk {
  namespace internal {

    //
    // Params conversion to descriptor set layout
    //

    template<size_t COUNT>
    auto descriptorsToLayout(const std::array<VkDescriptorType, COUNT> descTypes, VkShaderStageFlagBits stage)
    {
      auto arr = std::array<VkDescriptorSetLayoutBinding, COUNT>{};
      for (size_t desc = 0; desc < COUNT; ++desc) {
        // TODO support samplers (final nullptr)
        arr[desc] = { uint32_t(desc), descTypes[desc], 1, stage, nullptr };
      }
      return arr;
    }

    template<class T> struct DescTypeMapper
    {
      static constexpr auto type = T::descriptor_type;
    };

    template<class... Args> auto paramsToDescType() -> std::array<VkDescriptorType, sizeof...(Args)>
    {
      return {DescTypeMapper<Args>::type...};
    }

    template<class... Args> auto paramsToLayout(VkShaderStageFlagBits stage, Args&...) -> std::array<VkDescriptorSetLayoutBinding, sizeof...(Args)>
    {
      return descriptorsToLayout(paramsToDescType<Args...>(), stage);
    }

    template<class T, size_t... Indices>
    auto descriptorInfosToWriteDesc(VkDescriptorSet descriptorSet, std::index_sequence<Indices...>, const T& infos) -> std::array<VkWriteDescriptorSet, sizeof...(Indices)>
    {
      return std::array<VkWriteDescriptorSet, sizeof...(Indices)>{Vk::api::utils::writeDescriptorSet(descriptorSet, Indices, &infos[Indices])...};
    }

    template<size_t I, class T>
    auto tupleOffset(const T& tuple) -> uint32_t
    {
      auto elementPointer = reinterpret_cast<const char*>(&std::get<I>(tuple));
      auto startPointer = reinterpret_cast<const char*>(&tuple);
      return elementPointer - startPointer;
    }

    template<size_t I, class T>
    auto tupleTypeSize() -> uint32_t
    {
      return static_cast<uint32_t>(sizeof(typename std::tuple_element<I, T>::type));
    }

    template<class T, size_t... Indices>
    auto specToMapEntries(const T& specs, std::index_sequence<Indices...>) -> std::array<VkSpecializationMapEntry, sizeof...(Indices)>
    {
      return {{{uint32_t(Indices), tupleOffset<Indices, T>(specs), tupleTypeSize<Indices, T>()}...}};
    }

    template<class... Specs>
    auto specToMapEntries(const std::tuple<Specs...>& tuple) -> std::array<VkSpecializationMapEntry, sizeof...(Specs)>
    {
      return specToMapEntries(tuple, std::make_index_sequence<sizeof...(Specs)>());
    }

    //
    // ----
    //

    class ComputeProgramBase {
      protected:
        ComputeProgramBase(Vk::api::Device& device, const std::string& filename)
        : device(device)
        , shaderFilename(filename)
        , shader(device.createShader(shaderFilename))
        {}

        virtual ~ComputeProgramBase() {
          release();
        }

        template<class... Args>
        void setupPipelineLayout(uint32_t pushConstRangesCount, VkPushConstantRange* pushConstRanges, Args&... args)
        {
          if (pipelineLayout) {
            return;
          }

          auto bindings = paramsToLayout(shader->getStage(), args...);
          VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO, nullptr, 0, static_cast<uint32_t>(bindings.size()), bindings.data() };         
          descriptorSetLayout = device.createDescriptorSetLayout(&descriptorSetLayoutCreateInfo);

          VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = { VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO, nullptr, 0, 1, &descriptorSetLayout, pushConstRangesCount, pushConstRanges };

          pipelineLayout = device.createPipelineLayout(&pipelineLayoutCreateInfo);
        }

        template<class... SpecTs>
        void setupPipeline(const std::tuple<SpecTs...>& specs)
        {
          if (!pipelineCache) {
            pipelineCache = device.createPipelineCache();
          }

          if (pipeline) {
            return;
          }

          // Create pipeline
          auto specEntries = specToMapEntries(specs);

          VkSpecializationInfo specInfo = {
            static_cast<uint32_t>(specEntries.size()),
            specEntries.data(),
            sizeof(specs),
            &specs
          };

          pipeline = device.createComputePipeline(pipelineCache, pipelineLayout, shader->getPipelineShaderStageCI(&specInfo));
        }

        void releasePipeline()
        {
          if (pipeline)
          {
            device.releasePipeline(pipeline);
            pipeline = nullptr;
          }
        }

        template<class... Args>
        void setupDescriptorsSet(Args&&... args)
        {
          if (!descriptorSetPool) {
            std::array<VkDescriptorPoolSize, 1> sizes { Vk::api::utils::descriptorPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 4) };
            descriptorSetPool = device.createDescriptorPool(sizes.data(), static_cast<uint32_t>(sizes.size()));
          }

          if (!descriptorSet) {
            descriptorSet = descriptorSetPool->createDescriptorSet(descriptorSetLayout);
          }

          auto bufferInfos = std::array<VkDescriptorBufferInfo, sizeof...(Args)>{args.getApiBuffer().getBufferInfo()...};
          auto writeDescriptorSet = descriptorInfosToWriteDesc(descriptorSet->getHandle(), std::make_index_sequence<sizeof...(Args)>(), bufferInfos);

          device.updateDescriptorSets(writeDescriptorSet.data(), static_cast<uint32_t>(writeDescriptorSet.size()));
        }

        auto begin() -> void
        {
          if (!commandPool) {
            commandPool = device.createCommandPool();
          }
          if (!commandBuffer) {
            commandBuffer = commandPool->createCommandBuffer();
          }

          commandBuffer->begin();

          // Bind pipeline
          commandBuffer->bindPipeline(pipeline);

          // Bind descriptor sets
          commandBuffer->bindDescriptorSets(pipelineLayout, *descriptorSet);
        }

        auto end() -> void
        {
          // Finalize command buffer
          commandBuffer->end();

          // Submit command buffer
          device.submit(*commandBuffer);
        }

        auto dispatch() -> void
        {
          // Dispatch
          commandBuffer->dispatch(workGroups[0], workGroups[1], workGroups[2]);
        }

      private:

      void release()
      { 
        commandBuffer.reset();
        commandPool.reset();
        descriptorSet.reset();
        descriptorSetPool.reset();

        if (descriptorSetLayout) {
          device.releaseDescriptorSetLayout(descriptorSetLayout);
          descriptorSetLayout = nullptr;
        }

        if (pipelineLayout) {
          device.releasePipelineLayout(pipelineLayout);
          pipelineLayout = nullptr;
        }

        if (pipeline) {
          device.releasePipeline(pipeline);
          pipeline = nullptr;
        }

        if (pipelineCache) {
          device.releasePipelineCache(pipelineCache);
          pipelineCache = nullptr;
        }
      }

      protected:
        Vk::api::Device& device;
        const std::string shaderFilename;
        const std::unique_ptr<Vk::api::Shader> shader;
        std::unique_ptr<api::CommandPool> commandPool;
        std::unique_ptr<api::CommandBuffer> commandBuffer;
        
        std::array<uint32_t, 3> workGroups;
        // Constants constants = {};

        // Vulkan objects
        VkDescriptorSetLayout descriptorSetLayout = nullptr;
        VkPipelineLayout pipelineLayout = nullptr;
        VkPipelineCache pipelineCache = nullptr;
        VkPipeline pipeline = nullptr;
        std::unique_ptr<Vk::api::DescriptorPool> descriptorSetPool;
        std::unique_ptr<Vk::api::DescriptorSet> descriptorSet;
    };
  }
}