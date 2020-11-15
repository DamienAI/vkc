#pragma once

#include <vk/api/vkdevice.h>
#include <vk/api/vkutils.h>
#include <vk/internal/vk.hpp>

#include <vk/vkarraybuffer.hpp>

#include <array>
#include <iostream>

namespace Vk {
  using WorkGroupSize = std::array<uint32_t, 3>;
  using WorkGroupsCount = std::array<uint32_t, 3>;

  template<class... Ts> class typelist {};

  template<class Specs=typelist<>, class Constants=typelist<>>
  class ComputeProgram {
    ComputeProgram(Vk::api::Device&, const std::string&) {
      // TODO ensure this is correct definition, do we really need this ?
      // Specializations should be enough
    }
  };

  template<template<class...> class SpecTypes, class... SpecTs>
  class ComputeProgram<SpecTypes<SpecTs...>, typelist<>> : public internal::ComputeProgramBase {
    using super = internal::ComputeProgramBase;
    public:

      ComputeProgram(Vk::api::Device& device, const std::string& filename)
      : super(device, filename)
      {}

      auto withWorkGroups(uint32_t x, uint32_t y = 1, uint32_t z = 1) -> ComputeProgram&
      {
        super::workGroups = {x, y, z};
        return *this;
      }

      auto withWorkGroups(WorkGroupsCount count) -> ComputeProgram&
      {
        super::workGroups = count;
        return *this;
      }

      auto getWorkGroups() const -> WorkGroupsCount
      {
        return super::workGroups;
      }

      auto withSpecializations(SpecTs... values) -> ComputeProgram&
      {
        auto newSpecs = std::make_tuple(values...);
        if (newSpecs != specs) {
          specs = newSpecs;
          super::releasePipeline();
        }
        return *this;
      }

      template<class... Args>
      auto operator()(Args&&... args) -> void
      {
        // TODO we can use std::array of size 0
        super::setupPipelineLayout(0, nullptr, args...);
        super::setupPipeline(specs);
        super::setupDescriptorsSet(args...);

        super::begin();
        super::dispatch();
        super::end();
      }

    private:
      std::tuple<SpecTs...> specs;
  };

  template<template<class...> class SpecTypes, class... SpecTs, class Constants>
  class ComputeProgram<SpecTypes<SpecTs...>, Constants>: public internal::ComputeProgramBase {
    using super = internal::ComputeProgramBase;
    public:

      ComputeProgram(Vk::api::Device& device, const std::string& filename)
      : super(device, filename)
      {}

      auto withWorkGroups(uint32_t x, uint32_t y = 1, uint32_t z = 1) -> ComputeProgram&
      {
        super::workGroups = {x, y, z};
        return *this;
      }

      auto withWorkGroups(WorkGroupsCount count) -> ComputeProgram&
      {
        super::workGroups = count;
        return *this;
      }

      auto getWorkGroups() const -> WorkGroupsCount
      {
        return super::workGroups;
      }

      auto withSpecializations(SpecTs... values) -> ComputeProgram&
      {
        auto newSpecs = std::make_tuple(values...);
        if (newSpecs != specs) {
          specs = newSpecs;
          super::releasePipeline();
        }
        return *this;
      }

      template<class... Args>
      auto operator()(const Constants& constants, Args&&... args) -> void
      {
        auto pushConstantsRange = std::array<VkPushConstantRange, 1> { { shader->getStage(), 0, sizeof(Constants) } };
        super::setupPipelineLayout(static_cast<uint32_t>(pushConstantsRange.size()), pushConstantsRange.data(), args...);
        super::setupPipeline(specs);
        super::setupDescriptorsSet(args...);

        super::begin();
        super::commandBuffer->pushConstants(pipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, &constants, sizeof(Constants));
        super::dispatch();
        super::end();
      }

    private:
      std::tuple<SpecTs...> specs;
  };
}
