/********************************************************************
	Created:	21-4-2007   16:52
	FileName: 	UTL_FileTime.H
	Author:		ChenQingMing
	
	purpose:	
*********************************************************************/
#ifndef __UTL_FILETIME_H_
#define __UTL_FILETIME_H_
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <windows.h>
#include <xstring>

// bool GetFileModifyTime(LPCTSTR lpFile,FILETIME & modFT);
// bool FileTimeToString(const FILETIME & ft,LPTSTR lpTimeString, int nSize);
// bool TimeStringToFileTime(LPCTSTR lpTimeString,FILETIME & ft);

std::string ws2s(const std::wstring& wide, UINT CodePage);
std::wstring s2ws( const std::string& mb,UINT CodePage);
#endif//__UTL_FILETIME_H_