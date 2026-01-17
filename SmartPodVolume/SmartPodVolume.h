
// SmartPodVolume.h: PROJECT_NAME 应用程序的主头文�?
//

#pragma once

#ifndef __AFXWIN_H__
	#error "在包含此文件之前包含 'pch.h' 以生�?PCH"
#endif

#include "resource.h"		// 主符�?


// CSmartPodVolumeApp:
// 有关此类的实现，请参�?SmartPodVolume.cpp
//

class CSmartPodVolumeApp : public CWinApp
{
private:
	std::shared_ptr<spdlog::logger> m_logger;
	HANDLE m_instanceMutex = nullptr;

public:
	CSmartPodVolumeApp();

// 重写
public:
	virtual BOOL InitInstance();

// 实现

	DECLARE_MESSAGE_MAP()
};

extern CSmartPodVolumeApp theApp;
