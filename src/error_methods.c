/*
 * error_handle.c
 *
 *  Created on: Aug 7, 2018
 *      Author: gpi
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vulkan/vulkan.h>

#include "constants.h"

#include "error_methods.h"

void printVulkanError(VkResult error) {
  if (error != VK_SUCCESS) {
    fprintf(stderr, "%s: Vulkan error %i\n", APPNAME, (int)error);
  }
}

void printError(int errnum) {
  fprintf(stderr, "%s: %s\n", APPNAME, strerror(errnum));
}

void hardExit() { exit(EXIT_FAILURE); }
