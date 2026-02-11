
#include "vk_engine.h"

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


}


void VulkanEngine::cleanup()
{	
	if (_isInitialized) {
		
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

