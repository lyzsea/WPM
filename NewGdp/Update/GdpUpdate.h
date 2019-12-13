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
#include <winsock.h>
#include "net/HttpDownload/HttpClient.h"

class CHttpDownloader1
{
public:
	CHttpDownloader1(std::wstring strUrl, std::wstring strFile);
	~CHttpDownloader1(void);

	bool Download();
private:
	bool DownloadFromProxy(std::wstring strUrl, std::wstring strFile);

	std::wstring m_strUrl;
	std::wstring m_strFile;
};

class CGdpUpdate
{
public:
	CGdpUpdate();


	virtual void Start();

	virtual  void Stop();
private:

	static void __ThreadFunc( PVOID lParam );

	void ThreadFunc();

virtual	bool DownPacket();
virtual	bool DownLoadConf();

	HANDLE m_hStopEvent;

	std::wstring m_strDir;
	std::wstring m_strUpdateExe;

	std::wstring m_strFielVer;
	std::wstring m_strUrl;
	std::wstring m_strMd5;

	bool GdpBase64Decode(std::wstring& strInput, std::wstring& strOutput);
};
