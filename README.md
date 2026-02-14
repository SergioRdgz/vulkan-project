
Vulkan project from scratch to review the API

    TODO:
      Window not in full screen 
      Models
      Textures  
      Shaders ( descriptor sets, render pass, etc)  
      Resize window
      Looking into alpha blending equations, and transparency sorting
      Transfer queue for asset uploading
      Multithreading to practice submitting command buffers to a main thread for queue submission
  

***Command Buffers***

"Recording commands in Vulkan is relatively cheap. Most of the work goes into the vkQueueSubmit call, where the commands are validated in the driver and translated into real GPU commands."

<img width="473" height="411" alt="image" src="https://github.com/user-attachments/assets/3f0065c2-c0ef-4c82-b4e6-d1cccf543d6d" />


***About the render pass***

this is roughly what I intent the render pass to do with the color attachment
UNDEFINED -> RenderPass Begins -> Subpass 0 begins (Transition to Attachment Optimal) -> Subpass 0 renders -> Subpass 0 ends -> Renderpass Ends (Transitions to Present Source)
