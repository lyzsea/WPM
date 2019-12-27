#include ".\..\captureScreen.h"
#include "WPMService.h"
#include "cmdAnalyze/Cmdline.h"
#include "Utilities/Singleton.h"
#include "StrDefine.h"
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



extern WPMService* g_pService; 
extern SERVICEDATA	g_ServiceData;

WPMService::WPMService(BOOL bAfterXP, bool bDelayStartup )
{
	m_bIsAfterXP = bAfterXP;

  m_iMajorVersion = 2;
  m_iMinorVersion = 0;
  m_hEventSource = NULL;

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

WPMService::~WPMService()
{
  if (m_hEventSource) {
    ::DeregisterEventSource(m_hEventSource);
  }
}



void WPMService::LogEvent(WORD wType, DWORD dwID,
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


#include "../src/DumpLog/Log.h"

BOOL WPMService::OnRunService()
{
  
	SERVICE_TABLE_ENTRY st[] = {{g_ServiceData.lpszServiceName, ServiceMain},{NULL, NULL}};
	BOOL b = ::StartServiceCtrlDispatcher(st);
  LOG_INFO(L"WPMService::OnRunService, (%b).",b);
	return b;
}

// static member function (callback)
void WPMService::ServiceMain(DWORD dwArgc, LPTSTR* lpszArgv)
{
  // Get a pointer to the C++ object
  WPMService* pService = g_pService;
  // Register the control request handler
	pService->m_Status.dwCurrentState = SERVICE_START_PENDING;
	pService->m_hServiceStatus = RegisterServiceCtrlHandlerEx(g_ServiceData.lpszServiceName, (LPHANDLER_FUNCTION_EX)Handler,0);
	if (pService->m_hServiceStatus == NULL) {
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
}

///////////////////////////////////////////////////////////////////////////////////////////
// status functions
void WPMService::SetStatus(DWORD dwState)
{
  if (dwState == SERVICE_RUNNING || SERVICE_STOPPED == dwState) {
		m_Status.dwCheckPoint = 0;
	} else {
		m_Status.dwCheckPoint ++;
	}
  m_Status.dwCurrentState = dwState;
  ::SetServiceStatus(m_hServiceStatus, &m_Status);
}
///////////////////////////////////////////////////////////////////////////////////////////
// Service initialization
// make sure the initialize process is less than 30s
BOOL WPMService::Initialize()
{
  // Start the initialization
  SetStatus(SERVICE_START_PENDING);
  // Perform the actual initialization
  BOOL bResult = OnInit(); 
  // Set final state
  m_Status.dwWin32ExitCode = GetLastError();
  m_Status.dwCheckPoint = 0;
  m_Status.dwWaitHint = 0;
  if (!bResult) {
    return FALSE;
  }
  SetStatus(SERVICE_RUNNING);
  return TRUE;
}

///////////////////////////////////////////////////////////////////////////////////////////////
// main function to do the real work of the service

// This function performs the main work of the service. 
// When this function returns the service has stopped.
void WPMService::Run()
{
  while (m_bIsRunning) {
    //Sleep(5000);
	Singleton<CCaptureScreen>::instance()->SetTcpServer();
  }
}

BOOL WPMService::OnInit() 
{
  //m_update.Start();
  return TRUE;
}

void WPMService::OnStop() 
{
  //m_update.Stop();
  m_bIsRunning = FALSE;
}

//////////////////////////////////////////////////////////////////////////////////////
// Control request handlers

// static member function (callback) to handle commands from the
// service control manager
void WPMService::Handler(DWORD dwOpcode,DWORD evtype, PVOID evdata, PVOID Context)
{
  // Get a pointer to the object
  WPMService* pService = g_pService;
  switch (dwOpcode) {
  case SERVICE_CONTROL_STOP: // 1
    pService->SetStatus(SERVICE_STOP_PENDING);
    pService->OnStop();
    pService->m_bIsRunning = FALSE;
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
      pService->OnUserControl(dwOpcode);
    } 
    break;
  }
  ::SetServiceStatus(pService->m_hServiceStatus, &pService->m_Status);
}
