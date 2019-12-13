#include "APIService.h"

#include "cmdAnalyze/Cmdline.h"
#include "StrDefine.h"
#include "WPMService.h"

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

#include "../src/DumpLog/Log.h"

extern SERVICEDATA	g_ServiceData;

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
  if (EnumDependentServices(schService, SERVICE_ACTIVE,lpDependencies, 0, &dwBytesNeeded, &dwCount)) {
    // If the Enum call succeeds, then there are no dependent
    // services, so do nothing.
    return TRUE;
  } else {
    if (GetLastError() != ERROR_MORE_DATA) {
      return FALSE; // Unexpected error
    }
    // Allocate a buffer for the dependencies.
    lpDependencies = (LPENUM_SERVICE_STATUS) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwBytesNeeded);
    if (!lpDependencies) {
      return FALSE;
    }
    __try {
      // Enumerate the dependencies.
      if (!EnumDependentServices(schService, SERVICE_ACTIVE, lpDependencies, dwBytesNeeded, &dwBytesNeeded, &dwCount)) {
        return FALSE;
      }
      for (i = 0; i < dwCount; i++) {
        ess = *(lpDependencies + i);
        // Open the service.
        hDepService = OpenService(schSCManager, 
          ess.lpServiceName,
          SERVICE_STOP | SERVICE_QUERY_STATUS );
        if ( !hDepService ) {
          return FALSE;
        }
        __try {
          // Send a stop code.
          if ( !ControlService(hDepService, SERVICE_CONTROL_STOP,(LPSERVICE_STATUS)&ssp)) {
            return FALSE;
          }
          // Wait for the service to stop.
          while (ssp.dwCurrentState != SERVICE_STOPPED) {
            Sleep( ssp.dwWaitHint );
            if (!QueryServiceStatusEx(hDepService, 
              SC_STATUS_PROCESS_INFO,
              (LPBYTE)&ssp,
              sizeof(SERVICE_STATUS_PROCESS),
              &dwBytesNeeded)) {
                return FALSE;
            }
            if (ssp.dwCurrentState == SERVICE_STOPPED) {
              break;
            }
            if (GetTickCount() - dwStartTime > dwTimeout) {
              return FALSE;
            }
          }
        } __finally {
          // Always release the service handle.
          CloseServiceHandle( hDepService );
        }
      }
    } __finally {
      // Always free the enumeration buffer.
      HeapFree( GetProcessHeap(), 0, lpDependencies );
    }
  } 
  return TRUE;
}

namespace APIService {

BOOL UninstallService()
{
  if (IsInstalled()) {
    if (APIService::Uninstall()) {
      _TCHAR szFilePath[_MAX_PATH];
      ZeroMemory( szFilePath, MAX_PATH * sizeof( _TCHAR ) );
      ::GetModuleFileName(NULL, szFilePath, sizeof(szFilePath));
    } else {
      return FALSE;
    }
  }
  return TRUE;
}

BOOL StopService()
{
  SC_HANDLE schSCManager;
  SC_HANDLE schService;
  SERVICE_STATUS_PROCESS ssp;
  DWORD dwStartTime = GetTickCount();
  DWORD dwBytesNeeded;
  DWORD dwTimeout = 30000; // 30-second time-out
  BOOL bRet = FALSE;
  schSCManager = OpenSCManager(NULL,                    // local computer
    NULL,                    // ServicesActive database 
    SC_MANAGER_ALL_ACCESS);  // full access rights 
  if (NULL == schSCManager) {
    return bRet;
  }
  schService = OpenService(schSCManager,         // SCM database 
    g_ServiceData.lpszServiceName,             // name of service 
    SERVICE_STOP | SERVICE_QUERY_STATUS | SERVICE_ENUMERATE_DEPENDENTS);
  if (schService == NULL) { 
    CloseServiceHandle(schSCManager);
    return bRet;
  }  
  // Make sure the service is not already stopped.
  if (!QueryServiceStatusEx(schService, SC_STATUS_PROCESS_INFO, (LPBYTE)&ssp, 
    sizeof(SERVICE_STATUS_PROCESS), &dwBytesNeeded)) {
      goto stop_cleanup;
  }
  if (ssp.dwCurrentState == SERVICE_STOPPED) {
    bRet = TRUE;
    goto stop_cleanup;
  }
  // If a stop is pending, wait for it.
  while (ssp.dwCurrentState == SERVICE_STOP_PENDING) {
    Sleep( ssp.dwWaitHint );
    if (!QueryServiceStatusEx(schService, SC_STATUS_PROCESS_INFO,
      (LPBYTE)&ssp, sizeof(SERVICE_STATUS_PROCESS),
      &dwBytesNeeded)) {
        goto stop_cleanup;
    }
    if (ssp.dwCurrentState == SERVICE_STOPPED) {
      bRet = TRUE;
      goto stop_cleanup;
    }
    if (GetTickCount() - dwStartTime > dwTimeout) {
      goto stop_cleanup;
    }
  }
  // If the service is running, dependencies must be stopped first.
  StopDependentServices(schService,schSCManager);
  // Send a stop code to the service.
  if (!ControlService(schService, SERVICE_CONTROL_STOP, (LPSERVICE_STATUS) &ssp)) {
    goto stop_cleanup;
  }
  // Wait for the service to stop.
  while (ssp.dwCurrentState != SERVICE_STOPPED) {
    Sleep( ssp.dwWaitHint );
    if (!QueryServiceStatusEx(schService, SC_STATUS_PROCESS_INFO,
      (LPBYTE)&ssp, sizeof(SERVICE_STATUS_PROCESS),
      &dwBytesNeeded)) {
        goto stop_cleanup;
    }
    if (ssp.dwCurrentState == SERVICE_STOPPED) {
      break;
    }
    if (GetTickCount() - dwStartTime > dwTimeout) {
      goto stop_cleanup;
    }
  }
  bRet = TRUE;
stop_cleanup:
  CloseServiceHandle(schService); 
  CloseServiceHandle(schSCManager);
  return bRet;
}

BOOL StartService()
{
  SC_HANDLE schSCManager;
  SC_HANDLE schService;
  SERVICE_STATUS_PROCESS ssStatus; 
  DWORD dwOldCheckPoint; 
  DWORD dwStartTickCount;
  DWORD dwWaitTime;
  DWORD dwBytesNeeded;
  BOOL bRet = FALSE;
  LOG_INFO(L"WpmService start Begin, (%s).",L"Run");
  schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
  if (NULL == schSCManager) {
    LOG_ERR(L"WpmService OpenSCManager failed, (%s) , ErrorCode:  %d",L"Run",  GetLastError());
    return bRet;
  }
  schService = OpenService(schSCManager, g_ServiceData.lpszServiceName, SERVICE_ALL_ACCESS);
  if (schService == NULL) {
    schService = OpenService(schSCManager, SVC_OLDSERNAME, SERVICE_ALL_ACCESS);
    if (schService == NULL) {
      CloseServiceHandle(schSCManager);
      return bRet;
    }
  }
  if (!QueryServiceStatusEx(schService, SC_STATUS_PROCESS_INFO, (LPBYTE) &ssStatus, sizeof(SERVICE_STATUS_PROCESS), &dwBytesNeeded)) {
    CloseServiceHandle(schService);
    CloseServiceHandle(schSCManager);
    return bRet; 
  }

  if(ssStatus.dwCurrentState != SERVICE_STOPPED && ssStatus.dwCurrentState != SERVICE_STOP_PENDING) {
    CloseServiceHandle(schService); 
    CloseServiceHandle(schSCManager);
    return bRet; 
  }

  while (ssStatus.dwCurrentState == SERVICE_STOP_PENDING) {
    dwStartTickCount = GetTickCount();
    dwOldCheckPoint = ssStatus.dwCheckPoint;
    dwWaitTime = ssStatus.dwWaitHint / 10;
    if( dwWaitTime < 1000 ) {
      dwWaitTime = 1000;
    } else if ( dwWaitTime > 10000 ) {
      dwWaitTime = 10000;
    }
    Sleep( dwWaitTime );
    if (!QueryServiceStatusEx(schService, SC_STATUS_PROCESS_INFO, (LPBYTE) &ssStatus, sizeof(SERVICE_STATUS_PROCESS), &dwBytesNeeded)){
      CloseServiceHandle(schService);
      CloseServiceHandle(schSCManager);
      return bRet; 
    }
    if (ssStatus.dwCheckPoint > dwOldCheckPoint) {
      dwStartTickCount = GetTickCount();
      dwOldCheckPoint = ssStatus.dwCheckPoint;
    } else {
      if(GetTickCount()-dwStartTickCount > ssStatus.dwWaitHint) {
        CloseServiceHandle(schService);
        CloseServiceHandle(schSCManager);
        return bRet; 
      }
    }
  }
  if (!::StartService(schService, 0, NULL) ) {
    CloseServiceHandle(schService);
    CloseServiceHandle(schSCManager);
    return bRet; 
  } 
  if (!QueryServiceStatusEx(schService,SC_STATUS_PROCESS_INFO,(LPBYTE) &ssStatus,sizeof(SERVICE_STATUS_PROCESS),&dwBytesNeeded)) {
    CloseServiceHandle(schService);
    CloseServiceHandle(schSCManager);
    return FALSE;
  }
  dwStartTickCount = GetTickCount();
  dwOldCheckPoint = ssStatus.dwCheckPoint;
  while (ssStatus.dwCurrentState == SERVICE_START_PENDING) {
    dwWaitTime = ssStatus.dwWaitHint / 10;
    if(dwWaitTime < 1000) {
      dwWaitTime = 1000;
    } else if (dwWaitTime > 10000) {
      dwWaitTime = 10000;
    }
    Sleep( dwWaitTime );
    if (!QueryServiceStatusEx(schService, SC_STATUS_PROCESS_INFO, (LPBYTE) &ssStatus, sizeof(SERVICE_STATUS_PROCESS), &dwBytesNeeded)) {
      break;
    }
    if (ssStatus.dwCheckPoint > dwOldCheckPoint) {
      dwStartTickCount = GetTickCount();
      dwOldCheckPoint = ssStatus.dwCheckPoint;
    } else {
      if(GetTickCount()-dwStartTickCount > ssStatus.dwWaitHint) {
        break;
      }
    }
  } 
  if (ssStatus.dwCurrentState == SERVICE_RUNNING) {
    LOG_INFO(L"WpmService started successfully, (%s).",L"Run");
    bRet = TRUE;
  } else {
    LOG_ERR(L"WpmService not started., (%s).",L"Run");
    LOG_ERR(_T(" Current State: %d\n"), ssStatus.dwCurrentState); 
    LOG_ERR(_T("Exit Code: %d\n"), ssStatus.dwWin32ExitCode); 
    LOG_ERR(_T("Check Point: %d\n"), ssStatus.dwCheckPoint); 
    LOG_ERR(_T("Wait Hint: %d\n"), ssStatus.dwWaitHint); 
  }
  LOG_INFO(L"WpmService start End, (%s).",L"Run");
  CloseServiceHandle(schService); 
  CloseServiceHandle(schSCManager);
  return bRet;
}

BOOL UninstAndStopService()
{
  if (IsInstalled()) {
    if(!StopService()) {
      return FALSE;
    }
    if (!Uninstall()) {
      return FALSE;
    }
  }
  return TRUE;
}

BOOL InstAndStartService(BOOL bAfterXP, bool bDelayStartup)
{
  if (!IsInstalled()) {
    if (!Install(bAfterXP,bDelayStartup)) {
      return FALSE;
    }
  }
  return StartService();
}

BOOL IsInstalled()
{
  BOOL bResult = FALSE;
  SC_HANDLE hSCM = ::OpenSCManager(NULL,NULL,SC_MANAGER_ALL_ACCESS);
  if (hSCM) {
    SC_HANDLE hService = ::OpenService(hSCM, g_ServiceData.lpszServiceName, SERVICE_QUERY_CONFIG);
    if (hService) {
      bResult = TRUE;
      ::CloseServiceHandle(hService);
    }
    ::CloseServiceHandle(hSCM);
  }
  return bResult;
}

BOOL InstallService(BOOL bAfterXP, bool bDelayStartup)
{
  // Request to install.
  if (IsInstalled()) {
    // change config
    BOOL bResult = TRUE;
    SC_HANDLE hSCM = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (hSCM != NULL) {
      SC_HANDLE hService = ::OpenService( hSCM,
        g_ServiceData.lpszServiceName, 
        SERVICE_QUERY_CONFIG | SERVICE_CHANGE_CONFIG);
      if (hService != NULL) {
        ChangeServiceConfig(hService,
          SERVICE_WIN32_OWN_PROCESS,
          SERVICE_AUTO_START, // 默认自动启动服务
          SERVICE_ERROR_NORMAL,
          g_ServiceData.lpszBinaryPathName,
          bAfterXP ? SVC_ORDERGROUP_1 : SVC_ORDERGROUP_2,
          NULL,
          _T("RPCSS\0"),
          NULL,
          NULL,
          g_ServiceData.lpszDisplayName);
        ::CloseServiceHandle(hService);
      }
      ::CloseServiceHandle(hSCM);
    }
    return bResult;
  } else {
    // Try and install the copy that's running
    if (!Install(bAfterXP,bDelayStartup)) {
      return FALSE;
    }
  }
  return TRUE; // say we processed the argument
}

BOOL Uninstall()
{
  SC_HANDLE hSCM = ::OpenSCManager(NULL,NULL,SC_MANAGER_ALL_ACCESS);
  if (!hSCM){
    return FALSE;
  }
  BOOL bResult = FALSE;
  SC_HANDLE hService = ::OpenService(hSCM, g_ServiceData.lpszServiceName, DELETE);
  if (hService) {
    if (::DeleteService(hService)) {
      bResult = TRUE;
    } 
    ::CloseServiceHandle(hService);
  }
  ::CloseServiceHandle(hSCM);
  return bResult;
}

BOOL Install(BOOL bAfterXP, bool bDelayStartup)
{
  SC_HANDLE hSCM = ::OpenSCManager(NULL,NULL,SC_MANAGER_ALL_ACCESS);
  if (!hSCM) {
    return FALSE;
  }
  std::wstring strBinary = g_ServiceData.lpszBinaryPathName;
  strBinary += L" -";
  strBinary += CMDLINE_PARAM_NAME_RUANASSVR;
  SC_HANDLE hService = ::CreateService( hSCM,						// SCManager database
    g_ServiceData.lpszServiceName,		// name of service
    g_ServiceData.lpszDisplayName,		// name to display
    SERVICE_ALL_ACCESS,					// desired access
    g_ServiceData.dwServiceType,		// service type
    g_ServiceData.dwStartType,			// start type
    g_ServiceData.dwErrorControl,       // error control type
    strBinary.c_str(),	// service's binary
    bAfterXP ? SVC_ORDERGROUP_1 : SVC_ORDERGROUP_2,
    g_ServiceData.dwTagId!=0 ? &g_ServiceData.dwTagId : NULL,				// no tag identifier
    g_ServiceData.lpszDependencies,		// dependencies
    NULL,								// LocalSystem account
    NULL);								// no password
  if ( !hService ) {
    ::CloseServiceHandle(hSCM);
    return FALSE;
  }
  SERVICE_DESCRIPTION des;
  des.lpDescription=g_ServiceData.lpszDescription;
  ChangeServiceConfig2(hService,                // handle to service
    SERVICE_CONFIG_DESCRIPTION, // change: description
    &des);
  if (bDelayStartup == true) {// setup delay start up
    SERVICE_DELAYED_AUTO_START_INFO info = { TRUE };
    ::ChangeServiceConfig2( hService,  SERVICE_CONFIG_DELAYED_AUTO_START_INFO, &info);
  }
  // make registry entries to support logging messages
  // Add the source name as a subkey under the Application
  // key in the EventLog service portion of the registry.
  _TCHAR szKey[256];
  HKEY hKey = NULL;
  _tcscpy_s( szKey, 256, _T( "SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application\\")  );
  _tcscat_s( szKey, 256, g_ServiceData.lpszServiceName );
  if (::RegCreateKey(HKEY_LOCAL_MACHINE, szKey, &hKey) != ERROR_SUCCESS) {
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
  // tidy up
  ::CloseServiceHandle(hService);
  ::CloseServiceHandle(hSCM);
  return TRUE;
}

}
