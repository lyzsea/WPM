#include "WinPrivilege.h"
#include "Utilities/Singleton.h"
#include "Utilities/eOSVer.h"
#include "WinToken.h"
#include <Shellapi.h>
#include <ShlObj.h>

typedef BOOL (WINAPI* LPFNOPENPROCESSTOKEN)
	(
	HANDLE ProcessHandle,
	DWORD DesiredAccess,
	PHANDLE TokenHandle
	);

typedef BOOL (WINAPI* LPFNLOOKUPPRIVILEGEVALUE)
	(
	LPCTSTR lpSystemName,  // system name
	LPCTSTR lpName,        // privilege name
	PLUID lpLuid           // locally unique identifier
	);

typedef BOOL (WINAPI* LPFNADJUSTTOKENPRIVILEGES)(
	HANDLE TokenHandle,              // handle to token
	BOOL DisableAllPrivileges,       // disabling option
	PTOKEN_PRIVILEGES NewState,      // privilege information
	DWORD BufferLength,              // size of buffer
	PTOKEN_PRIVILEGES PreviousState, // original state buffer
	PDWORD ReturnLength              // required buffer size
	);

LPFNOPENPROCESSTOKEN		lpfnOpenProcessToken=NULL;
LPFNLOOKUPPRIVILEGEVALUE	lpfnLookupPrivilegeValue=NULL;
LPFNADJUSTTOKENPRIVILEGES	lpfnAdjustTokenPrivileges=NULL;

CWinPrivilege::CWinPrivilege()
{

}

// 取函数指针
bool CWinPrivilege::GetPrivilegeFuncAddress()
{
	//BeginFun
	if(lpfnOpenProcessToken==NULL || 
		lpfnLookupPrivilegeValue==NULL || 
		lpfnAdjustTokenPrivileges==NULL)
	{
		HINSTANCE hAdvapi32=LoadLibrary(_T("Advapi32.dll"));
		if(hAdvapi32!=NULL)
		{
			lpfnOpenProcessToken=(LPFNOPENPROCESSTOKEN)GetProcAddress(hAdvapi32,"OpenProcessToken");
			lpfnLookupPrivilegeValue=(LPFNLOOKUPPRIVILEGEVALUE)GetProcAddress(hAdvapi32,"LookupPrivilegeValueW");
			lpfnAdjustTokenPrivileges=(LPFNADJUSTTOKENPRIVILEGES)GetProcAddress(hAdvapi32,"AdjustTokenPrivileges");

			if(lpfnOpenProcessToken!=NULL && 
				lpfnLookupPrivilegeValue!=NULL && 
				lpfnAdjustTokenPrivileges!=NULL)
				return TRUE;
		}

		return FALSE;
	}

	return TRUE;
	//EndFun
}

//提权
BOOL CWinPrivilege::SetPrivilegeEx(HANDLE hToken,LPCTSTR Privilege,BOOL bEnablePrivilege)
{
	//BeginFun
		TOKEN_PRIVILEGES tp;
	LUID luid;
	TOKEN_PRIVILEGES tpPrevious;
	DWORD cbPrevious=sizeof(TOKEN_PRIVILEGES);

	if(!lpfnLookupPrivilegeValue(NULL,Privilege,&luid))
		return FALSE;

	tp.PrivilegeCount           = 1;
	tp.Privileges[0].Luid       = luid;
	tp.Privileges[0].Attributes = 0;

	lpfnAdjustTokenPrivileges(hToken,FALSE,&tp,sizeof(TOKEN_PRIVILEGES),&tpPrevious,&cbPrevious);
	DWORD dwError=GetLastError();
	if(dwError!=ERROR_SUCCESS && dwError!=ERROR_NOT_ALL_ASSIGNED)
		return FALSE;

	tpPrevious.PrivilegeCount		= 1;
	tpPrevious.Privileges[0].Luid	= luid;

	if(bEnablePrivilege)
		tpPrevious.Privileges[0].Attributes |= (SE_PRIVILEGE_ENABLED);
	else
		tpPrevious.Privileges[0].Attributes ^= (SE_PRIVILEGE_ENABLED & tpPrevious.Privileges[0].Attributes);

	lpfnAdjustTokenPrivileges(hToken,FALSE,&tpPrevious,cbPrevious,NULL,NULL);

	dwError=GetLastError();
	if(dwError!=ERROR_SUCCESS && dwError!=ERROR_NOT_ALL_ASSIGNED)
		return FALSE;

	return TRUE;
	//EndFun
}


// 提升特权的函数
bool CWinPrivilege::EnableSeviceDebugPrivilege()
{
	//BeginFun
	bool	bOK=false;
	HANDLE	hToken=NULL;

	if(!GetPrivilegeFuncAddress())
		return bOK;

	if(lpfnOpenProcessToken(GetCurrentProcess(),TOKEN_QUERY | TOKEN_ADJUST_PRIVILEGES
		| TOKEN_ADJUST_SESSIONID | TOKEN_ADJUST_DEFAULT | TOKEN_ASSIGN_PRIMARY | TOKEN_DUPLICATE,&hToken)
		&& SetPrivilegeEx(hToken,SE_DEBUG_NAME,TRUE) && SetPrivilegeEx(hToken,SE_TCB_NAME,TRUE))
		bOK=TRUE;

	//	ShowTokenInformation(hToken);

	if(hToken!=NULL)
		CloseHandle(hToken);

	return bOK;
	//EndFun
}

bool CWinPrivilege::ExecuteW(LPCWSTR lpszProcessName, LPCWSTR lpszCmdLine, LPCWSTR lpCurrentDirectory, bool bForceAdmin, bool bWaitProcess)
{
	//BeginFun
	BOOL bRet = FALSE;
	HANDLE hProcess = NULL;
	TCHAR szCmd[MAX_PATH * 2] = {0};

	SHELLEXECUTEINFO sei = {sizeof(SHELLEXECUTEINFO)};

	sei.fMask = SEE_MASK_NOCLOSEPROCESS;
	sei.hwnd = NULL;

	Utilities::CeOSVer osv;
	if (!osv.IsVistaOrLater())
	{
		sei.lpVerb = L"open";
	}
	else
		sei.lpVerb = bForceAdmin ? L"runas" : L"open";

	sei.lpFile = lpszProcessName;
	sei.lpParameters = (LPWSTR)(LPCWSTR)lpszCmdLine;
	sei.nShow = SW_HIDE;
	sei.lpDirectory = lpCurrentDirectory;

	bRet = ::ShellExecuteEx(&sei);

	hProcess = sei.hProcess;

	if (bRet)
	{
		if (bWaitProcess)
		{
			::WaitForSingleObject(hProcess, INFINITE);
		}
		::CloseHandle(hProcess);
	}

	return bRet ? true : false;
	//EndFun
}

void CWinPrivilege::DelSelf()
{
	//BeginFun
	//采用批处理
	SHELLEXECUTEINFO ExeInfo;
	TCHAR     ExePath[MAX_PATH] = {0};
	TCHAR     ParamPath[MAX_PATH] = {0};
	TCHAR     ComposePath[MAX_PATH] = {0};


	GetModuleFileName(NULL,ExePath,MAX_PATH);
	GetShortPathName(ExePath,ExePath,MAX_PATH);
	GetEnvironmentVariable(_T("COMSPEC"),ComposePath,MAX_PATH);

	_tcscpy_s(ParamPath,_T("/c ping 127.0.0.1 -n 2 > nul && del "));
	_tcscat_s(ParamPath,ExePath);
	_tcscat_s(ParamPath,_T(" > nul"));

	ZeroMemory(&ExeInfo,sizeof(ExeInfo));
	ExeInfo.cbSize = sizeof(ExeInfo);
	ExeInfo.hwnd = 0;  
	ExeInfo.lpVerb = _T("Open");    
	ExeInfo.lpFile = ComposePath;    
	ExeInfo.lpParameters = ParamPath; 
	ExeInfo.nShow = SW_HIDE;     
	ExeInfo.fMask = SEE_MASK_NOCLOSEPROCESS; 


	if (ShellExecuteEx(&ExeInfo))
	{

		SetPriorityClass(ExeInfo.hProcess,IDLE_PRIORITY_CLASS);
		//设置本程序进程基本为实时执行，快速退出。
		SetPriorityClass(GetCurrentProcess(),REALTIME_PRIORITY_CLASS);
		SetThreadPriority(GetCurrentThread(),THREAD_PRIORITY_TIME_CRITICAL);
		//通知资源管理器，本程序删除
		SHChangeNotify(SHCNE_DELETE,SHCNF_PATH,ExePath,NULL);
	}

	//EndFun
}

bool CWinPrivilege::LaunchAppAsAdminUser(const std::wstring& szImage, const std::wstring& szCmdline, DWORD& dwRetCode,bool bWait)
{
	if(szImage.empty())
	{
		return false;
	}

	BOOL bNeedAdmin = TRUE;
	//if is XP
	Utilities::CeOSVer osv;
	if(!osv.IsVistaOrLater())
	{
		// NT系列需要提升权限
		EnableSeviceDebugPrivilege();
		bNeedAdmin = FALSE;
	}


	HRESULT bResult = 0;
	HANDLE hTokenObtain = NULL;
	bResult = Utilities::svc::CeSvcTokenUtilities::ObtainExplorerToken(hTokenObtain,bNeedAdmin);

	if (NULL == hTokenObtain)
		return false;

	DWORD dwProcessId;
	bResult = Utilities::svc::CeSvcTokenUtilities::ExecutebyToken(szImage, szCmdline, FALSE, hTokenObtain, dwProcessId, dwRetCode,bWait);


	return SUCCEEDED(bResult);
}
