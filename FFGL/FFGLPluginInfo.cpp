//
// Copyright (c) 2004 - InfoMus Lab - DIST - University of Genova
//
// InfoMus Lab (Laboratorio di Informatica Musicale)
// DIST - University of Genova 
//
// http://www.infomus.dist.unige.it
// news://infomus.dist.unige.it
// mailto:staff@infomus.dist.unige.it
//
// Developer: Gualtiero Volpe
// mailto:volpe@infomus.dist.unige.it
//
// Last modified: 2004-11-10
//
// LJ Mod to original FFGL April 2015
// void SetPluginExtendedInfo(PluginExtendedInfoStruct *info)
// LJ Mod to original FFGL August 2015
// void SetPluginInfo(PluginInfoStruct *info)

#include "FFGLPluginInfo.h"

#include <stdlib.h> 
#include <memory.h>

extern CFFGLPluginInfo* g_CurrPluginInfo;

	
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CFFGLPluginInfo constructor and destructor
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CFFGLPluginInfo::CFFGLPluginInfo(
		FPCREATEINSTANCEGL* pCreateInstance,
		const char* pchUniqueID,
		const char* pchPluginName,
		unsigned int dwAPIMajorVersion,
		unsigned int dwAPIMinorVersion,
		unsigned int dwPluginMajorVersion,
		unsigned int dwPluginMinorVersion,
		unsigned int dwPluginType,
		const char* pchDescription,
		const char* pchAbout,
		unsigned int dwFreeFrameExtendedDataSize,
		const void* pFreeFrameExtendedDataBlock
	)
{
	m_pCreateInstance = pCreateInstance;

	// Filling PluginInfoStruct
	m_PluginInfo.APIMajorVersion = dwAPIMajorVersion;
	m_PluginInfo.APIMinorVersion = dwAPIMinorVersion;
	
	bool bEndFound = false;
	for (int i = 0; (i < 16) && (!bEndFound); ++i) {
		if (pchPluginName[i] == 0) bEndFound = true;
		(m_PluginInfo.PluginName)[i] = (bEndFound) ?  0 : pchPluginName[i];
	}

	bEndFound = false;
	for (int j = 0; (j < 4) && (!bEndFound); ++j) {
		if (pchUniqueID[j] == 0) bEndFound = true;
		(m_PluginInfo.PluginUniqueID)[j] = (bEndFound) ?  0 : pchUniqueID[j];
	}

	m_PluginInfo.PluginType = dwPluginType;

	// Filling PluginExtendedInfoStruct
	m_PluginExtendedInfo.About = _strdup(pchAbout); // LJ
	m_PluginExtendedInfo.Description = _strdup(pchDescription); // LJ
	m_PluginExtendedInfo.PluginMajorVersion = dwPluginMajorVersion;
	m_PluginExtendedInfo.PluginMinorVersion = dwPluginMinorVersion;
	if ((dwFreeFrameExtendedDataSize > 0) && (pFreeFrameExtendedDataBlock != NULL)) {
		memcpy(m_PluginExtendedInfo.FreeFrameExtendedDataBlock, pFreeFrameExtendedDataBlock, dwFreeFrameExtendedDataSize);
		m_PluginExtendedInfo.FreeFrameExtendedDataSize = dwFreeFrameExtendedDataSize;
	}
	else {
		m_PluginExtendedInfo.FreeFrameExtendedDataBlock = NULL;
		m_PluginExtendedInfo.FreeFrameExtendedDataSize = 0;
	}

	g_CurrPluginInfo = this;
}

CFFGLPluginInfo::~CFFGLPluginInfo()
{
	free(m_PluginExtendedInfo.About);
	free(m_PluginExtendedInfo.Description);
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CFFGLPluginInfo methods
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

const PluginInfoStruct* CFFGLPluginInfo::GetPluginInfo() const
{
	return &m_PluginInfo;
}

const PluginExtendedInfoStruct* CFFGLPluginInfo::GetPluginExtendedInfo() const
{
	return &m_PluginExtendedInfo;
}

FPCREATEINSTANCEGL* CFFGLPluginInfo::GetFactoryMethod() const
{
	return m_pCreateInstance;
}



// LJ - added to change extended info
void CFFGLPluginInfo::SetPluginExtendedInfo(PluginExtendedInfoStruct *info)
{
	free(m_PluginExtendedInfo.Description);
	m_PluginExtendedInfo.Description = _strdup(info->Description);

	free(m_PluginExtendedInfo.About);
	m_PluginExtendedInfo.About = _strdup(info->About);

}


// LJ - added to change info
void CFFGLPluginInfo::SetPluginInfo(PluginInfoStruct *info)
{
	// PluginUniqueID[4]; // 4 chars uniqueID - not null terminated
	// Use the same decection of null for line end
	bool bEndFound = false;
	for (int j = 0; (j < 4) && (!bEndFound); ++j) {
		if (info->PluginUniqueID[j] == 0) bEndFound = true;
		(m_PluginInfo.PluginUniqueID)[j] = (bEndFound) ? 0 : info->PluginUniqueID[j];
	}

	// PluginName[16]; // 16 chars plugin friendly name - not null terminated
	bEndFound = false;
	for (int i = 0; (i < 16) && (!bEndFound); ++i) {
		if (info->PluginName[i] == 0) bEndFound = true;
		(m_PluginInfo.PluginName)[i] = (bEndFound) ? 0 : info->PluginName[i];
	}

	// 13-12-17 - Plugin type
	m_PluginInfo.PluginType = info->PluginType;

}
