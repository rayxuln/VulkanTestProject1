//
// Created by Raiix on 2021/2/11.
//

#ifndef VULKANTESTPROJECT1_VULKANDESCRIPTOR_H
#define VULKANTESTPROJECT1_VULKANDESCRIPTOR_H

#include "Commons.h"

class VulkanDevice;

class VulkanDescriptor {
    VulkanDevice *deviceObj;
public:
    VkPipelineLayout pipelineLayout;
    std::vector<VkDescriptorSetLayout> descLayoutList;
    VkDescriptorPool descriptorPool;
    std::vector<VkDescriptorSet> descSetList;

    VulkanDescriptor();
    ~VulkanDescriptor();

    void CreateDescriptor(bool useTexture);
    void DestroyDescriptor();

    virtual void CreateDescriptorSetLayout(bool useTexture) = 0;
    void DestroyDescriptorLayout();

    virtual void CreateDescriptorPool(bool useTexture) = 0;
    void DestroyDescriptorPool();

    virtual void CreateDescriptorResources() = 0;

    virtual void CreateDescriptorSet(bool useTexture) = 0;
    void DestroyDescriptorSet();

    virtual void CreatePipelineLayout() = 0;
    void DestroyPipelineLayout();
};


#endif //VULKANTESTPROJECT1_VULKANDESCRIPTOR_H
