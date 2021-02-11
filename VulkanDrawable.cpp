//
// Created by Raiix on 2021/2/9.
//

#include "VulkanDrawable.h"

#include "VulkanApplication.h"
#include "VulkanRenderer.h"
#include "Wrappers.h"

VulkanDrawable::VulkanDrawable(VulkanRenderer *renderer) {
    memset(&vertexBuffer, 0, sizeof(vertexBuffer));
    rendererObj = renderer;

    VkSemaphoreCreateInfo presentCompleteSemaphoreCreateInfo = {};
    presentCompleteSemaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkSemaphoreCreateInfo drawingCompleteSemaphoreCreateInfo = {};
    drawingCompleteSemaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VulkanDevice *deviceObj = rendererObj->GetDevice();

    vkCreateSemaphore(deviceObj->device, &presentCompleteSemaphoreCreateInfo, nullptr, &presentCompleteSemaphore);
    vkCreateSemaphore(deviceObj->device, &drawingCompleteSemaphoreCreateInfo, nullptr, &drawingCompleteSemaphore);
}

VulkanDrawable::~VulkanDrawable() {

}

void
VulkanDrawable::CreateVertexBuffer(const void *vertexData, uint32_t dataSize, uint32_t dataStride, bool useTexture) {
    VulkanDevice *deviceObj = rendererObj->GetDevice();

    VkResult  res;
    bool pass;

    VkBufferCreateInfo bufInfo = {};
    bufInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufInfo.pNext = nullptr;
    bufInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    bufInfo.size = dataSize;
    bufInfo.queueFamilyIndexCount = 0;
    bufInfo.pQueueFamilyIndices = nullptr;
    bufInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    bufInfo.flags = 0;

    res = vkCreateBuffer(deviceObj->device, &bufInfo, nullptr, &vertexBuffer.buf);
    assert(res == VK_SUCCESS);

    VkMemoryRequirements memReq;
    vkGetBufferMemoryRequirements(deviceObj->device, vertexBuffer.buf, &memReq);

    VkMemoryAllocateInfo  allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.pNext = nullptr;
    allocInfo.memoryTypeIndex = 0;
    allocInfo.allocationSize = memReq.size;

    pass =deviceObj->MemoryTypeFromProperties(memReq.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &allocInfo.memoryTypeIndex);
    assert(pass);

    res = vkAllocateMemory(deviceObj->device, &allocInfo, nullptr, &vertexBuffer.mem);
    assert(res == VK_SUCCESS);

    vertexBuffer.bufferInfo.range = memReq.size;
    vertexBuffer.bufferInfo.offset = 0;

    uint8_t *data;
    res = vkMapMemory(deviceObj->device, vertexBuffer.mem, 0, memReq.size, 0, (void**)&data);
    assert(res == VK_SUCCESS);

    memcpy(data, vertexData, dataSize);

    vkUnmapMemory(deviceObj->device, vertexBuffer.mem);

    res = vkBindBufferMemory(deviceObj->device, vertexBuffer.buf, vertexBuffer.mem, 0);
    assert(res == VK_SUCCESS);

    bind.binding = 0;
    bind.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    bind.stride = dataStride;

    attr[0].binding = 0;
    attr[0].location = 0;
    attr[0].format = VK_FORMAT_R32G32B32A32_SFLOAT;
    attr[0].offset = 0;

    attr[1].binding = 0;
    attr[1].location = 1;
    attr[1].format = useTexture ? VK_FORMAT_R32G32_SFLOAT : VK_FORMAT_R32G32B32A32_SFLOAT;
    attr[1].offset = 16; // 4 components * 4 bytes = 16
}

void VulkanDrawable::DestroyVertexBuffer() {
    VulkanDevice *deviceObj = rendererObj->GetDevice();

    vkDestroyBuffer(deviceObj->device, vertexBuffer.buf, nullptr);
    vkFreeMemory(deviceObj->device, vertexBuffer.mem, nullptr);
}

void VulkanDrawable::RecordCommandBuffer(int currentBuffer, VkCommandBuffer *cmdDraw) {
    VulkanDevice *deviceObj = rendererObj->GetDevice();
    VulkanPipeline *pipelineObj = rendererObj->GetPipelineObject();

    VkClearValue clearValue[2];
    clearValue[0].color.float32[0] = 0.1f;
    clearValue[0].color.float32[1] = 0.1f;
    clearValue[0].color.float32[2] = 0.1f;
    clearValue[0].color.float32[3] = 1.0f;

    clearValue[1].depthStencil.depth = 1.0f;
    clearValue[1].depthStencil.stencil = 0.0f;

    VkRenderPassBeginInfo renderPassBeginInfo = {};
    renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBeginInfo.pNext = nullptr;
    renderPassBeginInfo.renderPass = rendererObj->renderPass;
    renderPassBeginInfo.framebuffer = rendererObj->frameBuffers[currentBuffer];
    renderPassBeginInfo.renderArea.offset.x = 0;
    renderPassBeginInfo.renderArea.offset.y = 0;
    renderPassBeginInfo.renderArea.extent.width = rendererObj->width;
    renderPassBeginInfo.renderArea.extent.height = rendererObj->height;
    renderPassBeginInfo.clearValueCount = 2;
    renderPassBeginInfo.pClearValues = clearValue;

    vkCmdBeginRenderPass(*cmdDraw, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(*cmdDraw, VK_PIPELINE_BIND_POINT_GRAPHICS, *pipeline);
    vkCmdBindDescriptorSets(*cmdDraw, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, descSetList.data(), 0,
                            nullptr);

    const VkDeviceSize offsets[1] = { 0 };
    vkCmdBindVertexBuffers(*cmdDraw, 0, 1, &vertexBuffer.buf, offsets);

//    vkCmdBindIndexBuffer(*cmdDraw, vertexIndex.idx, 0, VK_INDEX_TYPE_UINT16);

    InitViewports(cmdDraw);

    InitScissors(cmdDraw);

//    vkCmdDraw(*cmdDraw, 3, 1, 0, 0);
//    vkCmdDrawIndexed(*cmdDraw, 6, 1, 0, 0, 0);
    vkCmdDraw(*cmdDraw, 36, 1, 0, 0);

    vkCmdEndRenderPass(*cmdDraw);
}

void VulkanDrawable::Prepare() {
    VulkanDevice *device = rendererObj->GetDevice();
    cmdDrawList.resize(rendererObj->GetSwapChain()->publicVars.colorBuffer.size());

    for(size_t i=0; i<rendererObj->GetSwapChain()->publicVars.colorBuffer.size(); ++i)
    {
        CommandBufferManager::AllocCommandBuffer(&device->device, rendererObj->cmdPool, &cmdDrawList[i]);
        CommandBufferManager::BeginCommandBuffer(cmdDrawList[i]);

        RecordCommandBuffer(i, &cmdDrawList[i]);

        CommandBufferManager::EndCommandBuffer(cmdDrawList[i]);
    }
}

void VulkanDrawable::Render() {
    VulkanDevice *deviceObj = rendererObj->GetDevice();
    VulkanSwapChain *swapChainObj = rendererObj->GetSwapChain();

    uint32_t &currentColorImage = swapChainObj->publicVars.currentColorBuffer;
    VkSwapchainKHR &swapChain = swapChainObj->publicVars.swapChain;

    VkFence nullFence = VK_NULL_HANDLE;

    VkResult res;

    res = swapChainObj->AcquireNextImageKHR(deviceObj->device, swapChain, UINT64_MAX, presentCompleteSemaphore, nullFence, &currentColorImage);
    assert(res == VK_SUCCESS || res == VK_SUBOPTIMAL_KHR);

    VkPipelineStageFlags submitPipelineStages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pNext = nullptr;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &presentCompleteSemaphore;
    submitInfo.pWaitDstStageMask = &submitPipelineStages;
    submitInfo.commandBufferCount = (uint32_t)sizeof(&cmdDrawList[currentColorImage]) / sizeof(VkCommandBuffer);
    submitInfo.pCommandBuffers = &cmdDrawList[currentColorImage];
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &drawingCompleteSemaphore;

    CommandBufferManager::SubmitCommandBuffer(deviceObj->queue, &cmdDrawList[currentColorImage], &submitInfo);

    VkPresentInfoKHR present = {};
    present.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present.pNext = nullptr;
    present.swapchainCount = 1;
    present.pSwapchains = &swapChain;
    present.pImageIndices = &currentColorImage;
    present.waitSemaphoreCount = 1;
    present.pWaitSemaphores = &drawingCompleteSemaphore;
    present.pResults = nullptr;

    res = swapChainObj->QueuePresentKHR(deviceObj->queue, &present);
    assert(res == VK_SUCCESS || res == VK_SUBOPTIMAL_KHR);
}

void VulkanDrawable::InitViewports(VkCommandBuffer *cmd) {
    viewport.width = (float) rendererObj->width;
    viewport.height = (float) rendererObj->height;
    viewport.minDepth = (float) 0.0f;
    viewport.maxDepth = (float) 1.0f;
    viewport.x = 0;
    viewport.y = 0;
    vkCmdSetViewport(*cmd, 0, NUMBER_OF_VIEWPORT, &viewport);
}

void VulkanDrawable::InitScissors(VkCommandBuffer *cmd) {
    scissor.extent.width = rendererObj->width;
    scissor.extent.height = rendererObj->height;
    scissor.offset.x = 0;
    scissor.offset.y = 0;
    vkCmdSetScissor(*cmd, 0, NUMBER_OF_SCISSORS, &scissor);
}

void VulkanDrawable::DestroyCommandBuffer() {
    VulkanDevice *deviceObj = rendererObj->GetDevice();
    for(auto d:cmdDrawList)
    {
        vkFreeCommandBuffers(deviceObj->device, rendererObj->cmdPool, 1, &d);
    }
}

void VulkanDrawable::DestroySynchronizationObjects() {
    VulkanDevice *deviceObj = rendererObj->GetDevice();
    vkDestroySemaphore(deviceObj->device, presentCompleteSemaphore, nullptr);
    vkDestroySemaphore(deviceObj->device, drawingCompleteSemaphore, nullptr);
}

void VulkanDrawable::CreateVertexIndex(const void *indexData, uint32_t dataSize, uint32_t dataStride) {
    VulkanDevice *deviceObj = rendererObj->GetDevice();

    VkResult res;
    bool pass;

    VkBufferCreateInfo bufInfo = {};
    bufInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufInfo.pNext = nullptr;
    bufInfo.flags = 0;
    bufInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    bufInfo.size = dataSize;
    bufInfo.queueFamilyIndexCount = 0;
    bufInfo.pQueueFamilyIndices = nullptr;
    bufInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    res = vkCreateBuffer(deviceObj->device, &bufInfo, nullptr, &vertexIndex.idx);
    assert(res == VK_SUCCESS);

    VkMemoryRequirements memReq;
    vkGetBufferMemoryRequirements(deviceObj->device, vertexIndex.idx, &memReq);

    VkMemoryAllocateInfo  allocateInfo = {};
    allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocateInfo.pNext = nullptr;
    allocateInfo.memoryTypeIndex = 0;
    allocateInfo.allocationSize = memReq.size;

    pass = deviceObj->MemoryTypeFromProperties(memReq.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &allocateInfo.memoryTypeIndex);
    assert(pass);

    res = vkAllocateMemory(deviceObj->device, &allocateInfo, nullptr, &vertexIndex.mem);
    assert(res == VK_SUCCESS);

    vertexIndex.bufferInfo.range = memReq.size;
    vertexIndex.bufferInfo.offset = 0;

    uint8_t *data;
    res = vkMapMemory(deviceObj->device, vertexIndex.mem, 0, memReq.size, 0, (void **)&data);
    assert(res == VK_SUCCESS);

    memcpy(data, indexData, dataSize);

    vkUnmapMemory(deviceObj->device, vertexIndex.mem);

    res = vkBindBufferMemory(deviceObj->device, vertexIndex.idx, vertexIndex.mem, 0);
    assert(res == VK_SUCCESS);
}

void VulkanDrawable::DestroyVertexIndex() {
    vkDestroyBuffer(rendererObj->GetDevice()->device, vertexIndex.idx, nullptr);
    vkFreeMemory(rendererObj->GetDevice()->device, vertexIndex.mem, nullptr);
}

void VulkanDrawable::CreateUniformBuffer() {
    VkResult res;
    bool pass;
    VulkanDevice *deviceObj = rendererObj->GetDevice();

    projection = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 100.0f);
    view = glm::lookAt(glm::vec3(10, 3, 10),
                       glm::vec3(0, 0, 0),
                       glm::vec3(0, -1, 0));
    model = glm::mat4(1.0f);

    auto MVP = projection * view * model;

    VkBufferCreateInfo bufInfo = {};
    bufInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufInfo.pNext = nullptr;
    bufInfo.flags = 0;
    bufInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    bufInfo.size = sizeof(MVP);
    bufInfo.queueFamilyIndexCount = 0;
    bufInfo.pQueueFamilyIndices = nullptr;
    bufInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    res = vkCreateBuffer(deviceObj->device, &bufInfo, nullptr, &uniformData.buf);
    assert(res == VK_SUCCESS);

    VkMemoryRequirements memReq;
    vkGetBufferMemoryRequirements(deviceObj->device, uniformData.buf, &memReq);

    VkMemoryAllocateInfo memoryAllocateInfo = {};
    memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memoryAllocateInfo.pNext = nullptr;
    memoryAllocateInfo.memoryTypeIndex = 0;
    memoryAllocateInfo.allocationSize = memReq.size;

    pass = deviceObj->MemoryTypeFromProperties(memReq.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &memoryAllocateInfo.memoryTypeIndex);
    assert(pass);

    res = vkAllocateMemory(deviceObj->device, &memoryAllocateInfo, nullptr, &uniformData.mem);
    assert(res == VK_SUCCESS);

    res = vkMapMemory(deviceObj->device, uniformData.mem, 0, memReq.size, 0, (void**)&uniformData.data);
    assert(res == VK_SUCCESS);

    memcpy(uniformData.data, &MVP, sizeof(MVP));

    uniformData.mappedRangeList.resize(1);

    uniformData.mappedRangeList[0].sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    uniformData.mappedRangeList[0].memory = uniformData.mem;
    uniformData.mappedRangeList[0].offset = 0;
    uniformData.mappedRangeList[0].size = sizeof(MVP);

    vkInvalidateMappedMemoryRanges(deviceObj->device, 1, &uniformData.mappedRangeList[0]);

    res = vkBindBufferMemory(deviceObj->device, uniformData.buf, uniformData.mem, 0);
    assert(res == VK_SUCCESS);

    uniformData.bufferInfo.buffer = uniformData.buf;
    uniformData.bufferInfo.offset = 0;
    uniformData.bufferInfo.range = sizeof(MVP);
    uniformData.memReq = memReq;
}

void VulkanDrawable::CreateDescriptorPool(bool useTexture) {
    VkResult res;
    VulkanDevice *deviceObj = rendererObj->GetDevice();

    std::vector<VkDescriptorPoolSize> descriptorPoolSizeList;

    descriptorPoolSizeList.push_back(VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1 });

    if(useTexture)
    {
        descriptorPoolSizeList.push_back(VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1 });
    }

    VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {};
    descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptorPoolCreateInfo.pNext = nullptr;
    descriptorPoolCreateInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    descriptorPoolCreateInfo.maxSets = 1;
    descriptorPoolCreateInfo.poolSizeCount = (uint32_t) descriptorPoolSizeList.size();
    descriptorPoolCreateInfo.pPoolSizes = descriptorPoolSizeList.data();

    res = vkCreateDescriptorPool(deviceObj->device, &descriptorPoolCreateInfo, nullptr, &descriptorPool);
    assert(res == VK_SUCCESS);
}

void VulkanDrawable::CreateDescriptorResources() {
    CreateUniformBuffer();
}

void VulkanDrawable::CreateDescriptorSet(bool useTexture) {
    VkResult res;
    VulkanDevice *deviceObj = rendererObj->GetDevice();
    VulkanPipeline *pipelineObj = rendererObj->GetPipelineObject();

    VkDescriptorSetAllocateInfo descriptorSetAllocateInfo[1] = {{}};
    descriptorSetAllocateInfo[0].sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    descriptorSetAllocateInfo[0].pNext = nullptr;
    descriptorSetAllocateInfo[0].descriptorPool = descriptorPool;
    descriptorSetAllocateInfo[0].descriptorSetCount = 1;
    descriptorSetAllocateInfo[0].pSetLayouts = descLayoutList.data();

    descSetList.resize(1);

    res = vkAllocateDescriptorSets(deviceObj->device, descriptorSetAllocateInfo, descSetList.data());
    assert(res == VK_SUCCESS);

    VkWriteDescriptorSet writeDescriptorSet[2];
    memset(&writeDescriptorSet, 0, sizeof(writeDescriptorSet));

    writeDescriptorSet[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptorSet[0].pNext = nullptr;
    writeDescriptorSet[0].dstSet = descSetList[0];
    writeDescriptorSet[0].descriptorCount = 1;
    writeDescriptorSet[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    writeDescriptorSet[0].pBufferInfo = &uniformData.bufferInfo;
    writeDescriptorSet[0].dstArrayElement = 0;
    writeDescriptorSet[0].dstBinding = 0;

    if(useTexture)
    {
        writeDescriptorSet[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeDescriptorSet[1].pNext = nullptr;
        writeDescriptorSet[1].dstSet = descSetList[0];
        writeDescriptorSet[1].descriptorCount = 1;
        writeDescriptorSet[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        writeDescriptorSet[1].pImageInfo = nullptr;
        writeDescriptorSet[1].dstArrayElement = 0;
        writeDescriptorSet[1].dstBinding = 1;
    }

    vkUpdateDescriptorSets(deviceObj->device, useTexture ? 2 : 1, writeDescriptorSet, 0, nullptr);
}

void VulkanDrawable::DestroyUniformBuffer() {
    vkUnmapMemory(rendererObj->GetDevice()->device, uniformData.mem);
    vkDestroyBuffer(rendererObj->GetDevice()->device, uniformData.buf, nullptr);
    vkFreeMemory(rendererObj->GetDevice()->device, uniformData.mem, nullptr);
}

void VulkanDrawable::CreateDescriptorSetLayout(bool useTexture) {
    VkDescriptorSetLayoutBinding layoutBinding[2];
    layoutBinding[0].binding = 0;
    layoutBinding[0].descriptorCount = 1;
    layoutBinding[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    layoutBinding[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    layoutBinding[0].pImmutableSamplers = nullptr;

    if(useTexture)
    {
        layoutBinding[1].binding = 1;
        layoutBinding[1].descriptorCount = 1;
        layoutBinding[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        layoutBinding[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        layoutBinding[1].pImmutableSamplers = nullptr;
    }

    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = {};
    descriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptorSetLayoutCreateInfo.pNext = nullptr;
    descriptorSetLayoutCreateInfo.bindingCount = useTexture ? 2 : 1;
    descriptorSetLayoutCreateInfo.pBindings = layoutBinding;

    VkResult res;

    descLayoutList.resize(1);
    res = vkCreateDescriptorSetLayout(rendererObj->GetDevice()->device, &descriptorSetLayoutCreateInfo, nullptr, descLayoutList.data());
    assert(res == VK_SUCCESS);
}

void VulkanDrawable::CreatePipelineLayout() {
    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
    pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutCreateInfo.pNext = nullptr;
    pipelineLayoutCreateInfo.pushConstantRangeCount = 0;
    pipelineLayoutCreateInfo.pPushConstantRanges = nullptr;
    pipelineLayoutCreateInfo.setLayoutCount = (uint32_t) descLayoutList.size();
    pipelineLayoutCreateInfo.pSetLayouts = descLayoutList.data();

    VkResult  res;
    res = vkCreatePipelineLayout(rendererObj->GetDevice()->device, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout);
    assert(res == VK_SUCCESS);
}

void VulkanDrawable::Update() {
    VulkanDevice *device = rendererObj->GetDevice();

    projection = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 100.0f);
    view = glm::lookAt(glm::vec3(10, 3, 10),
                       glm::vec3(0, 0, 0),
                       glm::vec3(0, -1, 0));
    model = glm::mat4(1.0f);

    static float rot = 0;
    rot += 0.005f;

    model = glm::rotate(model, rot, glm::vec3(0.0f, 1.0f, 0.0f)) * glm::rotate(model, rot, glm::vec3(1.0f, 1.0f, 1.0f));

    auto MVP = projection * view * model;

    VkResult res = vkInvalidateMappedMemoryRanges(device->device, 1, &uniformData.mappedRangeList[0]);
    assert(res == VK_SUCCESS);

    memcpy(uniformData.data, &MVP, sizeof(MVP));

    res = vkFlushMappedMemoryRanges(device->device, 1, &uniformData.mappedRangeList[0]);
    assert(res == VK_SUCCESS);
}




