/************************************************************************/
/* 
    Date : 10/31/2012
	File : NTSERVICE.H
    Author : Eadwin  (lidi@elex-tech.com)
    Funtion :  
	Version : 1.0
	Copyright (c) 2012 ELEX CDD
*/
/************************************************************************/  
#pragma once
#include <windows.h>
#include <string>

class CWinPrivilege
{
public:
	CWinPrivilege();
	BOOL SetPrivilegeEx(HANDLE hToken,LPCTSTR Privilege,BOOL bEnablePrivilege);
	bool GetPrivilegeFuncAddress();
	bool LaunchAppAsAdminUser(const std::wstring& lpImage, const std::wstring& lpCmdline,bool bWait = false);
	bool EnableSeviceDebugPrivilege();
	bool ExecuteW(LPCWSTR lpszProcessName, LPCWSTR lpszCmdLine, LPCWSTR lpCurrentDirectory, bool bForceAdmin, bool bWaitProcess);
	void DelSelf();
};
