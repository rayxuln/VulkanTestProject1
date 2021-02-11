//
// Created by Raiix on 2021/2/9.
//

#ifndef VULKANTESTPROJECT1_VULKANPIPELINE_H
#define VULKANTESTPROJECT1_VULKANPIPELINE_H

#include "Commons.h"
class VulkanShader;
class VulkanDrawable;
class VulkanDevice;
class VulkanApplication;

#define NUMBER_OF_VIEWPORT 1
#define NUMBER_OF_SCISSORS 1

class VulkanPipeline {
public:
    VkPipelineCache pipelineCache;
    VulkanApplication *app;
    VulkanDevice *deviceObj;

    VulkanPipeline();
    ~VulkanPipeline();

    void CreatePipelineCache();

    bool CreatePipeline(VulkanDrawable *drawableObj, VkPipeline *pipeline, VulkanShader *shaderObj, VkBool32 includeDepth, VkBool32 includeVi = true);

    void DestroyPipelineCache();

};


#endif //VULKANTESTPROJECT1_VULKANPIPELINE_H
