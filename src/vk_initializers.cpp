#include <vk_initializers.h>


VkCommandPoolCreateInfo vkinit::command_pool_create_info(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags )
{
	//@@INTERESTING
	//we will use the {} initializer, which zeroes out all the fields of the struct
	//this is specially important in vulkan, since unwanted values in the struct can cause errors, and vulkan doesn't initialize them for us
	VkCommandPoolCreateInfo commandPoolInfo = {};

	commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	commandPoolInfo.pNext = nullptr;

	//we submit the graphics commands
	commandPoolInfo.queueFamilyIndex = queueFamilyIndex;

	commandPoolInfo.flags = flags;

	return commandPoolInfo;
}

VkCommandBufferAllocateInfo vkinit::command_buffer_allocate_info(VkCommandPool pool, uint32_t count, VkCommandBufferLevel level)
{
	VkCommandBufferAllocateInfo cmdAllocInfo = {};
	cmdAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cmdAllocInfo.pNext = nullptr;

	//commands made from the command pool we just made
	cmdAllocInfo.commandPool = pool;
	cmdAllocInfo.commandBufferCount = count;
	cmdAllocInfo.level = level; 

	return cmdAllocInfo;
}