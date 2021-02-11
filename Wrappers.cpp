//
// Created by Raiix on 2021/2/9.
//

#include "Wrappers.h"

void
CommandBufferManager::AllocCommandBuffer(const VkDevice *device, const VkCommandPool cmdPool, VkCommandBuffer *cmdBuf,
                                         const VkCommandBufferAllocateInfo *cmdBufferAllocateInfo) {
    VkResult  res;

    if(cmdBufferAllocateInfo) {
        res = vkAllocateCommandBuffers(*device, cmdBufferAllocateInfo, cmdBuf);
        assert(res == VK_SUCCESS);
        return;
    }

    VkCommandBufferAllocateInfo cmdInfo = {};
    cmdInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cmdInfo.pNext = nullptr;
    cmdInfo.commandPool = cmdPool;
    cmdInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cmdInfo.commandBufferCount = (uint32_t) sizeof(cmdBuf) / sizeof(VkCommandBuffer);
//    std::cout<<"> The count of cmd buffers: "<<cmdInfo.commandBufferCount<<std::endl;

    res = vkAllocateCommandBuffers(*device, &cmdInfo, cmdBuf);
    assert(res == VK_SUCCESS);
}

void CommandBufferManager::BeginCommandBuffer(VkCommandBuffer cmdBuf, VkCommandBufferBeginInfo *cmdBufferBeginInfo) {
    VkResult res;

    if(cmdBufferBeginInfo)
    {
        res = vkBeginCommandBuffer(cmdBuf, cmdBufferBeginInfo);
        assert(res == VK_SUCCESS);
        return;
    }

    VkCommandBufferInheritanceInfo cmdBufInheritInfo = {};
    cmdBufInheritInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
    cmdBufInheritInfo.pNext = nullptr;
    cmdBufInheritInfo.renderPass = VK_NULL_HANDLE;
    cmdBufInheritInfo.subpass = 0;
    cmdBufInheritInfo.framebuffer = VK_NULL_HANDLE;
    cmdBufInheritInfo.occlusionQueryEnable = VK_FALSE;
    cmdBufInheritInfo.queryFlags = 0;
    cmdBufInheritInfo.pipelineStatistics = 0;

    VkCommandBufferBeginInfo cmdInfo = {};
    cmdInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    cmdInfo.pNext = nullptr;
    cmdInfo.flags = 0;
    cmdInfo.pInheritanceInfo = &cmdBufInheritInfo;

    res = vkBeginCommandBuffer(cmdBuf, &cmdInfo);

    assert(res == VK_SUCCESS);
}

void CommandBufferManager::EndCommandBuffer(VkCommandBuffer cmdBUf) {
    VkResult res;
    res = vkEndCommandBuffer(cmdBUf);
    assert(res == VK_SUCCESS);
}

void CommandBufferManager::SubmitCommandBuffer(VkQueue const &queue, const VkCommandBuffer *cmdBufList,
                                               const VkSubmitInfo *inSubmitInfo, VkFence const &fence) {
    VkResult res;

    if(inSubmitInfo){
        res = vkQueueSubmit(queue, 1, inSubmitInfo, fence);
        assert(res == VK_SUCCESS);

        res = vkQueueWaitIdle(queue);
        assert(res == VK_SUCCESS);
        return;
    }

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pNext = nullptr;
    submitInfo.waitSemaphoreCount = 0;
    submitInfo.pWaitDstStageMask = nullptr;
    submitInfo.pWaitDstStageMask = nullptr;
    submitInfo.commandBufferCount = (uint32_t) sizeof(cmdBufList) / sizeof(VkCommandBuffer);
    submitInfo.pCommandBuffers = cmdBufList;
    submitInfo.signalSemaphoreCount = 0;
    submitInfo.pSignalSemaphores = nullptr;

    res = vkQueueSubmit(queue, 1, &submitInfo, fence);
    assert(res == VK_SUCCESS);

    res = vkQueueWaitIdle(queue);
    assert(res == VK_SUCCESS);
}



