/************************************************************************/
/* 
    Date : 4/20/2012
	File : NTSERVICE.CPP
    Author : Eadwin
    Funtion :  
	Version : 2.0
	Copyright (c) 2012 ELEX V9
*/
/************************************************************************/  
#include "stdafx.h"

#include <stdlib.h>
#include <stdio.h>
#include <dbt.h>
#include <setupapi.h>
#include <malloc.h>
#include <memory.h>
#include <string>

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // some CString constructors will be explicit
#include <atlbase.h>
#include <atlstr.h>

#include "BaseService.h"
#include "cmdAnalyze/Cmdline.h"


extern CBaseService* g_pService; 
extern SERVICEDATA	g_ServiceData;

CBaseService::CBaseService(BOOL bAfterXP, bool bDelayStartup )
{
    // copy the address of the current object so we can access it from
    // the static member callback functions. 
    // WARNING: This limits the application to only one CNTService object.
    
	m_bIsAfterXP = bAfterXP;
    // Set the default service name and version

    m_iMajorVersion = 2;
    m_iMinorVersion = 0;
    m_hEventSource = NULL;

    // set up the initial service status 
    m_hServiceStatus = NULL;
    m_Status.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    m_Status.dwCurrentState = SERVICE_STOPPED;
    m_Status.dwControlsAccepted = SERVICE_ACCEPT_STOP;
    m_Status.dwWin32ExitCode = 0;
    m_Status.dwServiceSpecificExitCode = 0;
    m_Status.dwCheckPoint = 0;
    m_Status.dwWaitHint = 0;
    m_bIsRunning = FALSE;
	m_bDelayStartup = bDelayStartup;

	_TCHAR szFilePath[_MAX_PATH] = {0};
	::GetModuleFileName( NULL, szFilePath, sizeof(szFilePath) );
	_tcscpy_s(g_ServiceData.lpszBinaryPathName,szFilePath);
}

CBaseService::~CBaseService()
{
    if (m_hEventSource) {
        ::DeregisterEventSource(m_hEventSource);
    }
}

////////////////////////////////////////////////////////////////////////////////////////
// Install/uninstall routines

// Test if the service is currently installed
BOOL CBaseService::IsInstalled()
{

    BOOL bResult = FALSE;

    // Open the Service Control Manager
    SC_HANDLE hSCM = ::OpenSCManager(NULL, // local machine
                                     NULL, // ServicesActive database
                                     SC_MANAGER_ALL_ACCESS); // full access
    if (hSCM) {

        // Try to open the service
        SC_HANDLE hService = ::OpenService(hSCM,
                                           g_ServiceData.lpszServiceName,
                                           SERVICE_QUERY_CONFIG);
        if (hService) {
            bResult = TRUE;
            ::CloseServiceHandle(hService);
        }

        ::CloseServiceHandle(hSCM);
    }
    
    return bResult;

}

BOOL CBaseService::Install()
{
    // Open the Service Control Manager
    SC_HANDLE hSCM = ::OpenSCManager(NULL, // local machine
                                     NULL, // ServicesActive database
                                     SC_MANAGER_ALL_ACCESS); // full access
    if (!hSCM) return FALSE;


    // Create the service
	std::wstring strBinary = g_ServiceData.lpszBinaryPathName;
	strBinary += L" -";
	strBinary += CMDLINE_PARAM_NAME_RUANASSVR;
	SC_HANDLE hService = ::CreateService(
									hSCM,						// SCManager database
									g_ServiceData.lpszServiceName,		// name of service
									g_ServiceData.lpszDisplayName,		// name to display
									SERVICE_ALL_ACCESS,					// desired access
									g_ServiceData.dwServiceType,		// service type
									g_ServiceData.dwStartType,			// start type
									g_ServiceData.dwErrorControl,       // error control type
									strBinary.c_str(),	// service's binary
									m_bIsAfterXP ? SVC_ORDERGROUP_1 : SVC_ORDERGROUP_2,
									g_ServiceData.dwTagId!=0 ? &g_ServiceData.dwTagId : NULL,				// no tag identifier
									g_ServiceData.lpszDependencies,		// dependencies
									NULL,								// LocalSystem account
									NULL);								// no password
    if ( !hService )
	{
        ::CloseServiceHandle(hSCM);
        return FALSE;
    }

	SERVICE_DESCRIPTION des;
	des.lpDescription=g_ServiceData.lpszDescription;

	ChangeServiceConfig2(
		hService,                // handle to service
		SERVICE_CONFIG_DESCRIPTION, // change: description
		&des);

	if ( m_bDelayStartup == true )
	{	// setup delay start up
		SERVICE_DELAYED_AUTO_START_INFO info = { TRUE };
		if ( 0 != ::ChangeServiceConfig2( hService,  SERVICE_CONFIG_DELAYED_AUTO_START_INFO, &info) )
		{
			LOG_ERR(L"WpmService change configuration to delay auto-startup failed, (%s).",L"Install");

			DebugMsg( _T("change configuration to delay auto-startup failed") );
		}
	}

    // make registry entries to support logging messages
    // Add the source name as a subkey under the Application
    // key in the EventLog service portion of the registry.
    _TCHAR szKey[256];
    HKEY hKey = NULL;
    _tcscpy_s( szKey, 256, _T( "SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application\\")  );
   _tcscat_s( szKey, 256, g_ServiceData.lpszServiceName );
    if (::RegCreateKey(HKEY_LOCAL_MACHINE, szKey, &hKey) != ERROR_SUCCESS) 
	{
        ::CloseServiceHandle(hService);
        ::CloseServiceHandle(hSCM);
        return FALSE;
    }

    // Add the Event ID message-file name to the 'EventMessageFile' subkey.
    ::RegSetValueEx(hKey, _T("EventMessageFile"), 0, REG_EXPAND_SZ, (CONST BYTE*)g_ServiceData.lpszBinaryPathName, _tcslen(g_ServiceData.lpszBinaryPathName) + 1);     

    // Set the supported types flags.
    DWORD dwData = EVENTLOG_ERROR_TYPE | EVENTLOG_WARNING_TYPE | EVENTLOG_INFORMATION_TYPE;
    ::RegSetValueEx(hKey, _T("TypesSupported"), 0, REG_DWORD, (CONST BYTE*)&dwData, sizeof(DWORD));
    ::RegCloseKey(hKey);

    LogEvent(EVENTLOG_INFORMATION_TYPE, EVMSG_INSTALLED, g_ServiceData.lpszServiceName);

    // tidy up
    ::CloseServiceHandle(hService);
    ::CloseServiceHandle(hSCM);
    return TRUE;

}

BOOL CBaseService::InstallService()
{
	// Request to install.
	if (IsInstalled())
	{
		DebugMsg( _T("%s is already installed\n"), g_ServiceData.lpszServiceName);

		// change config
		BOOL bResult = TRUE;

		SC_HANDLE hSCM = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);

		if (hSCM != NULL)
		{
			SC_HANDLE hService = ::OpenService(
				hSCM, 
				g_ServiceData.lpszServiceName, 
				SERVICE_QUERY_CONFIG | SERVICE_CHANGE_CONFIG
				);
			if (hService != NULL)
			{
				ChangeServiceConfig( 
					hService,
					SERVICE_WIN32_OWN_PROCESS,
					SERVICE_AUTO_START, // 默认自动启动服务
					SERVICE_ERROR_NORMAL,
					g_ServiceData.lpszBinaryPathName,
					m_bIsAfterXP ? SVC_ORDERGROUP_1 : SVC_ORDERGROUP_2,
					NULL,
					_T("RPCSS\0"),
					NULL,
					NULL,
					g_ServiceData.lpszDisplayName
					);
				::CloseServiceHandle(hService);
			}
			::CloseServiceHandle(hSCM);

			//return OnRunService();
		}
		return bResult;
	} 
	else 
	{
		// Try and install the copy that's running
		if (Install()) {
			DebugMsg( _T("%s installed\n"), g_ServiceData.lpszServiceName);
		}
		else
		{
			DebugMsg( _T("%s failed to install. Error %d\n"), g_ServiceData.lpszServiceName, GetLastError());
			return FALSE;
		}
	}
	return TRUE; // say we processed the argument

}

BOOL CBaseService::UninstallService()
{
	// Request to uninstall.
	if (!IsInstalled())
	{
		DebugMsg( _T("%s is not installed\n"), g_ServiceData.lpszServiceName);
	} 
	else 
	{
		// Try and remove the copy that's installed
		if (Uninstall())
		{
			// Get the executable file path
			_TCHAR szFilePath[_MAX_PATH];
			ZeroMemory( szFilePath, MAX_PATH * sizeof( _TCHAR ) );
			::GetModuleFileName(NULL, szFilePath, sizeof(szFilePath));
			DebugMsg( _T("%s removed. (You must delete the file (%s) yourself.)\n"), g_ServiceData.lpszServiceName, szFilePath);
		} 
		else 
		{
			DebugMsg( _T("Could not remove %s. Error %d\n"), g_ServiceData.lpszServiceName, GetLastError() );
			return FALSE;
			
		}
	}
	return TRUE; // say we processed the argument

}

BOOL CBaseService::InstAndStartService()
{
	if (IsInstalled())
	{
		LOG_ERR(L"WpmService is already installed, (%s).",L"Install");
		DebugMsg( _T("%s is already installed\n"), g_ServiceData.lpszServiceName);
		
	} 
	else 
	{
		if (Install()) {
			LOG_INFO(L"WpmService is  installed, (%s).",L"Install");
			DebugMsg( _T("%s installed\n"), g_ServiceData.lpszServiceName);

		}
		else
		{
			LOG_ERR(L"WpmService failed to install., (%s). ErrorCode: %d",L"Install", GetLastError());
			DebugMsg( _T("%s failed to install. Error %d\n"), g_ServiceData.lpszServiceName, GetLastError());
			
			return FALSE;
		}
	}
	return StartService();
}


BOOL CBaseService::UninstAndStopService()
{
	// Request to uninstall.
	if (!IsInstalled())
	{
		DebugMsg( _T("%s is not installed\n"), g_ServiceData.lpszServiceName);
	} 
	else 
	{
		if(!StopService())
			return FALSE;
		// Try and remove the copy that's installed
		if (!Uninstall())
		{
			DebugMsg( _T("Could not remove %s. Error %d\n"), g_ServiceData.lpszServiceName, GetLastError() );
			return FALSE;
		}

	}
	return TRUE; // say we processed the argument

}

//
// Purpose: 
//   Starts the service if possible.
//
// Parameters:
//   None
// 
// Return value:
//   None
//
BOOL CBaseService::StartService()
{

	SC_HANDLE schSCManager;
	SC_HANDLE schService;

	SERVICE_STATUS_PROCESS ssStatus; 
	DWORD dwOldCheckPoint; 
	DWORD dwStartTickCount;
	DWORD dwWaitTime;
	DWORD dwBytesNeeded;
	BOOL bRet = FALSE;

	// Get a handle to the SCM database. 
	LOG_INFO(L"WpmService start Begin, (%s).",L"Run");

	schSCManager = OpenSCManager( 
		NULL,                    // local computer
		NULL,                    // servicesActive database 
		SC_MANAGER_ALL_ACCESS);  // full access rights 

	if (NULL == schSCManager) 
	{
		LOG_ERR(L"WpmService OpenSCManager failed, (%s) , ErrorCode:  %d",L"Run",  GetLastError());
		DebugMsg(_T("OpenSCManager failed (%d)\n"), GetLastError());
		return bRet;
	}

	// Get a handle to the service.

	schService = OpenService( 
		schSCManager,         // SCM database 
		g_ServiceData.lpszServiceName,            // name of service 
		SERVICE_ALL_ACCESS);  // full access 

	if (schService == NULL)
	{ 
		schService = OpenService( 
			schSCManager,         // SCM database 
			SVC_OLDSERNAME,            // name of service 
			SERVICE_ALL_ACCESS);  // full access
		if (schService == NULL)
		{
			CloseServiceHandle(schSCManager);
			return bRet;
		}
	}    

	// Check the status in case the service is not stopped. 

	if (!QueryServiceStatusEx( 
		schService,                     // handle to service 
		SC_STATUS_PROCESS_INFO,         // information level
		(LPBYTE) &ssStatus,             // address of structure
		sizeof(SERVICE_STATUS_PROCESS), // size of structure
		&dwBytesNeeded ) )              // size needed if buffer is too small
	{
		DebugMsg(_T("QueryServiceStatusEx failed (%d)\n"), GetLastError());
		CloseServiceHandle(schService); 
		CloseServiceHandle(schSCManager);
		return bRet; 
	}

	// Check if the service is already running. It would be possible
	// to stop the service here, but for simplicity this example just returns. 

	if(ssStatus.dwCurrentState != SERVICE_STOPPED && ssStatus.dwCurrentState != SERVICE_STOP_PENDING)
	{
		DebugMsg(_T("Cannot start the service because it is already running\n"));
		CloseServiceHandle(schService); 
		CloseServiceHandle(schSCManager);
		return bRet; 
	}

	// Wait for the service to stop before attempting to start it.

	while (ssStatus.dwCurrentState == SERVICE_STOP_PENDING)
	{
		// Save the tick count and initial checkpoint.

		dwStartTickCount = GetTickCount();
		dwOldCheckPoint = ssStatus.dwCheckPoint;

		// Do not wait longer than the wait hint. A good interval is 
		// one-tenth of the wait hint but not less than 1 second  
		// and not more than 10 seconds. 

		dwWaitTime = ssStatus.dwWaitHint / 10;

		if( dwWaitTime < 1000 )
			dwWaitTime = 1000;
		else if ( dwWaitTime > 10000 )
			dwWaitTime = 10000;

		Sleep( dwWaitTime );

		// Check the status until the service is no longer stop pending. 

		if (!QueryServiceStatusEx( 
			schService,                     // handle to service 
			SC_STATUS_PROCESS_INFO,         // information level
			(LPBYTE) &ssStatus,             // address of structure
			sizeof(SERVICE_STATUS_PROCESS), // size of structure
			&dwBytesNeeded ) )              // size needed if buffer is too small
		{
			DebugMsg(_T("QueryServiceStatusEx failed (%d)\n"), GetLastError());
			CloseServiceHandle(schService); 
			CloseServiceHandle(schSCManager);
			return bRet; 
		}

		if ( ssStatus.dwCheckPoint > dwOldCheckPoint )
		{
			// Continue to wait and check.

			dwStartTickCount = GetTickCount();
			dwOldCheckPoint = ssStatus.dwCheckPoint;
		}
		else
		{
			if(GetTickCount()-dwStartTickCount > ssStatus.dwWaitHint)
			{
				DebugMsg(_T("Timeout waiting for service to stop\n"));
				CloseServiceHandle(schService); 
				CloseServiceHandle(schSCManager);
				return bRet; 
			}
		}
	}

	// Attempt to start the service.

	if (!::StartService(
		schService,  // handle to service 
		0,           // number of arguments 
		NULL) )      // no arguments 
	{
		DebugMsg(_T("StartService failed (%d)\n"), GetLastError());
		CloseServiceHandle(schService); 
		CloseServiceHandle(schSCManager);
		return bRet; 
	}
	else DebugMsg(_T("Service start pending...\n")); 

	// Check the status until the service is no longer start pending. 

	if (!QueryServiceStatusEx( 
		schService,                     // handle to service 
		SC_STATUS_PROCESS_INFO,         // info level
		(LPBYTE) &ssStatus,             // address of structure
		sizeof(SERVICE_STATUS_PROCESS), // size of structure
		&dwBytesNeeded ) )              // if buffer too small
	{
		DebugMsg(_T("QueryServiceStatusEx failed (%d)\n"), GetLastError());
		CloseServiceHandle(schService); 
		CloseServiceHandle(schSCManager);
		return FALSE; 
	}

	// Save the tick count and initial checkpoint.

	dwStartTickCount = GetTickCount();
	dwOldCheckPoint = ssStatus.dwCheckPoint;

	while (ssStatus.dwCurrentState == SERVICE_START_PENDING) 
	{ 
		// Do not wait longer than the wait hint. A good interval is 
		// one-tenth the wait hint, but no less than 1 second and no 
		// more than 10 seconds. 

		dwWaitTime = ssStatus.dwWaitHint / 10;

		if( dwWaitTime < 1000 )
			dwWaitTime = 1000;
		else if ( dwWaitTime > 10000 )
			dwWaitTime = 10000;

		Sleep( dwWaitTime );

		// Check the status again. 

		if (!QueryServiceStatusEx( 
			schService,             // handle to service 
			SC_STATUS_PROCESS_INFO, // info level
			(LPBYTE) &ssStatus,             // address of structure
			sizeof(SERVICE_STATUS_PROCESS), // size of structure
			&dwBytesNeeded ) )              // if buffer too small
		{
			DebugMsg(_T("QueryServiceStatusEx failed (%d)\n"), GetLastError());
			break; 
		}

		if ( ssStatus.dwCheckPoint > dwOldCheckPoint )
		{
			// Continue to wait and check.

			dwStartTickCount = GetTickCount();
			dwOldCheckPoint = ssStatus.dwCheckPoint;
		}
		else
		{
			if(GetTickCount()-dwStartTickCount > ssStatus.dwWaitHint)
			{
				// No progress made within the wait hint.
				break;
			}
		}
	} 

	// Determine whether the service is running.

	if (ssStatus.dwCurrentState == SERVICE_RUNNING) 
	{
		LOG_INFO(L"WpmService started successfully, (%s).",L"Run");
		DebugMsg(_T("Service started successfully.\n")); 
		bRet = TRUE;
	}
	else 
	{ 
		LOG_ERR(L"WpmService not started., (%s).",L"Run");
		DebugMsg(_T("Service not started. \n"));
		DebugMsg(_T(" Current State: %d\n"), ssStatus.dwCurrentState); 
		DebugMsg(_T("Exit Code: %d\n"), ssStatus.dwWin32ExitCode); 
		DebugMsg(_T("Check Point: %d\n"), ssStatus.dwCheckPoint); 
		DebugMsg(_T("Wait Hint: %d\n"), ssStatus.dwWaitHint); 
	} 

	LOG_INFO(L"WpmService start End, (%s).",L"Run");

	CloseServiceHandle(schService); 
	CloseServiceHandle(schSCManager);
	return bRet;

}


BOOL StopDependentServices(SC_HANDLE schService,SC_HANDLE schSCManager)
{

	DWORD i;
	DWORD dwBytesNeeded;
	DWORD dwCount;

	LPENUM_SERVICE_STATUS   lpDependencies = NULL;
	ENUM_SERVICE_STATUS     ess;
	SC_HANDLE               hDepService;
	SERVICE_STATUS_PROCESS  ssp;

	DWORD dwStartTime = GetTickCount();
	DWORD dwTimeout = 30000; // 30-second time-out

	// Pass a zero-length buffer to get the required buffer size.
	if ( EnumDependentServices( schService, SERVICE_ACTIVE, 
		lpDependencies, 0, &dwBytesNeeded, &dwCount ) ) 
	{
		// If the Enum call succeeds, then there are no dependent
		// services, so do nothing.
		return TRUE;
	} 
	else 
	{
		if ( GetLastError() != ERROR_MORE_DATA )
			return FALSE; // Unexpected error

		// Allocate a buffer for the dependencies.
		lpDependencies = (LPENUM_SERVICE_STATUS) HeapAlloc( 
			GetProcessHeap(), HEAP_ZERO_MEMORY, dwBytesNeeded );

		if ( !lpDependencies )
			return FALSE;

		__try {
			// Enumerate the dependencies.
			if ( !EnumDependentServices( schService, SERVICE_ACTIVE, 
				lpDependencies, dwBytesNeeded, &dwBytesNeeded,
				&dwCount ) )
				return FALSE;

			for ( i = 0; i < dwCount; i++ ) 
			{
				ess = *(lpDependencies + i);
				// Open the service.
				hDepService = OpenService( schSCManager, 
					ess.lpServiceName, 
					SERVICE_STOP | SERVICE_QUERY_STATUS );

				if ( !hDepService )
					return FALSE;

				__try {
					// Send a stop code.
					if ( !ControlService( hDepService, 
						SERVICE_CONTROL_STOP,
						(LPSERVICE_STATUS) &ssp ) )
						return FALSE;

					// Wait for the service to stop.
					while ( ssp.dwCurrentState != SERVICE_STOPPED ) 
					{
						Sleep( ssp.dwWaitHint );
						if ( !QueryServiceStatusEx( 
							hDepService, 
							SC_STATUS_PROCESS_INFO,
							(LPBYTE)&ssp, 
							sizeof(SERVICE_STATUS_PROCESS),
							&dwBytesNeeded ) )
							return FALSE;

						if ( ssp.dwCurrentState == SERVICE_STOPPED )
							break;

						if ( GetTickCount() - dwStartTime > dwTimeout )
							return FALSE;
					}
				} 
				__finally 
				{
					// Always release the service handle.
					CloseServiceHandle( hDepService );
				}
			}
		} 
		__finally 
		{
			// Always free the enumeration buffer.
			HeapFree( GetProcessHeap(), 0, lpDependencies );
		}
	} 
	return TRUE;

}

// Purpose: 
//   Stops the service.
//
// Parameters:
//   None
// 
// Return value:
//   None
//
BOOL CBaseService::StopService()
{

	SC_HANDLE schSCManager;
	SC_HANDLE schService;

		SERVICE_STATUS_PROCESS ssp;
		DWORD dwStartTime = GetTickCount();
		DWORD dwBytesNeeded;
		DWORD dwTimeout = 30000; // 30-second time-out

		BOOL bRet = FALSE;

		// Get a handle to the SCM database. 

		schSCManager = OpenSCManager( 
			NULL,                    // local computer
			NULL,                    // ServicesActive database 
			SC_MANAGER_ALL_ACCESS);  // full access rights 

		if (NULL == schSCManager) 
		{
			DebugMsg(_T("OpenSCManager failed (%d)\n"), GetLastError());
			return bRet;
		}

		// Get a handle to the service.

		schService = OpenService( 
			schSCManager,         // SCM database 
			g_ServiceData.lpszServiceName,             // name of service 
			SERVICE_STOP | 
			SERVICE_QUERY_STATUS | 
			SERVICE_ENUMERATE_DEPENDENTS);  

		if (schService == NULL)
		{ 
			DebugMsg(_T("OpenService failed (%d)\n"), GetLastError()); 
			CloseServiceHandle(schSCManager);
			return bRet;
		}    

		// Make sure the service is not already stopped.

		if ( !QueryServiceStatusEx( 
			schService, 
			SC_STATUS_PROCESS_INFO,
			(LPBYTE)&ssp, 
			sizeof(SERVICE_STATUS_PROCESS),
			&dwBytesNeeded ) )
		{
			DebugMsg(_T("QueryServiceStatusEx failed (%d)\n"), GetLastError()); 
			goto stop_cleanup;
		}

		if ( ssp.dwCurrentState == SERVICE_STOPPED )
		{
			DebugMsg(_T("Service is already stopped.\n"));
			bRet = TRUE;
			goto stop_cleanup;
		}

		// If a stop is pending, wait for it.

		while ( ssp.dwCurrentState == SERVICE_STOP_PENDING ) 
		{
			DebugMsg(_T("Service stop pending...\n"));
			Sleep( ssp.dwWaitHint );
			if ( !QueryServiceStatusEx( 
				schService, 
				SC_STATUS_PROCESS_INFO,
				(LPBYTE)&ssp, 
				sizeof(SERVICE_STATUS_PROCESS),
				&dwBytesNeeded ) )
			{
				DebugMsg(_T("QueryServiceStatusEx failed (%d)\n"), GetLastError()); 
				goto stop_cleanup;
			}

			if ( ssp.dwCurrentState == SERVICE_STOPPED )
			{
				DebugMsg(_T("Service stopped successfully.\n"));
				bRet = TRUE;
				goto stop_cleanup;
			}

			if ( GetTickCount() - dwStartTime > dwTimeout )
			{
				DebugMsg(_T("Service stop timed out.\n"));
				goto stop_cleanup;
			}
		}

		// If the service is running, dependencies must be stopped first.

		StopDependentServices(schService,schSCManager);

		// Send a stop code to the service.

		if ( !ControlService( 
			schService, 
			SERVICE_CONTROL_STOP, 
			(LPSERVICE_STATUS) &ssp ) )
		{
			DebugMsg(_T("ControlService failed (%d)\n"), GetLastError() );
			goto stop_cleanup;
		}

		// Wait for the service to stop.

		while ( ssp.dwCurrentState != SERVICE_STOPPED ) 
		{
			Sleep( ssp.dwWaitHint );
			if ( !QueryServiceStatusEx( 
				schService, 
				SC_STATUS_PROCESS_INFO,
				(LPBYTE)&ssp, 
				sizeof(SERVICE_STATUS_PROCESS),
				&dwBytesNeeded ) )
			{
				DebugMsg(_T("QueryServiceStatusEx failed (%d)\n"), GetLastError() );
				goto stop_cleanup;
			}

			if ( ssp.dwCurrentState == SERVICE_STOPPED )
				break;

			if ( GetTickCount() - dwStartTime > dwTimeout )
			{
				DebugMsg(_T("Wait timed out\n" ));
				goto stop_cleanup;
			}
		}
		DebugMsg(_T("Service stopped successfully\n"));
		bRet = TRUE;

stop_cleanup:
		CloseServiceHandle(schService); 
		CloseServiceHandle(schSCManager);
		return bRet;

}

BOOL CBaseService::Uninstall()
{
    // Open the Service Control Manager
    SC_HANDLE hSCM = ::OpenSCManager(NULL, // local machine
                                     NULL, // ServicesActive database
                                     SC_MANAGER_ALL_ACCESS); // full access
    if (!hSCM) return FALSE;

    BOOL bResult = FALSE;
    SC_HANDLE hService = ::OpenService(hSCM, g_ServiceData.lpszServiceName, DELETE);
    if (hService) {
        if (::DeleteService(hService)) {
            LogEvent(EVENTLOG_INFORMATION_TYPE, EVMSG_REMOVED, g_ServiceData.lpszServiceName);
            bResult = TRUE;
        } else {
            LogEvent(EVENTLOG_ERROR_TYPE, EVMSG_NOTREMOVED, g_ServiceData.lpszServiceName);
        }
        ::CloseServiceHandle(hService);
    }
    
    ::CloseServiceHandle(hSCM);
    return bResult;

}

///////////////////////////////////////////////////////////////////////////////////////
// Logging functions

// This function makes an entry into the application event log
void CBaseService::LogEvent(WORD wType, DWORD dwID,
                          const _TCHAR* pszS1,
                          const _TCHAR* pszS2,
                          const _TCHAR* pszS3)
{

    const _TCHAR* ps[3];
    ps[0] = pszS1;
    ps[1] = pszS2;
    ps[2] = pszS3;

    int iStr = 0;
    for (int i = 0; i < 3; i++) {
        if (ps[i] != NULL) iStr++;
    }
        
    // Check the event source has been registered and if
    // not then register it now
    if (!m_hEventSource) {
        m_hEventSource = ::RegisterEventSource(NULL,  // local machine
                                               g_ServiceData.lpszServiceName); // source name
    }

    if (m_hEventSource) {
        ::ReportEvent(m_hEventSource,
                      wType,
                      0,
                      dwID,
                      NULL, // sid
                      iStr,
                      0,
                      ps,
                      NULL);
    }

}

//////////////////////////////////////////////////////////////////////////////////////////////
// Service startup and registration

BOOL CBaseService::OnRunService()
{
	SERVICE_TABLE_ENTRY st[] = {
		{g_ServiceData.lpszServiceName, ServiceMain},
		{NULL, NULL}
	};
	
	DebugMsg( _T("Calling StartServiceCtrlDispatcher()"));
	BOOL b = ::StartServiceCtrlDispatcher(st);
	DebugMsg( _T("Returned from StartServiceCtrlDispatcher()"));
	return b;  

}

// static member function (callback)
void CBaseService::ServiceMain(DWORD dwArgc, LPTSTR* lpszArgv)
{
    // Get a pointer to the C++ object
    CBaseService* pService = g_pService;
    
    pService->DebugMsg( _T("Entering CNTService::ServiceMain()"));

	// Register the control request handler
	pService->m_Status.dwCurrentState = SERVICE_START_PENDING;
	pService->m_hServiceStatus = RegisterServiceCtrlHandlerEx(g_ServiceData.lpszServiceName, (LPHANDLER_FUNCTION_EX)Handler,0);
	if (pService->m_hServiceStatus == NULL) {
		pService->LogEvent(EVENTLOG_ERROR_TYPE, EVMSG_CTRLHANDLERNOTINSTALLED);
		return;
    }

#ifdef _DEBUG
	Sleep(10000);
#endif

    // Start the initialisation
    if (pService->Initialize()) {

        // Do the real work. 
        // When the Run function returns, the service has stopped.
        pService->m_bIsRunning = TRUE;
        pService->m_Status.dwWin32ExitCode = 0;
        pService->m_Status.dwCheckPoint = 0;
        pService->m_Status.dwWaitHint = 0;
        pService->Run();
    }

    // Tell the service manager we are stopped
    pService->SetStatus(SERVICE_STOPPED);

    pService->DebugMsg( _T("Leaving CNTService::ServiceMain()"));

}

///////////////////////////////////////////////////////////////////////////////////////////
// status functions

void CBaseService::SetStatus(DWORD dwState)
{
    DebugMsg( _T("CNTService::SetStatus(%lu, %lu)"), dwState, m_Status.dwCheckPoint);
	if (dwState == SERVICE_RUNNING || SERVICE_STOPPED == dwState)
	{
		m_Status.dwCheckPoint = 0;
	}
	else
	{
		m_Status.dwCheckPoint ++;
	}
    m_Status.dwCurrentState = dwState;
    ::SetServiceStatus(m_hServiceStatus, &m_Status);

}

///////////////////////////////////////////////////////////////////////////////////////////
// Service initialization
// make sure the initialize process is less than 30s
BOOL CBaseService::Initialize()
{
    DebugMsg( _T("Entering CNTService::Initialize()"));

    // Start the initialization
    SetStatus(SERVICE_START_PENDING);
    
    // Perform the actual initialization
    BOOL bResult = OnInit(); 
    
    // Set final state
    m_Status.dwWin32ExitCode = GetLastError();
    m_Status.dwCheckPoint = 0;
    m_Status.dwWaitHint = 0;
    if (!bResult) {
        LogEvent(EVENTLOG_ERROR_TYPE, EVMSG_FAILEDINIT);
        SetStatus(SERVICE_STOPPED);
        return FALSE;    
    }
    
    LogEvent(EVENTLOG_INFORMATION_TYPE, EVMSG_STARTED);
    SetStatus(SERVICE_RUNNING);

    DebugMsg( _T("Leaving CNTService::Initialize()"));
    return TRUE;
}

///////////////////////////////////////////////////////////////////////////////////////////////
// main function to do the real work of the service

// This function performs the main work of the service. 
// When this function returns the service has stopped.
void CBaseService::Run()
{
    DebugMsg( _T("Entering CNTService::Run()"));

    while (m_bIsRunning) {
        DebugMsg( _T("Sleeping..."));
        Sleep(5000);
    }

    // nothing more to do
    DebugMsg( _T("Leaving CNTService::Run()"));

}

//////////////////////////////////////////////////////////////////////////////////////
// Control request handlers

// static member function (callback) to handle commands from the
// service control manager
void CBaseService::Handler(DWORD dwOpcode,DWORD evtype, PVOID evdata, PVOID Context)
{

    // Get a pointer to the object
    CBaseService* pService = g_pService;
    
    pService->DebugMsg( _T("CeService::Handler(%lu)"), dwOpcode);
    switch (dwOpcode) {
    case SERVICE_CONTROL_STOP: // 1
        pService->SetStatus(SERVICE_STOP_PENDING);
        pService->OnStop();
        pService->m_bIsRunning = FALSE;
        pService->LogEvent(EVENTLOG_INFORMATION_TYPE, EVMSG_STOPPED);
        break;

    case SERVICE_CONTROL_PAUSE: // 2
        pService->OnPause();
        break;

    case SERVICE_CONTROL_CONTINUE: // 3
        pService->OnContinue();
        break;

    case SERVICE_CONTROL_INTERROGATE: // 4
        pService->OnInterrogate();
        break;

    case SERVICE_CONTROL_SHUTDOWN: // 5
        pService->OnShutdown();
        break;

    default:
        if (dwOpcode >= SERVICE_CONTROL_USER) {
            if (!pService->OnUserControl(dwOpcode)) {
                pService->LogEvent(EVENTLOG_ERROR_TYPE, EVMSG_BADREQUEST);
            }
        } else {
            pService->LogEvent(EVENTLOG_ERROR_TYPE, EVMSG_BADREQUEST);
        }
        break;
    }

    // Report current status
    pService->DebugMsg( _T("Updating status (%lu, %lu)"),
                       pService->m_hServiceStatus,
                       pService->m_Status.dwCurrentState);
    ::SetServiceStatus(pService->m_hServiceStatus, &pService->m_Status);

}
        
// Called when the service is first initialized
BOOL CBaseService::OnInit()
{

    DebugMsg( _T("CeService::OnInit()"));
	return TRUE;

}

// Called when the service control manager wants to stop the service
void CBaseService::OnStop()
{

    DebugMsg( _T("CeService::OnStop()"));
	m_bIsRunning = FALSE;

}

// called when the service is interrogated
void CBaseService::OnInterrogate()
{
    DebugMsg( _T("CeService::OnInterrogate()"));
}

// called when the service is paused
void CBaseService::OnPause()
{
    DebugMsg( _T("CeService::OnPause()"));
}

// called when the service is continued
void CBaseService::OnContinue()
{
    DebugMsg( _T("CeService::OnContinue()"));
}

// called when the service is shut down
void CBaseService::OnShutdown()
{
    DebugMsg( _T("CeService::OnShutdown()"));
}

// called when the service gets a user control message
BOOL CBaseService::OnUserControl(DWORD dwOpcode)
{
    DebugMsg( _T("CeService::OnUserControl(%8.8lXH)"), dwOpcode);
    return FALSE; // say not handled
}

////////////////////////////////////////////////////////////////////////////////////////////
// Debugging support

void CBaseService::DebugMsg(const _TCHAR* pszFormat, ...)
{

#ifdef _DEBUG_MSG
    _TCHAR buf[1024];
	ZeroMemory( buf, 1024*sizeof(_TCHAR ) );
    _stprintf_s(buf, 1024, _T("[%s](%lu): "), g_ServiceData.lpszServiceName, GetCurrentThreadId());
	va_list arglist;
	va_start( arglist, pszFormat );
	int nBuffSize= _tcslen(buf);
    _vstprintf_s( &buf[nBuffSize], 1024-nBuffSize, pszFormat, arglist);
	va_end( arglist );
	nBuffSize= _tcslen(buf);
    _tcscat_s( buf, 1024-nBuffSize, _T("\n") );
    OutputDebugString( buf );
#endif

}
