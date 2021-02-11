//
// Created by Raiix on 2021/2/9.
//

#ifndef VULKANTESTPROJECT1_VULKANRENDERER_H
#define VULKANTESTPROJECT1_VULKANRENDERER_H

#include "Commons.h"
#include "VulkanSwapChain.h"
#include "VulkanDrawable.h"
#include "VulkanShader.h"
#include "VulkanPipeline.h"
#include "Wrappers.h"

#define NUM_SAMPLES VK_SAMPLE_COUNT_1_BIT


class VulkanRenderer {
    VulkanApplication *app;
    VulkanDevice *deviceObj;
    VulkanSwapChain *swapChainObj;
    std::vector<VulkanDrawable*> drawableList;
    VulkanShader shaderObj;
    VulkanPipeline pipelineObj;
public:
    HINSTANCE winInstance;
    char title[80];
    HWND winHandle;

    struct{
        VkFormat format;
        VkImage image;
        VkDeviceMemory mem;
        VkImageView view;
    } depth;

    VkCommandBuffer cmdDepthImage;
    VkCommandPool cmdPool;
    VkCommandBuffer cmdVertexBuffer;
    VkCommandBuffer cmdTexture;

    VkRenderPass renderPass;
    std::vector<VkFramebuffer> frameBuffers;
    std::vector<VkPipeline*> pipelineList;

    int width, height;
    TextureData texture;

    VulkanRenderer(VulkanApplication *app, VulkanDevice *device);
    ~VulkanRenderer();

    void Initialize();
    void Prepare();
    bool Render();
    void Update();

    void CreatePresentationWindow(const int &width = 800, const int &height = 500);
    void SetImageLayout(VkImage image, VkImageAspectFlags aspectFlags, VkImageLayout oldImageLayout, VkImageLayout newImageLayout, const VkImageSubresourceRange &subresourceRange, const VkCommandBuffer &cmdBuf);

    static LRESULT CALLBACK WndProc(HWND winHandle, UINT msg, WPARAM wparam, LPARAM lparam);

    void DestroyPresentationWindow();

    inline VulkanApplication *GetApplication() {return app;}
    inline VulkanDevice *GetDevice() {return deviceObj;}
    inline VulkanSwapChain *GetSwapChain() {return swapChainObj;}
    inline std::vector<VulkanDrawable*> &GetDrawingItems(){return drawableList;}
    inline VulkanShader *GetShader() {return &shaderObj;}
    inline VulkanPipeline *GetPipelineObject() {return &pipelineObj;}

    void CreateCommandPool();
    void BuildSwapChainAndDepthImage();
    void CreateDepthImage();
    void CreateVertexBuffer();
    void CreateRenderPass(bool includeDepth, bool clear=true);
    void CreateFrameBuffers(bool includeDepth);
    void CreateShaders();
    void CreatePipelineStateManagement();
    void CreateDescriptors();
    void CreateTextureLinear(const char *filename, TextureData *tex, VkImageUsageFlags imageUsageFlags = VK_IMAGE_USAGE_SAMPLED_BIT, VkFormat format = VK_FORMAT_R8G8B8A8_UNORM);
    void CreateTextureOptimal(const char *filename, TextureData *tex, VkImageUsageFlags imageUsageFlags = VK_IMAGE_USAGE_SAMPLED_BIT, VkFormat format = VK_FORMAT_R8G8B8A8_UNORM);

    void DestroyCommandBuffer();
    void DestroyCommandPool();
    void DestroyDepthBuffer();
    void DestroyDrawableVertexBuffer();
    void DestroyDrawableCommandBuffer();
    void DestroyDrawableSynchronizationObjects();
    void DestroyRenderpass();
    void DestroyFrameBuffers();
    void DestroyPipeline();
    void DestroyDrawableUniformBuffer();
    void DestroyTextureResource();
};


#endif //VULKANTESTPROJECT1_VULKANRENDERER_H
