#pragma once

#define WIN32_LEAN_AND_MEAN             // 从 Windows 头文件中排除极少使用的内容
#define CCDT_EXPORT
// Windows 头文件
#include <windows.h>
#include <Uxtheme.h>
#include <dwmapi.h>
#include <set>
#include <memory>
#include <map>
#include <Vssym32.h>
#include <string>
#include <atltrace.h>

#pragma comment(lib,"uxtheme.lib")
#pragma comment(lib,"dwmapi.lib")
#pragma comment(lib,"comctl32.lib")