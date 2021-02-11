//
// Created by Raiix on 2021/2/7.
//
#ifndef VULKANTESTPROJECT1_VULKANAPPLICATION_H
#define VULKANTESTPROJECT1_VULKANAPPLICATION_H

#include "VulkanRenderer.h"
#include "VulkanInstance.h"
#include "VulkanDevice.h"


class VulkanApplication {
private:
    bool debugFlag;
    std::vector<std::string> cmdLineArgs;
    std::vector<VkPhysicalDevice> gpuList;

    VulkanApplication();
public:
    ~VulkanApplication();

    static VulkanApplication *Instance();

    VulkanInstance instanceObj;
    VulkanDevice *deviceObj;
    VulkanRenderer *rendererObj;
    bool isPrepared;
    bool isResizing;


    VkResult CreateVulkanInstance(std::vector<const char*> &layers, std::vector<const char*> &extensions, const char *appName);
    VkResult EnumeratePhysicalDevices(std::vector<VkPhysicalDevice> &gpuList);
    VkResult HandShakeWithDevice(VkPhysicalDevice *gpu, std::vector<const char*> &layers, std::vector<const char*> &extensions);

    std::string GetWorkingDir();

    void Initialize(std::vector<std::string> &args);
    void Prepare();
    void Update();
    bool Render();
    void DeInitialize();

    void Resize();
};


#endif //VULKANTESTPROJECT1_VULKANAPPLICATION_H
