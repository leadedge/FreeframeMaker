# FreeframeMaker
A source code template to create FreeframeGL plugins from shader and image files.

FreeframeMaker loads shader and image files into resources and saves them with the plugin. The plugin then accesses these resources when it loads. This project does not implement the ShaderToy specification. Rather it is a basic template to use shaders of all types adhering to Freeframe specifications. The programmer is responsible fo altering the shader uniforms and plugin user control variables. Search the code for "Make changes for your shader here".

For non-programmers, there is an alternative Windows program "ShaderToySender" - http://spout.zeal.co/shadertoysender- that can be used to generate Freeframe plugins from ShaderToy shaders directly from the menu without any programming at all. It also provides for editing and testing of the shader and customisation of parameters before saving a plugin.

##How to use
This is a Windows Visual Studio 2015 C++ project. Download the project and unzip into in any folder, open the VS2015 solution file and change to "release". It should build as-is.

Copy the shader text file you require into the SHADERS folder. Copy any images that the shader will use to the TEXTURES folder. Edit "shaderfiles.h" to reference those files. Define "EFFECT_PLUGIN" for an effect that uses a texture from the host. Define the RESOLUTION_WIDTH for a source plugin to balance performance with resolution. Change the plugin Name, ID and information in ShaderMaker.cpp and rename the resulting dll. Details in "Shaderfiles.h".

##Credits
(C) Lynn Jarvis spout@zeal.co - licence LGPL3
OpenGL image loading library SOIL http://www.lonesock.net/soil.html - licence plublic domain.
FreeFrameGL version by Resolume - https://github.com/resolume/ffgl
Example shader credit in the file source.
