
#include "Commons.h"
#include "VulkanApplication.h"

#include <iostream>

std::vector<const char*> layerNames = {
//        "VK_LAYER_LUNARG_api_dump",
        "VK_LAYER_GOOGLE_unique_objects",
        "VK_LAYER_LUNARG_core_validation",
        "VK_LAYER_LUNARG_image",
        "VK_LAYER_LUNARG_object_tracker",
        "VK_LAYER_LUNARG_parameter_validation",
        "VK_LAYER_LUNARG_swapchain",
        "VK_LAYER_GOOGLE_threading",
        "VK_LAYER_KHRONOS_validation",
};

std::vector<const char*> instanceExtensionNames = {
        VK_KHR_SURFACE_EXTENSION_NAME,
        VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
        VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
        VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
        VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME,
};

std::vector<const char*> deviceExtensionNames = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
};



int main(int argn, char **args) {
    VkResult res;

    std::vector<std::string> cmdLineArgs(argn);
    for(size_t i=0; i<argn; ++i) cmdLineArgs[i] = args[i];

    VulkanApplication *app = VulkanApplication::Instance();
    app->Initialize(cmdLineArgs);
    app->Prepare();
    do{
        app->Update();
    } while (app->Render());
    app->DeInitialize();

    return 0;
}

