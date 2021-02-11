//
// Created by Raiix on 2021/2/7.
//

#include "VulkanInstance.h"

VkResult VulkanInstance::CreateInstance(std::vector<const char *> &layers, std::vector<const char *> &extensions,
                                        const char *applicationName) {
    layerExtension.instanceExtensionNames = extensions;
    layerExtension.instanceLayerNames = layers;

    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pNext = nullptr;
    appInfo.pApplicationName = applicationName;
    appInfo.applicationVersion = 1;
    appInfo.pEngineName = applicationName;
    appInfo.engineVersion = 1;
    appInfo.apiVersion = VK_API_VERSION_1_2;

    VkInstanceCreateInfo instInfo = {};
    instInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instInfo.pNext = &layerExtension.dbgReportCreateInfo;
    instInfo.flags = 0;
    instInfo.pApplicationInfo = &appInfo;

    instInfo.enabledLayerCount = layers.size();
    instInfo.ppEnabledLayerNames = layers.empty() ? nullptr : layers.data();

    instInfo.enabledExtensionCount = extensions.size();
    instInfo.ppEnabledExtensionNames = extensions.empty() ? nullptr : extensions.data();

    VkResult res = vkCreateInstance(&instInfo, nullptr, &instance);
    assert(res == VK_SUCCESS);

    return res;
}


void VulkanInstance::DestroyInstance() {
    vkDestroyInstance(instance, nullptr);
}

