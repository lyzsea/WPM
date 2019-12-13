#ifndef WPMV2_NEWGDP_SERVICE_H_
#define WPMV2_NEWGDP_SERVICE_H_
#pragma once

#include "../Update/GdpUpdate.h"

#include <tchar.h> 
#include <windows.h>


#define SERVICE_CONTROL_USER 128
#define SERVICE_CONTROL_STOP_ALL_VDISK_SRV		SERVICE_CONTROL_USER + 1
#define SERVICE_CONTROL_START_CLIENT		SERVICE_CONTROL_USER + 2
#define SERVICE_CONTROL_PROTECT_CLIENT		SERVICE_CONTROL_USER + 3
//#define SERVICE_CONTROL_LOGIN		SERVICE_CONTROL_USER + 4

#define MAX_SERVICE_NAME_LENGTH		260

typedef struct tagServiceData
{
	TCHAR lpszServiceName[MAX_SERVICE_NAME_LENGTH];		// name of service to start
	TCHAR lpszDisplayName[MAX_SERVICE_NAME_LENGTH];		// display name
	DWORD dwServiceType;								// type of service
	DWORD dwStartType;									// when to start service
	DWORD dwErrorControl;								// severity of service failure
	TCHAR lpszBinaryPathName[MAX_SERVICE_NAME_LENGTH];	// name of binary file
	TCHAR lpszLoadOrderGroup[MAX_SERVICE_NAME_LENGTH];	// name of load ordering group
	DWORD dwTagId;										// tag identifier
	TCHAR lpszDependencies[MAX_SERVICE_NAME_LENGTH];	// array of dependency names
	DWORD dwControlsAccepted;							// 可接受的控制
	TCHAR lpszDescription[MAX_SERVICE_NAME_LENGTH];		// 描述
}SERVICEDATA,*PSERVICEDATA;

class WPMService
{
public:
  WPMService(BOOL bAfterXP, bool bDelayStartup = false );
  virtual ~WPMService();

	BOOL OnRunService();
  
  int m_iMajorVersion;
  int m_iMinorVersion;
  SERVICE_STATUS_HANDLE m_hServiceStatus;
  SERVICE_STATUS m_Status;
  BOOL m_bIsRunning;
	bool	 m_bDelayStartup;
  BOOL m_bIsAfterXP;

protected:
	virtual void OnContinue(){}
	virtual void Run();
	virtual BOOL OnUserControl(DWORD dwOpcode) {return FALSE;}
	virtual BOOL OnInit();
	virtual void OnStop();
	virtual void OnInterrogate(){}
	virtual void OnShutdown(){}
	virtual void OnPause(){}


private:
	BOOL Initialize();
	void SetStatus(DWORD dwState);
	// static member functions
	static void WINAPI Handler(DWORD dwOpcode,DWORD evtype, PVOID evdata, PVOID Context);
	static void WINAPI ServiceMain(DWORD dwArgc, LPTSTR* lpszArgv);

  HANDLE m_hEventSource;

  void LogEvent(WORD wType, DWORD dwID,
    const _TCHAR* pszS1 = NULL,
    const _TCHAR* pszS2 = NULL,
    const _TCHAR* pszS3 = NULL);

  CGdpUpdate	m_update;
};

#endif  // WPMV2_NEWGDP_SERVICE_H_


