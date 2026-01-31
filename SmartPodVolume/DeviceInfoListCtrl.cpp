// DeviceInfoListCtrl.cpp: 实现文件
//

#include "pch.h"
#include "SmartPodVolume.h"
#include "DeviceInfoListCtrl.h"
#include "../CommCtrlDarkThemer/CommCtrlDarkThemer.h"
#pragma comment(lib,"CommCtrlDarkThemer.lib")


// CDeviceInfoListCtrl

IMPLEMENT_DYNAMIC(CDeviceInfoListCtrl, CListCtrl)

CDeviceInfoListCtrl::CDeviceInfoListCtrl()
{

}

CDeviceInfoListCtrl::~CDeviceInfoListCtrl()
{
}


BEGIN_MESSAGE_MAP(CDeviceInfoListCtrl, CListCtrl)
END_MESSAGE_MAP()



// CDeviceInfoListCtrl 消息处理程序



void CDeviceInfoListCtrl::PreSubclassWindow() {
	//::SetWindowTheme(GetSafeHwnd(), L"Explorer", nullptr);
	DarkThemer_SafeSetWindowTheme(GetSafeHwnd(), L"Explorer");
	SetExtendedStyle(GetExtendedStyle() | LVS_EX_FULLROWSELECT);

	CRect rc; GetClientRect(&rc);
	InsertColumn(0, L"属性项", LVCFMT_LEFT, rc.Width() * 1 / 3.5);
	InsertColumn(1, L"属性值", LVCFMT_LEFT, rc.Width() * 2.5 / 3.5);
	//LVCOLUMNW col{};
	//col.mask = LVCF_FMT;
	//col.fmt = LVCFMT_FIXED_WIDTH; // 这属性在插入列时设置不管用，只有插入之后再设管用，莫名其妙
	//SetColumn(0, &col);
	//SetColumn(1, &col);

	// 后来发现可以隐藏header，我操，前面白搞了

	InsertItem(0, L"友好名称");
	InsertItem(1, L"描述");
	InsertItem(2, L"MM设备ID");

	CListCtrl::PreSubclassWindow();
}
