Github link：https://github.com/JiyefuChen/3015cw1.git

youtube link：https://youtu.be/-F6OA2LBTCg

## 1. Which version Visual Studio and Operating System you used to test/write the code  locally?

I am using vs2022. The OpenGL version is 4.6.0

## 2. How does it work? o How does your code fit together and how should a programmer  navigate it (not intended to be an exercise in formally documenting the code)? 



To initialize the OpenGL environment and Windows:

Create and manage application Windows using the GLFW library, and load OpenGL functions using GLAD.
Initializes a Window object, mainWindow, and sets the window size and other properties.
Load and prepare resources:

Load shader programs (such as basic render shaders, shadow mapping shaders, etc.).
Load 3D models and textures, prepare light objects (directional light, point light) and material properties.
Set camera parameters and define viewing and projection matrices.
Render loop:

In each frame, input is processed, camera perspective and position updated, and other possible dynamic scene elements.
Execute the shadow mapping rendering pass to generate shadow mapping for the light source.
Render scenes, including 3D models and sky boxes, applying textures, lighting and shadow effects.
Swap buffers and update window contents to show the latest rendered frame.
How do programmers manipulate it
Environment configuration:

Make sure you have all the necessary libraries and dependencies installed (GLFW, GLAD, GLM, etc.).



The w key moves the view forward
The s button shifts the viewing Angle backwards
The a key shifts the viewing Angle to the left
The d key shifts the viewing Angle to the right
The mouse is controlling the Angle of view
The 1 and 2 keys are used to change the texture of the model

## 3. Anything else which will help us to understand how your prototype works. 

Core component
Shadow Map：Shadow mapping is a technique used to generate shadows in 3D rendering by rendering the scene from the perspective of the light source to a depth map (i.e., shadow map), and then using this depth map during the actual rendering process to determine whether points on the geometry are directly hit by light, and thus whether those points are in shadow.The shadow mapping part of the code is concentrated in the SceneBasic_Uniform::initScene and SceneBasic_Uniform::renderShadow functions. 

In the SceneBasic_Uniform::initScene function, there is code to create and configure shadow depth textures (shadowDepthTex) and shadow frame buffer objects (shadowFbo).This code sets various parameters for the depth texture, such as the filtering and wrapping mode, and binds the texture to the frame buffer as its depth attachment.

In the SceneBasic_Uniform::renderShadow function, polygon offset is set to prevent depth conflict (commonly known as "Z-fighting" or "shadow acne") and the scene is rendered from the light source's perspective into the shadow frame buffer. Here, the scene is rendered into a depth map by a specific shader program (which may use simplified shading to calculate only depth), which is then used to calculate and apply shadows during subsequent rendering.

Texture: The code does include sections that handle textures. These sections mainly involve the loading, creation, and application of textures, which are essential to enhance the visual effects of 3D scenes. Here is a detailed breakdown of the main parts of the texture. 

Function loadTexture: This function is responsible for loading textures from files and supports different image formats, including high dynamic range images (HDR). It uses the stb_image.h library to load image data. This function loads the image data, creates an OpenGL texture, adjusts the formatting parameters based on whether the image is HDR or not, and finally generates mipmap to optimize rendering performance.

The loadCubemap function :This function is used to load a cube map, usually used to create a sky box or environment map. This function loops through the path list of the six faces of the cube map, loading and setting texture data for each face.

In the renderScene function, different textures are used by binding them to the corresponding texture unit. This section of code sets the correct texture units for each texture when rendering the scene and binds the texture to those units, allowing the shader program to access and use the texture data.
