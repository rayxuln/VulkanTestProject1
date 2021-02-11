//
// Created by Raiix on 2021/2/7.
//

#ifndef VULKANTESTPROJECT1_VULKANDEVICE_H
#define VULKANTESTPROJECT1_VULKANDEVICE_H

#include "Commons.h"

#include "Commons.h"

class VulkanDevice {
public:
    VulkanDevice(VkPhysicalDevice *_gpu);
    ~VulkanDevice();

    VkDevice device;
    VkPhysicalDevice *gpu;
    VkPhysicalDeviceProperties gpuProps;
    VkPhysicalDeviceMemoryProperties memoryProperties;

    VkQueue queue;
    std::vector<VkQueueFamilyProperties> queueFamilyProps;

    uint32_t graphicsQueueIndex;
    uint32_t graphicsQueueWithPresentIndex;
    uint32_t queueFamilyCount;
    VulkanLayerAndExtension layerAndExtension;
    VkPhysicalDeviceFeatures deviceFeatures;

    VkResult CreateDevice(std::vector<const char*> &layers, std::vector<const char*> &extensions);
    void DestroyDevice();

    void GetPhysicalDeviceQueuesAndProperties();

    uint32_t GetGraphicsQueueHandle();

    void GetDeviceQueue();

    bool MemoryTypeFromProperties(uint32_t typeBits, VkFlags requirementsMask, uint32_t *typeIndex);
};


#endif //VULKANTESTPROJECT1_VULKANDEVICE_H
