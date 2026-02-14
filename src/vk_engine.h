// vulkan_guide.h : Include file for standard system include files,
// or project specific include files.

#pragma once

#include <vk_types.h>

class VulkanEngine {
public:

	bool _isInitialized{ false };
	int _frameNumber {0};

	VkExtent2D _windowExtent{ 1700 , 900 };

	struct SDL_Window* _window{ nullptr };

	//initializes everything in the engine
	void init();

	//shuts down the engine
	void cleanup();

	//draw loop
	void draw();

	//run main loop
	void run();

	//actual vulkan elements
	VkInstance instance;								//library handle
	VkDebugUtilsMessengerEXT debug_messenger;			//debug output handle
	
	VkPhysicalDevice chosenGPU;							//the graphics card chosen
	VkDevice device; 									//logical device handle	
	
	VkSurfaceKHR surface; 								//the surface of the window in vulkan	

	VkSwapchainKHR swapchain;							//the swapchain handle
	VkFormat swapchainImageFormat;						//the format of the swapchain images

	std::vector<VkImage> swapchainImages;				//the images in the swapchain
	std::vector<VkImageView> swapchainImageViews;		//the image views for the swapchain images

	VkQueue graphicsQueue;								//the queue we will submit draw calls to
	uint32_t graphicsQueueFamily;						//the index for the family

	VkCommandPool commandPool;							//used to allocate command buffers from	
	VkCommandBuffer mainCommandBuffer;					//record drawing commands into this command buffer

	//finally getting to the fun stuff
	VkRenderPass renderPass; 							//describes the framebuffer attachments used in rendering	
	std::vector<VkFramebuffer> frameBuffers;			//framebuffers for the render pass, one per swapchain image

	VkSemaphore presentSemaphore,renderSemaphore;
	VkFence renderFence;


	VkPipelineLayout trianglePipelineLayout;
	VkPipeline trianglePipeline;


private:
	
	//this are some of the functions I need to pay more attention to
	//and better understand whats going on
	void init_vulkan();
	void init_swapchain();
	void init_commands();
	void init_default_renderpass();
	void init_framebuffers();
	void init_sync_structures();
	void init_pipelines();

	//loads a shader module from a spir-v file. Returns false if it errors
	bool load_shader_module(const char* filePath, VkShaderModule* outShaderModule);

};

class PipelineBuilder
{
public:
	std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
	VkPipelineVertexInputStateCreateInfo vertexInputInfo;
	VkPipelineInputAssemblyStateCreateInfo inputAssembly;
	VkViewport viewport;
	VkRect2D scissor;
	VkPipelineRasterizationStateCreateInfo rasterizer;
	VkPipelineColorBlendAttachmentState colorBlendAttachment;
	VkPipelineMultisampleStateCreateInfo multisampling;
	VkPipelineLayout pipelineLayout;

	VkPipeline build_pipeline(VkDevice device, VkRenderPass pass);
};