//
// Created by Raiix on 2021/2/7.
//

#ifndef VULKANTESTPROJECT1_COMMONS_H
#define VULKANTESTPROJECT1_COMMONS_H



#include <vector>
#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <assert.h>
#include <cmath>
#include <algorithm>

#include <memory>
#include <mutex>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vulkan/vulkan.h>

#ifdef ENABLE_GLSL_TRANSLATION
#include "glslang/SPIRV/GlslangToSpv.h"
#endif

#include <windows.h>
#include <vulkan/vulkan_win32.h>




#define SHOW_LAYER_EXTENSION_OUTPUT_OPTION 0
#define SHOW_LAYER_EXTENSION_OUTPUT(x) if(SHOW_LAYER_EXTENSION_OUTPUT_OPTION){x;}

struct LayerProperties {
    VkLayerProperties properties;
    std::vector<VkExtensionProperties> extensions;
};

class VulkanLayerAndExtension {

    PFN_vkCreateDebugReportCallbackEXT  dbgCreateDebugReportCallback;
    PFN_vkDestroyDebugReportCallbackEXT  dbgDestroyDebugReportCallback;
    VkDebugReportCallbackEXT debugReportCallback;
public:
    VkDebugReportCallbackCreateInfoEXT dbgReportCreateInfo = {};

    std::vector<const char*> instanceExtensionNames;
    std::vector<const char*> instanceLayerNames;

    VulkanLayerAndExtension();
    ~VulkanLayerAndExtension();

    // layer and extension
    std::vector<LayerProperties> layerPropertyList;
    VkResult GetInstanceLayerProperties();

    // global extension
    VkResult GetExtensionProperties(LayerProperties &layerProps, VkPhysicalDevice *gpu = nullptr);

    // extension based on device
    VkResult GetDeviceExtensionProperties(VkPhysicalDevice *gpu);

    VkBool32 AreLayersSupported(std::vector<const char*> &layers);
    VkResult CreateDebugReportCallback();
    void DestroyDebugReportCallback();

    static VKAPI_ATTR VkBool32 VKAPI_CALL DebugFunction(
            VkFlags msgFlags,
            VkDebugReportObjectTypeEXT objType,
            uint64_t srcObject,
            size_t location,
            int32_t msgCode,
            const char *layerPrefix,
            const char *msg,
            void *userData
            );
};

class Utils{
public:
    static void *ReadFile(const char *fileName, size_t *size);
    static void *ReadFile(const std::string &fileName, size_t *size);
    static std::string ReadString(const char *fileName);

    static LONGLONG GetCurrentTimeMilliSec();
};

#endif //VULKANTESTPROJECT1_COMMONS_H
