//
// Created by Raiix on 2021/2/9.
//

#ifndef VULKANTESTPROJECT1_VULKANSWAPCHAIN_H
#define VULKANTESTPROJECT1_VULKANSWAPCHAIN_H

#include "Commons.h"

class VulkanInstance;
class VulkanDevice;
class VulkanRenderer;
class VulkanApplication;

struct SwapChainBuffer{
    VkImage image;
    VkImageView view;
};

struct SwapChainPrivateVariables{
    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    uint32_t presentModeCount;
    std::vector<VkPresentModeKHR> presentModes;
    VkExtent2D swapChainExtent;
    uint32_t desiredNumberOfSwapChainImages;
    VkSurfaceTransformFlagBitsKHR preTransform;
    VkPresentModeKHR swapChainPresentMode;
    std::vector<VkImage> swapChainImages;
    std::vector<VkSurfaceFormatKHR> surfaceFormats;
};

struct SwapChainPublicVariables{
    VkSurfaceKHR surface;
    uint32_t swapChainImageCount;
    VkSwapchainKHR swapChain;
    std::vector<SwapChainBuffer> colorBuffer;
    VkSemaphore presentCompleteSemaphore;
    uint32_t currentColorBuffer;
    VkFormat format;
};

class VulkanSwapChain {
    PFN_vkGetPhysicalDeviceSurfaceSupportKHR GetPhysicalDeviceSurfaceSupportKHR;
    PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR GetPhysicalDeviceSurfaceCapabilitiesKHR;
    PFN_vkGetPhysicalDeviceSurfaceFormatsKHR GetPhysicalDeviceSurfaceFormatsKHR;
    PFN_vkGetPhysicalDeviceSurfacePresentModesKHR GetPhysicalDeviceSurfacePresentModesKHR;
    PFN_vkDestroySurfaceKHR DestroySurfaceKHR;

    PFN_vkCreateSwapchainKHR CreateSwapchainKHR;
    PFN_vkDestroySwapchainKHR DestroySwapchainKHR;
    PFN_vkGetSwapchainImagesKHR GetSwapchainImagesKHR;

    SwapChainPrivateVariables privateVars;
    VulkanRenderer *rendererObj;
    VulkanApplication *app;

    VkResult CreateSwapChainExtensions();
    void GetSupportedFormats();
    VkResult CreateSurface();
    uint32_t GetGraphicsQueueWithPresentationSupport();
    void GetSurfaceCapabilitiesAndPresentMode();
    void ManagePresentMode();
    void CreateSwapChainColorImages();
    void CreateColorImageView();

public:
    SwapChainPublicVariables publicVars;
    PFN_vkQueuePresentKHR QueuePresentKHR;
    PFN_vkAcquireNextImageKHR AcquireNextImageKHR;

    VulkanSwapChain(VulkanRenderer *renderer);
    ~VulkanSwapChain();
    void InitializeSwapChain();
    void CreateSwapChain();
    void DestroySwapChain();
    void SetSwapChainExtent(uint32_t w, uint32_t h);
};


#endif //VULKANTESTPROJECT1_VULKANSWAPCHAIN_H
