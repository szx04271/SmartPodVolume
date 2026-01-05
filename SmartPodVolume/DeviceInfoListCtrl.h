#pragma once


// CDeviceInfoListCtrl

class CDeviceInfoListCtrl : public CListCtrl
{
	DECLARE_DYNAMIC(CDeviceInfoListCtrl)

public:
	CDeviceInfoListCtrl();
	virtual ~CDeviceInfoListCtrl();

protected:
	DECLARE_MESSAGE_MAP()
	virtual void PreSubclassWindow();
};


