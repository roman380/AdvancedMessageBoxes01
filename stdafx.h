////////////////////////////////////////////////////////////
// stdafx.h - include file for standard system include files,
//            or project specific include files that are used 
//            frequently, but are changed infrequently
//
// Copyright (C) Alax.Info, 2006
// http://alax.info
//
// A permission to use the source code is granted as long as reference to 
// source website http://alax.info is retained.

#pragma once

////////////////////////////////////////////////////////////
// Windows definitions

//#define STRICT
#define WINVER						0x0501	// Windows XP
#define _WIN32_WINNT				0x0501	// Windows XP
#define _WIN32_WINDOWS				0x0410	// Windows 98
#define _WIN32_IE					0x0501	// Internet Explorer 5.01
#define _RICHEDIT_VER				0x0200	// RichEdit 2.0
#define INLINE_HRESULT_FROM_WIN32

////////////////////////////////////////////////////////////
// ATL definitions

#define _ATL_APARTMENT_THREADED
#define _ATL_NO_AUTOMATIC_NAMESPACE
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS
#define _ATL_ALL_WARNINGS
#define _ATL_ATTRIBUTES

#include <atlstr.h>
#include <atltypes.h>
#include <atlbase.h>
#include <atlcom.h>
#include <atlwin.h>
#include <atlctl.h>
#include <atlhost.h>
#include <atlcoll.h>

using namespace ATL;

////////////////////////////////////////////////////////////
// WTL definitions

#define _WTL_NO_CSTRING					// Use ATL7's CString implementation
#define _WTL_NO_WTYPES					// Use ATL7's CRect, CPoint, CSize implementation
#define _WTL_NO_UNION_CLASSES			// Use ATL7's union classes
#define _WTL_NEW_PAGE_NOTIFY_HANDLERS	// Use new property page syntax

#include <atlapp.h>
#include <atlgdi.h>
#include <atluser.h>
#include <atlcrack.h>
#include <atlmisc.h>
#include <atltheme.h>
#include <atldlgs.h>

using namespace WTL;

