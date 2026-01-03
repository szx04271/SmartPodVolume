#pragma once

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN            // �?Windows 头中排除极少使用的资�?
#endif

#include "targetver.h"

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // 某些 CString 构造函数将是显式的
#define _AFX_NO_MFC_CONTROLS_IN_DIALOGS         // 移除对话框中�?MFC 控件的支�?

// 关闭 MFC 的一些常见且经常可放心忽略的隐藏警告消息
#define _AFX_ALL_WARNINGS

#define SPDLOG_WCHAR_TO_UTF8_SUPPORT 1

#include <afxwin.h>         // MFC 核心组件和标准组�?
#include <afxext.h>         // MFC 扩展
#include "spdlog/spdlog.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include <Dbt.h>
#include <rpc.h>
#include <SetupAPI.h>
#include <string>
#include <mmdeviceapi.h>
#include <endpointvolume.h>
#include <functiondiscoverykeys_devpkey.h>
#include <cfgmgr32.h> // 用于 CM_ 函授，遍历设备树
#include <vector>
#include <algorithm>
#include "nlohmann/json.hpp"

#pragma comment(lib,"Rpcrt4.lib")
#pragma comment(lib,"setupapi.lib")
#pragma comment(lib, "Mmdevapi.lib")
#pragma comment(lib, "Cfgmgr32.lib")

#ifndef _AFX_NO_OLE_SUPPORT
#include <afxdtctl.h>           // MFC �?Internet Explorer 4 公共控件的支�?
#endif
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>             // MFC �?Windows 公共控件的支�?
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <afxcontrolbars.h>     // MFC 支持功能区和控制�?









#ifdef _UNICODE
#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
#endif


