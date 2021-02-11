//
// Created by Raiix on 2021/2/9.
//

#include "VulkanSwapChain.h"

#include "VulkanDevice.h"
#include "VulkanInstance.h"
#include "VulkanRenderer.h"
#include "VulkanApplication.h"

#define INSTANCE_FUNC_PTR(ins, entry){ \
    entry = (PFN_vk##entry) vkGetInstanceProcAddr(ins, "vk"#entry); \
    if (entry == nullptr){             \
        std::cout << "Can't find instance func: "#entry<<std::endl; \
        exit(-1);\
    }                                       \
}

#define DEVICE_FUNC_PTR(dev, entry){ \
    entry = (PFN_vk##entry) vkGetDeviceProcAddr(dev, "vk"#entry); \
    if (entry == nullptr){             \
        std::cout << "Can't find device func: "#entry<<std::endl; \
        exit(-1);\
    } \
}

VulkanSwapChain::VulkanSwapChain(VulkanRenderer *renderer) {
    rendererObj = renderer;
    app = VulkanApplication::Instance();
}

VulkanSwapChain::~VulkanSwapChain() {
    privateVars.swapChainImages.clear();
    privateVars.surfaceFormats.clear();
    privateVars.presentModes.clear();
}

VkResult VulkanSwapChain::CreateSwapChainExtensions() {
    VkInstance &instance = app->instanceObj.instance;
    VkDevice &device = app->deviceObj->device;

    INSTANCE_FUNC_PTR(instance, GetPhysicalDeviceSurfaceSupportKHR);
    INSTANCE_FUNC_PTR(instance, GetPhysicalDeviceSurfaceCapabilitiesKHR);
    INSTANCE_FUNC_PTR(instance, GetPhysicalDeviceSurfaceFormatsKHR);
    INSTANCE_FUNC_PTR(instance, GetPhysicalDeviceSurfacePresentModesKHR);
    INSTANCE_FUNC_PTR(instance, DestroySurfaceKHR);

    DEVICE_FUNC_PTR(device, CreateSwapchainKHR);
    DEVICE_FUNC_PTR(device, DestroySwapchainKHR);
    DEVICE_FUNC_PTR(device, GetSwapchainImagesKHR);
    DEVICE_FUNC_PTR(device, AcquireNextImageKHR);
    DEVICE_FUNC_PTR(device, QueuePresentKHR);

    return VK_SUCCESS;
}

VkResult VulkanSwapChain::CreateSurface() {
    VkResult res;

    VkInstance &instance = app->instanceObj.instance;

    VkWin32SurfaceCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    createInfo.pNext = nullptr;
    createInfo.hinstance = rendererObj->winInstance;
    createInfo.hwnd = rendererObj->winHandle;

    res = vkCreateWin32SurfaceKHR(instance, &createInfo, nullptr, &publicVars.surface);
    assert(res == VK_SUCCESS);

    return res;
}

uint32_t VulkanSwapChain::GetGraphicsQueueWithPresentationSupport() {
    VulkanDevice *device = app->deviceObj;
    uint32_t  queueCount = device->queueFamilyCount;
    VkPhysicalDevice gpu = *device->gpu;
    std::vector<VkQueueFamilyProperties> &queueProps = device->queueFamilyProps;

    VkBool32 *supportsPresent = (VkBool32*) malloc(queueCount * sizeof(VkBool32));
    for(uint32_t i=0; i<queueCount; ++i)
    {
        GetPhysicalDeviceSurfaceSupportKHR(gpu, i, publicVars.surface, &supportsPresent[i]);
    }

    uint32_t graphicsQueueNodeIndex = UINT32_MAX;
    uint32_t presentQueueNodeIndex = UINT32_MAX;
    for(uint32_t i=0; i<queueCount; ++i)
    {
        if((queueProps[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0)
        {
            if(graphicsQueueNodeIndex == UINT32_MAX)
            {
                graphicsQueueNodeIndex = i;
            }

            if(supportsPresent[i] == VK_TRUE)
            {
                graphicsQueueNodeIndex = i;
                presentQueueNodeIndex = i;
                break;
            }
        }
    }

    if(presentQueueNodeIndex == UINT32_MAX)
    {
        for(uint32_t i=0; i<queueCount; ++i)
        {
            if(supportsPresent[i] == VK_TRUE)
            {
                presentQueueNodeIndex = i;
                break;
            }
        }
    }

    free(supportsPresent);

    if(graphicsQueueNodeIndex == UINT32_MAX || presentQueueNodeIndex == UINT32_MAX)
    {
        return UINT32_MAX;
    }

    return graphicsQueueNodeIndex;
}

void VulkanSwapChain::GetSupportedFormats() {
    VkPhysicalDevice gpu = *app->deviceObj->gpu;
    VkResult res;

    uint32_t formatCount;
    res = GetPhysicalDeviceSurfaceFormatsKHR(gpu, publicVars.surface, &formatCount, nullptr);
    assert(res == VK_SUCCESS);

    privateVars.surfaceFormats.clear();
    privateVars.surfaceFormats.resize(formatCount);

    res = GetPhysicalDeviceSurfaceFormatsKHR(gpu, publicVars.surface, &formatCount, privateVars.surfaceFormats.data());
    assert(res == VK_SUCCESS);

    if(formatCount == 1 && privateVars.surfaceFormats[0] .format == VK_FORMAT_UNDEFINED)
    {
        publicVars.format = VK_FORMAT_B8G8R8A8_UNORM;
    }else
    {
        assert(formatCount >= 1);
        publicVars.format = privateVars.surfaceFormats[0].format;
    }

}

void VulkanSwapChain::InitializeSwapChain() {
    CreateSwapChainExtensions();

    CreateSurface();

    uint32_t index = GetGraphicsQueueWithPresentationSupport();
    if(index == UINT32_MAX)
    {
        std::cout<<"Can't find a graphics and a present queue!"<<std::endl;
        exit(-1);
    }
    app->deviceObj->graphicsQueueWithPresentIndex = index;

    GetSupportedFormats();
}

void VulkanSwapChain::CreateSwapChain() {
    GetSurfaceCapabilitiesAndPresentMode();

    ManagePresentMode();

    CreateSwapChainColorImages();

    CreateColorImageView();
}

void VulkanSwapChain::GetSurfaceCapabilitiesAndPresentMode() {
    VkResult res;
    VkPhysicalDevice gpu = *app->deviceObj->gpu;

    res = GetPhysicalDeviceSurfaceCapabilitiesKHR(gpu, publicVars.surface, &privateVars.surfaceCapabilities);
    assert(res == VK_SUCCESS);

    res = GetPhysicalDeviceSurfacePresentModesKHR(gpu, publicVars.surface, &privateVars.presentModeCount, nullptr);
    assert(res == VK_SUCCESS);

    privateVars.presentModes.clear();
    privateVars.presentModes.resize(privateVars.presentModeCount);
    assert(privateVars.presentModes.size()>=1);

    res = GetPhysicalDeviceSurfacePresentModesKHR(gpu, publicVars.surface, &privateVars.presentModeCount, privateVars.presentModes.data());
    assert(res == VK_SUCCESS);

    if(privateVars.surfaceCapabilities.currentExtent.width == UINT32_MAX)
    {
        privateVars.swapChainExtent.width = rendererObj->width;
        privateVars.swapChainExtent.height = rendererObj->height;
    }else
    {
        privateVars.swapChainExtent = privateVars.surfaceCapabilities.currentExtent;
    }
}

void VulkanSwapChain::ManagePresentMode() {
    privateVars.swapChainPresentMode = VK_PRESENT_MODE_FIFO_KHR;
    for(size_t i=0; i<privateVars.presentModeCount; ++i)
    {
        if(privateVars.presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            privateVars.swapChainPresentMode = VK_PRESENT_MODE_MAILBOX_KHR;
            break;
        }
        if(privateVars.swapChainPresentMode != VK_PRESENT_MODE_MAILBOX_KHR && privateVars.presentModes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR)
        {
            privateVars.swapChainPresentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
        }
    }

    privateVars.desiredNumberOfSwapChainImages = privateVars.surfaceCapabilities.minImageCount + 1;
    if(privateVars.surfaceCapabilities.maxImageCount > 0 && privateVars.desiredNumberOfSwapChainImages > privateVars.surfaceCapabilities.maxImageCount)
    {
        privateVars.desiredNumberOfSwapChainImages = privateVars.surfaceCapabilities.maxImageCount;
    }

    if(privateVars.surfaceCapabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
    {
        privateVars.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    }else
    {
        privateVars.preTransform = privateVars.surfaceCapabilities.currentTransform;
    }
}

void VulkanSwapChain::CreateSwapChainColorImages() {
    VkResult res;

    VkSwapchainCreateInfoKHR swapChainInfo = {};
    swapChainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapChainInfo.pNext = nullptr;
    swapChainInfo.surface = publicVars.surface;
    swapChainInfo.minImageCount = privateVars.desiredNumberOfSwapChainImages;
    swapChainInfo.imageFormat = publicVars.format;
    swapChainInfo.imageExtent.width = privateVars.swapChainExtent.width;
    swapChainInfo.imageExtent.height = privateVars.swapChainExtent.height;
    swapChainInfo.preTransform = privateVars.preTransform;
    swapChainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapChainInfo.imageArrayLayers = 1;
    swapChainInfo.presentMode = privateVars.swapChainPresentMode;
    swapChainInfo.oldSwapchain = VK_NULL_HANDLE;
    swapChainInfo.clipped = true;
    swapChainInfo.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    swapChainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    swapChainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapChainInfo.queueFamilyIndexCount = 0;
    swapChainInfo.pQueueFamilyIndices = nullptr;

    res = CreateSwapchainKHR(app->deviceObj->device, &swapChainInfo, nullptr, &publicVars.swapChain);
    assert(res == VK_SUCCESS);

    res = GetSwapchainImagesKHR(app->deviceObj->device, publicVars.swapChain, &publicVars.swapChainImageCount, nullptr);
    assert(res == VK_SUCCESS);

    privateVars.swapChainImages.clear();
    privateVars.swapChainImages.resize(publicVars.swapChainImageCount);
    res = GetSwapchainImagesKHR(app->deviceObj->device, publicVars.swapChain, &publicVars.swapChainImageCount, privateVars.swapChainImages.data());
    assert(res == VK_SUCCESS);
}

void VulkanSwapChain::CreateColorImageView() {
    VkResult res;
    publicVars.colorBuffer.clear();
    for(uint32_t i=0; i<publicVars.swapChainImageCount; ++i)
    {
        SwapChainBuffer buffer;

        VkImageViewCreateInfo imgViewInfo = {};
        imgViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imgViewInfo.pNext = nullptr;
        imgViewInfo.format = publicVars.format;
        imgViewInfo.components = { VK_COMPONENT_SWIZZLE_IDENTITY };
        imgViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imgViewInfo.subresourceRange.baseMipLevel = 0;
        imgViewInfo.subresourceRange.levelCount = 1;
        imgViewInfo.subresourceRange.baseArrayLayer = 0;
        imgViewInfo.subresourceRange.layerCount = 1;
        imgViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        imgViewInfo.flags = 0;

        buffer.image = privateVars.swapChainImages[i];
        imgViewInfo.image = buffer.image;

        res = vkCreateImageView(app->deviceObj->device, &imgViewInfo, nullptr, &buffer.view);
        assert(res == VK_SUCCESS);

        publicVars.colorBuffer.push_back(buffer);
    }
    publicVars.currentColorBuffer = 0;
}

void VulkanSwapChain::DestroySwapChain() {
    VulkanDevice *deviceObj = app->deviceObj;

    for(uint32_t i=0; i<publicVars.swapChainImageCount; ++i)
    {
        vkDestroyImageView(deviceObj->device, publicVars.colorBuffer[i].view, nullptr);
    }

//    if(!app->isResizing)
//    {
        DestroySwapchainKHR(deviceObj->device, publicVars.swapChain, nullptr);
        vkDestroySurfaceKHR(app->instanceObj.instance, publicVars.surface, nullptr);
//    }
}

void VulkanSwapChain::SetSwapChainExtent(uint32_t w, uint32_t h) {
    privateVars.swapChainExtent.width = w;
    privateVars.swapChainExtent.height = h;
}
