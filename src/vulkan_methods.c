#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <vulkan.h>

#include "util_methods.h"
#include "constants.h"
#include "error_methods.h"
#include "vulkan_methods.h"

static VKAPI_ATTR VkBool32 VKAPI_CALL
debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
		void *pUserData) {

	/* set severity */
	uint32_t errcode;
	switch (messageSeverity) {
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
		errcode = INFO;
		break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
		errcode = INFO;
		break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
		errcode = WARN;
		break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
		errcode = ERROR;
		break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_FLAG_BITS_MAX_ENUM_EXT:
		errcode = UNKNOWN;
		break;
	default:
		errcode = UNKNOWN;
		break;
	}
	/* log error */
	errLog(errcode, "Vulkan validation layer: %s\n", pCallbackData->pMessage);
	return (VK_FALSE);
}

VkInstance createInstance(struct InstanceInfo instanceInfo,
		uint32_t enabledExtensionCount,
		const char *const *ppEnabledExtensionNames,
		uint32_t enabledLayerCount,
		const char *const *ppEnabledLayerNames) {

	/* check layers and extensions */
	{
		uint32_t matchnum = 0;
		findMatchingStrings((const char* const *) instanceInfo.ppExtensionNames,
				instanceInfo.extensionCount, ppEnabledExtensionNames,
				enabledExtensionCount,
				NULL, 0, &matchnum);

		if (matchnum != enabledExtensionCount) {
			errLog(FATAL, "failed to find required extension\n");
			hardExit();
		}

		findMatchingStrings((const char* const *) instanceInfo.ppLayerNames,
				instanceInfo.layerCount, ppEnabledLayerNames, enabledLayerCount,
				NULL, 0, &matchnum);

		if (matchnum != enabledLayerCount) {
			errLog(FATAL, "failed to find required layer\n");
			hardExit();
		}
	}
	/* Create app info */
	VkApplicationInfo appInfo = {0};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = APPNAME;
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "None";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0;

	/* Create info */
	VkInstanceCreateInfo createInfo = {0};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;
	createInfo.enabledExtensionCount = enabledExtensionCount;
	createInfo.ppEnabledExtensionNames = ppEnabledExtensionNames;
	createInfo.enabledLayerCount = enabledLayerCount;
	createInfo.ppEnabledLayerNames = ppEnabledLayerNames;

	VkInstance instance = {0};
	VkResult result = vkCreateInstance(&createInfo, NULL, &instance);
	if (result != VK_SUCCESS) {
		errLog(ERROR, "Failed to create instance, error code: %d",
				(uint32_t) result);
		hardExit();
	}
	return (instance);
}

void destroyInstance(VkInstance instance) { vkDestroyInstance(instance, NULL); }

/**
 * Requires the debug utils extension
 */
VkDebugUtilsMessengerEXT createDebugCallback(VkInstance instance) {
	VkDebugUtilsMessengerCreateInfoEXT createInfo = {0};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = debugCallback;

	VkDebugUtilsMessengerEXT callback = {0};

	/* Returns a function pointer */
	PFN_vkCreateDebugUtilsMessengerEXT func =
			(PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
					instance, "vkCreateDebugUtilsMessengerEXT");
	if (!func) {
		errLog(FATAL, "failed to find extension function\n");
		hardExit();
	}
	VkResult result = func(instance, &createInfo, NULL, &callback);
	if (result != VK_SUCCESS) {
		errLog(ERROR, "Failed to create debug callback, error code: %d",
				(uint32_t) result);
		hardExit();
	}
	return (callback);
}

/**
 * Requires the debug utils extension
 */
void destroyDebugCallback(VkInstance instance,
		VkDebugUtilsMessengerEXT callback) {
	PFN_vkDestroyDebugUtilsMessengerEXT func =
			(PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
					instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != NULL) {
		func(instance, callback, NULL);
	}
}

VkPhysicalDevice createPhysicalDevice(VkInstance instance) {
	uint32_t deviceCount = 0;
	VkResult res = vkEnumeratePhysicalDevices(instance, &deviceCount, NULL);
	if (res != VK_SUCCESS || deviceCount == 0) {
		errLog(FATAL, "no Vulkan capable device found");
		hardExit();
	}
	VkPhysicalDevice *arr = malloc(deviceCount * sizeof(VkPhysicalDevice));
	if (!arr) {
		printError(errno);
		hardExit();
	}
	vkEnumeratePhysicalDevices(instance, &deviceCount, arr);

	VkPhysicalDeviceProperties deviceProperties;
	VkPhysicalDevice selectedDevice = VK_NULL_HANDLE;
	for (uint32_t i = 0; i < deviceCount; i++) {
		vkGetPhysicalDeviceProperties(arr[i], &deviceProperties);
		int32_t ret = getDeviceQueueIndex(arr[i], VK_QUEUE_GRAPHICS_BIT |
				VK_QUEUE_COMPUTE_BIT);
		if (ret != -1) {
			selectedDevice = arr[i];
		}
	}

	if (selectedDevice == VK_NULL_HANDLE) {
		errLog(ERROR, "no suitable Vulkan device found\n");
		hardExit();
	}

	free(arr);
	return (selectedDevice);
}

void destroyDevice(VkDevice device) { vkDestroyDevice(device, NULL); }

int32_t getDeviceQueueIndex(VkPhysicalDevice device, VkQueueFlags bit) {
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, NULL);
	VkQueueFamilyProperties *arr =
			malloc(queueFamilyCount * sizeof(VkQueueFamilyProperties));
	if (!arr) {
		printError(errno);
		hardExit();
	}
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, arr);
	for (uint32_t i = 0; i < queueFamilyCount; i++) {
		if (arr[i].queueCount > 0 && (arr[0].queueFlags & bit)) {
			free(arr);
			return (i);
		}
	}
	free(arr);
	return (-1);
}

int32_t getPresentQueueIndex(VkPhysicalDevice device, VkSurfaceKHR surface) {
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, NULL);
	VkQueueFamilyProperties *arr =
			malloc(queueFamilyCount * sizeof(VkQueueFamilyProperties));
	if (!arr) {
		printError(errno);
		hardExit();
	}
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, arr);
	for (uint32_t i = 0; i < queueFamilyCount; i++) {
		VkBool32 surfaceSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &surfaceSupport);
		if (surfaceSupport) {
			return (i);
		}
	}
	free(arr);
	return (-1);
}

struct DeviceIndices getDeviceIndices(VkPhysicalDevice physicalDevice,
		VkSurfaceKHR surface) {
	struct DeviceIndices deviceIndices = { 0 };

	int32_t tmp = getDeviceQueueIndex(physicalDevice, VK_QUEUE_GRAPHICS_BIT);
	if (tmp != -1) {
		deviceIndices.hasGraphics = true;
		deviceIndices.graphicsIndex = (uint32_t) tmp;
	} else {
		deviceIndices.hasGraphics = false;
	}

	tmp = getDeviceQueueIndex(physicalDevice, VK_QUEUE_COMPUTE_BIT);
	if (tmp != -1) {
		deviceIndices.hasCompute = true;
		deviceIndices.computeIndex = (uint32_t) tmp;
	} else {
		deviceIndices.hasCompute = false;
	}

	deviceIndices.hasPresent = false;
	if (surface != VK_NULL_HANDLE) {
		tmp = getPresentQueueIndex(physicalDevice, surface);
		if (tmp != -1) {
			deviceIndices.hasPresent = true;
			deviceIndices.presentIndex = (uint32_t) tmp;
		}
	}
	return (deviceIndices);
}

struct InstanceInfo getInstanceInfo() {
	struct InstanceInfo info;
	vkEnumerateInstanceLayerProperties(&info.layerCount, NULL);
	vkEnumerateInstanceExtensionProperties(NULL, &info.extensionCount, NULL);


	/* alloc number dependent info */
	info.ppLayerNames = malloc(info.layerCount * sizeof(char*));
	info.ppExtensionNames = malloc(info.extensionCount * sizeof(char*));
	VkLayerProperties* pLayerProperties = malloc(
			info.layerCount * sizeof(VkLayerProperties));
	VkExtensionProperties* pExtensionProperties = malloc(
			info.extensionCount * sizeof(VkExtensionProperties));

	/* enumerate */
	vkEnumerateInstanceLayerProperties(&info.layerCount, pLayerProperties);
	vkEnumerateInstanceExtensionProperties(NULL, &info.extensionCount,
			pExtensionProperties);

	/* check if not null */
	if (!info.ppExtensionNames || !info.ppLayerNames || !pLayerProperties
			|| !pExtensionProperties) {
		printError(errno);
		hardExit();
	}

	/* copy names to info */
	for (uint32_t i = 0; i < info.layerCount; i++) {
		info.ppLayerNames[i] = malloc(
				VK_MAX_EXTENSION_NAME_SIZE * sizeof(char));
		if (!info.ppLayerNames[i]) {
			printError(errno);
			hardExit();
		}
		strncpy(info.ppLayerNames[i], pLayerProperties[i].layerName,
				VK_MAX_EXTENSION_NAME_SIZE);
	}
	for (uint32_t i = 0; i < info.extensionCount; i++) {
		info.ppExtensionNames[i] = malloc(
				VK_MAX_EXTENSION_NAME_SIZE * sizeof(char));
		if (!info.ppExtensionNames[i]) {
			printError(errno);
			hardExit();
		}
		strncpy(info.ppExtensionNames[i], pExtensionProperties[i].extensionName,
				VK_MAX_EXTENSION_NAME_SIZE);
	}
	free(pLayerProperties);
	free(pExtensionProperties);
	return (info);
}


void destroyInstanceInfo(struct InstanceInfo instanceInfo) {
	for (uint32_t i = 0; i < instanceInfo.extensionCount; i++) {
		free(instanceInfo.ppExtensionNames[i]);
	}
	free(instanceInfo.ppExtensionNames);

	for (uint32_t i = 0; i < instanceInfo.layerCount; i++) {
		free(instanceInfo.ppLayerNames[i]);
	}
	free(instanceInfo.ppLayerNames);
}


struct DeviceInfo getDeviceInfo(VkPhysicalDevice physicalDevice)
{
	/* Instantiate DeviceInfo */
	struct DeviceInfo info;
	/* Set device properties and features */
	vkGetPhysicalDeviceProperties(physicalDevice, &info.deviceProperties);
	vkGetPhysicalDeviceFeatures(physicalDevice, &info.deviceFeatures);

	/* set extension and layer counts */
	vkEnumerateDeviceLayerProperties(physicalDevice, &info.layerCount,
			NULL);
	vkEnumerateDeviceExtensionProperties(physicalDevice, NULL,
			&info.extensionCount, NULL);

	/* alloc count dependent info */
	info.ppLayerNames = malloc(info.layerCount * sizeof(char*));
	info.ppExtensionNames = malloc(info.extensionCount * sizeof(char*));
	VkLayerProperties* pLayerProperties = malloc(
			info.layerCount * sizeof(VkLayerProperties));
	VkExtensionProperties* pExtensionProperties = malloc(
			info.extensionCount * sizeof(VkExtensionProperties));

	if (!info.ppExtensionNames || !info.ppLayerNames || !pLayerProperties
			|| !pExtensionProperties) {
		printError(errno);
		hardExit();
	}

	/* grab values */
	vkEnumerateDeviceLayerProperties(physicalDevice, &info.layerCount,
			pLayerProperties);
	vkEnumerateDeviceExtensionProperties(physicalDevice, NULL,
			&info.extensionCount,
			pExtensionProperties);

	/* copy names to info, mallocing as we go. */
	for (uint32_t i = 0; i < info.layerCount; i++) {
		info.ppLayerNames[i] = malloc(
		VK_MAX_EXTENSION_NAME_SIZE * sizeof(char));
		if (!info.ppLayerNames[i]) {
			printError(errno);
			hardExit();
		}
		strncpy(info.ppLayerNames[i], pLayerProperties[i].layerName,
		VK_MAX_EXTENSION_NAME_SIZE);
	}
	for (uint32_t i = 0; i < info.extensionCount; i++) {
		info.ppExtensionNames[i] = malloc(
		VK_MAX_EXTENSION_NAME_SIZE * sizeof(char));
		if (!info.ppExtensionNames[i]) {
			printError(errno);
			hardExit();
		}
		strncpy(info.ppExtensionNames[i], pExtensionProperties[i].extensionName,
		VK_MAX_EXTENSION_NAME_SIZE);
	}
	free(pLayerProperties);
	free(pExtensionProperties);
	return (info);
}


void destroyDeviceInfo(struct DeviceInfo deviceInfo) {
	for (uint32_t i = 0; i < deviceInfo.extensionCount; i++) {
		free(deviceInfo.ppExtensionNames[i]);
	}
	free(deviceInfo.ppExtensionNames);

	for (uint32_t i = 0; i < deviceInfo.layerCount; i++) {
		free(deviceInfo.ppLayerNames[i]);
	}
	free(deviceInfo.ppLayerNames);
}

VkDevice createLogicalDevice(struct DeviceInfo deviceInfo,
		VkPhysicalDevice physicalDevice,
		uint32_t deviceQueueIndex,
		uint32_t enabledExtensionCount,
		const char *const *ppEnabledExtensionNames,
		uint32_t enabledLayerCount,
		const char *const *ppEnabledLayerNames) {

	/* check layers and extensions */
	{
		uint32_t matchnum = 0;
		findMatchingStrings((const char* const *) deviceInfo.ppExtensionNames,
				deviceInfo.extensionCount, ppEnabledExtensionNames,
				enabledExtensionCount,
				NULL, 0, &matchnum);

		if (matchnum != enabledExtensionCount) {
			errLog(FATAL, "failed to find required device extension\n");
			hardExit();
		}

		findMatchingStrings((const char* const *) deviceInfo.ppLayerNames,
				deviceInfo.layerCount, ppEnabledLayerNames, enabledLayerCount,
				NULL, 0, &matchnum);

		if (matchnum != enabledLayerCount) {
			errLog(FATAL, "failed to find required device layer\n");
			hardExit();
		}
	}

	VkPhysicalDeviceFeatures deviceFeatures = {0};

	VkDeviceQueueCreateInfo queueCreateInfo = {0};
	queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueCreateInfo.queueFamilyIndex = deviceQueueIndex;
	queueCreateInfo.queueCount = 1;
	float queuePriority = 1.0f;
	queueCreateInfo.pQueuePriorities = &queuePriority;

	VkDeviceCreateInfo createInfo = {0};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.pQueueCreateInfos = &queueCreateInfo;
	createInfo.queueCreateInfoCount = 1;
	createInfo.pEnabledFeatures = &deviceFeatures;
	createInfo.enabledExtensionCount = enabledExtensionCount;
	createInfo.ppEnabledExtensionNames = ppEnabledExtensionNames;
	createInfo.enabledLayerCount = enabledLayerCount;
	createInfo.ppEnabledLayerNames = ppEnabledLayerNames;

	VkDevice device;
	VkResult res = vkCreateDevice(physicalDevice, &createInfo, NULL, &device);
	if (res != VK_SUCCESS) {
		errLog(ERROR, "Failed to create device, error code: %d",
				(uint32_t) res);
		hardExit();
	}

	return (device);
}

VkQueue createQueue(VkDevice device, uint32_t deviceQueueIndex) {
	VkQueue queue;
	vkGetDeviceQueue(device, deviceQueueIndex, 0, &queue);
	return (queue);
}

VkSwapchainKHR createSwapChain(VkSwapchainKHR oldSwapChain,
		VkDevice device,
		VkPhysicalDevice physicalDevice,
		VkSurfaceKHR surface, VkExtent2D extent,
		struct DeviceIndices deviceIndices) {

	VkSwapchainKHR swapChain;
	VkSurfaceCapabilitiesKHR capabilities;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface,
			&capabilities);

	VkSwapchainCreateInfoKHR createInfo = { 0 };
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = surface;

	createInfo.minImageCount = 2;
	createInfo.imageFormat = VK_FORMAT_B8G8R8A8_UNORM;
	createInfo.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR; /* TODO what if this fails (but good enough for now) */
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	if (!deviceIndices.hasGraphics || !deviceIndices.hasPresent) {
		errLog(FATAL, "Invalid device to create swap chain\n");
	}

	uint32_t queueFamilyIndices[] = { deviceIndices.graphicsIndex,
			deviceIndices.presentIndex };
	if (deviceIndices.graphicsIndex != deviceIndices.presentIndex) {
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	} else {
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0; /* Optional */
		createInfo.pQueueFamilyIndices = NULL; /* Optional */
	}

	createInfo.preTransform = capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR;
	createInfo.clipped = VK_TRUE;

	createInfo.oldSwapchain = oldSwapChain;

	VkResult res = vkCreateSwapchainKHR(device, &createInfo, NULL, &swapChain);
	if (res != VK_SUCCESS) {
		errLog(ERROR, "Failed to create swap chain, error code: %d",
				(uint32_t) res);
		hardExit();
	}

	return (swapChain);
}

void destroySwapChain(VkDevice device, VkSwapchainKHR swapChain) {
	vkDestroySwapchainKHR(device, swapChain, NULL);
}

void getSwapChainImages(VkDevice device, VkSwapchainKHR swapChain,
		uint32_t *imageCount, VkImage *images) {
	vkGetSwapchainImagesKHR(device, swapChain, imageCount, NULL);
	VkImage* tmp = realloc(images, (*imageCount) * sizeof(VkImage));
	if (!tmp) {
		printError(errno);
		hardExit();
	} else {
		images = tmp;
	}
	vkGetSwapchainImagesKHR(device, swapChain, imageCount, images);
}
