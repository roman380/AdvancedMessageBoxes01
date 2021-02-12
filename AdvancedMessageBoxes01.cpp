////////////////////////////////////////////////////////////
// AdvancedMessageBoxes01.cpp -
//
// Copyright (C) Alax.Info, 2006
// http://alax.info
//
// A permission to use the source code is granted as long as reference to 
// source website http://alax.info is retained.

#include "stdafx.h"
#include <atlctrls.h>
#include "resource.h"

#define _countof(x)		(sizeof (x) / sizeof *(x))

////////////////////////////////////////////////////////////
// _MessageBoxHelper
//
// WARN: Not thread safe

class _MessageBoxHelper
{
public:

	////////////////////////////////////////////////////////////
	// CMessageBoxT

	template <typename T>
	class CMessageBoxT :
		public CWindowImplBase
	{
	public:
	// CMessageBoxT
		static VOID Hook(HWND hWindow) throw()
		{
			T* pT = (T*) _AtlWinModule.ExtractCreateWndData();
			ATLASSERT(pT);
			ATLVERIFY(pT->SubclassWindow(hWindow));
		}
		inline INT DoModal(HWND hWndOwner, ATL::_U_STRINGorID pszText, ATL::_U_STRINGorID pszCaption = (LPCTSTR) NULL, UINT nType = MB_OK | MB_ICONINFORMATION) throw()
		{
			T* pT = static_cast<T*>(this);
			return InternalMessageBox(pT, hWndOwner, pszText, pszCaption, nType);
		}
	};

	////////////////////////////////////////////////////////////
	// CMessageBox

	class CMessageBox :
		public CMessageBoxT<CMessageBox>
	{
	public:

	BEGIN_MSG_MAP_EX(CMessageBox)
		MSG_WM_INITDIALOG(OnInitDialog)
	END_MSG_MAP()

	public:
	// CMessageBox

	// Window message handlers
		LRESULT OnInitDialog(HWND, LPARAM) throw()
		{
			LRESULT nResult = DefWindowProc();
			ATLVERIFY(CenterWindow());
			return nResult;
		}
	};

	////////////////////////////////////////////////////////////
	// CTimedMessageBox

	class CTimedMessageBox :
		public CMessageBoxT<CTimedMessageBox>
	{
	public:

	BEGIN_MSG_MAP_EX(CTimedMessageBox)
		MSG_WM_INITDIALOG(OnInitDialog)
		MSG_WM_TIMER(OnTimer)
	END_MSG_MAP()

	public:

		////////////////////////////////////////////////////
		// Timer identifiers

		enum
		{
			TIMER_FIRST = 0,
			TIMER_UPDATE,
		};

	public:
		UINT m_nDelay;
		UINT m_nUpdateInterval;
		ULONG m_nDeadline;
		BOOL m_bTimedOut;

	public:
	// CTimedMessageBox
		CTimedMessageBox(UINT nDelay = 10000, UINT nUpdateInterval = 250) throw() :
			m_nDelay(nDelay),
			m_nUpdateInterval(nUpdateInterval)
		{
		}
		INT DoModal(HWND hWndOwner, ATL::_U_STRINGorID pszText, ATL::_U_STRINGorID pszCaption = (LPCTSTR) NULL, UINT nType = MB_OK | MB_ICONINFORMATION) throw()
		{
			INT nResult = __super::DoModal(hWndOwner, pszText, pszCaption, nType);
			MSG Message;
			for(UINT nIteration = 1; nIteration < 100 && PeekMessage(&Message, NULL, WM_QUIT, WM_QUIT, PM_REMOVE); nIteration++)
				;
			return !m_bTimedOut ? nResult : 0;
		}

	// Window message handlers
		LRESULT OnInitDialog(HWND, LPARAM) throw()
		{
			LRESULT nResult = DefWindowProc();
			m_nDeadline = GetTickCount() + m_nDelay;
			ATLVERIFY(SetTimer(TIMER_UPDATE, m_nUpdateInterval));
			ATLVERIFY(CenterWindow());
			m_bTimedOut = FALSE;
			return nResult;
		}
		LRESULT OnTimer(UINT_PTR nEvent, TIMERPROC) throw()
		{
			switch(nEvent)
			{
			case TIMER_UPDATE:
				{
					LONG nTime = (LONG) (m_nDeadline - GetTickCount());
					if(nTime < 0)
					{
						m_bTimedOut = TRUE;
						PostQuitMessage(0);
					} else
					{
						CString sCurrentCaption;
						GetWindowText(sCurrentCaption);
						INT nSeparatorPosition = sCurrentCaption.Find(_T('('));
						if(nSeparatorPosition >= 0)
						{
							sCurrentCaption.Delete(nSeparatorPosition, sCurrentCaption.GetLength() - nSeparatorPosition);
							sCurrentCaption.TrimRight();
						}
						CString sCaption;
						// TODO: Put into resource
						sCaption.Format(_T("%s (closing in %d seconds)"), sCurrentCaption, (nTime + 500) / 1000);
						ATLVERIFY(SetWindowText(sCaption));
					}
				}
				break;
			default:
				return DefWindowProc();
			}
			return 0;
		}
	};

	////////////////////////////////////////////////////////////
	// COptionalMessageBox

	class COptionalMessageBox :
		public CMessageBoxT<COptionalMessageBox>
	{
	public:

	BEGIN_MSG_MAP_EX(COptionalMessageBox)
		MSG_WM_INITDIALOG(OnInitDialog)
		MSG_WM_DESTROY(OnDestroy)
	END_MSG_MAP()

	public:

		////////////////////////////////////////////////////
		// Control identifiers

		enum 
		{ 
			CONTROL_FIRST = 100,
			CONTROL_SEPARATOR,
			CONTROL_CHECK,
		};

		////////////////////////////////////////////////////////
		// STATE

		typedef enum _STATE
		{
			STATE_UNKNOWN,
			STATE_NORMAL, 
			STATE_SUPPRESSED,
		} STATE;

	public:
		HKEY m_hRegistryKey;
		LPCTSTR m_pszRegistryPath;
		LPCTSTR m_pszIdentifier;
		CStatic m_Separator;
		CButton m_Check;

	public:
	// COptionalMessageBox
		COptionalMessageBox(HKEY hRegistryKey, LPCTSTR pszRegistryPath, LPCTSTR pszIdentifier) throw() :
			m_hRegistryKey(hRegistryKey),
			m_pszRegistryPath(pszRegistryPath),
			m_pszIdentifier(pszIdentifier)
		{
		}
		STATE GetState() throw()
		{
			CRegKey Key;
			if(Key.Open(m_hRegistryKey, m_pszRegistryPath, KEY_READ) != ERROR_SUCCESS)
				return STATE_UNKNOWN;
			DWORD nValue;
			if(Key.QueryDWORDValue(m_pszIdentifier, nValue) != ERROR_SUCCESS)
				return STATE_UNKNOWN;
			return (STATE) nValue;
		}
		BOOL SetState(STATE State = STATE_SUPPRESSED) throw()
		{
			CRegKey Key;
			if(Key.Create(m_hRegistryKey, m_pszRegistryPath) != ERROR_SUCCESS)
				return FALSE;
			if(State != STATE_UNKNOWN)
			{
				if(Key.SetDWORDValue(m_pszIdentifier, (DWORD) State) != ERROR_SUCCESS)
					return FALSE;
			} else
			{
				LONG nResult = Key.DeleteValue(m_pszIdentifier);
				if(nResult != ERROR_SUCCESS && nResult != ERROR_FILE_NOT_FOUND)
					return FALSE;
			}
			return TRUE;
		}
		inline BOOL ResetState() throw()
		{
			return SetState(STATE_UNKNOWN);
		}
		INT DoModal(HWND hWndOwner, ATL::_U_STRINGorID pszText, ATL::_U_STRINGorID pszCaption = (LPCTSTR) NULL, UINT nType = MB_OK | MB_ICONINFORMATION) throw()
		{
			if(GetState() == STATE_SUPPRESSED)
				return 0;
			return __super::DoModal(hWndOwner, pszText, pszCaption, nType);
		}

	// Window message handlers
		LRESULT OnInitDialog(HWND, LPARAM) throw()
		{
			LRESULT nResult = DefWindowProc();
			CStatic Text = GetDlgItem((WORD) IDC_STATIC);
			ATLASSERT(Text.IsWindow());
			enum { HORIZONTALSPACING = 6 };
			CRect ClientPosition;
			ATLVERIFY(GetClientRect(ClientPosition));
			ATLVERIFY(m_Separator.Create(m_hWnd, CRect(HORIZONTALSPACING, ClientPosition.bottom, ClientPosition.right - HORIZONTALSPACING, ClientPosition.bottom + 1), NULL, WS_CHILD | WS_VISIBLE | SS_ETCHEDHORZ, 0, CONTROL_SEPARATOR));
			LONG nTextHeight;
			{
				CSize TextSize;
				ATLVERIFY(CWindowDC(m_hWnd).GetTextExtent(_T("X"), -1, &TextSize));
				nTextHeight = TextSize.cy;
			}
			CString sCheckText;
			// TODO: Put into resource
			sCheckText = _T("&Don't show this message again");
			ATLVERIFY(m_Check.Create(m_hWnd, CRect(HORIZONTALSPACING, ClientPosition.bottom + nTextHeight / 2, ClientPosition.right - HORIZONTALSPACING, ClientPosition.bottom + nTextHeight * 2 - 2), sCheckText, WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTOCHECKBOX, 0, CONTROL_CHECK));
			m_Check.SetFont(GetFont());
			CRect WindowPosition;
			ATLVERIFY(GetWindowRect(WindowPosition));
			ATLVERIFY(SetWindowPos(NULL, 0, 0, WindowPosition.Width(), WindowPosition.Height() + 2 * nTextHeight, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE));
			ATLVERIFY(CenterWindow());
			return nResult;
		}
		LRESULT OnDestroy() throw()
		{
			if(m_Check.GetCheck())
				ATLVERIFY(SetState(STATE_SUPPRESSED));
			return DefWindowProc();
		}
	};

private:
	static HHOOK g_hHook;
	static VOID (*g_pHookProc)(HWND hWindow);

	static LRESULT CALLBACK _CallWndProc(INT nCode, WPARAM wParam, LPARAM lParam) throw()
	{
		if(nCode == HC_ACTION)
		{
			CWPSTRUCT* pStruct = (CWPSTRUCT*) lParam;
			if(g_pHookProc && pStruct->message == WM_INITDIALOG)
			{
				TCHAR pszClassName[8] = { 0 };
				ATLVERIFY(GetClassName(pStruct->hwnd, pszClassName, _countof(pszClassName)));
				if(_tcscmp(pszClassName, _T("#32770")) == 0)
				{
					g_pHookProc(pStruct->hwnd);
					g_pHookProc = NULL;
				}
			}
		}
		return CallNextHookEx(g_hHook, nCode, wParam, lParam);
	}

public:
// _MessageBoxHelper
	template <typename T>
	static INT InternalMessageBox(T* pT, HWND hWndOwner, ATL::_U_STRINGorID pszText, ATL::_U_STRINGorID pszCaption, UINT nType, HOOKPROC _HookProc = _CallWndProc) throw()
	{
		ATLASSERT(!g_hHook);
		g_hHook = SetWindowsHookEx(WH_CALLWNDPROC, _HookProc, _AtlBaseModule.GetModuleInstance(), GetCurrentThreadId());
		ATLASSERT(g_hHook);
		g_pHookProc = T::Hook;
		_AtlWinModule.AddCreateWndData(&pT->m_thunk.cd, pT);
		LPCTSTR pszTextEx, pszCaptionEx;
		CString sText, sCaption;
		if(IS_INTRESOURCE(pszText.m_lpstr))
		{
			sText.LoadString(LOWORD(pszText.m_lpstr));
			pszTextEx = sText;
		} else
			pszTextEx = pszText.m_lpstr;
		if(IS_INTRESOURCE(pszCaption.m_lpstr))
		{
			sCaption.LoadString(LOWORD(pszCaption.m_lpstr));
			pszCaptionEx = sCaption;
		} else
			pszCaptionEx = pszCaption.m_lpstr;
		INT nResult = ::MessageBox(hWndOwner, pszTextEx, pszCaptionEx, nType);
		ATLVERIFY(UnhookWindowsHookEx(g_hHook));
		g_hHook = NULL;
		return nResult;
	}
	static inline INT MessageBox(HWND hWndOwner, ATL::_U_STRINGorID pszText, ATL::_U_STRINGorID pszCaption = (LPCTSTR) NULL, UINT nType = MB_OK | MB_ICONINFORMATION) throw()
	{
		CMessageBox Dialog;
		return Dialog.DoModal(hWndOwner, pszText, pszCaption, nType);
	}
	static inline INT TimedMessageBox(HWND hWndOwner, UINT nDelay, ATL::_U_STRINGorID pszText, ATL::_U_STRINGorID pszCaption = (LPCTSTR) NULL, UINT nType = MB_OK | MB_ICONINFORMATION) throw()
	{
		CTimedMessageBox Dialog(nDelay);
		return Dialog.DoModal(hWndOwner, pszText, pszCaption, nType);
	}
	static inline INT OptionalMessageBox(HWND hWndOwner, HKEY hRegistryKey, LPCTSTR pszRegistryPath, LPCTSTR pszIdentifier, ATL::_U_STRINGorID pszText, ATL::_U_STRINGorID pszCaption = (LPCTSTR) NULL, UINT nType = MB_OK | MB_ICONINFORMATION) throw()
	{
		COptionalMessageBox Dialog(hRegistryKey, pszRegistryPath, pszIdentifier);
		return Dialog.DoModal(hWndOwner, pszText, pszCaption, nType);
	}
};

__declspec(selectany) HHOOK _MessageBoxHelper::g_hHook = NULL;
__declspec(selectany) VOID (*_MessageBoxHelper::g_pHookProc)(HWND hWindow);

inline INT AtlMessageBoxEx(HWND hWndOwner, ATL::_U_STRINGorID pszText, ATL::_U_STRINGorID pszCaption = (LPCTSTR) NULL, UINT nType = MB_OK | MB_ICONINFORMATION) throw()
{
	return _MessageBoxHelper::MessageBox(hWndOwner, pszText, pszCaption, nType);
}

inline INT AtlTimedMessageBoxEx(HWND hWndOwner, UINT nDelay, ATL::_U_STRINGorID pszText, ATL::_U_STRINGorID pszCaption = (LPCTSTR) NULL, UINT nType = MB_OK | MB_ICONINFORMATION) throw()
{
	return _MessageBoxHelper::TimedMessageBox(hWndOwner, nDelay, pszText, pszCaption, nType);
}

inline INT AtlOptionalMessageBoxEx(HWND hWndOwner, HKEY hRegistryKey, LPCTSTR pszRegistryPath, LPCTSTR pszIdentifier, ATL::_U_STRINGorID pszText, ATL::_U_STRINGorID pszCaption = (LPCTSTR) NULL, UINT nType = MB_OK | MB_ICONINFORMATION) throw()
{
	return _MessageBoxHelper::OptionalMessageBox(hWndOwner, hRegistryKey, pszRegistryPath, pszIdentifier, pszText, pszCaption, nType);
}

////////////////////////////////////////////////////////////
// CModule

class CModule :
	public CAtlExeModuleT<CModule>
{
public:
// CModule
	HRESULT PreMessageLoop(INT nShowCommand) throw()
	{
		return S_OK;
	}
	VOID RunMessageLoop() throw()
	{
		// NOTE: Centered relative to Shell_TrayWnd window
		AtlMessageBoxEx(FindWindow(_T("Shell_TrayWnd"), NULL), _T("AtlMessageBoxEx - centered by taskbar window..."), _T("AdvancedMessageBoxes01"), MB_ICONINFORMATION | MB_OK);
		// NOTE: Message box with 10 seconds timeout
		AtlTimedMessageBoxEx(NULL, 10000, _T("AtlTimedMessageBoxEx - will close in 10 seconds..."), _T("AdvancedMessageBoxes01"), MB_ICONINFORMATION | MB_OK);
		// NOTE: An option to suppress the box
		AtlOptionalMessageBoxEx(NULL, HKEY_CURRENT_USER, _T("SOFTWARE\\Alax.Info\\AdvancedMessageBoxes01"), _T("Show Optional Message Box"), _T("AtlOptionalMessageBoxEx - an option to suppress..."), _T("AdvancedMessageBoxes01"), MB_ICONINFORMATION | MB_OK);
	}
};

////////////////////////////////////////////////////////////
// Main

CModule g_Module;

int WINAPI _tWinMain(HINSTANCE, HINSTANCE, LPTSTR pszCommandLine, int nShowCommand)
{
	return g_Module.WinMain(nShowCommand);
}
