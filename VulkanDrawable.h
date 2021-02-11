//
// Created by Raiix on 2021/2/9.
//

#ifndef VULKANTESTPROJECT1_VULKANDRAWABLE_H
#define VULKANTESTPROJECT1_VULKANDRAWABLE_H

#include "Commons.h"
#include "VulkanDescriptor.h"
class VulkanRenderer;
#include "Wrappers.h"

class VulkanDrawable : public VulkanDescriptor{
    std::vector<VkCommandBuffer> cmdDrawList;
    VulkanRenderer *rendererObj;
    VkPipeline *pipeline;
    VkViewport viewport;
    VkRect2D scissor;
    VkSemaphore presentCompleteSemaphore;
    VkSemaphore drawingCompleteSemaphore;
    TextureData *textures;

    glm::mat4 projection;
    glm::mat4 view;
    glm::mat4 model;

    void RecordCommandBuffer(int currentBuffer, VkCommandBuffer *cmdDraw);
public:
    struct {
        VkBuffer buf;
        VkDeviceMemory mem;
        VkDescriptorBufferInfo bufferInfo;
        VkMemoryRequirements memReq;
        std::vector<VkMappedMemoryRange> mappedRangeList;
        uint8_t *data;
    } uniformData;

    struct {
        VkBuffer buf;
        VkDeviceMemory mem;
        VkDescriptorBufferInfo bufferInfo;
    } vertexBuffer;

    struct {
        VkBuffer idx;
        VkDeviceMemory mem;
        VkDescriptorBufferInfo bufferInfo;
    } vertexIndex;

    VkVertexInputBindingDescription  bind;
    VkVertexInputAttributeDescription  attr[2];

    VulkanDrawable(VulkanRenderer *renderer = nullptr);
    ~VulkanDrawable();

    void CreateVertexBuffer(const void *vertexData, uint32_t dataSize, uint32_t dataStride, bool useTexture);
    void DestroyVertexBuffer();

    void CreateVertexIndex(const void *indexData, uint32_t dataSize, uint32_t dataStride);
    void DestroyVertexIndex();

    void Prepare();
    void Render();
    void Update();

    void InitViewports(VkCommandBuffer *cmd);
    void InitScissors(VkCommandBuffer *cmd);

    void DestroyCommandBuffer();
    void DestroySynchronizationObjects();

    inline void SetTextures(TextureData *texs) {textures = texs;}

    inline void SetPipeline(VkPipeline *vulkanPipeline) {pipeline = vulkanPipeline;}
    inline VkPipeline *GetPipeline() {return pipeline;}

    void CreateUniformBuffer();
    void CreateDescriptorPool(bool useTexture) override;
    void CreateDescriptorResources() override;
    void CreateDescriptorSet(bool useTexture) override;
    void CreateDescriptorSetLayout(bool useTexture) override;
    void CreatePipelineLayout() override;

    void DestroyUniformBuffer();
};


#endif //VULKANTESTPROJECT1_VULKANDRAWABLE_H
