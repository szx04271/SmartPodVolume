#pragma once
#include "utils.h"

// CDeviceInfoListCtrl

class CDeviceInfoListCtrl : public CListCtrl
{
	DECLARE_DYNAMIC(CDeviceInfoListCtrl)

public:
	CDeviceInfoListCtrl();
	virtual ~CDeviceInfoListCtrl();

	void SetDeviceInfo(const utils::MmDeviceInfo& info) {
		SetItemText(0, 1, info.friendlyName.c_str());
		SetItemText(1, 1, info.description.c_str());
		SetItemText(2, 1, info.id.c_str());

		SetColumnWidth(1, LVSCW_AUTOSIZE);
	}

protected:
	DECLARE_MESSAGE_MAP()
	virtual void PreSubclassWindow();
};


