//
// Created by Raiix on 2021/2/9.
//

#include "VulkanShader.h"
#include "VulkanDevice.h"
#include "VulkanApplication.h"

VulkanShader::VulkanShader() {

}

VulkanShader::~VulkanShader() {

}

void VulkanShader::BuildShaderModuleWithSPV(uint32_t *vertShaderText, size_t vertSPVSize, uint32_t *fragShaderText,
                                            size_t fragSPVSize) {
    VulkanDevice *deviceObj = VulkanApplication::Instance()->deviceObj;

    VkResult res;

    shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStages[0].pNext = nullptr;
    shaderStages[0].pSpecializationInfo = nullptr;
    shaderStages[0].flags = 0;
    shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    shaderStages[0].pName = "main";

    VkShaderModuleCreateInfo  moduleCreateInfo = {};
    moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    moduleCreateInfo.pNext = nullptr;
    moduleCreateInfo.flags = 0;
    moduleCreateInfo.codeSize = vertSPVSize;
    moduleCreateInfo.pCode = vertShaderText;
    res = vkCreateShaderModule(deviceObj->device, &moduleCreateInfo, nullptr, &shaderStages[0].module);
    assert(res == VK_SUCCESS);

    shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStages[1].pNext = nullptr;
    shaderStages[1].pSpecializationInfo = nullptr;
    shaderStages[1].flags = 0;
    shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    shaderStages[1].pName = "main";

    moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    moduleCreateInfo.pNext = nullptr;
    moduleCreateInfo.flags = 0;
    moduleCreateInfo.codeSize = fragSPVSize;
    moduleCreateInfo.pCode = fragShaderText;
    res = vkCreateShaderModule(deviceObj->device, &moduleCreateInfo, nullptr, &shaderStages[1].module);
    assert(res == VK_SUCCESS);
}

void VulkanShader::DestroyShaders() {
    VulkanDevice *deviceObj = VulkanApplication::Instance()->deviceObj;
    vkDestroyShaderModule(deviceObj->device, shaderStages[0].module, nullptr);
    vkDestroyShaderModule(deviceObj->device, shaderStages[1].module, nullptr);
}

#ifdef ENABLE_GLSL_TRANSLATION
void VulkanShader::BuildShader(const char *vertShaderText, const char *fragShaderText) {
    bool pass;

    glslang::InitializeProcess();

    std::vector<uint32_t> vertSPV;
    pass = GLSLtoSPV(VK_SHADER_STAGE_VERTEX_BIT, vertShaderText, vertSPV);
    assert(pass);

    std::vector<uint32_t> fragSPV;
    pass = GLSLtoSPV(VK_SHADER_STAGE_FRAGMENT_BIT, fragShaderText, fragSPV);
    assert(pass);

    BuildShaderModuleWithSPV(vertSPV.data(), vertSPV.size() * sizeof(uint32_t), fragSPV.data(), fragSPV.size() * sizeof(uint32_t));
}

bool VulkanShader::GLSLtoSPV(const VkShaderStageFlagBits shaderType, const char *shaderText, std::vector<uint32_t> &SPIRV) {
    glslang::TProgram *program = new glslang::TProgram;
    const char *shaderString[1];
    TBuiltInResource resource;
    InitializeResources(resource);

    EShMessages messages = (EShMessages) (EShMsgSpvRules | EShMsgVulkanRules);

    EShLanguage stage = GetLanguage(shaderType);
    glslang::TShader *shader = new glslang::TShader(stage);

    shaderString[0] = shaderText;
    shader->setStrings(shaderString, 1);

    if(!shader->parse(&resource, 100, false, messages))
    {
        std::cout<<shader->getInfoLog()<<std::endl;
        std::cout<<shader->getInfoDebugLog()<<std::endl;
        return false;
    }

    program->addShader(shader);

    if(!program->link(messages))
    {
        std::cout<<shader->getInfoLog()<<std::endl;
        std::cout<<shader->getInfoDebugLog()<<std::endl;
        return false;
    }

    glslang::GlslangToSpv(*program->getIntermediate(stage),SPIRV);
    delete program;
    delete shader;

    return true;
}

EShLanguage VulkanShader::GetLanguage(const VkShaderStageFlagBits shaderType) {
    switch (shaderType) {
        case VK_SHADER_STAGE_VERTEX_BIT:
            return EShLangVertex;
        case VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT:
            return EShLangTessControl;
        case VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT:
            return EShLangTessEvaluation;
        case VK_SHADER_STAGE_GEOMETRY_BIT:
            return EShLangGeometry;
        case VK_SHADER_STAGE_FRAGMENT_BIT:
            return EShLangFragment;
        case VK_SHADER_STAGE_COMPUTE_BIT:
            return EShLangCompute;
        default:
            std::cout<<"Unknown shader type: "<<shaderType<<"!"<<std::endl;
            exit(1);
    }
}

void VulkanShader::InitializeResources(TBuiltInResource &resource) {
    resource.maxLights = 32;
    resource.maxClipPlanes = 6;
    resource.maxTextureUnits = 32;
    resource.maxTextureCoords = 32;
    resource.maxVertexAttribs = 64;
    resource.maxVertexUniformComponents = 4096;
    resource.maxVaryingFloats = 64;
    resource.maxVertexTextureImageUnits = 32;
    resource.maxCombinedTextureImageUnits = 80;
    resource.maxTextureImageUnits = 32;
    resource.maxFragmentUniformComponents = 4096;
    resource.maxDrawBuffers = 32;
    resource.maxVertexUniformVectors = 128;
    resource.maxVaryingVectors = 8;
    resource.maxFragmentUniformVectors = 16;
    resource.maxVertexOutputVectors = 16;
    resource.maxFragmentInputVectors = 15;
    resource.minProgramTexelOffset = -8;
    resource.maxProgramTexelOffset = 7;
    resource.maxClipDistances = 8;
    resource.maxComputeWorkGroupCountX = 65535;
    resource.maxComputeWorkGroupCountY = 65535;
    resource.maxComputeWorkGroupCountZ = 65535;
    resource.maxComputeWorkGroupSizeX = 1024;
    resource.maxComputeWorkGroupSizeY = 1024;
    resource.maxComputeWorkGroupSizeZ = 64;
    resource.maxComputeUniformComponents = 1024;
    resource.maxComputeTextureImageUnits = 16;
    resource.maxComputeImageUniforms = 8;
    resource.maxComputeAtomicCounters = 8;
    resource.maxComputeAtomicCounterBuffers = 1;
    resource.maxVaryingComponents = 60;
    resource.maxVertexOutputComponents = 64;
    resource.maxGeometryInputComponents = 64;
    resource.maxGeometryOutputComponents = 128;
    resource.maxFragmentInputComponents = 128;
    resource.maxImageUnits = 8;
    resource.maxCombinedImageUnitsAndFragmentOutputs = 8;
    resource.maxCombinedShaderOutputResources = 8;
    resource.maxImageSamples = 0;
    resource.maxVertexImageUniforms = 0;
    resource.maxTessControlImageUniforms = 0;
    resource.maxTessEvaluationImageUniforms = 0;
    resource.maxGeometryImageUniforms = 0;
    resource.maxFragmentImageUniforms = 8;
    resource.maxCombinedImageUniforms = 8;
    resource.maxGeometryTextureImageUnits = 16;
    resource.maxGeometryOutputVertices = 256;
    resource.maxGeometryTotalOutputComponents = 1024;
    resource.maxGeometryUniformComponents = 1024;
    resource.maxGeometryVaryingComponents = 64;
    resource.maxTessControlInputComponents = 128;
    resource.maxTessControlOutputComponents = 128;
    resource.maxTessControlTextureImageUnits = 16;
    resource.maxTessControlUniformComponents = 1024;
    resource.maxTessControlTotalOutputComponents = 4096;
    resource.maxTessEvaluationInputComponents = 128;
    resource.maxTessEvaluationOutputComponents = 128;
    resource.maxTessEvaluationTextureImageUnits = 16;
    resource.maxTessEvaluationUniformComponents = 1024;
    resource.maxTessPatchComponents = 120;
    resource.maxPatchVertices = 32;
    resource.maxTessGenLevel = 64;
    resource.maxViewports = 16;
    resource.maxVertexAtomicCounters = 0;
    resource.maxTessControlAtomicCounters = 0;
    resource.maxTessEvaluationAtomicCounters = 0;
    resource.maxGeometryAtomicCounters = 0;
    resource.maxFragmentAtomicCounters = 8;
    resource.maxCombinedAtomicCounters = 8;
    resource.maxAtomicCounterBindings = 1;
    resource.maxVertexAtomicCounterBuffers = 0;
    resource.maxTessControlAtomicCounterBuffers = 0;
    resource.maxTessEvaluationAtomicCounterBuffers = 0;
    resource.maxGeometryAtomicCounterBuffers = 0;
    resource.maxFragmentAtomicCounterBuffers = 1;
    resource.maxCombinedAtomicCounterBuffers = 1;
    resource.maxAtomicCounterBufferSize = 16384;
    resource.maxTransformFeedbackBuffers = 4;
    resource.maxTransformFeedbackInterleavedComponents = 64;
    resource.maxCullDistances = 8;
    resource.maxCombinedClipAndCullDistances = 8;
    resource.maxSamples = 4;
    resource.limits.nonInductiveForLoops = 1;
    resource.limits.whileLoops = 1;
    resource.limits.doWhileLoops = 1;
    resource.limits.generalUniformIndexing = 1;
    resource.limits.generalAttributeMatrixVectorIndexing = 1;
    resource.limits.generalVaryingIndexing = 1;
    resource.limits.generalSamplerIndexing = 1;
    resource.limits.generalVariableIndexing = 1;
    resource.limits.generalConstantMatrixVectorIndexing = 1;
}

#endif
