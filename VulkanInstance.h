//
// Created by Raiix on 2021/2/7.
//

#ifndef VULKANTESTPROJECT1_VULKANINSTANCE_H
#define VULKANTESTPROJECT1_VULKANINSTANCE_H

#include "Commons.h"

class VulkanInstance {
public:

    VkInstance instance;
    VulkanLayerAndExtension layerExtension;

    VkResult CreateInstance(std::vector<const char*> &layers, std::vector<const char*> &extensions, const char *applicationName);

    void DestroyInstance();


};


#endif //VULKANTESTPROJECT1_VULKANINSTANCE_H
