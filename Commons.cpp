//
// Created by Raiix on 2021/2/7.
//

#include "Commons.h"
#include "VulkanApplication.h"

#include <iostream>

VkResult VulkanLayerAndExtension::GetInstanceLayerProperties() {
    uint32_t  instanceLayerCount;
    std::vector<VkLayerProperties> layerProperties;
    VkResult res;

    // looking for all layers
    do {
        res = vkEnumerateInstanceLayerProperties(&instanceLayerCount, nullptr);
        if(res) return res;
        if(instanceLayerCount == 0) return VK_INCOMPLETE;

        layerProperties.resize(instanceLayerCount);
        res = vkEnumerateInstanceLayerProperties(&instanceLayerCount, layerProperties.data());
    } while (res == VK_INCOMPLETE);

    // looking for all extensions of each layer
    SHOW_LAYER_EXTENSION_OUTPUT(std::cout << "\n>====| Instanced Layers |====<" << std::endl);
    for(auto globalLayerProp : layerProperties) {
        SHOW_LAYER_EXTENSION_OUTPUT(std::cout << "\n" << globalLayerProp.description << "\n\t|\n\t|---[Layer Name]--> " << globalLayerProp.layerName << std::endl);

        LayerProperties layerProps;
        layerProps.properties = globalLayerProp;

        res = GetExtensionProperties(layerProps);

        if(res) continue;

        layerPropertyList.push_back(layerProps);

        for(auto i : layerProps.extensions) {
            SHOW_LAYER_EXTENSION_OUTPUT(std::cout << "\t\t|\n\t\t|---[Layer Extension]--> " << i.extensionName << std::endl);
        }
    }

    return res;
}

VkResult VulkanLayerAndExtension::GetExtensionProperties(LayerProperties &layerProps, VkPhysicalDevice *gpu) {
    uint32_t extensionCount;
    VkResult res;
    char *layerName = layerProps.properties.layerName;

    do {
        if(gpu){
            res = vkEnumerateDeviceExtensionProperties(*gpu, layerName, &extensionCount, nullptr);
        }else{
            res = vkEnumerateInstanceExtensionProperties(layerName, &extensionCount, nullptr);
        }

        if (res || extensionCount == 0) continue;

        layerProps.extensions.resize(extensionCount);

        if(gpu){
            res = vkEnumerateDeviceExtensionProperties(*gpu, layerName, &extensionCount, layerProps.extensions.data());
        }else{
            res = vkEnumerateInstanceExtensionProperties(layerName, &extensionCount, layerProps.extensions.data());
        }
    }while(res == VK_INCOMPLETE);

    return res;
}

VkResult VulkanLayerAndExtension::GetDeviceExtensionProperties(VkPhysicalDevice *gpu) {
    VkResult res;

    SHOW_LAYER_EXTENSION_OUTPUT(std::cout << ">===| Device extensions |===<" << std::endl);
    auto app = VulkanApplication::Instance();
    std::vector<LayerProperties> *instanceLayerProp = &app->instanceObj.layerExtension.layerPropertyList;
    for (auto globalLayerProp:*instanceLayerProp)
    {
        LayerProperties layerProps;
        layerProps.properties = globalLayerProp.properties;

        res = GetExtensionProperties(layerProps, gpu);
        if(res)
            continue;

        SHOW_LAYER_EXTENSION_OUTPUT(std::cout << std::endl);
        SHOW_LAYER_EXTENSION_OUTPUT(std::cout << globalLayerProp.properties.description << "\n\t|\n\t|---[Layer Name]---> " << globalLayerProp.properties.layerName << std::endl);
        layerPropertyList.push_back(layerProps);

        if (!layerProps.extensions.empty()) {
            for(auto i:layerProps.extensions)
            {
                SHOW_LAYER_EXTENSION_OUTPUT(std::cout << "\t\t|\n\t\t|---[Device Extension]--> " << i.extensionName << std::endl);
            }
        }else {
            SHOW_LAYER_EXTENSION_OUTPUT(std::cout << "\t\t|\n\t\t|---[Device Extension]--> No extension found" << std::endl);
        }
    }

    return res;
}

VkBool32 VulkanLayerAndExtension::AreLayersSupported(std::vector<const char *> &layers) {
    uint32_t checkCount = layers.size();
    uint32_t layerCount = layerPropertyList.size();

    std::vector<const char*> unsupportLayers;
    for(uint32_t i=0; i<checkCount; ++i)
    {
        VkBool32 isSupported = false;
        for(uint32_t j=0; j<layerCount; ++j)
        {
            if(!strcmp(layers[i], layerPropertyList[j].properties.layerName))
            {
                isSupported = true;
            }
        }

        if(!isSupported)
        {
            std::cout<<"Layer not supported: " <<layers[i] << std::endl;
            unsupportLayers.push_back(layers[i]);
        }
    }

    for(auto i:unsupportLayers)
    {
        auto it = std::find(layers.begin(), layers.end(), i);
        if (it != layers.end()) layers.erase(it);
    }

    return true;
}

VkResult VulkanLayerAndExtension::CreateDebugReportCallback() {
    VkResult res;

    auto app = VulkanApplication::Instance();
    VkInstance *instance = &app->instanceObj.instance;

    dbgCreateDebugReportCallback = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(*instance, "vkCreateDebugReportCallbackEXT");
    if(dbgCreateDebugReportCallback == nullptr)
    {
        std::cout<<"Error: vkCreateDebugReportCallbackEXT not found!"<<std::endl;
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    dbgDestroyDebugReportCallback = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(*instance, "vkDestroyDebugReportCallbackEXT");
    if(dbgDestroyDebugReportCallback == nullptr)
    {
        std::cout<<"Error: vkDestroyDebugReportCallbackEXT not found!"<<std::endl;
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    dbgReportCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
    dbgReportCreateInfo.pfnCallback = DebugFunction;
    dbgReportCreateInfo.pUserData = nullptr;
    dbgReportCreateInfo.pNext = nullptr;
    dbgReportCreateInfo.flags = VK_DEBUG_REPORT_WARNING_BIT_EXT |
                                VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT |
                                VK_DEBUG_REPORT_ERROR_BIT_EXT;// |
                                //VK_DEBUG_REPORT_DEBUG_BIT_EXT;

    res = dbgCreateDebugReportCallback(*instance, &dbgReportCreateInfo, nullptr, &debugReportCallback);

    return res;
}

void VulkanLayerAndExtension::DestroyDebugReportCallback() {
    auto app = VulkanApplication::Instance();
    VkInstance &instance = app->instanceObj.instance;
    dbgDestroyDebugReportCallback(instance, debugReportCallback, nullptr);
}

VKAPI_ATTR
VkBool32
VKAPI_CALL
VulkanLayerAndExtension::DebugFunction(VkFlags msgFlags, VkDebugReportObjectTypeEXT objType, uint64_t srcObject,
                                       size_t location, int32_t msgCode, const char *layerPrefix, const char *msg,
                                       void *userData) {
    const char *prefix = "[VK_DEBUG_REPORT]";
    if(msgFlags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
    {
        std::cout<<prefix<<" ERROR: ["<<layerPrefix<<"] Code" <<msgCode<<":"<<msg<<std::endl;
    }else if(msgFlags & VK_DEBUG_REPORT_WARNING_BIT_EXT)
    {
        std::cout<<prefix<<" WARNING: ["<<layerPrefix<<"] Code" <<msgCode<<":"<<msg<<std::endl;
    }else if(msgFlags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT)
    {
        std::cout<<prefix<<" INFO: ["<<layerPrefix<<"] Code" <<msgCode<<":"<<msg<<std::endl;
    }else if(msgFlags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT)
    {
        std::cout<<prefix<<" PERFORMANCE: ["<<layerPrefix<<"] Code" <<msgCode<<":"<<msg<<std::endl;
    }else if(msgFlags & VK_DEBUG_REPORT_DEBUG_BIT_EXT)
    {
        std::cout<<prefix<<" DEBUG: ["<<layerPrefix<<"] Code" <<msgCode<<":"<<msg<<std::endl;
    }else {
        return VK_FALSE;
    }

    fflush(stdout);
    return VK_TRUE;
}

VulkanLayerAndExtension::VulkanLayerAndExtension() {
    dbgCreateDebugReportCallback = nullptr;
    dbgDestroyDebugReportCallback = nullptr;
    debugReportCallback = nullptr;

    dbgReportCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
}

VulkanLayerAndExtension::~VulkanLayerAndExtension() {
    dbgCreateDebugReportCallback = nullptr;
    dbgDestroyDebugReportCallback = nullptr;
    debugReportCallback = nullptr;
}

void *Utils::ReadFile(const char *fileName, size_t *fileSize) {
    FILE *fp = nullptr;
    fopen_s(&fp, fileName, "rb");
    if(!fp) {
        std::cout<<"Could not read file: "<<fileName<<std::endl;
        return nullptr;
    }

    fseek(fp, 0L, SEEK_END);
    long long size = ftell(fp);
    fseek(fp, 0L, SEEK_SET);

    void *res = malloc(size+1);//TODO: Memory leaking issue here
    memset(res, 0, size+1);

    size_t t = fread(res, size, 1, fp);
    assert(t == 1);

    *fileSize = size;
    fclose(fp);
    return res;
}

std::string Utils::ReadString(const char *fileName) {
    size_t size = 0;
    char *cs =(char*) ReadFile(fileName, &size);
    if(cs == nullptr)
    {
        return "";
    }

    return std::string(cs, size);
}

void *Utils::ReadFile(const std::string &fileName, size_t *size) {
    return ReadFile(fileName.c_str(), size);
}

LONGLONG Utils::GetCurrentTimeMilliSec() {
    static LARGE_INTEGER f;
    static bool isGetF = false;
    if(!isGetF)
    {
        QueryPerformanceFrequency(&f);
        isGetF = true;
    }

    LARGE_INTEGER c;
    QueryPerformanceCounter(&c);

    return (c.QuadPart / (f.QuadPart / 1000));
}
