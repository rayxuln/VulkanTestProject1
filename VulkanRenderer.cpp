//
// Created by Raiix on 2021/2/9.
//

#include "VulkanRenderer.h"
#include "VulkanApplication.h"
#include "Wrappers.h"
#include "MeshData.h"

VulkanRenderer::VulkanRenderer(VulkanApplication *_app, VulkanDevice *device) {
    assert(app != nullptr);
    assert(device != nullptr);

    memset(&depth, 0, sizeof(depth));
    memset(&winInstance, 0, sizeof(HINSTANCE));

    app = _app;
    deviceObj = device;

    swapChainObj = new VulkanSwapChain(this);
    VulkanDrawable *drawable = new VulkanDrawable(this);
    drawableList.push_back(drawable);
}

VulkanRenderer::~VulkanRenderer() {
    delete swapChainObj;
    swapChainObj = nullptr;

    for(auto d : drawableList)
    {
        delete d;
    }
    drawableList.clear();
}

void VulkanRenderer::Initialize() {
    swapChainObj->InitializeSwapChain();

    CreateCommandPool();

    BuildSwapChainAndDepthImage();

    CreateVertexBuffer();

    const bool includeDepth = true;
    CreateRenderPass(includeDepth);
    CreateFrameBuffers(includeDepth);

    CreateShaders();

    CreateDescriptors();

    CreatePipelineStateManagement();
}

bool VulkanRenderer::Render() {
    MSG msg;
    PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE);
    if(msg.message == WM_QUIT)
    {
        return false;
    }
    TranslateMessage(&msg);
    DispatchMessage(&msg);


//    RedrawWindow(winHandle, nullptr, nullptr, RDW_INTERNALPAINT);
    static const ULONGLONG interFrame = 16; //ms for 60fps
    static ULONGLONG lastTime = 0;

    auto currentTime = Utils::GetCurrentTimeMilliSec();
    auto time = currentTime - lastTime;
    if(time < interFrame)
    {
        Sleep((interFrame - time)/2);
    }
    currentTime = Utils::GetCurrentTimeMilliSec();
    lastTime = currentTime;

    if(!app->isResizing)
    {
        for(auto d:drawableList)
        {
            d->Render();
        }
    }

    return true;
}

void VulkanRenderer::CreatePresentationWindow(const int &_width, const int &_height) {
    width = _width;
    height = _height;
    assert(width > 0 || height > 0);

    WNDCLASSEX winInfo;

    sprintf(title, "一个超长的Vulkan的Hello world示例");
    memset(&winInfo, 0, sizeof(WNDCLASSEX));

    winInfo.cbSize = sizeof(WNDCLASSEX);
    winInfo.style = CS_HREDRAW | CS_VREDRAW;
    winInfo.lpfnWndProc = WndProc;
    winInfo.cbClsExtra = 0;
    winInfo.cbWndExtra = 0;
    winInfo.hInstance = winInstance;
    winInfo.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    winInfo.hCursor = LoadCursor(nullptr, IDC_ARROW);
    winInfo.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    winInfo.lpszMenuName = nullptr;
    winInfo.lpszClassName = "RAIIX_VULKAN_WINDOW_CLASS";
    winInfo.hIconSm = LoadIcon(nullptr, IDI_WINLOGO);

    if(!RegisterClassEx(&winInfo))
    {
        std::cout<<"Can't not register the window class!"<<std::endl;
        fflush(stdout);
        exit(1);
    }

    RECT wr = { 0, 0, width, height};
    AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);
    winHandle = CreateWindowEx(0,
                                 "RAIIX_VULKAN_WINDOW_CLASS",
                                 title,
                                 WS_OVERLAPPEDWINDOW | WS_VISIBLE | WS_SYSMENU,
                                 100, 100,
                                 wr.right - wr.left,
                                 wr.bottom - wr.top,
                                 nullptr,
                                 nullptr,
                                 winInstance,
                                 nullptr
                                 );

    if(!winHandle)
    {
        std::cout<<"Can't create window!"<<std::endl;
        fflush(stdout);
        exit(1);
    }

    SetWindowLongPtr(winHandle, GWLP_USERDATA, (LONG_PTR)&app);

}

void VulkanRenderer::DestroyPresentationWindow() {
    DestroyWindow(winHandle);
}

LRESULT VulkanRenderer::WndProc(HWND winHandle, UINT msg, WPARAM wparam, LPARAM lparam) {
    VulkanApplication *app = VulkanApplication::Instance();


    switch (msg) {
        case WM_CLOSE:
            PostQuitMessage(0);
            break;
        case WM_PAINT:
        {

        }
        break;
        case WM_SIZE:
            if (wparam != SIZE_MINIMIZED)
            {
                app->rendererObj->width = LOWORD(lparam);
                app->rendererObj->height = HIWORD(lparam);
                app->rendererObj->GetSwapChain()->SetSwapChainExtent(app->rendererObj->width, app->rendererObj->height);
                app->Resize();
            }
            break;
        case WM_GETMINMAXINFO:
        {
            auto minmaxinfo = (LPMINMAXINFO)lparam;
            minmaxinfo->ptMinTrackSize.x = 50;
            minmaxinfo->ptMinTrackSize.y = 50;
            return 0;
        }
        default:
            break;
    }
    return DefWindowProc(winHandle, msg, wparam, lparam);
}

void VulkanRenderer::CreateCommandPool() {
    VkResult res;

    VkCommandPoolCreateInfo cmdPoolInfo = {};
    cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    cmdPoolInfo.pNext = nullptr;
    cmdPoolInfo.queueFamilyIndex = deviceObj->graphicsQueueWithPresentIndex;
    cmdPoolInfo.flags = 0;

    res = vkCreateCommandPool(deviceObj->device, &cmdPoolInfo, nullptr, &cmdPool);
    assert(res == VK_SUCCESS);
}

void VulkanRenderer::CreateDepthImage() {
    VkResult res;
    bool pass;

    VkImageCreateInfo imageInfo = {};

    if(depth.format == VK_FORMAT_UNDEFINED)
    {
        depth.format = VK_FORMAT_D16_UNORM;
    }

    VkFormatProperties props;
    vkGetPhysicalDeviceFormatProperties(*deviceObj->gpu, depth.format, &props);
    if(props.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
    {
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    }else if(props.linearTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
    {
        imageInfo.tiling = VK_IMAGE_TILING_LINEAR;
    }else {
        std::cout<<"Unsupported depth format!"<<std::endl;
        exit(-1);
    }

    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.pNext = nullptr;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.format = depth.format;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.samples = NUM_SAMPLES;
    imageInfo.queueFamilyIndexCount = 0;
    imageInfo.pQueueFamilyIndices = nullptr;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    imageInfo.flags = 0;

    res = vkCreateImage(deviceObj->device, &imageInfo, nullptr, &depth.image);
    assert(res == VK_SUCCESS);

    VkMemoryRequirements  memReq;
    vkGetImageMemoryRequirements(deviceObj->device, depth.image, &memReq);

    VkMemoryAllocateInfo memAlloc = {};
    memAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memAlloc.pNext = nullptr;
    memAlloc.allocationSize = 0;
    memAlloc.memoryTypeIndex = 0;
    memAlloc.allocationSize = memReq.size;

    pass = deviceObj->MemoryTypeFromProperties(memReq.memoryTypeBits, 0, &memAlloc.memoryTypeIndex);
    assert(pass);

    res = vkAllocateMemory(deviceObj->device, &memAlloc, nullptr, &depth.mem);
    assert(res == VK_SUCCESS);

    res = vkBindImageMemory(deviceObj->device, depth.image, depth.mem, 0);
    assert(res == VK_SUCCESS);

    VkImageViewCreateInfo imgViewInfo = {};
    imgViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    imgViewInfo.pNext = nullptr;
    imgViewInfo.image = VK_NULL_HANDLE;
    imgViewInfo.format = depth.format;
    imgViewInfo.components = {VK_COMPONENT_SWIZZLE_IDENTITY };
    imgViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    imgViewInfo.subresourceRange.baseMipLevel = 0;
    imgViewInfo.subresourceRange.levelCount = 1;
    imgViewInfo.subresourceRange.baseArrayLayer = 0;
    imgViewInfo.subresourceRange.layerCount = 1;
    imgViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    imgViewInfo.flags = 0;

    if (depth.format == VK_FORMAT_D16_UNORM_S8_UINT ||
        depth.format == VK_FORMAT_D24_UNORM_S8_UINT ||
        depth.format == VK_FORMAT_D32_SFLOAT_S8_UINT)
    {
        imgViewInfo.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
    }

    CommandBufferManager::AllocCommandBuffer(&deviceObj->device, cmdPool, &cmdDepthImage);
    CommandBufferManager::BeginCommandBuffer(cmdDepthImage);
    {
        SetImageLayout(depth.image, imgViewInfo.subresourceRange.aspectMask, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, (VkAccessFlagBits)0, cmdDepthImage);
    }
    CommandBufferManager::EndCommandBuffer(cmdDepthImage);
    CommandBufferManager::SubmitCommandBuffer(deviceObj->queue, &cmdDepthImage);

    imgViewInfo.image = depth.image;
    res = vkCreateImageView(deviceObj->device, &imgViewInfo, nullptr, &depth.view);
    assert(res == VK_SUCCESS);
}

void VulkanRenderer::DestroyDepthBuffer() {
    vkDestroyImageView(deviceObj->device, depth.view, nullptr);
    vkDestroyImage(deviceObj->device, depth.image, nullptr);
    vkFreeMemory(deviceObj->device, depth.mem, nullptr);
}

void VulkanRenderer::DestroyCommandBuffer() {
    VkCommandBuffer cmdBufs[] = { cmdDepthImage, cmdVertexBuffer };
    vkFreeCommandBuffers(deviceObj->device, cmdPool, sizeof(cmdBufs)/sizeof(VkCommandBuffer), cmdBufs);
}

void VulkanRenderer::DestroyCommandPool() {
    vkDestroyCommandPool(deviceObj->device, cmdPool, nullptr);
}

void VulkanRenderer::BuildSwapChainAndDepthImage() {
    deviceObj->GetDeviceQueue();

    swapChainObj->CreateSwapChain();

    CreateDepthImage();
}

void VulkanRenderer::SetImageLayout(VkImage image, VkImageAspectFlags aspectFlags, VkImageLayout oldImageLayout,
                                    VkImageLayout newImageLayout, VkAccessFlagBits srcAccessMask,
                                    VkCommandBuffer const &cmdBuf) {
    assert(cmdBuf != VK_NULL_HANDLE);

    assert(deviceObj->queue != VK_NULL_HANDLE);

    VkImageMemoryBarrier imgMemoryBarrier = {};
    imgMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    imgMemoryBarrier.pNext = nullptr;
    imgMemoryBarrier.srcAccessMask = srcAccessMask;
    imgMemoryBarrier.dstAccessMask = 0;
    imgMemoryBarrier.oldLayout = oldImageLayout;
    imgMemoryBarrier.newLayout = newImageLayout;
    imgMemoryBarrier.image = image;
    imgMemoryBarrier.subresourceRange.aspectMask = aspectFlags;
    imgMemoryBarrier.subresourceRange.baseMipLevel = 0;
    imgMemoryBarrier.subresourceRange.levelCount = 1;
    imgMemoryBarrier.subresourceRange.layerCount = 1;

    if(oldImageLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
    {
        imgMemoryBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    }

    switch (newImageLayout)
    {
        case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
        case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
            imgMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            break;

        case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
            imgMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            imgMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            break;

        case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
            imgMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
            break;

        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
            imgMemoryBarrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            break;
    }

    VkPipelineStageFlags srcStages = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    VkPipelineStageFlags dstStages = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

    if(imgMemoryBarrier.dstAccessMask & VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT)
    {
        dstStages = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    }

    vkCmdPipelineBarrier(cmdBuf, srcStages, dstStages, 0, 0, nullptr, 0, nullptr, 1, &imgMemoryBarrier);
}

void VulkanRenderer::CreateVertexBuffer() {
    CommandBufferManager::AllocCommandBuffer(&deviceObj->device, cmdPool, &cmdVertexBuffer);
    CommandBufferManager::BeginCommandBuffer(cmdVertexBuffer);

    for(auto d : drawableList)
    {
//        d->CreateVertexBuffer(triangleData, sizeof(triangleData), sizeof(triangleData[0]), false);
//        d->CreateVertexIndex(squareIndices, sizeof(squareIndices), 0);
        d->CreateVertexBuffer(geometryData, sizeof(geometryData), sizeof(geometryData[0]), false);
    }

    CommandBufferManager::EndCommandBuffer(cmdVertexBuffer);
    CommandBufferManager::SubmitCommandBuffer(deviceObj->queue, &cmdVertexBuffer);
}

void VulkanRenderer::DestroyDrawableVertexBuffer() {
    for(auto d : drawableList)
    {
        d->DestroyVertexBuffer();
//        d->DestroyVertexIndex();
    }
}

void VulkanRenderer::CreateRenderPass(bool includeDepth, bool clear) {
    VkResult res;

    VkAttachmentDescription attachments[2];
    attachments[0].format = swapChainObj->publicVars.format;
    attachments[0].samples = NUM_SAMPLES;
    attachments[0].loadOp = clear ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    attachments[0].flags = VK_ATTACHMENT_DESCRIPTION_MAY_ALIAS_BIT;

    if(includeDepth)
    {
        attachments[1].format = depth.format;
        attachments[1].samples = NUM_SAMPLES;
        attachments[1].loadOp = clear ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
        attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        attachments[1].flags = VK_ATTACHMENT_DESCRIPTION_MAY_ALIAS_BIT;
    }

    VkAttachmentReference colorRef = {};
    colorRef.attachment = 0;
    colorRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthRef = {};
    depthRef.attachment = 1;
    depthRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.flags = 0;
    subpass.inputAttachmentCount = 0;
    subpass.pInputAttachments = nullptr;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorRef;
    subpass.pResolveAttachments = nullptr;
    subpass.pDepthStencilAttachment = includeDepth ? &depthRef : nullptr;
    subpass.preserveAttachmentCount = 0;
    subpass.pPreserveAttachments = nullptr;

    VkRenderPassCreateInfo rpInfo = {};
    rpInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    rpInfo.pNext = nullptr;
    rpInfo.attachmentCount = includeDepth ? 2 : 1;
    rpInfo.pAttachments = attachments;
    rpInfo.subpassCount = 1;
    rpInfo.pSubpasses = &subpass;
    rpInfo.pDependencies = nullptr;

    res = vkCreateRenderPass(deviceObj->device, &rpInfo, nullptr, &renderPass);
    assert(res == VK_SUCCESS);
}

void VulkanRenderer::DestroyRenderpass() {
    vkDestroyRenderPass(deviceObj->device, renderPass, nullptr);
}

void VulkanRenderer::CreateFrameBuffers(bool includeDepth) {
    VkResult res;
    VkImageView attachments[2];
    attachments[1] = depth.view;

    VkFramebufferCreateInfo fbInfo = {};
    fbInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    fbInfo.pNext = nullptr;
    fbInfo.renderPass = renderPass;
    fbInfo.attachmentCount = includeDepth ? 2 : 1;
    fbInfo.pAttachments = attachments;
    fbInfo.width = width;
    fbInfo.height = height;
    fbInfo.layers = 1;

    frameBuffers.clear();
    frameBuffers.resize(swapChainObj->publicVars.swapChainImageCount);
    for(uint32_t i=0; i<swapChainObj->publicVars.swapChainImageCount; ++i)
    {
        attachments[0] = swapChainObj->publicVars.colorBuffer[i].view;
        res = vkCreateFramebuffer(deviceObj->device, &fbInfo, nullptr, &frameBuffers.at(i));
        assert(res == VK_SUCCESS);
    }
}

void VulkanRenderer::DestroyFrameBuffers() {
    for(uint32_t i=0; i<swapChainObj->publicVars.swapChainImageCount; ++i)
    {
        vkDestroyFramebuffer(deviceObj->device, frameBuffers.at(i), nullptr);
    }
    frameBuffers.clear();
}

void VulkanRenderer::Prepare() {
    for(auto d:drawableList)
    {
        d->Prepare();
    }
}

void VulkanRenderer::CreateShaders() {
    if(app->isResizing) return;

    size_t vertSize, fragSize;

    std::string workingDir = app->GetWorkingDir();

    auto vert = (uint32_t *)Utils::ReadFile(workingDir + "/shaders/draw_vert.spv", &vertSize);
    auto frag = (uint32_t *)Utils::ReadFile(workingDir + "/shaders/draw_frag.spv", &fragSize);

    shaderObj.BuildShaderModuleWithSPV(vert, vertSize, frag, fragSize);

    free(vert);
    free(frag);
}

void VulkanRenderer::CreatePipelineStateManagement() {
    for(auto d:drawableList)
    {
        d->CreatePipelineLayout();
    }

    pipelineObj.CreatePipelineCache();

    const bool depthPresent = true;
    for(auto d : drawableList)
    {
        VkPipeline *pipeline = (VkPipeline*) malloc(sizeof(VkPipeline));
        if(pipelineObj.CreatePipeline(d, pipeline, &shaderObj, depthPresent))
        {
            pipelineList.push_back(pipeline);
            d->SetPipeline(pipeline);
        }else
        {
            free(pipeline);
            pipeline = nullptr;
        }
    }
}

void VulkanRenderer::DestroyPipeline() {
    for(auto p : pipelineList)
    {
        vkDestroyPipeline(deviceObj->device, *p, nullptr);
        free(p);
    }
    pipelineList.clear();
}

void VulkanRenderer::DestroyDrawableCommandBuffer() {
    for(auto d : drawableList)
    {
        d->DestroyCommandBuffer();
    }
}

void VulkanRenderer::DestroyDrawableSynchronizationObjects() {
    for(auto d : drawableList)
    {
        d->DestroySynchronizationObjects();
    }
}

void VulkanRenderer::Update() {
    for(auto d:drawableList)
    {
        d->Update();
    }
}

void VulkanRenderer::DestroyDrawableUniformBuffer() {
    for(auto d:drawableList)
    {
        d->DestroyUniformBuffer();
    }
}

void VulkanRenderer::CreateDescriptors() {
    for(auto d:drawableList)
    {
        d->CreateDescriptorSetLayout(false);
        d->CreateDescriptor(false);
    }
}












