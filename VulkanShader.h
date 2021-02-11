//
// Created by Raiix on 2021/2/9.
//

#ifndef VULKANTESTPROJECT1_VULKANSHADER_H
#define VULKANTESTPROJECT1_VULKANSHADER_H

#include "Commons.h"

class VulkanShader {
public:
    VkPipelineShaderStageCreateInfo  shaderStages[2];

    VulkanShader();
    ~VulkanShader();

    void BuildShaderModuleWithSPV(uint32_t *vertShaderText, size_t vertSPVSize, uint32_t *fragShaderText, size_t fragSPVSize);

    void DestroyShaders();

#ifdef ENABLE_GLSL_TRANSLATION
    bool GLSLtoSPV(const VkShaderStageFlagBits shaderType, const char *shader, std::vector<uint32_t> &SPIRV);

    void BuildShader(const char *vertShaderText, const char *fragShaderText);

    EShLanguage GetLanguage(const VkShaderStageFlagBits shaderType);

    void InitializeResources(TBuiltInResource &resource);
#endif

};


#endif //VULKANTESTPROJECT1_VULKANSHADER_H
