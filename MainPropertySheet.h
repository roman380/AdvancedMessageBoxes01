////////////////////////////////////////////////////////////
// MainPropertySheet.h -
//
// Copyright (C) Alax.Info, 2006
// http://alax.info
//
// A permission to use the source code is granted as long as reference to 
// source website http://alax.info is retained.

#pragma once

#include "resource.h"

////////////////////////////////////////////////////////////
// CScrollablePropertyPageT

template <typename T, typename _PropertyPageWindow = CPropertyPageWindow>
class CScrollablePropertyPageT :
	public CPropertyPageImpl<T, _PropertyPageWindow>,
	public _DialogSplitHelper
{
	typedef CPropertyPageImpl<T, _PropertyPageWindow> CPropertyPage;
	typedef CScrollablePropertyPageT<T, _PropertyPageWindow> CScrollablePropertyPage;

public:

BEGIN_MSG_MAP_EX(CScrollablePropertyPage)
	CHAIN_MSG_MAP(CPropertyPage)
	MSG_WM_INITDIALOG(OnInitDialog)
	MSG_WM_VSCROLL(OnVScroll)
	MSG_WM_TIMER(OnTimer)
	MSG_WM_MOUSEWHEEL(OnMouseWheel)
END_MSG_MAP()

public:

	////////////////////////////////////////////////////////
	// Timer identifier

	enum
	{
		TIMER_FIRST = 0,
		TIMER_TRACKFOCUS,
		TIMER_LAST = TIMER_TRACKFOCUS,
	};

protected:
	const DLGTEMPLATEEX* m_pOriginalDialogTemplate;
	CHeapPtr<DLGTEMPLATEEX> m_pDialogTemplate;
	const DLGITEMTEMPLATEEX* m_pSeparatorDialogTemplateItem;
	LONG m_nPosition;
	CWindow m_PreviousFocusedWindow;

	static const DLGITEMTEMPLATEEX* GetFirstDialogTemplateItem(const DLGTEMPLATEEX* pDialogTemplate)
	{
		const BYTE* pnPointer = (const BYTE*) (pDialogTemplate + 1);
		CStringW sMenu;
		if(*((const WCHAR*) pnPointer) == 0xFFFF) 
		{
			//sMenu = CStringW(_StringHelper::Format(_T("#%d"), *((const WCHAR*) pnPointer + 1)));
			pnPointer += 2 * sizeof (WCHAR);
		} else
		{
			sMenu = (const WCHAR*) pnPointer;
			pnPointer += (sMenu.GetLength() + 1) * sizeof (WCHAR);
		}
		CStringW sWindowClass;
		if(*((const WCHAR*) pnPointer) == 0xFFFF) 
		{
			//sWindowClass = CStringW(_StringHelper::Format(_T("#%d"), *((const WCHAR*) pnPointer + 1)));
			pnPointer += 2 * sizeof (WCHAR);
		} else
		{
			sWindowClass = (const WCHAR*) pnPointer;
			pnPointer += (sWindowClass.GetLength() + 1) * sizeof (WCHAR);
		}
		CStringW sTitle = (const WCHAR*) pnPointer;
		pnPointer += (sTitle.GetLength() + 1) * sizeof (WCHAR);
		WORD nPointSize;
		WORD nWeight;
		BYTE nItalic;
		BYTE nCharset;
		CStringW sTypeFace;
		if(pDialogTemplate->style & (DS_SETFONT | DS_SHELLFONT))
		{
			nPointSize = *((const WORD*) pnPointer);
			pnPointer += sizeof nPointSize;
			nWeight = *((const WORD*) pnPointer);
			pnPointer += sizeof nWeight;
			nItalic = *((const BYTE*) pnPointer);
			pnPointer += sizeof nItalic;
			nCharset = *((const BYTE*) pnPointer);
			pnPointer += sizeof nCharset;
			sTypeFace = (const WCHAR*) pnPointer;
			pnPointer += (sTypeFace.GetLength() + 1) * sizeof (WCHAR);
		}
		// NOTE: Items are aligned on 32-bit boundary
		return (const DLGITEMTEMPLATEEX*) (((UINT_PTR) pnPointer + 3) & ~3);
	}
	static const DLGITEMTEMPLATEEX* GetNextDialogTemplateItem(const DLGITEMTEMPLATEEX* pDialogTemplateItem)
	{
		const BYTE* pnPointer = (const BYTE*) (pDialogTemplateItem + 1);
		CStringW sWindowClass;
		if(*((const WCHAR*) pnPointer) == 0xFFFF) 
		{
			//sWindowClass = CStringW(_StringHelper::Format(_T("#%d"), *((const WCHAR*) pnPointer + 1)));
			pnPointer += 2 * sizeof (WCHAR);
		} else
		{
			sWindowClass = (const WCHAR*) pnPointer;
			pnPointer += (sWindowClass.GetLength() + 1) * sizeof (WCHAR);
		}
		CStringW sTitle;
		if(*((const WCHAR*) pnPointer) == 0xFFFF) 
		{
			//sTitle = CStringW(_StringHelper::Format(_T("#%d"), *((const WCHAR*) pnPointer + 1)));
			pnPointer += 2 * sizeof (WCHAR);
		} else
		{
			sTitle = (const WCHAR*) pnPointer;
			pnPointer += (sTitle.GetLength() + 1) * sizeof (WCHAR);
		}
		WORD nItemDataSize = *((const WORD*) pnPointer);
		pnPointer += sizeof nItemDataSize + nItemDataSize;
		// NOTE: Items are aligned on 32-bit boundary
		return (const DLGITEMTEMPLATEEX*) (((UINT_PTR) pnPointer + 3) & ~3);
	}
	VOID CopyDialogTemplate()
	{
		ATLASSERT(!(m_psp.dwFlags & PSP_DLGINDIRECT));
		HINSTANCE hInstance = _AtlBaseModule.GetResourceInstance();
		HRSRC hDialogResourceInformation = FindResource(hInstance, MAKEINTRESOURCE(T::IDD), RT_DIALOG);
		ATLASSERT(hDialogResourceInformation);
		HGLOBAL hDialogResource = LoadResource(hInstance, hDialogResourceInformation);
		DWORD nDialogResourceSize = SizeofResource(hInstance, hDialogResourceInformation);
		const VOID* pvDialogTemplate = LockResource(hDialogResource);
		m_pOriginalDialogTemplate = (const DLGTEMPLATEEX*) pvDialogTemplate;
		ATLVERIFY(m_pDialogTemplate.AllocateBytes(nDialogResourceSize));
		CopyMemory(m_pDialogTemplate, m_pOriginalDialogTemplate, nDialogResourceSize);
		UnlockResource(hDialogResource);
		m_psp.dwFlags |= PSP_DLGINDIRECT;
		m_psp.pResource = reinterpret_cast<const DLGTEMPLATE*>((const DLGTEMPLATEEX*) m_pDialogTemplate);
		ATLASSERT(m_pDialogTemplate->dlgVer == 1 && m_pDialogTemplate->signature == 0xFFFF);
		m_pSeparatorDialogTemplateItem = NULL;
		const DLGITEMTEMPLATEEX* pDialogTemplateItem = GetFirstDialogTemplateItem(m_pDialogTemplate);
		for(WORD nIndex = 0; nIndex < m_pDialogTemplate->cDlgItems; nIndex++, pDialogTemplateItem = GetNextDialogTemplateItem(pDialogTemplateItem))
			if((WORD) pDialogTemplateItem->id == (WORD) -2)
			{
				m_pSeparatorDialogTemplateItem = pDialogTemplateItem;
				ATLASSERT(m_pSeparatorDialogTemplateItem->y < m_pDialogTemplate->cy);
				m_pDialogTemplate->style |= WS_VSCROLL;
				m_pDialogTemplate->cy = m_pSeparatorDialogTemplateItem->y;
				break;
			}
	}
	VOID SetPosition(LONG nPosition) throw()
	{
		T* pT = static_cast<T*>(this);
		SCROLLINFO ScrollInfo;
		ZeroMemory(&ScrollInfo, sizeof ScrollInfo);
		ScrollInfo.cbSize = sizeof ScrollInfo;
		ScrollInfo.fMask = SIF_RANGE | SIF_PAGE;
		ATLVERIFY(pT->GetScrollInfo(SB_VERT, &ScrollInfo));
		if(nPosition < ScrollInfo.nMin)
			nPosition = ScrollInfo.nMin;
		else if(nPosition > (LONG) (ScrollInfo.nMax - ScrollInfo.nPage))
			nPosition = (LONG) (ScrollInfo.nMax - ScrollInfo.nPage);
		if(m_nPosition != nPosition)
		{
			HDWP hDeferWindowPosition = BeginDeferWindowPos(16);
			for(CWindow Window = GetWindow(GW_CHILD); Window; Window = Window.GetWindow(GW_HWNDNEXT))
			{
				CRect WindowPosition;
				ATLVERIFY(Window.GetWindowRect(WindowPosition));
				ATLVERIFY(ScreenToClient(WindowPosition));
				hDeferWindowPosition = Window.DeferWindowPos(hDeferWindowPosition, NULL, WindowPosition.left, WindowPosition.top - nPosition + m_nPosition, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
			}
			ATLVERIFY(EndDeferWindowPos(hDeferWindowPosition));
			SCROLLINFO ScrollInfo;
			ZeroMemory(&ScrollInfo, sizeof ScrollInfo);
			ScrollInfo.cbSize = sizeof ScrollInfo;
			ScrollInfo.fMask = SIF_POS;
			ScrollInfo.nPos = nPosition;
			SetScrollInfo(SB_VERT, &ScrollInfo);
			m_nPosition = nPosition;
		}
	}

public:
// CScrollablePropertyPageT
	CScrollablePropertyPageT(_U_STRINGorID Title = (LPCTSTR) NULL) throw() :
		CPropertyPage(Title),
		m_pOriginalDialogTemplate(NULL),
		m_pSeparatorDialogTemplateItem(NULL)
	{
	}
	operator const PROPSHEETPAGE* ()
	{
		if(!m_pDialogTemplate)
			CopyDialogTemplate();
		return &m_psp;
	}
	BOOL IsChildVisible(CWindow ChildWindow, LONG* pnDistance = NULL) const throw()
	{
		CRect ClientPosition;
		ATLVERIFY(GetClientRect(ClientPosition));
		CRect ChildPosition;
		ATLVERIFY(ChildWindow.GetWindowRect(ChildPosition));
		ATLVERIFY(ScreenToClient(ChildPosition));
		CRect VisibleChildPosition;
		VisibleChildPosition.IntersectRect(&ChildPosition, &ClientPosition);
		if(VisibleChildPosition.Height() >= ChildPosition.Height())
			return TRUE;
		if(pnDistance)
			if(ChildPosition.top < ClientPosition.top)
				*pnDistance = ChildPosition.top - ClientPosition.top;
			else
				*pnDistance = ChildPosition.bottom - ClientPosition.bottom;
		return FALSE;
	}
	CWindow FocusNextChild(CWindow ChildWindow, UINT nGetWindowCommand = GW_HWNDNEXT) const throw()
	{
		for(CWindow Window = ChildWindow.GetWindow(nGetWindowCommand); Window; Window = Window.GetWindow(nGetWindowCommand))
			if(Window.IsWindowVisible() && Window.IsWindowEnabled() && Window.GetStyle() & WS_TABSTOP)
				if(IsChildVisible(Window))
				{
					Window.SetFocus();
					return Window;
				}
		return NULL;
	}

// Window message handlers
	LRESULT OnInitDialog(HWND, LPARAM)
	{
		m_nPosition = 0;
		if(m_pSeparatorDialogTemplateItem)
		{
			T* pT = static_cast<T*>(this);
			CRect ClientPosition;
			ATLVERIFY(pT->GetClientRect(ClientPosition));
			SCROLLINFO ScrollInfo;
			ZeroMemory(&ScrollInfo, sizeof ScrollInfo);
			ScrollInfo.cbSize = sizeof ScrollInfo;
			ScrollInfo.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
			ScrollInfo.nMin = 0;
			ScrollInfo.nMax = ClientPosition.Height() * m_pOriginalDialogTemplate->cy / m_pSeparatorDialogTemplateItem->y;
			ScrollInfo.nPage = ClientPosition.Height();
			ScrollInfo.nPos = m_nPosition;
			pT->SetScrollInfo(SB_VERT, &ScrollInfo);
			pT->SetTimer(TIMER_TRACKFOCUS, 200);
		}
		SetMsgHandled(FALSE);
		return TRUE;
	}
	LRESULT OnVScroll(UINT nCode, UINT nPosition, HWND)
	{
		ATLTRACE(atlTraceUI, 4, _T("nCode %d, nPosition %d\n"), nCode, nPosition);
		T* pT = static_cast<T*>(this);
		SCROLLINFO ScrollInfo;
		ZeroMemory(&ScrollInfo, sizeof ScrollInfo);
		ScrollInfo.cbSize = sizeof ScrollInfo;
		ScrollInfo.fMask = SIF_ALL;
		ATLVERIFY(pT->GetScrollInfo(SB_VERT, &ScrollInfo));
		BOOL bUpdatePosition = FALSE;
		CSize TextExtent;
		ATLVERIFY(GetTextExtentPoint(CClientDC(m_hWnd), _T("X"), 1, &TextExtent));
		switch(nCode)
		{
		case SB_LINEUP:
			ScrollInfo.nPos -= 1 + TextExtent.cy / 3;
			bUpdatePosition = TRUE;
			break;
		case SB_LINEDOWN:
			ScrollInfo.nPos += 1 + TextExtent.cy / 3;
			bUpdatePosition = TRUE;
			break;
		case SB_PAGEUP:
			ScrollInfo.nPos -= 2 * TextExtent.cy;
			bUpdatePosition = TRUE;
			break;
		case SB_PAGEDOWN:
			ScrollInfo.nPos += 2 * TextExtent.cy;
			bUpdatePosition = TRUE;
			break;
		case SB_THUMBTRACK:
		case SB_THUMBPOSITION:
			ScrollInfo.nPos = nPosition;
			bUpdatePosition = TRUE;
			break;
		case SB_ENDSCROLL:
			if(m_PreviousFocusedWindow && IsChildVisible(m_PreviousFocusedWindow))
				m_PreviousFocusedWindow.SetFocus();
			m_PreviousFocusedWindow = NULL;
			break;
		}
		if(bUpdatePosition)
		{
			SetPosition(ScrollInfo.nPos);
			CWindow FocusedWindow = GetFocus();
			LONG nDistance;
			if(FocusedWindow && pT->IsChild(FocusedWindow) && !IsChildVisible(FocusedWindow, &nDistance))
			{
				m_PreviousFocusedWindow = FocusedWindow;
				if(!FocusNextChild(FocusedWindow, (nDistance < 0) ? GW_HWNDNEXT : GW_HWNDPREV))
					::SetFocus(NULL);
			}
		}
		return 0;
	}
	LRESULT OnTimer(UINT_PTR nEvent, TIMERPROC)
	{
		T* pT = static_cast<T*>(this);
		switch(nEvent)
		{
		case TIMER_TRACKFOCUS:
			{
				CWindow FocusedWindow = GetFocus();
				if(FocusedWindow && IsChild(FocusedWindow))
				{
					CRect ClientPosition;
					ATLVERIFY(pT->GetClientRect(ClientPosition));
					if(!IsChildVisible(FocusedWindow))
					{
						CRect FocusedPosition;
						ATLVERIFY(FocusedWindow.GetWindowRect(FocusedPosition));
						ATLVERIFY(pT->ScreenToClient(FocusedPosition));
						LONG nPosition;
						if(FocusedPosition.Height() < ClientPosition.Height())
							nPosition = FocusedPosition.CenterPoint().y - ClientPosition.CenterPoint().y - m_nPosition;
						else
							nPosition = m_nPosition + FocusedPosition.top - ClientPosition.top;
						SetPosition(nPosition);
					}
				}
			}
			break;
		default:
			SetMsgHandled(FALSE);
			break;
		}
		return 0;
	}
	LRESULT OnMouseWheel(UINT, SHORT nDelta, CPoint)
	{
		T* pT = static_cast<T*>(this);
		CSize TextExtent;
		ATLVERIFY(GetTextExtentPoint(CClientDC(m_hWnd), _T("X"), 1, &TextExtent));
		SetPosition(m_nPosition - (nDelta / WHEEL_DELTA) * 2 * TextExtent.cy);
		CWindow FocusedWindow = GetFocus();
		LONG nDistance;
		if(FocusedWindow && pT->IsChild(FocusedWindow) && !IsChildVisible(FocusedWindow, &nDistance))
			if(!FocusNextChild(FocusedWindow, (nDistance < 0) ? GW_HWNDNEXT : GW_HWNDPREV))
				::SetFocus(NULL);
		return 0;
	}
};

////////////////////////////////////////////////////////////
// CMainPropertySheet

class CMainPropertySheet :
	public CPropertySheetImpl<CMainPropertySheet>
{
public:

BEGIN_MSG_MAP_EX(CMainPropertySheet)
	CHAIN_MSG_MAP(CPropertySheetImpl<CMainPropertySheet>)
END_MSG_MAP()

public:

	///////////////////////////////////////////////////////////
	// CPage1
	
	class CPage1 :
		public CPropertyPageImpl<CPage1>
	{
	public:

		enum { IDD = IDD_MAINPROPERTYPAGE1 };

	BEGIN_MSG_MAP_EX(CPage1)
		CHAIN_MSG_MAP(CPropertyPageImpl<CPage1>)
		MSG_WM_INITDIALOG(OnInitDialog)
	END_MSG_MAP()

	private:
		CMainPropertySheet& m_Owner;

	public:
	// CPage1
		CPage1(CMainPropertySheet* pOwner) :
			CPropertyPageImpl<CPage1>(_T("Page 1")),
			m_Owner(*pOwner)
		{
		}

	// Window message handlers
		LRESULT OnInitDialog(HWND, LPARAM)
		{
			return TRUE;
		}
	};

	///////////////////////////////////////////////////////////
	// CPage2
	
	class CPage2 :
		public CScrollablePropertyPageT<CPage2>
	{
	public:

		enum { IDD = IDD_MAINPROPERTYPAGE2 };

	BEGIN_MSG_MAP_EX(CPage2)
		CHAIN_MSG_MAP(CScrollablePropertyPageT<CPage2>)
	END_MSG_MAP()

	private:
		CMainPropertySheet& m_Owner;

	public:
	// CPage2
		CPage2(CMainPropertySheet* pOwner) :
			CScrollablePropertyPageT<CPage2>(_T("Page 2")),
			m_Owner(*pOwner)
		{
		}

	// Window message handlers
	};

private:
	CPage1 m_Page1;
	CPage2 m_Page2;

public:
// CMainPropertySheet
	CMainPropertySheet() throw() :
		CPropertySheetImpl<CMainPropertySheet>(_T("Property Sheet")),
		m_Page1(this),
		m_Page2(this)
	{
		AddPage(m_Page1);
		AddPage(m_Page2);
	}

// Window message handlers
};
