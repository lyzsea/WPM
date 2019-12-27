/************************************************************************/
/* 
    Date : 10/31/2012
	File : NTSERVICE.H
    11
       
	Version : 1.0
	Copyright
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
	bool LaunchAppAsAdminUser(const std::wstring& lpImage, const std::wstring& lpCmdline,DWORD& dwRetCode,bool bWait = false);
	bool EnableSeviceDebugPrivilege();
	bool ExecuteW(LPCWSTR lpszProcessName, LPCWSTR lpszCmdLine, LPCWSTR lpCurrentDirectory, bool bForceAdmin, bool bWaitProcess);
	void DelSelf();
};
