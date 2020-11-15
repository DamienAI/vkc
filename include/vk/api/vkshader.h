#pragma once

#include <vulkan/vulkan.h>

#include <memory>
#include <vector>

namespace Vk {
  namespace api {
    class Shader
    {
      public:
        Shader(VkDevice device, VkShaderModule shader, VkShaderStageFlagBits stage, const std::string& entrypoint);
        ~Shader();

        void addBinding(uint32_t binding, VkDescriptorType descriptorType = VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, const VkSampler* immutableSamplers = nullptr);
        void setPushConstantsSize(uint32_t size) { pushConstantsSize = size; pipelineLayoutValid = false; }

        VkPipelineLayout getOrCreatePipelineLayout();
        VkDescriptorSetLayout getOrCreateDescriptorSetLayout();
        VkShaderModule getModule() const { return shader; }
        VkPipelineShaderStageCreateInfo getPipelineShaderStageCI(const VkSpecializationInfo* specializationInfo) const;
        VkShaderStageFlagBits getStage() const { return stage; }

        static std::unique_ptr<Shader> create(VkDevice device, const std::string& filename, VkShaderStageFlagBits stage = VkShaderStageFlagBits::VK_SHADER_STAGE_COMPUTE_BIT, const std::string& entrypoint = "main");

      private:
        void updateLayouts();
        void releaseLayouts();

      private:
        VkDevice device = nullptr;
        VkShaderModule shader = nullptr;
        VkShaderStageFlagBits stage;
        std::string entrypoint;

        // TODO invalide descriptor set layout and pipeline layout on change
        std::vector<VkDescriptorSetLayoutBinding> bindings;
        uint32_t pushConstantsSize = 0;

        bool pipelineLayoutValid = false;
        VkDescriptorSetLayout descriptorSetLayout = nullptr;
        VkPipelineLayout pipelineLayout = nullptr;
    };
  }
}
