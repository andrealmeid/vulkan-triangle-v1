///
/// Copyright 2019 Govind Pimpale
/// vulkan_methods.h
///
///  Created on: Aug 8, 2018
///      Author: gpi
///

#ifndef SRC_VULKAN_UTILS_H_
#define SRC_VULKAN_UTILS_H_

#include <stdbool.h>
#include <stdint.h>

#include <vulkan/vulkan.h>

#include "linmath.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "errors.h"
#include "utils.h"

/// Creates a new VkInstance with the specified extensions and layers
/// --- PRECONDITIONS ---
/// `ppEnabledExtensionNames` must be a pointer to at least
/// `enabledExtensionCount` extensions `ppEnabledLayerNames` must be a pointer
/// to at least `enabledLayerCount` layers
/// --- POSTCONDITONS ---
/// Returns the status of creating the vulkan instance
/// If enableGLFWRequiredExtensions, then all extensions needed by GLFW will be
/// enabled If enableDebugRequiredExtensions, then the extensions needed for
/// new_DebugCallback will be enabled
/// --- PANICS ---
/// Panics if memory allocation fails
/// Panics if Vulkan is not supported by GLFW
/// --- CLEANUP ---
/// To cleanup, call delete_Instance on the created instance
ErrVal new_Instance(VkInstance *pInstance, const uint32_t enabledLayerCount,
                    const char *const *ppEnabledLayerNames,
                    const uint32_t enabledExtensionCount,
                    const char *const *ppEnabledExtensionNames,
                    const bool enableGLFWRequiredExtensions,
                    const bool enableDebugRequiredExtensions);

/// Destroys the instance created in new_Instance
/// --- PRECONDITIONS ---
/// pInstance is a valid pointer to an instance created by new_Instance
/// --- POSTCONDITONS ---
/// the instance is no longer valid
/// pInstance has been set to VK_NULL_HANDLE
void delete_Instance(VkInstance *pInstance);

/// Creates a new window with GLFW
/// --- PRECONDITIONS ---
/// ppGLFWwindow is a valid pointer
/// --- POSTCONDITONS ---
/// returns error status
/// on success, *ppGlfwWindow is a valid GLFW window
ErrVal new_GlfwWindow(GLFWwindow **ppGlfwWindow, const char *name,
                      VkExtent2D dimensions);

/// Gets the size of the framebuffer of the window
/// --- PRECONDITIONS ---
/// `pExtent` is a valid pointer
/// `pWindow` is a valid pointer to a GLFWwindow
/// --- POSTCONDITONS ---
/// returns error status
/// on success, pExtent is set to the size of the framebuffer of pWindow
ErrVal getWindowExtent(VkExtent2D *pExtent, GLFWwindow *pWindow);

/// Sets a debug callback to log error messages from validation layers
/// --- PRECONDITIONS ---
/// `instance` must have been created with enableDebugRequiredExtensions true
/// `pCallback` must be a valid pointer
/// --- POSTCONDITONS ---
/// returns error status
/// on success, `*pCallback` is set to a valid callback
/// on success, will log all errors to stdout
/// --- CLEANUP ---
/// call delete_DebugCallback on the created callback
ErrVal new_DebugCallback(VkDebugUtilsMessengerEXT *pCallback,
                         const VkInstance instance);

/// Destroys the debug callback
/// --- PRECONDITIONS ---
/// `*pCallback` must have been created with `new_DebugCallback`
/// `instance` must be the instance with which `pCallback` was created
/// --- POSTCONDITONS ---
/// `*pCallback` is no longer a valid callback
/// `*pCallback` is set to VK_NULL_HANDLE
void delete_DebugCallback(VkDebugUtilsMessengerEXT *pCallback,
                          const VkInstance instance);

/// Gets a the first physical device with both graphics and compute capabilities
/// --- PRECONDITIONS ---
/// `pDevice` must be a valid pointer
/// `instance` must be a valid instance
/// --- POSTCONDITONS ---
/// returns error status
/// on success, sets `*pDevice` to a valid physical device supporting graphics
/// and compute
ErrVal getPhysicalDevice(VkPhysicalDevice *pDevice, const VkInstance instance);

/// Creates a new logical device with the given physical device
/// --- PRECONDITIONS ---
/// `pDevice` must be a valid pointer
/// `physicalDevice` must be a valid physical device created from
/// `getPhysicalDevice` `queueFamilyIndex` must be the index of the queue family
/// to use `ppEnabledExtensionNames` must be a pointer to at least
/// `enabledExtensionCount` extensions
/// --- POSTCONDITIONS ---
/// returns error status
/// on success, `*pDevice` will be a new logical device
/// --- CLEANUP ---
/// call delete_Device
ErrVal new_Device(VkDevice *pDevice, const VkPhysicalDevice physicalDevice,
                  const uint32_t queueFamilyIndex,
                  const uint32_t enabledExtensionCount,
                  const char *const *ppEnabledExtensionNames);

/// Deletes a logical device created from new_Device
/// --- PRECONDITIONS ---
/// `pDevice` must be a valid pointer to a logical device created from
/// new_Device
/// --- POSTCONDITIONS ---
/// `*pDevice` is no longer a valid logical device
/// `*pDevice` is set to VK_NULL_HANDLE
void delete_Device(VkDevice *pDevice);

/// Gets the first queue family index with the stated capabilities
/// --- PRECONDITIONS ---
/// `pQueueFamilyIndex` must be a valid pointer
/// `device` must be created by getPhysicalDevice
/// --- POSTCONDITIONS ---
/// sets `*pQueueFamilyIndex` contains the index of the first queue family
/// supporting `bit`
ErrVal getQueueFamilyIndexByCapability(uint32_t *pQueueFamilyIndex,
                                       const VkPhysicalDevice device,
                                       const VkQueueFlags bit);

/// Gets the first queue family index which can support rendering to `surface`
/// --- PRECONDITIONS ---
/// * `pQueueFamilyIndex` must be a valid pointer
/// * `device` must be created by getPhysicalDevice
/// * `surface` must be a valid surface from the same instance as
/// `physicalDevice`
/// --- POSTCONDITIONS ---
/// * returns error status.
/// * on success, sets `*pQueueFamilyIndex` contains the index of the first
/// queue supporting presenting to `surface`
ErrVal getPresentQueueFamilyIndex(uint32_t *pQueueFamilyIndex,
                                  const VkPhysicalDevice physicalDevice,
                                  const VkSurfaceKHR surface);

/// Gets the queue associated with the queue family
/// --- PRECONDITIONS ---
/// * `pQueue` is a valid pointer
/// * `device` is a logical device created by `new_Device`
/// * `queueFamilyIndex` is a valid index for a queue family in the
/// corresponding physical device
/// --- POSTCONDITIONS ---
/// * returns error status
/// * on success, `pQueue` is set to a new 	queue in the given queue family
ErrVal getQueue(VkQueue *pQueue, const VkDevice device,
                const uint32_t queueFamilyIndex);

/// Gets a surface format that can be rendered to 
/// --- PRECONDITIONS ---
/// * `pSurfaceFormat` is a valid pointer
/// * `surface` has been allocated from the same instance as `physicalDevice`
/// --- POSTCONDITIONS ---
/// * returns error status
/// * 
ErrVal getPreferredSurfaceFormat(VkSurfaceFormatKHR *pSurfaceFormat,
                                 const VkPhysicalDevice physicalDevice,
                                 const VkSurfaceKHR surface);

ErrVal new_SwapChain(VkSwapchainKHR *pSwapChain, uint32_t *pSwapChainImageCount,
                     const VkSwapchainKHR oldSwapChain,
                     const VkSurfaceFormatKHR surfaceFormat,
                     const VkPhysicalDevice physicalDevice,
                     const VkDevice device, const VkSurfaceKHR surface,
                     const VkExtent2D extent, const uint32_t graphicsIndex,
                     const uint32_t presentIndex);

void delete_SwapChain(VkSwapchainKHR *pSwapChain, const VkDevice device);

ErrVal new_SwapChainImages(VkImage **ppSwapChainImages, uint32_t *pImageCount,
                           const VkDevice device,
                           const VkSwapchainKHR swapChain);

ErrVal new_Image(VkImage *pImage, VkDeviceMemory *pImageMemory,
                 const uint32_t width, const uint32_t height,
                 const VkFormat format, const VkImageTiling tiling,
                 const VkImageUsageFlags usage,
                 const VkMemoryPropertyFlags properties,
                 const VkPhysicalDevice physicalDevice, const VkDevice device);

void delete_Image(VkImage *pImage, const VkDevice device);

void delete_SwapChainImages(VkImage **ppImages);

ErrVal new_ImageView(VkImageView *pImageView, const VkDevice device,
                     const VkImage image, const VkFormat format,
                     const uint32_t aspectMask);

ErrVal new_SwapChainImageViews(VkImageView **ppImageViews,
                               const VkDevice device, const VkFormat format,
                               const uint32_t imageCount,
                               const VkImage *pSwapChainImages);

void delete_ImageView(VkImageView *pImageView, VkDevice device);

void delete_SwapChainImageViews(VkImageView **ppImageViews, uint32_t imageCount,
                                const VkDevice device);

ErrVal new_ShaderModule(VkShaderModule *pShaderModule, const VkDevice device,
                        const uint32_t codeSize, const uint32_t *pCode);

ErrVal new_ShaderModuleFromFile(VkShaderModule *pShaderModule,
                                const VkDevice device, char *filename);

void delete_ShaderModule(VkShaderModule *pShaderModule, const VkDevice device);

ErrVal new_VertexDisplayRenderPass(VkRenderPass *pRenderPass,
                                   const VkDevice device,
                                   const VkFormat swapChainImageFormat);

void delete_RenderPass(VkRenderPass *pRenderPass, const VkDevice device);

ErrVal new_VertexDisplayPipelineLayout(VkPipelineLayout *pPipelineLayout,
                                       const VkDevice device);

void delete_PipelineLayout(VkPipelineLayout *pPipelineLayout,
                           const VkDevice device);

ErrVal new_VertexDisplayPipeline(VkPipeline *pVertexDisplayPipeline,
                                 const VkDevice device,
                                 const VkShaderModule vertShaderModule,
                                 const VkShaderModule fragShaderModule,
                                 const VkExtent2D extent,
                                 const VkRenderPass renderPass,
                                 const VkPipelineLayout pipelineLayout);

void delete_Pipeline(VkPipeline *pPipeline, const VkDevice device);

ErrVal new_Framebuffer(VkFramebuffer *pFramebuffer, const VkDevice device,
                       const VkRenderPass renderPass,
                       const VkImageView imageView,
                       const VkImageView depthImageView,
                       const VkExtent2D swapChainExtent);

void delete_Framebuffer(VkFramebuffer *pFramebuffer, VkDevice device);

ErrVal new_SwapChainFramebuffers(VkFramebuffer **ppFramebuffers,
                                 const VkDevice device,
                                 const VkRenderPass renderPass,
                                 const VkExtent2D swapChainExtent,
                                 const uint32_t imageCount,
                                 const VkImageView depthImageView,
                                 const VkImageView *pSwapChainImageViews);

void delete_SwapChainFramebuffers(VkFramebuffer **ppFramebuffers,
                                  const uint32_t imageCount,
                                  const VkDevice device);

ErrVal new_CommandPool(VkCommandPool *pCommandPool, const VkDevice device,
                       const uint32_t queueFamilyIndex);

void delete_CommandPool(VkCommandPool *pCommandPool, const VkDevice device);

ErrVal new_VertexDisplayCommandBuffers(
    VkCommandBuffer **ppCommandBuffers, const VkBuffer vertexBuffer,
    const uint32_t vertexCount, const VkDevice device,
    const VkRenderPass renderPass,
    const VkPipelineLayout vertexDisplayPipelineLayout,
    const VkPipeline vertexDisplayPipeline, const VkCommandPool commandPool,
    const VkExtent2D swapChainExtent, const uint32_t swapChainFramebufferCount,
    const VkFramebuffer *pSwapChainFramebuffers, const mat4x4 cameraTransform);

void delete_CommandBuffers(VkCommandBuffer **ppCommandBuffers,
                           const uint32_t commandBufferCount,
                           const VkCommandPool commandPool,
                           const VkDevice device);

ErrVal new_Semaphore(VkSemaphore *pSemaphore, const VkDevice device);

void delete_Semaphore(VkSemaphore *pSemaphore, const VkDevice device);

ErrVal new_Semaphores(VkSemaphore **ppSemaphores, const uint32_t semaphoreCount,
                      const VkDevice device);

void delete_Semaphores(VkSemaphore **ppSemaphores,
                       const uint32_t semaphoreCount, const VkDevice device);

ErrVal new_Fence(VkFence *pFence, const VkDevice device);

void delete_Fence(VkFence *pFence, const VkDevice device);

ErrVal new_Fences(VkFence **ppFences, const uint32_t fenceCount,
                  const VkDevice device);

void delete_Fences(VkFence **ppFences, const uint32_t fenceCount,
                   const VkDevice device);

ErrVal drawFrame(uint32_t *pCurrentFrame, const uint32_t maxFramesInFlight,
                 const VkDevice device, const VkSwapchainKHR swapChain,
                 const VkCommandBuffer *pCommandBuffers,
                 const VkFence *pInFlightFences,
                 const VkSemaphore *pImageAvailableSemaphores,
                 const VkSemaphore *pRenderFinishedSemaphores,
                 const VkQueue graphicsQueue, const VkQueue presentQueue);

ErrVal new_SurfaceFromGLFW(VkSurfaceKHR *pSurface, GLFWwindow *pWindow,
                           const VkInstance instance);

void delete_Surface(VkSurfaceKHR *pSurface, const VkInstance instance);

ErrVal new_Buffer_DeviceMemory(VkBuffer *pBuffer, VkDeviceMemory *pBufferMemory,
                               const VkDeviceSize size,
                               const VkPhysicalDevice physicalDevice,
                               const VkDevice device,
                               const VkBufferUsageFlags usage,
                               const VkMemoryPropertyFlags properties);

ErrVal copyBuffer(VkBuffer destinationBuffer, const VkBuffer sourceBuffer,
                  const VkDeviceSize size, const VkCommandPool commandPool,
                  const VkQueue queue, const VkDevice device);

void delete_Buffer(VkBuffer *pBuffer, const VkDevice device);

void delete_DeviceMemory(VkDeviceMemory *pDeviceMemory, const VkDevice device);

ErrVal new_begin_OneTimeSubmitCommandBuffer(VkCommandBuffer *pCommandBuffer,
                                            const VkDevice device,
                                            const VkCommandPool commandPool);

ErrVal delete_end_OneTimeSubmitCommandBuffer(VkCommandBuffer *pCommandBuffer,
                                             const VkDevice device,
                                             const VkQueue queue,
                                             const VkCommandPool commandPool);

ErrVal copyToDeviceMemory(VkDeviceMemory *pDeviceMemory,
                          const VkDeviceSize deviceSize, const void *source,
                          const VkDevice device);

void getDepthFormat(VkFormat *pFormat);

ErrVal new_DepthImageView(VkImageView *pImageView, const VkDevice device,
                          const VkImage depthImage);

ErrVal new_DepthImage(VkImage *pImage, VkDeviceMemory *pImageMemory,
                      const VkExtent2D swapChainExtent,
                      const VkPhysicalDevice physicalDevice,
                      const VkDevice device);

ErrVal getMemoryTypeIndex(uint32_t *memoryTypeIndex,
                          const uint32_t memoryTypeBits,
                          const VkMemoryPropertyFlags memoryPropertyFlags,
                          const VkPhysicalDevice physicalDevice);

ErrVal new_ComputePipeline(VkPipeline *pPipeline,
                           const VkPipelineLayout pipelineLayout,
                           const VkShaderModule shaderModule,
                           const VkDevice device);

ErrVal new_ComputeStorageDescriptorSetLayout(
    VkDescriptorSetLayout *pDescriptorSetLayout, const VkDevice device);

void delete_DescriptorSetLayout(VkDescriptorSetLayout *pDescriptorSetLayout,
                                const VkDevice device);

ErrVal new_DescriptorPool(VkDescriptorPool *pDescriptorPool,
                          const VkDescriptorType descriptorType,
                          const uint32_t maxAllocFrom, const VkDevice device);

void delete_DescriptorPool(VkDescriptorPool *pDescriptorPool,
                           const VkDevice device);

ErrVal new_ComputeBufferDescriptorSet(
    VkDescriptorSet *pDescriptorSet, const VkBuffer computeBufferDescriptorSet,
    const VkDeviceSize computeBufferSize,
    const VkDescriptorSetLayout descriptorSetLayout,
    const VkDescriptorPool descriptorPool, const VkDevice device);

void delete_DescriptorSets(VkDescriptorSet **ppDescriptorSets);

#endif /* SRC_VULKAN_UTILS_H_ */