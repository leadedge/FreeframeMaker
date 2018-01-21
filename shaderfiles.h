//
// ============================ OPTIONS ================================
//
//
// Define "EFFECT_PLUGIN" here to build the plugin as an effect rather than a source
//
#define EFFECT_PLUGIN
//
// Optionally define the resolution width to be used by a source plugin.
// The default is the host viewport which can affect fps for some shaders.
// If the plugin is an effect this is not used.
// Only the width is needed. The height is scaled to the aspect ratio of the host viewport
#define RESOLUTION_WIDTH 640

//
// SHADER FILES
//
// Enter the name of the fragment shader file to be loaded here
//
// The shader file path is entered here but the entire file is
// embedded in resources when the program is compiled. 
// Thereafter the plugin is independent of the file itself.
// The full path is not necessary if the file is the same folder
// as the plugin source.
//

//
// IMAGE FILES
//
// Also define any images that the shader uses here.
//
// If you do not define "EFFECT_PLUGIN", the plugin will appear to the
// host as a SOURCE and the shader will use it's own images - IMAGEFILE0 to IMAGEFILE3.
//
// If you define "EFFECT_PLUGIN", IMAGEFILE0 will be ignored
// and the texture provided by the host will be used.
// Other images IMAGEFILE1 to IMAGEFILE3 can still be used by an effect shader
// =============================================================================

//
// EFFECT SHADERS (EFFECT_PLUGIN is defined)
//
// Adapted from a Resolume freeframe example
// https://github.com/resolume/ffgl
// The inputColour alpha control is not used in this shader
#define SHADERFILE "SHADERS\\EFFECT\\Add-Subtract.txt" 

// Effect shaders can also use images.
// However the first image IMAGEFILE0 is reserved for the host.
// If you do not define EFFECT_PLUGIN above, the shader becomes a source
// and the image for IMAGEFILE0 that you enter here is used by the shader.
//
// These can be used as an effect if EFFECT_PLUGIN is defined
// or as a source using the specified image if it is not defined.
//
// #define SHADERFILE "SHADERS\\EFFECT\\Thermal-Imaging.txt"
// #define IMAGEFILE0 "TEXTURES\\penguins.jpg"

// Use the Red control to alter the filter radiius
// #define SHADERFILE "SHADERS\\EFFECT\\Kuwahara-Filter.txt"
// #define IMAGEFILE0 "TEXTURES\\penguins.jpg"

//
// SOURCE SHADERS (EFFECT_PLUGIN is not defined)
//

//
// Source shaders without images
//
// #define SHADERFILE "SHADERS\\SOURCE\\Voronoi-Distances.txt"

//
// Source shaders with images
//
// Define the images used by the shader IMAGEFILE0 to IMAGEFILE3
// 

// flakes
// #define SHADERFILE "SHADERS\\SOURCE\\flakes.txt"
// #define IMAGEFILE0 "TEXTURES\\tex03.jpg"

// Mystery Mountains
// Resolution must be set low or this is slow
// #define SHADERFILE "SHADERS\\SOURCE\\Mystery-Mountains.txt"
// #define IMAGEFILE0 "TEXTURES\\tex03.jpg"







