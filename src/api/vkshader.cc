#include <vk/api/vkshader.h>
#include <vk/api/vkutils.h>

#include <fstream>
#include <stdexcept>
#include <vector>

namespace Vk {
  namespace api {
    Shader::Shader(VkDevice device, VkShaderModule shader, VkShaderStageFlagBits stage, const std::string& entrypoint)
    : device(device)
    , shader(shader)
    , stage(stage)
    , entrypoint(entrypoint)
    {}

    Shader::~Shader()
    {
      releaseLayouts();

      if (shader) {
        vkDestroyShaderModule(device, shader, nullptr);
        shader = nullptr;
      }
    }

    void Shader::addBinding(uint32_t binding, VkDescriptorType descriptorType, const VkSampler* immutableSamplers) {
      VkDescriptorSetLayoutBinding layoutBinding = {
        binding,
        descriptorType,
        1,
        stage,
        immutableSamplers
      };
      bindings.push_back(layoutBinding);
      pipelineLayoutValid = false;
    }

    std::vector<char> readFile(const std::string& filename) {
      std::ifstream file(filename, std::ios::ate | std::ios::binary);

      if (!file.is_open()) {
          throw std::runtime_error(std::string("failed to open file ") + filename);
      }
      size_t fileSize = (size_t)file.tellg();
      std::vector<char> buffer(fileSize);
      file.seekg(0);
      file.read(buffer.data(), fileSize);
      file.close();
      return buffer;
    }

    std::unique_ptr<Shader> Shader::create(VkDevice device, const std::string& filename, VkShaderStageFlagBits stage, const std::string& entrypoint)
    {
      auto shaderContent = readFile(filename);
      
      VkShaderModuleCreateInfo createInfo = {
        VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        nullptr,
        0,
        shaderContent.size(),
        reinterpret_cast<const uint32_t*>(shaderContent.data())
      };

      VkShaderModule shaderModule;
      utils::validateResult(vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule), "vkCreateShaderModule");

      return std::make_unique<Shader>(device, shaderModule, stage, entrypoint);
    }

    VkPipelineLayout Shader::getOrCreatePipelineLayout() {
      updateLayouts();
      return pipelineLayout;
    }


    VkDescriptorSetLayout Shader::getOrCreateDescriptorSetLayout() {
      updateLayouts();
      return descriptorSetLayout;
    }

    void Shader::updateLayouts() {
      if (pipelineLayoutValid) {
        return;
      }
      releaseLayouts();

      VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = {
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        nullptr,
        0,
        static_cast<uint32_t>(bindings.size()),
        bindings.data()
      };

      utils::validateResult(vkCreateDescriptorSetLayout(device, &descriptorSetLayoutCreateInfo, nullptr, &descriptorSetLayout), "vkCreateDescriptorSetLayout");

      std::vector<VkPushConstantRange> pushConstantRanges;
      if (pushConstantsSize) {
        VkPushConstantRange range = {
          stage,
          0,
          pushConstantsSize
        };
        pushConstantRanges.push_back(range);
      }

      VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {
        VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        nullptr,
        0,
        1,
        &descriptorSetLayout,
        static_cast<uint32_t>(pushConstantRanges.size()),
        pushConstantRanges.data()
      };
      utils::validateResult(vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout), "vkCreatePipelineLayout");
      pipelineLayoutValid = true;
    }

    void Shader::releaseLayouts() {
      if (descriptorSetLayout) {
        vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
        descriptorSetLayout = nullptr;
      }
      if (pipelineLayout) {
        vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
        pipelineLayout = nullptr;
      }
    }

    VkPipelineShaderStageCreateInfo Shader::getPipelineShaderStageCI(const VkSpecializationInfo* specializationInfo) const {
      VkPipelineShaderStageCreateInfo shaderStageCreateInfo = {
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        nullptr,
        0,
        stage,
        shader,
        entrypoint.c_str(),
        specializationInfo
      };
      return shaderStageCreateInfo;
    }
  }
}