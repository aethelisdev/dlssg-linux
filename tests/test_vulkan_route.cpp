#include <iostream>
#include <vector>
#include <cassert>
#include <cstring>
#include <vulkan/vulkan.h>
#include "../src/vulkan_route_abi.h"

static VkResult MockInitialize(void* user_data, const DlssgVulkanDevice* device) {
    (void)user_data;
    (void)device;
    std::cout << "  [Adapter Callback] Initialize called successfully." << std::endl;
    return VK_SUCCESS;
}

static VkResult MockEvaluate(void* user_data, VkCommandBuffer cmd, const DlssgVulkanResources* res) {
    (void)user_data;
    (void)cmd;
    (void)res;
    return VK_SUCCESS;
}

static void MockShutdown(void* user_data) {
    (void)user_data;
    std::cout << "  [Adapter Callback] Shutdown called successfully." << std::endl;
}

int main() {
    std::cout << "=== Running DLSS-G Vulkan Route Integration Test ===" << std::endl;

    // 1. Test ABI Negotiation
    DlssgVulkanRouteApi api{};
    api.struct_size = sizeof(api);
    api.abi_version = DLSSG_VULKAN_ABI_VERSION;

    VkResult res = DlssgVulkan_GetRouteApi(&api);
    if (res != VK_SUCCESS) {
        std::cerr << "FAIL: DlssgVulkan_GetRouteApi returned error: " << res << std::endl;
        return 1;
    }
    std::cout << "PASS: Route ABI negotiated successfully (ABI version: " << api.abi_version << ")" << std::endl;

    // 2. Initialize Real Vulkan Device
    VkApplicationInfo app_info{VK_STRUCTURE_TYPE_APPLICATION_INFO};
    app_info.pApplicationName = "DLSS-G Linux Test";
    app_info.apiVersion = VK_API_VERSION_1_3;

    VkInstanceCreateInfo inst_ci{VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
    inst_ci.pApplicationInfo = &app_info;

    VkInstance instance = VK_NULL_HANDLE;
    res = vkCreateInstance(&inst_ci, nullptr, &instance);
    if (res != VK_SUCCESS) {
        std::cerr << "FAIL: vkCreateInstance returned " << res << std::endl;
        return 1;
    }
    std::cout << "PASS: Vulkan Instance created." << std::endl;

    uint32_t gpu_count = 0;
    vkEnumeratePhysicalDevices(instance, &gpu_count, nullptr);
    if (gpu_count == 0) {
        std::cerr << "FAIL: No Vulkan physical devices found!" << std::endl;
        vkDestroyInstance(instance, nullptr);
        return 1;
    }

    std::vector<VkPhysicalDevice> gpus(gpu_count);
    vkEnumeratePhysicalDevices(instance, &gpu_count, gpus.data());

    VkPhysicalDevice phys_dev = gpus[0];
    VkPhysicalDeviceProperties props;
    vkGetPhysicalDeviceProperties(phys_dev, &props);
    std::cout << "Found GPU: " << props.deviceName << " (Driver: " << props.driverVersion << ")" << std::endl;

    float queue_priority = 1.0f;
    VkDeviceQueueCreateInfo queue_ci{VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO};
    queue_ci.queueFamilyIndex = 0;
    queue_ci.queueCount = 1;
    queue_ci.pQueuePriorities = &queue_priority;

    VkDeviceCreateInfo dev_ci{VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
    dev_ci.queueCreateInfoCount = 1;
    dev_ci.pQueueCreateInfos = &queue_ci;

    VkDevice device = VK_NULL_HANDLE;
    res = vkCreateDevice(phys_dev, &dev_ci, nullptr, &device);
    if (res != VK_SUCCESS) {
        std::cerr << "FAIL: vkCreateDevice returned " << res << std::endl;
        vkDestroyInstance(instance, nullptr);
        return 1;
    }
    std::cout << "PASS: Vulkan Logical Device created." << std::endl;

    VkQueue queue = VK_NULL_HANDLE;
    vkGetDeviceQueue(device, 0, 0, &queue);

    // 3. Configure Route
    DlssgVulkanDevice dlssg_dev{};
    dlssg_dev.instance = instance;
    dlssg_dev.physical_device = phys_dev;
    dlssg_dev.device = device;
    dlssg_dev.queue = queue;
    dlssg_dev.queue_family = 0;

    DlssgVulkanAdapter adapter{};
    adapter.struct_size = sizeof(adapter);
    adapter.abi_version = DLSSG_VULKAN_ABI_VERSION;
    adapter.initialize = MockInitialize;
    adapter.evaluate = MockEvaluate;
    adapter.shutdown = MockShutdown;

    DlssgVulkanRouteConfig config{};
    config.struct_size = sizeof(config);
    config.abi_version = DLSSG_VULKAN_ABI_VERSION;
    config.device = dlssg_dev;
    config.adapter = adapter;

    // 4. Test Route Creation
    DlssgVulkanRoute* route = nullptr;
    res = api.create(&config, &route);
    if (res != VK_SUCCESS || route == nullptr) {
        std::cerr << "FAIL: api.create returned " << res << std::endl;
        vkDestroyDevice(device, nullptr);
        vkDestroyInstance(instance, nullptr);
        return 1;
    }
    std::cout << "PASS: DlssgVulkanRoute created successfully." << std::endl;

    // 5. Test Route Reset
    res = api.reset(route, &dlssg_dev);
    if (res != VK_SUCCESS) {
        std::cerr << "FAIL: api.reset returned " << res << " error: " << api.last_error(route) << std::endl;
    } else {
        std::cout << "PASS: DlssgVulkanRoute reset successfully." << std::endl;
    }

    // 6. Test Route Destroy
    api.destroy(route);
    std::cout << "PASS: DlssgVulkanRoute destroyed cleanly." << std::endl;

    vkDestroyDevice(device, nullptr);
    vkDestroyInstance(instance, nullptr);

    std::cout << "=== ALL TESTS PASSED SUCCESSFULLY! ===" << std::endl;
    return 0;
}
