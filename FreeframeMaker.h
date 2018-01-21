//
//		FreeframeMaker.h
//
//		Version 1.001 using resources
//
//		------------------------------------------------------------
//		Copyright (C) 2018. Lynn Jarvis, Leading Edge. Pty. Ltd.
//		Based on ShaderMaker https://github.com/leadedge/ShaderMaker
//		and ShaderToySender 
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
#pragma once
#ifndef FreeframeMaker_H
#define FreeframeMaker_H

#include <FFGL.h> // windows : msvc project needs the FFGL folder in its include path
#include <FFGLLib.h>
#include <FFGLShader.h>
#include "FFGLPluginSDK.h"
#include "ffgl/utilities/utilities.h" // for STRINGIFY
#include <string>
#include <time.h> // for date
#include <Shlwapi.h> // for PathStripPath

//For resource loading
#include "resource.h" // See shaderfiles.h and resource.rc
#include "SOIL_EXT\soil.h" // SOIL_EXT - Simple OpenGL Image Library - http://www.lonesock.net/soil.html

#pragma comment(lib, "Shlwapi") // for PathStripPath

// To get the dll hModule since there is no main
#ifndef _delayimp_h
extern "C" IMAGE_DOS_HEADER __ImageBase;
#endif

#if (!(defined(WIN32) || defined(_WIN32) || defined(__WIN32__)))
// posix
typedef uint8_t  CHAR;
typedef uint16_t WORD;
typedef uint32_t DWORD;
typedef int8_t  BYTE;
typedef int16_t SHORT;
typedef int32_t LONG;
typedef LONG INT;
typedef INT BOOL;
typedef int64_t __int64; 
typedef int64_t LARGE_INTEGER;
#include <ctime>
#include <chrono> // c++11 timer
#endif

class FreeframeMaker : public CFreeFrameGLPlugin
{

public:

	FreeframeMaker();
	~FreeframeMaker();

	///////////////////////////////////////////////////
	// FreeFrameGL plugin methods
	///////////////////////////////////////////////////
    FFResult SetFloatParameter(unsigned int index, float value);
    float GetFloatParameter(unsigned int index);
	FFResult ProcessOpenGL(ProcessOpenGLStruct* pGL);
	FFResult InitGL(const FFGLViewportStruct *vp);
	FFResult DeInitGL();

	///////////////////////////////////////////////////
	// Factory method
	///////////////////////////////////////////////////
	static FFResult __stdcall CreateInstance(CFreeFrameGLPlugin **ppOutInstance) {
  		*ppOutInstance = new FreeframeMaker();
		if (*ppOutInstance != NULL)
			return FF_SUCCESS;
		return FF_FAIL;
	}

protected:

	// For elapsed time
	double elapsedTime, lastTime;
#if (defined(WIN32) || defined(_WIN32) || defined(__WIN32__))
	// windows
	double PCFreq;
	__int64 CounterStart;
#else
	// posix c++11
	std::chrono::steady_clock::time_point start;
	std::chrono::steady_clock::time_point end;
#endif

	//
	// Shader uniforms etc.
	//
	FFGLShader m_shader;
	GLint m_inputTextureLocation0; // This will be common for all effect shaders
	GLint m_inputTextureLocation1; // extra images iChannel1-3
	GLint m_inputTextureLocation2;
	GLint m_inputTextureLocation3;
	GLint m_timeLocation; // Time for animation speed
	GLint m_resolutionLocation; // Current viewport dimensions - (resolution)

	// ===============================================================
	// Make changes for your shader here
	// Colour uniform (vec4)
	GLint m_inputColourLocation;
	// FFGL user parameters
	float m_UserSpeed;
	float m_UserRed;
	float m_UserGreen;
	float m_UserBlue;
	float m_UserAlpha;
	// ===============================================================

	// Variables
	float m_time;
	float m_vpWidth;
	float m_vpHeight;
	float m_Scale;

	// Flags
	bool bStarted;
	bool bResolume;
	bool bInitialized;

	// Textures
	GLuint m_glTexture;
	GLuint m_glTexture0;
	GLuint m_glTexture1;
	GLuint m_glTexture2;
	GLuint m_glTexture3;
	GLuint m_fbo;

	// Counter for time
	void StartCounter();
	double GetCounter();

	// Functions for resource loading
	HMODULE m_hModule; // A handle to the dll module
	char m_HostName[MAX_PATH];
	std::string m_vertexShaderCode;
	std::string m_fragmentShaderCode;
	HMODULE GetCurrentModule();
	bool LoadShaderResource(std::string &shaderString);
	void CleanShaderString(std::string &shaderString);
	bool LoadResourceImage(GLuint ResourceID, GLuint &TextureID);
	bool LoadShader(std::string shaderString);
	bool DrawToTexture(GLuint TextureID, GLuint TextureTarget, GLuint HostFBO);
	void DrawTexture();
	void CreateGLtexture(GLuint &texID, unsigned int width, unsigned int height, void *data);


};


#endif
