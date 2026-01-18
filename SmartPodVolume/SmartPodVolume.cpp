
// SmartPodVolume.cpp: 定义应用程序的类行为�?
//

#include "pch.h"
#include "framework.h"
#include "SmartPodVolume.h"
#include "SmartPodVolumeDlg.h"
#include "utils.h"
#include "constants.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CSmartPodVolumeApp

BEGIN_MESSAGE_MAP(CSmartPodVolumeApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CSmartPodVolumeApp 构�?

CSmartPodVolumeApp::CSmartPodVolumeApp()
{
	// TODO: 在此处添加构造代码，
	// 将所有重要的初始化放置在 InitInstance �?
}


// 唯一�?CSmartPodVolumeApp 对象

CSmartPodVolumeApp theApp;


// CSmartPodVolumeApp 初始�?

BOOL CSmartPodVolumeApp::InitInstance()
{
	// 如果应用程序存在以下情况，Windows XP 上需�?InitCommonControlsEx()
	// 使用 ComCtl32.dll 版本 6 或更高版本来启用可视化方式，
	//则需�?InitCommonControlsEx()�? 否则，将无法创建窗口�?
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// 将它设置为包括所有要在应用程序中使用�?
	// 公共控件类�?
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();

	utils::SetWorkingDirToExeDir();

	m_logger = spdlog::rotating_logger_mt("0", "logs/smart_pod_volume.log", 1048576ui64 * 5, 3);
	spdlog::set_default_logger(m_logger);
#ifdef _DEBUG
	spdlog::set_level(spdlog::level::debug);
	spdlog::flush_on(spdlog::level::debug);
#else
	spdlog::flush_on(spdlog::level::warn);
#endif
	spdlog::info(L"SmartPodVolume started.");

	// limit to 1 instance
	m_instanceMutex = CreateMutexW(nullptr, TRUE, INSTANCE_MUTEX_NAME);
	if (!m_instanceMutex) {
		spdlog::error(L"Error creating instance mutex (lasterror={}). Exiting.", GetLastError());
		::MessageBoxW(nullptr, L"创建互斥体失败，程序将退出。", L"SmartPodVolume 错误", MB_ICONERROR);

		spdlog::info(L"SmartPodVolume ended.");
		return FALSE;
	}
	if (GetLastError() == ERROR_ALREADY_EXISTS) {
		CloseHandle(m_instanceMutex);

		spdlog::warn("Attempting to launch multiple instances of the program. Exiting.");
		::MessageBoxW(nullptr, L"SmartPodVolume 后台进程已存在，请勿重复启动。", L"提示", MB_ICONASTERISK);

		spdlog::info(L"SmartPodVolume ended.");
		return FALSE;
	}

	HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
	if (FAILED(hr)) {
		spdlog::error(L"CoInitializeEx failed: hr=0x{:08X}. Exiting.", hr);
		::MessageBoxW(nullptr, L"初始化 COM 库失败，程序将退出。", L"SmartPodVolume 错误", MB_ICONERROR);

		CloseHandle(m_instanceMutex);
		spdlog::info(L"SmartPodVolume ended.");
		return FALSE;
	}

	CSmartPodVolumeDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	/*
	if (nResponse == IDOK)
	{
		// TO*DO: 在此放置处理何时�?
		//  “确定”来关闭对话框的代码
	}
	else if (nResponse == IDCANCEL)
	{
		// TO*DO: 在此放置处理何时�?
		//  “取消”来关闭对话框的代码
	}
	else*/ if (nResponse == -1)
	{
		TRACE(traceAppMsg, 0, "警告: 对话框创建失败，应用程序将意外终止。\n");
		spdlog::error(L"Dialog creation failed, the application will unexpectedly terminate.");
	}

cleanup:

#if !defined(_AFXDLL) && !defined(_AFX_NO_MFC_CONTROLS_IN_DIALOGS)
	ControlBarCleanUp();
#endif

	if (SUCCEEDED(hr)) {
		CoUninitialize();
	}

	CloseHandle(m_instanceMutex);
	spdlog::info(L"SmartPodVolume ended.");
	// 由于对话框已关闭，所以将返回 FALSE 以便退出应用程序，
	//  而不是启动应用程序的消息泵�?
	return FALSE;
}

