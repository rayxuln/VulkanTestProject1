//
// Created by Raiix on 2021/2/9.
//

#ifndef VULKANTESTPROJECT1_WRAPPERS_H
#define VULKANTESTPROJECT1_WRAPPERS_H


#include "Commons.h"

class CommandBufferManager
{
public:
    static void AllocCommandBuffer(const VkDevice *device, const VkCommandPool cmdPool, VkCommandBuffer *cmdBuf, const VkCommandBufferAllocateInfo *cmdBufferAllocateInfo = nullptr);
    static void BeginCommandBuffer(VkCommandBuffer cmdBuf, VkCommandBufferBeginInfo *cmdBufferBeginInfo = nullptr);
    static void EndCommandBuffer(VkCommandBuffer cmdBUf);
    static void SubmitCommandBuffer(const VkQueue &queue, const VkCommandBuffer *cmdBufList, const VkSubmitInfo *inSubmitInfo = nullptr, const VkFence &fence = VK_NULL_HANDLE);
};


#endif //VULKANTESTPROJECT1_WRAPPERS_H
