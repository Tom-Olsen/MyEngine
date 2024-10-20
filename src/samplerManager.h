#pragma once
#ifndef __INCLUDE_GUARD_samplerManager_h__
#define __INCLUDE_GUARD_samplerManager_h__
#include <unordered_map>
#include <memory>
#include <string>
#include "vulkanSampler.h"



/// <summary>
/// Purely static class that takes care of lifetime of all Material objects.
/// </summary>
class SamplerManager
{
public: // Members

private: // Members
    static bool isInitialized;
    static VulkanContext* context;
    static std::unordered_map<std::string, std::unique_ptr<VulkanSampler>> samplers;

public: // Methods
    static void Init(VulkanContext* vulkanContext);
    static void Clear();

    static void AddSampler(const std::string name, VulkanSampler* sampler);
    static VulkanSampler* GetSampler(const std::string& name);
    static void DeleteSampler(const std::string& name);

    static void PrintAllSamplerNames();

private: // Methods
    // Delete all constructors:
    SamplerManager() = delete;
    SamplerManager(const SamplerManager&) = delete;
    SamplerManager& operator=(const SamplerManager&) = delete;
    ~SamplerManager() = delete;
};



#endif // __INCLUDE_GUARD_samplerManager_h__