//
// Created by Raiix on 2021/2/7.
//

#include "VulkanDevice.h"
#include "VulkanInstance.h"
#include "VulkanApplication.h"

VulkanDevice::VulkanDevice(VkPhysicalDevice *_gpu) {
    gpu = _gpu;
}
VulkanDevice::~VulkanDevice() {

}


VkResult VulkanDevice::CreateDevice(std::vector<const char *> &layers, std::vector<const char *> &extensions) {
    layerAndExtension.instanceLayerNames = layers;
    layerAndExtension.instanceExtensionNames = extensions;

    VkResult res;
    float queuePriorities[1] = { 0.0 };
    VkDeviceQueueCreateInfo  queueInfo = {};
    queueInfo.queueFamilyIndex = graphicsQueueIndex;
    queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueInfo.pNext = nullptr;
    queueInfo.queueCount = 1;
    queueInfo.pQueuePriorities = queuePriorities;

    VkDeviceCreateInfo deviceInfo = {};
    deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceInfo.pNext = nullptr;
    deviceInfo.queueCreateInfoCount = 1;
    deviceInfo.pQueueCreateInfos = &queueInfo;
    deviceInfo.enabledLayerCount = 0;
    deviceInfo.ppEnabledLayerNames = nullptr;
    deviceInfo.enabledExtensionCount = extensions.size();
    deviceInfo.ppEnabledExtensionNames = extensions.empty() ? nullptr : extensions.data();
    deviceInfo.pEnabledFeatures = nullptr;

    res = vkCreateDevice(*gpu, &deviceInfo, nullptr, &device);
    assert(res == VK_SUCCESS);

    return res;
}

void VulkanDevice::GetPhysicalDeviceQueuesAndProperties() {
    vkGetPhysicalDeviceQueueFamilyProperties(*gpu, &queueFamilyCount, nullptr);

    queueFamilyProps.resize(queueFamilyCount);

    vkGetPhysicalDeviceQueueFamilyProperties(*gpu, &queueFamilyCount, queueFamilyProps.data());
}

uint32_t VulkanDevice::GetGraphicsQueueHandle() {
    bool found = false;

    for(uint32_t i=0; i<queueFamilyCount; ++i)
    {
        if(queueFamilyProps[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            found = true;
            graphicsQueueIndex = i;
            break;
        }
    }
    return 0;
}

void VulkanDevice::DestroyDevice() {
    vkDestroyDevice(device, nullptr);
}

void VulkanDevice::GetDeviceQueue() {
    vkGetDeviceQueue(device, graphicsQueueWithPresentIndex, 0, &queue);
}

bool VulkanDevice::MemoryTypeFromProperties(uint32_t typeBits, VkFlags requirementsMask, uint32_t *typeIndex) {
    for (uint32_t i=0; i<32; ++i)
    {
        if((typeBits&1) == 1)
        {
            if((memoryProperties.memoryTypes[i].propertyFlags & requirementsMask) == requirementsMask)
            {
                *typeIndex = i;
                return true;
            }
        }
    }
    return false;
}



