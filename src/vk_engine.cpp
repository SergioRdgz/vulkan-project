
#include "vk_engine.h"
#include "vk_initializers.h"
#include <SDL.h>
#include <SDL_vulkan.h>

#include <vk_types.h>
#include <vk_initializers.h>

#include "VkBootstrap.h"

#include <iostream> //will need this for error output

using namespace std;

#define VK_CHECK(x)												\
	do {														\
		VkResult err = x;										\
		if (err) {												\
			cout << "Detected Vulkan error: " << err << endl;	\
			abort();											\
		}														\
	} while (0)

void VulkanEngine::init()
{
	// We initialize SDL and create a window with it. 
	SDL_Init(SDL_INIT_VIDEO);

	SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_VULKAN);
	
	_window = SDL_CreateWindow(
		"Vulkan Engine",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		_windowExtent.width,
		_windowExtent.height,
		window_flags
	);
	
	init_vulkan();

	init_swapchain();

	init_commands();

	init_default_renderpass();

	init_framebuffers();

	init_sync_structures();

	//everything went fine
	_isInitialized = true;
}

void VulkanEngine::init_vulkan()
{
	//VkBootstrap library, simplifies the creation of a Vulkan VkInstance
	//@@INTERESTING
	vkb::InstanceBuilder builder;

	//make the instance, with basic debug features
	auto inst_ret = builder.set_app_name("vulkan exercise")
		.request_validation_layers(true)
		.require_api_version(1, 1, 0)		//got 1.3.something myself
		.build();

	vkb::Instance vkb_inst = inst_ret.value();

	instance = vkb_inst.instance;

	debug_messenger = vkb_inst.debug_messenger;

	//now the surface of the window
	SDL_Vulkan_CreateSurface(_window, instance, &surface);

	//use vkbootstrap again to select a gpu
	vkb::PhysicalDeviceSelector selector { vkb_inst };

	vkb::PhysicalDevice physicalDevice = selector
		.set_minimum_version(1, 1)
		.set_surface(surface)
		.select()
		.value();
	//@@INTERESTING
	//I need to look into whats going on in this call
	//how it selects one gpu from the ones available
	//and what criteria it uses 

	//create the Vulkan device
	vkb::DeviceBuilder deviceBuilder{ physicalDevice };

	vkb::Device vkbdevice = deviceBuilder.build().value();

	device = vkbdevice.device;
	chosenGPU = physicalDevice.physical_device;

	graphicsQueue = vkbdevice.get_queue(vkb::QueueType::graphics).value();
	graphicsQueueFamily = vkbdevice.get_queue_index(vkb::QueueType::graphics).value();

}

void VulkanEngine::init_swapchain()
{
	//as with the vulkan init function, use vkb library to simplify the process

	vkb::SwapchainBuilder swapchainBuilder{ chosenGPU, device, surface };

	vkb::Swapchain vkbSwapchain = swapchainBuilder
		.use_default_format_selection()
		.set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
		.set_desired_extent(_windowExtent.width, _windowExtent.height)
		.build()
		.value();
	//@@INTERESTING
	//what does the value() function do in vkb?
	 //if build is a success it returns the vkb::swapchain, if not it will trigger an error
		//shouldn't this be handled with a check to catch the error then???


	swapchain = vkbSwapchain.swapchain;
	
	swapchainImages = vkbSwapchain.get_images().value();
	swapchainImageViews = vkbSwapchain.get_image_views().value();

	swapchainImageFormat = vkbSwapchain.image_format;
}

void VulkanEngine::init_commands() 
{
	//implementation at vk_initializers.cpp
	VkCommandPoolCreateInfo commandPoolInfo = vkinit::command_pool_create_info(graphicsQueueFamily, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
	
	//macro defined at line 16, check result, and prints error if it fails
	//also crashes the program if fail
	VK_CHECK(vkCreateCommandPool(device, &commandPoolInfo, nullptr, &commandPool)); 


	VkCommandBufferAllocateInfo cmdAllocInfo = vkinit::command_buffer_allocate_info(commandPool, 1, VK_COMMAND_BUFFER_LEVEL_PRIMARY);

	VK_CHECK(vkAllocateCommandBuffers(device, &cmdAllocInfo, &mainCommandBuffer));
}

void VulkanEngine::init_default_renderpass()
{
	//the color attachment
	VkAttachmentDescription colorAttDesc = {};
	colorAttDesc.format = swapchainImageFormat;
	colorAttDesc.samples = VK_SAMPLE_COUNT_1_BIT; //no MSAA, 1 sample 
	colorAttDesc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; //clear when this attachment is loaded
	colorAttDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE; //keep the attachment stored when we are done

	colorAttDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

	//@@INTERESTING
	//the tutorial says we dont care about the starting layout
	//I am kind of confused about what this means and its implications
	//is this because we clear whenever we load the attachment?
	colorAttDesc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

	colorAttDesc.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; // when renderpass done be in a layout display friendly

	VkAttachmentReference colorAttRef = {};
	colorAttRef.attachment = 0; //index of the attachment in the renderpass (we only have one)

	//Optimal because despite the atrocities this vulkan engine will do
	//we want those who know how to do their job to do it well :)
	colorAttRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	//making the minimum subpass needed, 1
	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttRef;

	VkRenderPassCreateInfo renderPassInfo = {};

	//@@INTERESTING
	//why do I need a flagto say this class is doing what its name suggests??
	//is there any other thing it could do?????
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;

	//giving the elements we made before, the color attachments description
	//and the subpass
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments = &colorAttDesc;

	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	
	VK_CHECK(vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass));
}

void VulkanEngine::init_framebuffers()
{
	VkFramebufferCreateInfo fbInfo = {};
	fbInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	fbInfo.pNext = nullptr;

	fbInfo.renderPass = renderPass;
	fbInfo.attachmentCount = 1; //only color attachment
	fbInfo.width = _windowExtent.width;
	fbInfo.height = _windowExtent.height;
	fbInfo.layers = 1; 

	//get image amount from swapchain
	const uint32_t swapchainImageCount = swapchainImages.size();
	frameBuffers = std::vector<VkFramebuffer>(swapchainImageCount);

	for (int i = 0; i < swapchainImageCount; i++)
	{
		fbInfo.pAttachments = &swapchainImageViews[i];
		VK_CHECK(vkCreateFramebuffer(device, &fbInfo, nullptr, &frameBuffers[i]));
	}

}

void VulkanEngine::init_sync_structures()
{
	VkFenceCreateInfo fenceInfo = {};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.pNext = nullptr;

	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT; 

	VK_CHECK(vkCreateFence(device, &fenceInfo, nullptr, &renderFence));

	VkSemaphoreCreateInfo semaphoreInfo = {};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	semaphoreInfo.pNext = nullptr;
	semaphoreInfo.flags = 0;

	VK_CHECK(vkCreateSemaphore(device, &semaphoreInfo, nullptr, &presentSemaphore));
	VK_CHECK(vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderSemaphore));
}

void VulkanEngine::cleanup()
{	
	if (_isInitialized) 
	{	
		//ensure they are done being used
		vkWaitForFences(device, 1, &renderFence, true, 1000000000);
		vkDestroyFence(device, renderFence, nullptr);

		vkDestroySemaphore(device, presentSemaphore, nullptr);
		vkDestroySemaphore(device, renderSemaphore, nullptr);

		vkDestroyCommandPool(device, commandPool, nullptr);

		vkDestroySwapchainKHR(device, swapchain, nullptr);

		vkDestroyRenderPass(device, renderPass, nullptr);

		for (int i = 0; i < swapchainImageViews.size(); i++)
		{
			vkDestroyFramebuffer(device, frameBuffers[i], nullptr);
			vkDestroyImageView(device, swapchainImageViews[i], nullptr);
		}


		vkDestroyDevice(device, nullptr);

		vkDestroySurfaceKHR(instance, surface, nullptr);

		vkb::destroy_debug_utils_messenger(instance, debug_messenger);
		vkDestroyInstance(instance, nullptr);


		SDL_DestroyWindow(_window);
	}
}

void VulkanEngine::draw()
{

	VK_CHECK(vkWaitForFences(device, 1, &renderFence, true, 1000000000)); 
	VK_CHECK(vkResetFences(device, 1, &renderFence));

	uint32_t swapchainImageIndex;
	VK_CHECK(vkAcquireNextImageKHR(device, swapchain, 1000000000, presentSemaphore, nullptr, &swapchainImageIndex));

	//now we are sure the commands finished executing
	VK_CHECK(vkResetCommandBuffer(mainCommandBuffer, 0));

	VkCommandBufferBeginInfo cmdBeginInfo = {};
	cmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	cmdBeginInfo.pNext = nullptr;

	//@@INTERESTING
	//look into what is this and why set to nullptr
	// "Inheritance info on a command buffer is used for secondary command buffers"
	cmdBeginInfo.pInheritanceInfo = nullptr; 
	cmdBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT; 

	VK_CHECK(vkBeginCommandBuffer(mainCommandBuffer, &cmdBeginInfo));

	VkClearValue clearValue;
	float flash = abs(sin(_frameNumber / 120.f));
	clearValue.color = { { 0.0f, 0.0f, flash, 1.0f } };

	//@@INTERESTING
	//why is this needed? didnt we make a render pass already?
	VkRenderPassBeginInfo rpInfo = {};
	rpInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	rpInfo.pNext = nullptr;

	rpInfo.renderPass = renderPass;
	rpInfo.renderArea.offset.x = 0;
	rpInfo.renderArea.offset.y = 0;
	rpInfo.renderArea.extent = _windowExtent;
	rpInfo.framebuffer = frameBuffers[swapchainImageIndex];

	rpInfo.clearValueCount = 1;
	rpInfo.pClearValues = &clearValue;

	vkCmdBeginRenderPass(mainCommandBuffer, &rpInfo, VK_SUBPASS_CONTENTS_INLINE);

	vkCmdEndRenderPass(mainCommandBuffer);

	VK_CHECK(vkEndCommandBuffer(mainCommandBuffer));


	// prepare the submission to the queue.
//wait on the presentSemaphore, as that semaphore is signaled when the swapchain is ready
//signal the renderSemaphore, to signal that rendering has finished

	VkSubmitInfo submit = {};
	submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit.pNext = nullptr;

	VkPipelineStageFlags  waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

	submit.pWaitDstStageMask = &waitStage;
	submit.waitSemaphoreCount = 1;

	submit.pWaitSemaphores = &presentSemaphore;

	submit.signalSemaphoreCount = 1;
	submit.pSignalSemaphores = &renderSemaphore;

	submit.commandBufferCount = 1;
	submit.pCommandBuffers = &mainCommandBuffer;

	VK_CHECK(vkQueueSubmit(graphicsQueue, 1, &submit, renderFence));


	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.pNext = nullptr;

	presentInfo.pSwapchains = &swapchain;
	presentInfo.swapchainCount = 1;

	presentInfo.pWaitSemaphores = &renderSemaphore;
	presentInfo.waitSemaphoreCount = 1;

	presentInfo.pImageIndices = &swapchainImageIndex;

	VK_CHECK(vkQueuePresentKHR(graphicsQueue, &presentInfo));

	_frameNumber++;
}

void VulkanEngine::run()
{
	SDL_Event e;
	bool bQuit = false;

	//main loop
	while (!bQuit)
	{
		//Handle events on queue
		while (SDL_PollEvent(&e) != 0)
		{
			//close the window when user alt-f4s or clicks the X button			
			if (e.type == SDL_QUIT) bQuit = true;
		}

		draw();
	}
}

