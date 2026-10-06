#include "vulkan_ngx_loader.h"

#if defined(_WIN32)
#include <windows.h>
using ModuleHandle = HMODULE;
#else
#include <dlfcn.h>
#include <cwchar>
#include <vector>
using ModuleHandle = void*;
#endif

#include <new>
#include <string>

struct DlssgVulkanNgx {
    ModuleHandle module = nullptr;
    uint32_t resolved = 0;
    DlssgVulkanDevice device{};
    DlssgVulkanLifecycle lifecycle{};
    std::string error;
};

namespace {

constexpr uint32_t kRequiredExports = 11;
constexpr const char* kExports[] = {
    "NVSDK_NGX_VULKAN_Init",
    "NVSDK_NGX_VULKAN_Init_Ext",
    "NVSDK_NGX_VULKAN_Init_Ext2",
    "NVSDK_NGX_VULKAN_CreateFeature",
    "NVSDK_NGX_VULKAN_CreateFeature1",
    "NVSDK_NGX_VULKAN_GetFeatureRequirements",
    "NVSDK_NGX_VULKAN_GetScratchBufferSize",
    "NVSDK_NGX_VULKAN_EvaluateFeature",
    "NVSDK_NGX_VULKAN_ReleaseFeature",
    "NVSDK_NGX_VULKAN_Shutdown",
    "NVSDK_NGX_VULKAN_Shutdown1",
};

bool HasSize(uint32_t actual, size_t required) {
    return actual >= required;
}

void Error(DlssgVulkanNgx* loader, const char* message) {
    loader->error = message;
}

static ModuleHandle OpenNgxLibrary(const wchar_t* path) {
#if defined(_WIN32)
    return LoadLibraryW(path != nullptr ? path : L"nvngx_dlssg.dll");
#else
    std::string path_str;
    if (path != nullptr) {
        size_t len = wcslen(path);
        std::vector<char> buf(len * 4 + 1);
        size_t converted = wcstombs(buf.data(), path, buf.size());
        if (converted != static_cast<size_t>(-1)) {
            path_str = buf.data();
        } else {
            for (size_t i = 0; i < len; ++i) {
                path_str.push_back(static_cast<char>(path[i]));
            }
        }
    } else {
        path_str = "libnvngx_dlssg.so";
    }

    void* handle = dlopen(path_str.c_str(), RTLD_NOW | RTLD_LOCAL);
    if (!handle && path == nullptr) {
        handle = dlopen("./libnvngx_dlssg.so", RTLD_NOW | RTLD_LOCAL);
        if (!handle) {
            handle = dlopen("nvngx_dlssg.so", RTLD_NOW | RTLD_LOCAL);
        }
        if (!handle) {
            handle = dlopen("nvngx_dlssg.dll", RTLD_NOW | RTLD_LOCAL);
        }
    }
    return handle;
#endif
}

static void* GetFunctionSymbol(ModuleHandle mod, const char* name) {
#if defined(_WIN32)
    return reinterpret_cast<void*>(GetProcAddress(mod, name));
#else
    return dlsym(mod, name);
#endif
}

static void CloseNgxLibrary(ModuleHandle mod) {
#if defined(_WIN32)
    FreeLibrary(mod);
#else
    dlclose(mod);
#endif
}

}  // namespace

extern "C" DLSSG_NGX_API VkResult DLSSG_NGX_CALL
DlssgVulkanNgx_Load(const wchar_t* path, DlssgVulkanNgx** output) {
    if (output == nullptr) {
        return VK_ERROR_INITIALIZATION_FAILED;
    }
    *output = nullptr;
    DlssgVulkanNgx* loader = new (std::nothrow) DlssgVulkanNgx();
    if (loader == nullptr) {
        return VK_ERROR_OUT_OF_HOST_MEMORY;
    }
    loader->module = OpenNgxLibrary(path);
    if (loader->module == nullptr) {
        Error(loader, "Could not load the NGX Vulkan runtime");
        delete loader;
        return VK_ERROR_INITIALIZATION_FAILED;
    }
    for (const char* name : kExports) {
        if (GetFunctionSymbol(loader->module, name) != nullptr) {
            ++loader->resolved;
        }
    }
    if (loader->resolved != kRequiredExports) {
        Error(loader, "NGX runtime is missing required Vulkan exports");
        CloseNgxLibrary(loader->module);
        delete loader;
        return VK_ERROR_INCOMPATIBLE_DRIVER;
    }
    *output = loader;
    return VK_SUCCESS;
}

extern "C" DLSSG_NGX_API void DLSSG_NGX_CALL
DlssgVulkanNgx_Unload(DlssgVulkanNgx* loader) {
    if (loader == nullptr) {
        return;
    }
    if (loader->module != nullptr) {
        CloseNgxLibrary(loader->module);
    }
    delete loader;
}

extern "C" DLSSG_NGX_API VkResult DLSSG_NGX_CALL
DlssgVulkanNgx_GetStatus(const DlssgVulkanNgx* loader,
                         DlssgVulkanNgxStatus* status) {
    if (loader == nullptr || status == nullptr ||
        !HasSize(status->struct_size, sizeof(*status)) ||
        status->abi_version != DLSSG_VULKAN_ABI_VERSION) {
        return VK_ERROR_INITIALIZATION_FAILED;
    }
    status->required_exports = kRequiredExports;
    status->resolved_exports = loader->resolved;
    status->ready = loader->resolved == kRequiredExports;
    status->supports_extended_init = 1;
    status->supports_feature_v1 = 1;
    status->supports_shutdown_v1 = 1;
    return status->ready ? VK_SUCCESS : VK_ERROR_INCOMPATIBLE_DRIVER;
}

extern "C" DLSSG_NGX_API VkResult DLSSG_NGX_CALL
DlssgVulkanNgx_OnDeviceCreated(DlssgVulkanNgx* loader,
                               const DlssgVulkanDevice* device) {
    if (loader == nullptr || device == nullptr ||
        device->instance == VK_NULL_HANDLE ||
        device->physical_device == VK_NULL_HANDLE ||
        device->device == VK_NULL_HANDLE) {
        return VK_ERROR_INITIALIZATION_FAILED;
    }
    loader->device = *device;
    return VK_SUCCESS;
}

extern "C" DLSSG_NGX_API VkResult DLSSG_NGX_CALL
DlssgVulkanNgx_OnSwapchainChanged(DlssgVulkanNgx* loader,
                                  const DlssgVulkanLifecycle* lifecycle) {
    if (loader == nullptr || lifecycle == nullptr ||
        !HasSize(lifecycle->struct_size, sizeof(*lifecycle)) ||
        lifecycle->abi_version != DLSSG_VULKAN_ABI_VERSION ||
        lifecycle->swapchain == VK_NULL_HANDLE || lifecycle->width == 0 ||
        lifecycle->height == 0) {
        return VK_ERROR_INITIALIZATION_FAILED;
    }
    if (loader->device.device != VK_NULL_HANDLE &&
        lifecycle->device.device != loader->device.device) {
        Error(loader, "Swapchain belongs to a different Vulkan device");
        return VK_ERROR_DEVICE_LOST;
    }
    loader->lifecycle = *lifecycle;
    return VK_SUCCESS;
}

extern "C" DLSSG_NGX_API const char* DLSSG_NGX_CALL
DlssgVulkanNgx_LastError(const DlssgVulkanNgx* loader) {
    return loader == nullptr ? "NGX loader handle is null" : loader->error.c_str();
}
