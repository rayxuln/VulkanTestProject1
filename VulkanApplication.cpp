//
// Created by Raiix on 2021/2/7.
//

#include "VulkanApplication.h"

extern std::vector<const char*> layerNames;
extern std::vector<const char*> instanceExtensionNames;
extern std::vector<const char*> deviceExtensionNames;

VulkanApplication::VulkanApplication() {
    instanceObj.layerExtension.GetInstanceLayerProperties();

    deviceObj = nullptr;
    debugFlag = true;
    rendererObj = nullptr;

    isPrepared = false;
    isResizing = false;
}

VulkanApplication::~VulkanApplication() {
    if(rendererObj) delete rendererObj;
    rendererObj = nullptr;
}



VulkanApplication *VulkanApplication::Instance() {
    static VulkanApplication app;
    return &app;
}

VkResult VulkanApplication::CreateVulkanInstance(std::vector<const char *> &layers,
                                                 std::vector<const char *> &extensions, const char *appName) {
    return instanceObj.CreateInstance(layers, extensions, appName);;
}


VkResult VulkanApplication::EnumeratePhysicalDevices(std::vector<VkPhysicalDevice> &gpuList) {
    uint32_t gpuDeviceCount;

    vkEnumeratePhysicalDevices(instanceObj.instance, &gpuDeviceCount, nullptr);

    gpuList.resize(gpuDeviceCount);

    return vkEnumeratePhysicalDevices(instanceObj.instance, &gpuDeviceCount, gpuList.data());
}

VkResult VulkanApplication::HandShakeWithDevice(VkPhysicalDevice *gpu, std::vector<const char *> &layers,
                                                std::vector<const char *> &extensions) {
    deviceObj = new VulkanDevice(gpu);
    if(deviceObj == nullptr)
    {
        return VK_ERROR_OUT_OF_HOST_MEMORY;
    }

    deviceObj->layerAndExtension.GetDeviceExtensionProperties(gpu);

    vkGetPhysicalDeviceProperties(*gpu, &deviceObj->gpuProps);

    std::cout<<"> Using GPU: "<<deviceObj->gpuProps.deviceName<<std::endl;

    vkGetPhysicalDeviceMemoryProperties(*gpu, &deviceObj->memoryProperties);

    deviceObj->GetPhysicalDeviceQueuesAndProperties();

    deviceObj->GetGraphicsQueueHandle();

    VkPhysicalDeviceFeatures features;
    vkGetPhysicalDeviceFeatures(*gpu, &features);

    return deviceObj->CreateDevice(layers, extensions);
}

void VulkanApplication::Initialize(std::vector<std::string> &args) {
    char title[] = "你好使劲儿";
    cmdLineArgs = args;

    if(debugFlag)
    {
        instanceObj.layerExtension.AreLayersSupported(layerNames);
    }

    CreateVulkanInstance(layerNames, instanceExtensionNames, title);

    if(debugFlag)
    {
        instanceObj.layerExtension.CreateDebugReportCallback();
    }

    EnumeratePhysicalDevices(gpuList);

    if (!gpuList.empty())
    {
        HandShakeWithDevice(&gpuList[0], layerNames, deviceExtensionNames);
    }

    if(!rendererObj)
    {
        rendererObj = new VulkanRenderer(this, deviceObj);
        rendererObj->CreatePresentationWindow();
    }


    rendererObj->Initialize();
}

void VulkanApplication::DeInitialize() {
    rendererObj->DestroyPipeline();
    rendererObj->GetPipelineObject()->DestroyPipelineCache();
    for(auto d:rendererObj->GetDrawingItems())
    {
        d->DestroyDescriptor();
    }
    rendererObj->GetShader()->DestroyShaders();

    rendererObj->DestroyFrameBuffers();
    rendererObj->DestroyRenderpass();
    rendererObj->DestroyDrawableVertexBuffer();
    rendererObj->DestroyDrawableUniformBuffer();
    rendererObj->DestroyDrawableCommandBuffer();
    rendererObj->DestroyDepthBuffer();
    rendererObj->GetSwapChain()->DestroySwapChain();
    rendererObj->DestroyCommandBuffer();
    rendererObj->DestroyDrawableSynchronizationObjects();
    rendererObj->DestroyCommandPool();
    rendererObj->DestroyPresentationWindow();
    rendererObj->DestroyTextureResource();
    deviceObj->DestroyDevice();
    if(debugFlag)
    {
        instanceObj.layerExtension.DestroyDebugReportCallback();
    }
    instanceObj.DestroyInstance();
}

bool VulkanApplication::Render() {
    if (!isPrepared) return false;

    return rendererObj->Render();
}

void VulkanApplication::Prepare() {
    isPrepared = false;
    rendererObj->Prepare();
    isPrepared = true;
}

std::string VulkanApplication::GetWorkingDir() {
    auto &first = cmdLineArgs[0];
    int slash_pos = first.rfind('/');
    int back_slashpos = first.rfind('\\');
#undef max
    int pos = std::max(slash_pos, back_slashpos);
    if(pos != -1)
        return std::string(first.begin(), first.begin()+pos);
    return first;
}

void VulkanApplication::Resize() {
    if(!isPrepared) return;

    isResizing = true;

    vkDeviceWaitIdle(deviceObj->device);
    rendererObj->DestroyFrameBuffers();
    rendererObj->DestroyCommandPool();
    rendererObj->DestroyPipeline();
    rendererObj->GetPipelineObject()->DestroyPipelineCache();
    for(auto d:rendererObj->GetDrawingItems())
    {
        d->DestroyDescriptor();
    }
    rendererObj->DestroyRenderpass();
    rendererObj->GetSwapChain()->DestroySwapChain();
    rendererObj->DestroyDrawableVertexBuffer();
    rendererObj->DestroyDrawableUniformBuffer();
    rendererObj->DestroyTextureResource();
    rendererObj->DestroyDepthBuffer();
    rendererObj->Initialize();
    Prepare();

    isResizing = false;
}

void VulkanApplication::Update() {
    rendererObj->Update();
}


