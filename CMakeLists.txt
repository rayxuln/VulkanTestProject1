cmake_minimum_required(VERSION 3.17)

message(STATUS "Locating Vulkan...")

find_package(Vulkan)

if(NOT ${Vulkan_INCLUDE_DIRS} STREQUAL "")
    set(VULKAN_PATH ${Vulkan_INCLUDE_DIRS})
    STRING(REGEX REPLACE "/Include" "" VULKAN_PATH ${VULKAN_PATH})
endif()

if(NOT Vulkan_FOUND)
    message(STATUS "Vulkan SDK not found!")
    if (EXISTS "${VULKAN_PATH}")
        message(STATUS "But, locate it at: ${VULKAN_PATH}")
    else()
        message(STATUS "Have you installed the Vulkan SDK? Can't build without it!")
        return()
    endif ()
endif()

project(VulkanTestProject1)

add_definitions(-DVKK_USE_PLATFORM_WIN32_KHR)

set(CMAKE_CXX_STANDARD 17)

set(VULKAN_LINK_LIST vulkan-1 )

set(ENABLE_GLSL_TRANSLATION OFF)


if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    include_directories(AFTER ${VULKAN_PATH}/Include)
    link_directories(${VULKAN_PATH}/Lib)
endif()

include_directories(thirdparty)

add_executable(VulkanTestProject1 main.cpp Commons.h VulkanLayerAndExtension.cpp VulkanLayerAndExtension.h VulkanInstance.cpp VulkanInstance.h VulkanDevice.cpp VulkanDevice.h Commons.cpp VulkanApplication.cpp VulkanApplication.h Wrappers.cpp Wrappers.h VulkanRenderer.cpp VulkanRenderer.h VulkanSwapChain.cpp VulkanSwapChain.h MeshData.h VulkanDrawable.cpp VulkanDrawable.h VulkanShader.cpp VulkanShader.h VulkanPipeline.cpp VulkanPipeline.h VulkanDescriptor.cpp VulkanDescriptor.h)

if(ENABLE_GLSL_TRANSLATION)
    message(STATUS "adding libs for GLSL translation support")

    target_compile_definitions(VulkanTestProject1 PRIVATE ENABLE_GLSL_TRANSLATION)

    # Add glsl support libs
    set(GLSL_SUPPORT_LIBS SPIRV glslang OGLCompiler OSDependent HLSL)
    foreach(x ${GLSL_SUPPORT_LIBS})
        list(APPEND VULKAN_LINK_LIST debug ${x} optimized ${x})
    endforeach()
endif()

target_link_libraries(VulkanTestProject1 ${VULKAN_LINK_LIST})

# Copy shaders
if(ENABLE_GLSL_TRANSLATION)
    add_custom_command(
            TARGET VulkanTestProject1
            PRE_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/shaders ${CMAKE_BINARY_DIR}/shaders
            COMMENT "Copying shaders..."
    )
else()
    set(GLVALIDATION_EXE ${VULKAN_PATH}/Bin/glslangValidator.exe)
    add_custom_command(
            TARGET VulkanTestProject1
            PRE_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/shaders
            COMMAND ${CMAKE_SOURCE_DIR}/compile_shaders.bat ${CMAKE_SOURCE_DIR}/shaders ${CMAKE_BINARY_DIR}/shaders ${GLVALIDATION_EXE}
            COMMENT "Compiling shaders to spir-v"
    )
endif()