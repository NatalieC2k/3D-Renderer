#include "pipeline.h"

namespace render{
VkShaderModule Shader::CompileGlsl(size_t buffer_size, char* buffer){
    return VK_NULL_HANDLE;
}
VkShaderModule Shader::CompileSpirv(size_t buffer_size, char* buffer){
    VkShaderModuleCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    create_info.pNext = nullptr;
    create_info.flags = 0;
    create_info.codeSize = buffer_size;
    create_info.pCode = (uint32_t*)buffer;
    
    VkShaderModule vk_shader_module;
    VkResult vk_result = vkCreateShaderModule(render::context.vk_device, &create_info, nullptr, &vk_shader_module);
    if(vk_result != VK_SUCCESS){
        throw std::runtime_error("FAILED TO CREATE SHADER MODULE FROM SPIRV");
    }
    return vk_shader_module;
}

// --- Member Functions --- //
Shader::Shader(ShaderStage shader_stage, ShaderFormat shader_code_format, size_t buffer_size, char* buffer)
: shader_stage_(shader_stage) {
    switch(shader_code_format){
        case SHADER_FORMAT_GLSL:
            break;
        case SHADER_FORMAT_SPIRV:
            vk_shader_module_ = CompileSpirv(buffer_size, buffer);
            break;
    }
}
Shader::Shader(ShaderInfo info)
: shader_stage_(info.shader_stage) {
    std::ifstream file(info.filepath);
    file.seekg(0, std::ios::end);
    size_t size = file.tellg();
    file.seekg(0);
    
    char* buffer = new char[size];
    file.read(buffer, size);
    
    switch(info.shader_code_format){
        case SHADER_FORMAT_GLSL:
            break;
        case SHADER_FORMAT_SPIRV:
            vk_shader_module_ = CompileSpirv(size, buffer);
            break;
    }
}
Shader::~Shader(){
    vkDestroyShaderModule(render::context.vk_device, vk_shader_module_, nullptr);
}

ShaderStage Shader::GetStage(){
    return shader_stage_;
}
VkShaderModule Shader::GetModule(){
    return vk_shader_module_;
}

// --- Pipeline --- //
Pipeline::Pipeline(){}
Pipeline::~Pipeline(){}

void Pipeline::Initialize(PipelineInfo info){
    std::vector<VkPipelineShaderStageCreateInfo> vk_shader_stage_info{};
    VkPipelineShaderStageCreateInfo stage_info{};
    stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stage_info.pName = "main";
    for(Shader* shader : info.shaders){
        stage_info.stage = (VkShaderStageFlagBits)shader->GetStage();
        stage_info.module = shader->GetModule();
        vk_shader_stage_info.emplace_back(stage_info);
    }

    VkDynamicState dynamic_states[] = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };
    
    VkPipelineDynamicStateCreateInfo dynamic_state{};
    dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamic_state.dynamicStateCount = 2;
    dynamic_state.pDynamicStates = dynamic_states;
    
    VkPipelineVertexInputStateCreateInfo vertex_info{};
    vertex_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertex_info.vertexBindingDescriptionCount = (uint32_t)info.vertex_bindings.size();
    vertex_info.pVertexBindingDescriptions    = (VkVertexInputBindingDescription*)info.vertex_bindings.data();
    vertex_info.vertexAttributeDescriptionCount = (uint32_t)info.vertex_attributes.size();
    vertex_info.pVertexAttributeDescriptions    = (VkVertexInputAttributeDescription*)info.vertex_attributes.data();
    
    VkPipelineInputAssemblyStateCreateInfo input_assembly{};
    input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    input_assembly.primitiveRestartEnable = VK_FALSE;
    
     
     VkPipelineViewportStateCreateInfo viewport_state{};
     viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
     viewport_state.viewportCount = 1;
     viewport_state.scissorCount  = 1;
    
    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    
    rasterizer.cullMode  = (VkCullModeFlags)info.cull_mode;
    rasterizer.frontFace = (VkFrontFace)info.front_face;
    
    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f;
    rasterizer.depthBiasClamp = 0.0f;
    rasterizer.depthBiasSlopeFactor = 0.0f;
    
    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading = 1.0f;
    multisampling.pSampleMask = nullptr;
    multisampling.alphaToCoverageEnable = VK_FALSE;
    multisampling.alphaToOneEnable = VK_FALSE;
    
    VkPipelineColorBlendAttachmentState blend_attachment{};
    blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    blend_attachment.blendEnable = VK_FALSE;
    blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;
    blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;
    
    VkPipelineDepthStencilStateCreateInfo depth_stencil{};
    depth_stencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depth_stencil.flags = 0;
    depth_stencil.depthTestEnable  = info.depth_test_enabled;
    depth_stencil.depthWriteEnable = info.depth_write_enabled;
    depth_stencil.depthCompareOp   = VK_COMPARE_OP_LESS;
    
    depth_stencil.depthBoundsTestEnable = VK_FALSE;
    depth_stencil.minDepthBounds = 0.0f;
    depth_stencil.maxDepthBounds = 1.0f;
    
    depth_stencil.stencilTestEnable = VK_FALSE;
    depth_stencil.front = {};
    depth_stencil.back  = {};
    
    VkPipelineColorBlendStateCreateInfo blend_state{};
    blend_state.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    blend_state.logicOpEnable = VK_FALSE;
    blend_state.logicOp = VK_LOGIC_OP_COPY;
    blend_state.attachmentCount = 1;
    blend_state.pAttachments = &blend_attachment;
    blend_state.blendConstants[0] = 0.0f;
    blend_state.blendConstants[1] = 0.0f;
    blend_state.blendConstants[2] = 0.0f;
    blend_state.blendConstants[3] = 0.0f;
    
    /* Pipeline Layout */
    VkPipelineLayoutCreateInfo layout_info{};
    layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    layout_info.pushConstantRangeCount = (uint32_t)info.push_constant_ranges.size();
    layout_info.pPushConstantRanges    = (VkPushConstantRange*)info.push_constant_ranges.data();
    layout_info.setLayoutCount = (uint32_t)info.descriptor_set_layouts.size();
    layout_info.pSetLayouts    = (VkDescriptorSetLayout*)info.descriptor_set_layouts.data();
    
    VkPipelineLayout pipeline_layout;
    VkResult vk_result = vkCreatePipelineLayout(render::context.vk_device,
                                                &layout_info, nullptr, &pipeline_layout);
    if (vk_result != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout!");
    }
    
    VkGraphicsPipelineCreateInfo pipeline_info{};
    pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipeline_info.stageCount = (uint32_t)vk_shader_stage_info.size();
    pipeline_info.pStages    =           vk_shader_stage_info.data();
    
    pipeline_info.pVertexInputState   = &vertex_info;
    pipeline_info.pInputAssemblyState = &input_assembly;
    pipeline_info.pViewportState      = &viewport_state;
    pipeline_info.pRasterizationState = &rasterizer;
    pipeline_info.pMultisampleState   = &multisampling;
    pipeline_info.pDepthStencilState  = &depth_stencil;
    pipeline_info.pColorBlendState    = &blend_state;
    pipeline_info.pDynamicState       = &dynamic_state;
    
    pipeline_info.layout = pipeline_layout;
    
    pipeline_info.renderPass = info.render_buffer->vk_render_pass;
    pipeline_info.subpass = 0;
    
    pipeline_info.basePipelineHandle = VK_NULL_HANDLE;
    pipeline_info.basePipelineHandle = VK_NULL_HANDLE;
    pipeline_info.basePipelineIndex = -1;
    
    VkPipeline pipeline;
    vk_result = vkCreateGraphicsPipelines(render::context.vk_device, VK_NULL_HANDLE,
                                          1, &pipeline_info, nullptr, &pipeline);
    
    render::pipeline_manager.compilation_mutex.lock();
    vk_pipeline_layout = pipeline_layout;
    vk_pipeline        = pipeline;
    render::pipeline_manager.compilation_mutex.unlock();

    if (vk_result != VK_SUCCESS) {
        throw std::runtime_error("FAILED TO CREATE GRAPHICS PIPELINE");
    }
}
void Pipeline::Terminate(){
    vkDestroyPipeline(render::context.vk_device, vk_pipeline, nullptr);
    vkDestroyPipelineLayout(render::context.vk_device, vk_pipeline_layout, nullptr);
}

void Pipeline::Bind(VkCommandBuffer command_buffer){
    vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vk_pipeline);
}
void Pipeline::PushConstant(VkCommandBuffer vk_command_buffer,
                            VkDeviceSize size, VkDeviceSize offset, void* data){
    vkCmdPushConstants(vk_command_buffer, vk_pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT, 0, 64, data);
}
void Pipeline::BindDescriptorSet(VkCommandBuffer vk_command_buffer, 
                                 DescriptorSet descriptor_set, uint32_t binding){
    vkCmdBindDescriptorSets(vk_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vk_pipeline_layout, 
                            binding, 1, &descriptor_set.vk_descriptor_set, 0, nullptr);
}




PipelineManager pipeline_manager{};
void PipelineManager::Initialize(){}
void PipelineManager::Terminate(){}

Pipeline* PipelineManager::Compile(PipelineInfo info){
    Pipeline* new_pipeline = new Pipeline{};
    
    core::threadpool.Dispatch([this, new_pipeline, info]{
        new_pipeline->Initialize(info);
        compilation_condition_variable.notify_all();
        return core::Threadpool::TASK_COMPLETE;
    });
    
    return new_pipeline;
}
void PipelineManager::AwaitCompilation(Pipeline* pipeline){
    std::unique_lock<std::mutex> lock(compilation_mutex);
    compilation_condition_variable.wait(lock, [pipeline]{
        return pipeline->vk_pipeline != VK_NULL_HANDLE;
    });
}

void PipelineManager::Destroy(Pipeline* pipeline){
    pipeline->Terminate();
    delete pipeline;
}
}
