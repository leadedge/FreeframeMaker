//
//		FreeframeMaker.cpp
//
//
//		A source code template that can be used to compile FreeframeGL plugins 
//		from source or from shader files and include images for the plugin to use.
//
//		------------------------------------------------------------
//		Revisions :
//		20.01-18	- Uses the SOIL library - http://www.lonesock.net/soil.html
//					- Uses Resolume FFGL - https://github.com/resolume/ffgl
//		21.01.18	- added source shader resolution control
//					  Version 1.001
//						
//		------------------------------------------------------------
//
//		Copyright (C) 2018. Lynn Jarvis, Leading Edge. Pty. Ltd.
//
//		This program is free software: you can redistribute it and/or modify
//		it under the terms of the GNU Lesser General Public License as published by
//		the Free Software Foundation, either version 3 of the License, or
//		(at your option) any later version.
//
//		This program is distributed in the hope that it will be useful,
//		but WITHOUT ANY WARRANTY; without even the implied warranty of
//		MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//		GNU Lesser General Public License for more details.
//
//		You will receive a copy of the GNU Lesser General Public License along 
//		with this program.  If not, see http://www.gnu.org/licenses/.
//		--------------------------------------------------------------
//
//
#include "FreeframeMaker.h"

#if (defined(WIN32) || defined(_WIN32) || defined(__WIN32__))
int (*cross_secure_sprintf)(char *, size_t, const char *,...) = sprintf_s;
#else 
// posix
int (*cross_secure_sprintf)(char *, size_t, const char *, ...) = snprintf;
#endif


//
// Parameter definitions
//
// ===============================================================
// Make changes for your shader here
// This is an example using "inputColour"
#define FFPARAM_SPEED (0)
#define FFPARAM_RED   (1)
#define FFPARAM_GREEN (2)
#define FFPARAM_BLUE  (3)
#define FFPARAM_ALPHA (4)
// ===============================================================


// =====================================================================
// Shader fragment code is loaded from a file specified in shaderfiles.h
// Optional defines and images are also entered in that file.
// ======================================================================


// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// ++++++++++++ IMPORTANT : DEFINE YOUR PLUGIN INFORMATION HERE ++++++++++++
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
static CFFGLPluginInfo PluginInfo(
	FreeframeMaker::CreateInstance,		// Create method
	"ZZZZ",								// *** Plugin unique ID (4 chars) - this must be unique for each plugin
	"FreeframeMaker",					// *** Plugin name - make it different for each plugin 
	1,						   			// API major version number 													
	006,								// API minor version number	
	1,									// *** Plugin major version number
	001,								// *** Plugin minor version number
#ifdef EFFECT_PLUGIN
	FF_EFFECT,							// Plugin is an effect
#else
	FF_SOURCE,							// FF_SOURCE for shaders that do not use a host texture
#endif
	"FreeframeMaker - Version 1.001\nFreeframe plugin template", // *** Plugin description - you can expand on this
	"(C) Lynn Jarvis (spout.zeal.co)." // *** About - add your own name and details but please retain the copyright
);

// Common vertex shader code as per FreeFrame examples
static const std::string vertexShaderCode = STRINGIFY(
	void main()
{
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
	gl_TexCoord[0] = gl_MultiTexCoord0;
	gl_FrontColor = gl_Color;
}
);

////////////////////////////////////////////////////////////////////////////////////////////////////
//  Constructor and destructor
////////////////////////////////////////////////////////////////////////////////////////////////////
FreeframeMaker::FreeframeMaker():CFreeFrameGLPlugin()
{
	// Debug console window so printf works
	FILE* pCout; // should really be freed on exit 
	AllocConsole();
	freopen_s(&pCout, "CONOUT$", "w", stdout); 
	printf("FreeframeMaker Vers 1.001\n");

	// Input properties allow for up to two textures for an effect
	// For Resolume, only one of these is used unless the plugin is a mix plugin
	// Here we limit an to an effect by using only one texture.
	#ifdef EFFECT_PLUGIN
	SetMinInputs(1); // Resolume requires a minimum of 1 for an effect
	SetMaxInputs(1); // Second input texture not used
	#else
	SetMinInputs(0); // No texture inputs for a source
	SetMaxInputs(0);
	#endif

	//
	// Parameters
	//
	// ===============================================================
	// Make changes for your shader here
	SetParamInfo(FFPARAM_SPEED, "Speed", FF_TYPE_STANDARD, 0.5f); m_UserSpeed = 0.5f;
	SetParamInfo(FFPARAM_RED,   "Red",   FF_TYPE_STANDARD, 0.5f); m_UserRed   = 0.5f;
	SetParamInfo(FFPARAM_GREEN, "Green", FF_TYPE_STANDARD, 0.5f); m_UserGreen = 0.5f;
	SetParamInfo(FFPARAM_BLUE,  "Blue",  FF_TYPE_STANDARD, 0.5f); m_UserBlue  = 0.5f;
	SetParamInfo(FFPARAM_ALPHA, "Alpha", FF_TYPE_STANDARD, 1.0f); m_UserAlpha = 1.0f;
	// ===============================================================


}

////////////////////////////////////////////////////////////////////////////////////////////////////
//  Methods
////////////////////////////////////////////////////////////////////////////////////////////////////
FFResult FreeframeMaker::InitGL(const FFGLViewportStruct *vp)
{
	// Initialize flags
	bStarted = false;
	bResolume = false;
	bInitialized = false;

	// Find the host executable name
	HMODULE hModule = GetModuleHandle(NULL);
	GetModuleFileNameA(hModule, m_HostName, MAX_PATH);
	PathStripPathA(m_HostName);
	PathRemoveExtensionA(m_HostName);

	// Set a started flag false so that the source plugin thumbnail
	// is flipped for Resolume Avenue 32 bit
	bStarted = false;

	// Resolume displays a source plugin inverted so flip is always needed
	bResolume = false;
	if (strstr(m_HostName, "Avenue") != 0 || strstr(m_HostName, "Arena") != 0)
		bResolume = true;

	// printf("Host [%s] bResolume = %d\n", m_HostName, bResolume);

	// Set the viewport variables
	m_vpWidth = (float)vp->width;
	m_vpHeight = (float)vp->height;
	m_Scale = 1.0;

	//
	// Scaling for a source plugin
	//
#ifndef EFFECT_PLUGIN
	float resolutionwidth = m_vpWidth;
	// Set the source shader resolution width if it has been defined
#ifdef RESOLUTION_WIDTH
	resolutionwidth = RESOLUTION_WIDTH;
#endif
	// Set the scale factors relative to the viewport width if it is downscaled
	if (resolutionwidth < m_vpWidth)
		m_Scale = resolutionwidth / m_vpWidth;
#endif // not an effect plugin

	// Create an fbo for rendering to a texture
	glGenFramebuffersEXT(1, &m_fbo);

	// Create an empty render texture the size of the scaled viewport
	CreateGLtexture(m_glTexture, (unsigned int)(m_vpWidth*m_Scale), (unsigned int)(m_vpHeight*m_Scale), NULL);

	// A handle to the dll module is needed to load files and images from resources
	m_hModule = GetCurrentModule();

	//
	// Load the fragment shader file from resources
	// The file path is entered in "shaderfiles.h"
	// Images used by the shader are also entered there
	//
	std::string shaderstring;
	if (LoadShaderResource(shaderstring)) {
		// Initialize the shader
		if (LoadShader(shaderstring)) {
			// Images can be loaded for both source and effect plugins
			// but for an effect, the first one is reserved for the host
			// If images are not defined, the load is ignored

			// Initialize the texture ID's to zero for checks
			m_glTexture0 = 0;
			m_glTexture1 = 0;
			m_glTexture2 = 0;
			m_glTexture3 = 0;

			// Load images if they have been defined
#ifndef EFFECT_PLUGIN
			LoadResourceImage(IDC_IMAGEFILE0, m_glTexture0);
#endif
			LoadResourceImage(IDC_IMAGEFILE1, m_glTexture1);
			LoadResourceImage(IDC_IMAGEFILE2, m_glTexture2);
			LoadResourceImage(IDC_IMAGEFILE3, m_glTexture3);
			bInitialized = true;
		}

	}

	return FF_SUCCESS;
}

FreeframeMaker::~FreeframeMaker()
{

}


FFResult FreeframeMaker::DeInitGL()
{
	if (bInitialized)
		m_shader.FreeGLResources();

	// Image textures
	if (m_glTexture) glDeleteTextures(1, &m_glTexture);
	if(m_glTexture0) glDeleteTextures(1, &m_glTexture0);
	if(m_glTexture1) glDeleteTextures(1, &m_glTexture1);
	if(m_glTexture2) glDeleteTextures(1, &m_glTexture2);
	if(m_glTexture3) glDeleteTextures(1, &m_glTexture3);
	m_glTexture = 0;
	m_glTexture0 = 0;
	m_glTexture1 = 0;
	m_glTexture2 = 0;
	m_glTexture3 = 0;

	bInitialized = false;

	return FF_SUCCESS;
}

FFResult FreeframeMaker::ProcessOpenGL(ProcessOpenGLStruct *pGL)
{
	if (bInitialized) {

		GLuint textureID = 0; // The first texture
		FFGLTexCoords maxCoords;
		maxCoords.s = 1.0;
		maxCoords.t = 1.0;
		textureID = m_glTexture0; // assuming a source plugin

		// Default viewport size
		float vpWidth  = m_vpWidth;
		float vpHeight = m_vpHeight;

		// Scale the viewport to reduce the size for complex source shaders
		vpWidth  = m_vpWidth*m_Scale;
		vpHeight = m_vpHeight*m_Scale;
		glViewport(0, 0, (GLsizei)vpWidth, (GLsizei)vpHeight);

		// Calculate elapsed time
		lastTime = elapsedTime;
		elapsedTime = GetCounter() / 1000.0; // In seconds - higher resolution than timeGetTime()
		m_time = m_time + (float)(elapsedTime - lastTime)*m_UserSpeed*2.0f; // increment scaled by user input 0.0 - 2.0

		// Get the incoming host texture for an effect
#ifdef EFFECT_PLUGIN
		if (pGL->numInputTextures<1)
			return FF_FAIL;
		if (pGL->inputTextures[0] == NULL)
			return FF_FAIL;
		FFGLTextureStruct &Texture = *(pGL->inputTextures[0]);
		textureID = Texture.Handle;
		// get the max s,t that correspond to the 
		// width,height of the used portion of the allocated texture space
		maxCoords = GetMaxGLTexCoords(Texture);
#endif

		// activate our shader
		m_shader.BindShader();

		// resolution (viewport size)
		if (m_resolutionLocation >= 0)
			glUniform2fARB(m_resolutionLocation, vpWidth, vpHeight);

		// ===============================================================
		// Make changes for your shader here
		//
		// Elapsed time
		if (m_timeLocation >= 0)
			glUniform1fARB(m_timeLocation, m_time);

		// Input colour is linked to the user controls Red, Green, Blue, Alpha
		if (m_inputColourLocation >= 0)
			glUniform4fARB(m_inputColourLocation, m_UserRed, m_UserGreen, m_UserBlue, m_UserAlpha);
		// ===============================================================


		// First input texture (textureID)
		// The shader will use the first texture bound to GL texture unit 0
		if (m_inputTextureLocation0 >= 0 && textureID > 0)
			glUniform1iARB(m_inputTextureLocation0, 0);

		// Second input texture
		// The shader will use the texture bound to GL texture unit 1 etc.
		if (m_inputTextureLocation1 >= 0 && m_glTexture1 > 0)
			glUniform1iARB(m_inputTextureLocation1, 1);

		// Third
		if (m_inputTextureLocation2 >= 0 && m_glTexture2 > 0)
			glUniform1iARB(m_inputTextureLocation2, 2);

		// Fourth
		if (m_inputTextureLocation3 >= 0 && m_glTexture3 > 0)
			glUniform1iARB(m_inputTextureLocation3, 3);

		// activate texture units and bind the input textures
		// Bind a texture if the shader needs one
		if (m_inputTextureLocation0 >= 0 && m_glTexture0 > 0) {
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, m_glTexture0);
		}

		// If there is a second texture, bind it to texture unit 1
		if (m_inputTextureLocation1 >= 0 && m_glTexture1 > 0) {
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, m_glTexture1);
		}

		// Texture units 2 and 3
		if (m_inputTextureLocation2 >= 0 && m_glTexture2 > 0) {
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, m_glTexture2);
		}

		if (m_inputTextureLocation3 >= 0 && m_glTexture3 > 0) {
			glActiveTexture(GL_TEXTURE3);
			glBindTexture(GL_TEXTURE_2D, m_glTexture3);
		}

		// Draw an incoming effect texture directly
#ifdef EFFECT_PLUGIN
		glBegin(GL_QUADS);
		glMultiTexCoord2f(GL_TEXTURE0, 0.0f, 0.0f); // lower left texture coord	
		glVertex2f(-1.0, -1.0); // lower left vertex
		glMultiTexCoord2f(GL_TEXTURE0, 0.0f, (float)maxCoords.t); // upper left texture coord	
		glVertex2f(-1.0, 1.0); // upper left vertex
		glMultiTexCoord2f(GL_TEXTURE0, (float)maxCoords.s, (float)maxCoords.t); // upper right texture coord	
		glVertex2f(1.0, 1.0); // upper right
		glMultiTexCoord2f(GL_TEXTURE0, (float)maxCoords.s, 0.0f); // lower right texture coord	
		glVertex2f(1.0, -1.0); // lower right
		glEnd();
#else
		// Draw the output of a source shader into a texture via FBO
		// so that the shader resolution is independent of the size of the host viewport
		DrawToTexture(m_glTexture, GL_TEXTURE_2D, pGL->HostFBO);
#endif

		// unbind input texture 3
		if (m_inputTextureLocation3 >= 0 && m_glTexture3 > 0) {
			glActiveTexture(GL_TEXTURE3);
			glBindTexture(GL_TEXTURE_2D, 0);
		}

		// unbind input texture 2
		if (m_inputTextureLocation2 >= 0 && m_glTexture2 > 0) {
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, 0);
		}

		// unbind input texture 1
		if (m_inputTextureLocation1 >= 0 && m_glTexture1 > 0) {
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, 0);
		}
		
		// unbind input texture 0 (textureID)
		glActiveTexture(GL_TEXTURE0); // set default active texture
		if (m_inputTextureLocation0 >= 0 && textureID > 0) {
			glBindTexture(GL_TEXTURE_2D, 0);
		}

		// unbind the shader
		m_shader.UnbindShader();

		// reset the viewport 
		glViewport(0, 0, (GLsizei)m_vpWidth, (GLsizei)m_vpHeight);

#ifndef EFFECT_PLUGIN
		// Draw the result of a source shader
		DrawTexture();
#endif

		bStarted = true; // It has started

	} // endif bInitialized

	return FF_SUCCESS;
}



// ===============================================================
// Make changes for your shader here
// ===============================================================
float FreeframeMaker::GetFloatParameter(unsigned int index)
{
	float retValue = 0.0;

	switch (index)	{

		case FFPARAM_SPEED:
			return  m_UserSpeed;
		case FFPARAM_RED:
			retValue = m_UserRed;
			return retValue;
		case FFPARAM_GREEN:
			retValue = m_UserGreen;
			return retValue;
		case FFPARAM_BLUE:
			retValue = m_UserBlue;
			return retValue;
		default:
			return retValue;
	}

}

// ===============================================================
// Make changes for your shader here
// ===============================================================
FFResult FreeframeMaker::SetFloatParameter(unsigned int index, float value)
{
	switch (index)	{

		case FFPARAM_SPEED:
			m_UserSpeed = value;
			break;
		case FFPARAM_RED:
			m_UserRed = value;
			break;
		case FFPARAM_GREEN:
			m_UserGreen = value;
			break;
		case FFPARAM_BLUE:
			m_UserBlue = value;
			break;
		case FFPARAM_ALPHA:
			m_UserAlpha = value;
			break;
		default:
			return FF_FAIL;
	}

	return FF_SUCCESS;

}

bool FreeframeMaker::LoadShader(std::string shaderString) {
		
	//
	// Set up your shader here
	//

	// printf("%s\n", shaderString.c_str());

	// Set uniform locations to -1 so that they are only used if they are found in the shader
	m_inputTextureLocation0 = -1;
	m_inputTextureLocation1 = -1;
	m_inputTextureLocation2 = -1;
	m_inputTextureLocation3 = -1;

	m_resolutionLocation    = -1;

	// ===============================================================
	// Make changes for your shader here
	// ===============================================================
	// Time is commonly used for shader animation
	// Input colour can be linked to the user controls Red, Green, Blue, Alpha
	// As per ShaderToySender, ShaderMaker etc.
	m_timeLocation = -1;
	m_inputColourLocation = -1;
	// ===============================================================

	// initialize gl shader
	if (m_shader.Compile(vertexShaderCode, shaderString.c_str()) != 0) {

		// activate our shader
		if (m_shader.BindShader() != 0) {
			
			// to assign values to parameters in the shader, we have to lookup
			// the "location" of each value.. then call one of the glUniform* methods
			// to assign a value

			// inputTexture
			// iChannel0 - shadertoy
			// tex0 - glsl sandbox
			m_inputTextureLocation0 = m_shader.FindUniform("inputTexture");
			if (m_inputTextureLocation0 < 0)
				m_inputTextureLocation0 = m_shader.FindUniform("tex0");
			if(m_inputTextureLocation0 < 0)
				m_inputTextureLocation0 = m_shader.FindUniform("iChannel0");

			// Three more textures - retain the GLSL Sandbox and ShaderToy nomenclature
			m_inputTextureLocation1 = m_shader.FindUniform("tex1");
			if (m_inputTextureLocation0 < 0)
				m_inputTextureLocation0 = m_shader.FindUniform("iChannel1");
			m_inputTextureLocation2 = m_shader.FindUniform("tex2");
			if (m_inputTextureLocation0 < 0)
				m_inputTextureLocation0 = m_shader.FindUniform("iChannel2");
			m_inputTextureLocation3 = m_shader.FindUniform("tex3");
			if (m_inputTextureLocation0 < 0)
				m_inputTextureLocation0 = m_shader.FindUniform("iChannel3");

			// resolution - GLSL Sanbox
			// iResolution - shadertoy
			m_resolutionLocation = m_shader.FindUniform("resolution");
			if (m_resolutionLocation < 0)
				m_resolutionLocation = m_shader.FindUniform("iResolution");
			
			// ===============================================================
			// Make changes for your shader here
			// ===============================================================
			// time
			// iTime
			// iGlobalTime
			m_timeLocation = m_shader.FindUniform("time");
			if (m_timeLocation < 0)
				m_timeLocation = m_shader.FindUniform("iTime");
			if (m_timeLocation < 0)
				m_timeLocation = m_shader.FindUniform("iGlobalTime");

			// inputColour
			m_inputColourLocation = m_shader.FindUniform("inputColour");;
			// ===============================================================

			m_shader.UnbindShader();

			// Start the clock
			StartCounter();

			return true;

		} // bind shader OK

	} // compile shader OK

	return false;
}

//
// SHADER DRAW INTO A TEXTURE VIA FBO
//
bool FreeframeMaker::DrawToTexture(GLuint TextureID, GLuint TextureTarget, GLuint HostFBO)
{
	GLenum status;

	if (m_fbo == 0 || m_glTexture == 0)
		return false;

	// Bind our fbo and attach the texture to it
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_fbo);
	glColor4f(1.f, 1.f, 1.f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT);
	glFramebufferTexture2DEXT(GL_READ_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, TextureID, 0);
	status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
	if (status == GL_FRAMEBUFFER_COMPLETE_EXT) {

		glEnable(GL_TEXTURE_2D);
		glBegin(GL_QUADS);
		glColor4f(1.f, 1.f, 1.f, 1.f);
		// Shader textures are already bound for draw
		glTexCoord2f(0.0, 0.0); glVertex2f(-1.0, -1.0); // bottom left
		glTexCoord2f(0.0, 1.0);	glVertex2f(-1.0, 1.0); // top left
		glTexCoord2f(1.0, 1.0);	glVertex2f(1.0, 1.0); // top right
		glTexCoord2f(1.0, 0.0);	glVertex2f(1.0, -1.0); // bottom right
		glEnd();
		glDisable(GL_TEXTURE_2D);
		glColor4f(1.f, 1.f, 1.f, 1.f);
		glEnd();
		glDisable(GL_TEXTURE_2D);

	}
	else {
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, HostFBO);
		return false;
	}

	// restore the previous fbo - default is 0
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, HostFBO);

	return true;

} // end DrawToTexture


// Draw the texture generated by drawing into the fbo
// Only used for a source plugin
void FreeframeMaker::DrawTexture()
{
	if (m_glTexture == 0)
		return;

	// Draw the texture
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, m_glTexture); // bind texture
	glBegin(GL_QUADS);
	glColor4f(1.f, 1.f, 1.f, 1.f);

	// Arena 5.1.4 and less and Avenue 4.6.4 and less (32 bit) need a flip only for the thumbnail
#ifndef _WIN64
	if (!bStarted) {
		glTexCoord2f(0.0, 1.0); // upper left texture coord	
		glVertex2f(-1.0, -1.0); // lower left
		glTexCoord2f(0.0, 0.0); // lower left texture coord
		glVertex2f(-1.0, 1.0); // upper left
		glTexCoord2f(1.0, 0.0); // lower right texture coord	
		glVertex2f(1.0, 1.0); // upper right
		glTexCoord2f(1.0, 1.0); // upper right texture coord
		glVertex2f(1.0, -1.0); // lower right
	}
	else {
		glTexCoord2f(0.0, 0.0); // lower left texture coord	
		glVertex2f(-1.0, -1.0); // lower left
		glTexCoord2f(0.0, 1.0); // upper left texture coord	
		glVertex2f(-1.0, 1.0); // upper left
		glTexCoord2f(1.0, 1.0); // upper right texture coord	
		glVertex2f(1.0, 1.0); // upper right
		glTexCoord2f(1.0, 0.0); // lower right texture coord	
		glVertex2f(1.0, -1.0); // lower right
	}
#else
	// Arena 6 and Avenue 6 (64 bit) need a flip
	if (bResolume) {
		glTexCoord2f(0.0, 1.0);	glVertex2f(-1.0, -1.0);
		glTexCoord2f(0.0, 0.0);	glVertex2f(-1.0, 1.0);
		glTexCoord2f(1.0, 0.0);	glVertex2f(1.0, 1.0);
		glTexCoord2f(1.0, 1.0);	glVertex2f(1.0, -1.0);
	}
	else {
		glTexCoord2f(0.0, 0.0);	glVertex2f(-1.0, -1.0);
		glTexCoord2f(0.0, 1.0);	glVertex2f(-1.0, 1.0);
		glTexCoord2f(1.0, 1.0);	glVertex2f(1.0, 1.0);
		glTexCoord2f(1.0, 0.0);	glVertex2f(1.0, -1.0);
	}
#endif

	glEnd();
	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_TEXTURE_2D);

}

// Create an opengl texture and fill with the data provided
void FreeframeMaker::CreateGLtexture(GLuint &texID, unsigned int width, unsigned int height, void *data)
{
	if (texID != 0)
		glDeleteTextures(1, &texID);

	glGenTextures(1, &texID);

	glBindTexture(GL_TEXTURE_2D, texID);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	glBindTexture(GL_TEXTURE_2D, 0);

}

// ========================================
// Functions for resource loading
// ========================================

//
//
// http://www.codeguru.com/cpp/w-p/dll/tips/article.php/c3635/Tip-Detecting-a-HMODULEHINSTANCE-Handle-Within-the-Module-Youre-Running-In.htm
//
HMODULE FreeframeMaker::GetCurrentModule()
{
	return reinterpret_cast<HMODULE>(&__ImageBase);
}

//
// Load a shader from resources
//
bool FreeframeMaker::LoadShaderResource(std::string &shaderString)
{
	HRSRC hResource = NULL;
	DWORD size = 0;
	HGLOBAL rcShaderData = NULL;
	const char* shaderdata = NULL;

	// Load the shader code from resources - the shader file path is defined in shaderfiles.h
	hResource = FindResource(m_hModule, MAKEINTRESOURCE(IDC_SHADERFILE), RT_RCDATA);
	if (hResource) {
		size = SizeofResource(m_hModule, hResource);
		rcShaderData = LoadResource(m_hModule, hResource);
		if (rcShaderData) {
			shaderdata = static_cast<const char*>(LockResource(rcShaderData));
			shaderString = shaderdata; // the shader file string for processing
			// It is possible that the size of the string is one character larger
			// than the size of the resource (unknown cause) - reduce the size if so
			if (shaderString.size() > size)
				shaderString.resize(size);
			CleanShaderString(shaderString);
			shaderString += ""; // ensure a NULL terminator
			return true;
		}
	}
	return false;
}

void FreeframeMaker::CleanShaderString(std::string & shaderString)
{
	std::string newString;
	char c = 10; // start with something

	// Copy character by character to remove any out-of range
	for (int i = 0; i < (int)shaderString.size(); i++) {
		c = shaderString.at(i);
		if (c == 13 || c == 10 || c == 9 || (c >= 32 && c < 128)) {
			newString += c;
		}
	}
	shaderString = newString;
}


//
// Load an image from resources
//
// Uses SOIL library
// http://www.lonesock.net/soil.html
//
// See also : https://github.com/fenbf/SOIL_ext
//
bool FreeframeMaker::LoadResourceImage(GLuint ResourceID, GLuint &TextureID)
{
	HMODULE hModule = NULL;
	DWORD size = 0;
	HRSRC hResInfo = NULL;
	HGLOBAL rcImageData = NULL;
	const unsigned char* imagedata = NULL;
	unsigned char* buffer = NULL;
	int width = 0;
	int height = 0;
	int channels = 0;

	unsigned int flags = SOIL_FLAG_TEXTURE_REPEATS | SOIL_FLAG_INVERT_Y;

	int image_number = ResourceID-IDC_IMAGEFILE0;

	// If the resource (IMAGEFILE0 - IMAGEFILE3) is not defined
	// FindResource fails and the function returns false
	// See resource.rc
	hResInfo = FindResource(m_hModule, MAKEINTRESOURCE(ResourceID), RT_RCDATA);
	if(hResInfo) {
		size = SizeofResource(m_hModule, hResInfo);
		if(size > 0) {
			rcImageData = LoadResource(m_hModule, hResInfo);
			if(rcImageData != NULL) {
				imagedata = static_cast<const unsigned char*>(LockResource(rcImageData));
				switch(image_number) {
						case 0:
							// printf("Loaded image0 - %dx%d\n", width, height);
							m_glTexture0 = SOIL_load_OGL_texture_from_memory(imagedata, (int)size, 4, 0, flags);
							break;
						case 1:
							m_glTexture1 = SOIL_load_OGL_texture_from_memory(imagedata, (int)size, 4, 0, flags);
							break;
						case 2:
							m_glTexture2 = SOIL_load_OGL_texture_from_memory(imagedata, (int)size, 4, 0, flags);
							break;
						case 3:
							m_glTexture3 = SOIL_load_OGL_texture_from_memory(imagedata, (int)size, 4, 0, flags);
							break;
						default:
							break;
				}
				return true;
			}
		}
	}

	return false;

} // end LoadResourceImage


void FreeframeMaker::StartCounter()
{

#if (defined(WIN32) || defined(_WIN32) || defined(__WIN32__))
	LARGE_INTEGER li;
	// Find frequency
	QueryPerformanceFrequency(&li);
	PCFreq = double(li.QuadPart) / 1000.0;
	// Second call needed
	QueryPerformanceCounter(&li);
	CounterStart = li.QuadPart;
#else
	// posix c++11
	start = std::chrono::steady_clock::now();
#endif

}

double FreeframeMaker::GetCounter()
{

#if (defined(WIN32) || defined(_WIN32) || defined(__WIN32__))
	LARGE_INTEGER li;
	QueryPerformanceCounter(&li);
	return double(li.QuadPart - CounterStart) / PCFreq;
#else
	// posix c++11
	end = std::chrono::steady_clock::now();
	return std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() / 1000.;
#endif
	return 0;
}

