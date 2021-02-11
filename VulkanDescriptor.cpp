//
// Created by Raiix on 2021/2/11.
//

#include "VulkanDescriptor.h"
#include "VulkanDevice.h"
#include "VulkanApplication.h"

VulkanDescriptor::VulkanDescriptor() {
    deviceObj = VulkanApplication::Instance()->deviceObj;
}

VulkanDescriptor::~VulkanDescriptor() {

}

void VulkanDescriptor::CreateDescriptor(bool useTexture) {
    CreateDescriptorResources();
    CreateDescriptorPool(useTexture);
    CreateDescriptorSet(useTexture);
}

void VulkanDescriptor::DestroyDescriptor() {
    DestroyDescriptorLayout();
    DestroyPipelineLayout();
    DestroyDescriptorSet();
    DestroyDescriptorPool();
}

void VulkanDescriptor::DestroyDescriptorLayout() {
    for(auto & i : descLayoutList)
    {
        vkDestroyDescriptorSetLayout(deviceObj->device, i, nullptr);
    }
    descLayoutList.clear();
}

void VulkanDescriptor::DestroyPipelineLayout() {
    vkDestroyPipelineLayout(deviceObj->device, pipelineLayout, nullptr);
}

void VulkanDescriptor::DestroyDescriptorPool() {
    vkDestroyDescriptorPool(deviceObj->device, descriptorPool, nullptr);
}

void VulkanDescriptor::DestroyDescriptorSet() {
    vkFreeDescriptorSets(deviceObj->device, descriptorPool, (uint32_t)descSetList.size(), descSetList.data());
    descSetList.clear();
}








