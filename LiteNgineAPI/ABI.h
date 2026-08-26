//dummy class for later

#pragma once
#include <cstdint>
#include <vulkan/vulkan_core.h>
constexpr uint32_t ENGINE_ABI_VERSION = 1;

class IMemoryAllocator {
public:
    virtual void* Allocate(size_t size) = 0;
    virtual void Free(void* ptr) = 0;
};
namespace lt {
    struct formlessData
    {
        void* data;
        size_t size;
    };
}
namespace ltVK 
{
    struct ShaderStageABI {
        VkShaderStageFlagBits stage;       // e.g., VK_SHADER_STAGE_VERTEX_BIT
        const char* entryPoint;  // Usually "main"
        lt::formlessData     spirvCode;   // The compiled shader bytecode
    };

    struct VertexBindingABI {
        uint32_t             binding;
        uint32_t             stride;
        VkVertexInputRate    inputRate;    // VK_VERTEX_INPUT_RATE_VERTEX or INSTANCE
    };

    struct VertexAttributeABI {
        uint32_t             location;
        uint32_t             binding;
        VkFormat             format;       // e.g., VK_FORMAT_R32G32B32_SFLOAT
        uint32_t             offset;
    };

    struct RasterizationStateABI {
        VkPolygonMode        polygonMode;  // VK_POLYGON_MODE_FILL / LINE
        VkCullModeFlags      cullMode;     // VK_CULL_MODE_BACK_BIT
        VkFrontFace          frontFace;    // VK_FRONT_FACE_COUNTER_CLOCKWISE
        float                lineWidth;
        bool                 depthBiasEnable;
    };

    struct DepthStencilStateABI {
        bool                 depthTestEnable;
        bool                 depthWriteEnable;
        VkCompareOp          depthCompareOp; // VK_COMPARE_OP_LESS
        // (Add stencil ops here if your engine uses stencil buffers)
    };

    struct ColorBlendAttachmentABI {
        bool                     blendEnable;
        VkBlendFactor            srcColorBlendFactor;
        VkBlendFactor            dstColorBlendFactor;
        VkBlendOp                colorBlendOp;
        VkBlendFactor            srcAlphaBlendFactor;
        VkBlendFactor            dstAlphaBlendFactor;
        VkBlendOp                alphaBlendOp;
        VkColorComponentFlags    colorWriteMask; // VK_COLOR_COMPONENT_R_BIT | ...
    };

    struct GraphicsPipelineDescription {
        //Shaders
        ShaderStageABI* shaders;
        size_t                      shaderCount;

        //Vertex Input
        VertexBindingABI* vertexBindings;
        size_t                      vertexBindingCount;
        VertexAttributeABI* vertexAttributes;
        size_t                      vertexAttributeCount;

        //Input Assembly
        VkPrimitiveTopology         topology; // VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST

        //Rasterization & Depth
        RasterizationStateABI       rasterization;
        DepthStencilStateABI        depthStencil;

        //Blending
        ColorBlendAttachmentABI* colorBlendAttachments;
        size_t                      colorBlendAttachmentCount;

        //Dynamic States (Allows changing Viewport/Scissor without rebuilding pipeline)
        VkDynamicState* dynamicStates; // e.g., VK_DYNAMIC_STATE_VIEWPORT
        size_t                      dynamicStateCount;

        VkPipelineLayout            pipelineLayout;

        VkRenderPass                renderPass;
        uint32_t                    subpassIndex;

        VkFormat* colorAttachmentFormats;
        size_t                      colorAttachmentCount;
        VkFormat                    depthAttachmentFormat;
    };
}

class CallInterface
{
    //this is for calls between plugins to do stuff
    //each dll gets their own interface so the engine knows which calls are from where
public:
    struct func_rules
    {
        //will expand later
        bool isDefaultPublic = true;
        char** allowedPlugins;
        uint16_t count;
    };
    virtual void call(const char* pluginName,const char* func, lt::formlessData* input, lt::formlessData* output) = 0;
    virtual void func_register(void* (const char*, lt::formlessData*, lt::formlessData*),func_rules rules) = 0;//this function pointer is held by call interface
    virtual void func_unregister(const char* name) = 0;
    virtual void func_addruleallow(const char* plugin) = 0;
};
class DebugInterface
{
    //this is for calls between plugins to do stuff
    //each dll gets their own interface so the engine knows which calls are from where
public:
    //basically just console stuff

};
class IGPUBuilder
{
    //assumes that you dont just have 30 ass gpus but all of them are somewhat competent
    //get the physical device, query for features, then request it 
    virtual VkPhysicalDevice getPhysicalDevice(int id) const = 0;
    virtual void requireFeatureStruct(const void* pFeatureStruct, size_t structSize, int gpuID) = 0;
};
class IGPUManager
{
public:
    
    enum class dType
    {

    };
    virtual VkDevice         getDevice(int id) const = 0;
    virtual VkInstance       getInstance() const = 0;
    virtual VkCommandBuffer  getActiveCommandBuffer(int deviceID) const = 0;
    virtual void CreatePipeline(ltVK::GraphicsPipelineDescription* info) = 0;
    //? stuff here
    virtual void testFunc() = 0;
};
//virtual interface
class IEnginePlugin {
public:
    virtual const char* GetName() const = 0;//only use const for getters and handlers

    // Lifecycle hooks
    //these are called from the engine to the plugins
    virtual void OnBootload() = 0;
    virtual void PreGraphicsCreation(class IGPUBuilder* gpu) = 0;

    virtual void OnGraphicsInjection(class IGPUManager* gpu        ) = 0;
    virtual void OnCallAPIDecl(class CallInterface* interface) = 0;
    virtual void OnDebugAPI(class    DebugInterface* interface) = 0;
    virtual void OnHibernation() = 0;
    virtual void OnEditorOpen() = 0;
    virtual void OnEditorTick( float deltaTime) = 0;

    virtual void OnProjectLoading() = 0;
    virtual void OnProjectOpen() = 0;
    virtual void OnProjectTick(float deltaTime) = 0;
    virtual void OnProjectClosing() = 0;

    virtual void OnGameLoading() = 0;
    virtual void OnGameOpen() = 0;
    virtual void OnGameTick(float deltaTime) = 0;
    virtual void OnGameClosing() = 0;

    virtual void OnQuitEditor() = 0;
};

//export macros
#if defined(_WIN32)
#define PLUGIN_EXPORT extern "C" __declspec(dllexport) 
#else
#define PLUGIN_EXPORT extern "C"
#endif

PLUGIN_EXPORT IEnginePlugin*    CreatePlugin(IMemoryAllocator* allocator);
PLUGIN_EXPORT void              DestroyPlugin(IEnginePlugin* plugin);
