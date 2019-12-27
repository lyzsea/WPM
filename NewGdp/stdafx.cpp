// stdafx.cpp : source file that includes just the standard includes
// NewGdp.pch will be the pre-compiled header
// stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"

// TODO: reference any additional headers you need in STDAFX.H
// and not in this file
CStrEncrypt g_StrFun;
std::string g_strGuid;

void DebugOutputMsg(const _TCHAR* pszFormat, ...)
{
	_TCHAR buf[1024];
	ZeroMemory(buf, 1024 * sizeof(_TCHAR));
	_stprintf_s(buf, 1024, _T("[%lu](%lu): "), GetProcessId(NULL), GetCurrentThreadId());
	va_list arglist;
	va_start(arglist, pszFormat);
	int nBuffSize = _tcslen(buf);
	_vstprintf_s(&buf[nBuffSize], 1024 - nBuffSize, pszFormat, arglist);
	va_end(arglist);
	nBuffSize = _tcslen(buf);
	_tcscat_s(buf, 1024 - nBuffSize, _T("\n"));
	OutputDebugString(buf);
}