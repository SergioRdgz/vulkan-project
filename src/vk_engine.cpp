
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
void VulkanEngine::cleanup()
{	
	if (_isInitialized) 
	{
		vkDestroyCommandPool(device, commandPool, nullptr);

		vkDestroySwapchainKHR(device, swapchain, nullptr);

		for (int i = 0; i < swapchainImageViews.size(); i++)
		{
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
	//nothing yet
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

