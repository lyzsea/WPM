/************************************************************************/
/* 
    Date : 3/26/2013
	File : ESVCTOKENUTILITIES.H
    Author : Eadwin Li
    Function :  
	Version : 1.0
	Copyright (c) 2012 - 2013 iWinPhone Studio
*/
/************************************************************************/   

#pragma once

#include <windows.h>
#include <string>

namespace Utilities{namespace svc{

#define FLAG_SET(flag,value)					((flag)=(flag) | (value))
#define FLAG_ISSET(flag,value)					(((flag) & (value))==(value))
#define FLAG_CLEAN(flag,value)					((flag)=(flag) & (~(value)))

#define FLAG_MARK_SET(flag,mark,value)			((flag)=((flag) & (~(mark))) | ((value) & (mark)))
#define FLAG_MARK_ISSET(flag,mark,value)		((((flag) & (value)) & (mark))==((value) & (mark)))
#define FLAG_MARK_CLEAN(flag,mark)				((flag)=(flag) & (~(mark)))
#define FLAG_MARK_GET(flag,mark)				((flag) & (mark))

	// 这三个标志位必须有一个
#define GES_SESSIONONLY						0x00000001			// 仅仅返回SessionID对应的Explorer的PID
#define GES_FIRST							0x00000002			// 返回发现的第一个Explorer的PID
#define GES_MINORSESSION					0x00000004			// 返回发现的所有的Explorer中，Session ID最小的那个的PID

	typedef struct tagDOSDEVICEINFO
	{
		TCHAR tcDosName[4];
		TCHAR tcDeviceName[MAX_PATH*2];
	}DOSDEVICEINFO,*PDOSDEVICEINFO;

	typedef struct _MY_TOKEN_MANDATORY_LABEL {
	SID_AND_ATTRIBUTES Label;
} MY_TOKEN_MANDATORY_LABEL, *PMY_TOKEN_MANDATORY_LABEL;


typedef struct _MY_TOKEN_LINKED_TOKEN {
	HANDLE LinkedToken;
} MY_TOKEN_LINKED_TOKEN, *PMY_TOKEN_LINKED_TOKEN;

typedef enum _MY_TOKEN_INFORMATION_CLASS {
	/*
	TokenUser = 1,
	TokenGroups,
	TokenPrivileges,
	TokenOwner,
	TokenPrimaryGroup,
	TokenDefaultDacl,
	TokenSource,
	TokenType,
	TokenImpersonationLevel,
	TokenStatistics,
	TokenRestrictedSids,
	TokenSessionId,
	TokenGroupsAndPrivileges,
	TokenSessionReference,
	TokenSandBoxInert,
	TokenAuditPolicy,
	TokenOrigin,*/
	MYTokenElevationType  = 18, // MaxTokenInfoClass
	MYTokenLinkedToken,
	MYTokenElevation,
	MYTokenHasRestrictions,
	MYTokenAccessInformation,
	MYTokenVirtualizationAllowed,
	MYTokenVirtualizationEnabled,
	MYTokenIntegrityLevel,
	MYTokenUIAccess,
	MYTokenMandatoryPolicy,
	MYTokenLogonSid,
	//MaxTokenInfoClass  // MaxTokenInfoClass should always be the last enum
} MY_TOKEN_INFORMATION_CLASS, *PMY_TOKEN_INFORMATION_CLASS;


	class CeSvcTokenUtilities
	{
	public:
		CeSvcTokenUtilities(){}
		~CeSvcTokenUtilities(){}

		static HRESULT ExecutebyToken( 
			const std::wstring& strExePath, 
			const std::wstring& strCmdline, 
			BOOL bShow,
			HANDLE m_hToken,
			DWORD& dwProcessId,
			bool bWait = false
			);
		static HRESULT ObtainExplorerToken(HANDLE& hTokenObtain,BOOL bNeedAdmin = TRUE);
	private:
		static BOOL GetDosDeviceInfo(PDOSDEVICEINFO pInfo,DWORD& dwInfoNum);
		static DWORD GetExplorerIDBySessionID(DWORD dwFlag,DWORD dwSessionID);
		static BOOL GetProcessImage(DWORD dwPID,LPTSTR lpszImage,PDOSDEVICEINFO pInfo,DWORD dwInfoNum);
	};


}}