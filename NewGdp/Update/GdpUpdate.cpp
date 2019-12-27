#include "gdpupdate.h"
#include "../Configure.h"
#include "Utilities/Singleton.h"
#include "Strings/StringsHelper.h"
#include "../Service/WinPrivilege.h"
#include "Safe/MD5.h"
#include "Safe/eBase64.h"
#include "Safe/ConvStr.h"
#include "json/json.h"
using namespace Json;

#include <ShlObj.h>
#include <fstream>
#include <algorithm>

#define  MAX_WSTRING (10*1024)
CHttpDownloader1::CHttpDownloader1(std::wstring strUrl, std::wstring strFile)
	: m_strUrl(strUrl)
	, m_strFile(strFile)
{
}


CHttpDownloader1::~CHttpDownloader1(void)
{
}

bool CHttpDownloader1::Download( )
{
	if (DownloadFromProxy(m_strUrl, m_strFile))
		return true;

	CHttpClient client;
	std::wstring strRedirection;
	HRESULT hr = client.DownLoadFile(m_strUrl.c_str(), m_strFile.c_str(), strRedirection);
	long lCount = 0;

	while (hr == E_REDIRECTION && strRedirection.length() > 0 && lCount ++ < 10)
	{
		std::wstring strUrl = strRedirection;
		hr = client.DownLoadFile(strUrl.c_str(), m_strFile.c_str(), strRedirection);
		if (hr == S_OK)
			break;
		Sleep(1000);
	}

	return S_OK == hr;
}

bool CHttpDownloader1::DownloadFromProxy( std::wstring strUrl, std::wstring strFile )
{
	CHttpClient client;
	TCHAR szIP[256] = {0};
	WORD dwPort	= 0;
	HRESULT hr = S_FALSE;
	if(client.GetProxyEnable(szIP, 256, dwPort))
	{
		std::wstring strRedirection;
		hr = client.DownLoadFileFromProxy(szIP, dwPort, strUrl.c_str(), strFile.c_str(), strRedirection);
		long lCount = 0;
		while (hr == E_REDIRECTION && strRedirection.length() > 0 && lCount ++ < 10)
		{
			std::wstring strUrl = strRedirection;
			hr = client.DownLoadFileFromProxy(szIP, dwPort, strUrl.c_str(), strFile.c_str(), strRedirection);
			if (hr == S_OK)
				break;
			Sleep(1000);
		}
	}

	return S_OK == hr;
}
//////////////////////////////////////////////////////////////////////////

CGdpUpdate::CGdpUpdate()
{

}

void CGdpUpdate::Start()
{
	wchar_t strAppPath[MAX_PATH] = {0};

	::SHGetFolderPathW(NULL, CSIDL_COMMON_APPDATA , NULL, SHGFP_TYPE_DEFAULT, strAppPath);
	m_strDir = strAppPath;
	m_strDir += L"\\";
	m_strDir += SVC_MODULE_DIR;
	CreateDirectory(m_strDir.c_str(), NULL);
	/*
	__asm
	{
		pushad
			mov eax, 0
			mov ebx, 1
			mov eax, ebx
			add eax,ebx
			mov ecx, eax
			popad
			jmp EXIT_FLAG_1
			HUGE_SHIT_BLOCK
			HUGE_SHIT_BLOCK
			mov eax, 0
			mov ebx, 1
			mov eax, ebx
			add eax,ebx
			mov ecx, eax
			HUGE_SHIT_BLOCK
			HUGE_SHIT_BLOCK
			HUGE_SHIT_BLOCK
			HUGE_SHIT_BLOCK
			HUGE_SHIT_BLOCK
			HUGE_SHIT_BLOCK
			HUGE_SHIT_BLOCK
			HUGE_SHIT_BLOCK
			HUGE_SHIT_BLOCK
			HUGE_SHIT_BLOCK
			HUGE_SHIT_BLOCK
			HUGE_SHIT_BLOCK
			HUGE_SHIT_BLOCK
			HUGE_SHIT_BLOCK
			HUGE_SHIT_BLOCK
			HUGE_SHIT_BLOCK
			HUGE_SHIT_BLOCK
			HUGE_SHIT_BLOCK
			HUGE_SHIT_BLOCK
			HUGE_SHIT_BLOCK
			mov eax, 0
			mov ebx, 1
			mov eax, ebx
			add eax,ebx
			mov ecx, eax
			HUGE_SHIT_BLOCK
			mov ebx, 1
			mov eax, ebx
			add eax,ebx
			mov ecx, eax
			mov eax, 0
			mov ebx, 1
			mov eax, ebx
			add eax,ebx
			mov ecx, eax
			HUGE_SHIT_BLOCK
			HUGE_SHIT_BLOCK
	}


EXIT_FLAG_1:
	*/
	m_strDir += L"\\update\\";
	m_strUpdateExe = m_strDir + L"update.exe";
	CreateDirectory(m_strDir.c_str(), NULL);

	m_hStopEvent = CreateEvent(NULL, FALSE, FALSE, FALSE);

	_beginthread(__ThreadFunc, 0, this);
}

void CGdpUpdate::Stop()
{
	SetEvent(m_hStopEvent);
}

void CGdpUpdate::__ThreadFunc( PVOID lParam )
{
	CGdpUpdate* pThis = (CGdpUpdate*)lParam;

	pThis->ThreadFunc();
}

void CGdpUpdate::ThreadFunc()
{
	//BeginFun
	DWORD dwTimeWait = 60 * 1000;
#ifdef _DEBUG
	dwTimeWait = 10;
#endif // _DEBUG

	if(WaitForSingleObject(m_hStopEvent, dwTimeWait) == WAIT_OBJECT_0 )
		return;
	
	DWORD dwTimeUpdate = 3600 * 1000;
#ifdef _DEBUG
	dwTimeUpdate = 2*60*1000;
#endif // _DEBUG
	do
	{
		if (DownLoadConf())
		{
			if(DownPacket())
			{
				std::wstring strCmdline = L"-upv=" + m_strFielVer;
// 					::elex::log::ISAFE_WriteProgramLogStringNoMask(szcmdline.c_str());
// 					::elex::log::ISAFE_WriteProgramLogStringNoMask(_szLocalPath.c_str());
				Singleton<CWinPrivilege>::instance()->LaunchAppAsAdminUser(m_strUpdateExe, strCmdline);
//					::elex::log::ISAFE_WriteProgramLogStringNoMask(L"finish");
			}
		}
	}while (WaitForSingleObject(m_hStopEvent, dwTimeUpdate) == WAIT_TIMEOUT);

	//EndFun
}

bool CGdpUpdate::DownLoadConf()
{
	//BeginFun

	std::wstring strPtid = Singleton<CConfigure>::instance()->m_strPtid;
	std::wstring strSid = PRODUCT_NAME;
	std::wstring strLang = Singleton<CConfigure>::instance()->m_strLang;
	std::wstring strNation = Singleton<CConfigure>::instance()->m_strNation;
	std::wstring strGdpVer = Singleton<CConfigure>::instance()->m_strVer;
	std::wstring strGuid = Singleton<CConfigure>::instance()->m_strGuid;
	std::wstring strUpdateVersion = L"";
	Singleton<CConfigure>::instance()->GetSvcInfoFromReg(REG_UPDATEVER, strUpdateVersion);


	wchar_t wcsRequest[MAX_WSTRING] = {0};
	// http://up.soft365.com/Wpm/up?ptid={smk,渠道号}&sid={gl,软件ID}&ln={en_us,语言_国家}&ver={1.4.1.39,版本号}&uid={SAMSUNG-898dfsdf,硬盘ID}&pid={ddt,项目ID}
	swprintf_s(wcsRequest, MAX_WSTRING,
		UPDATE_CONF_URL,//L"http://up.soft365.com/Wpm/up?ptid=%s&sid=%s&ln=%s_%s&ver=%s&uid=%s&upv=%s",
		strPtid.c_str(),
		strSid.c_str(),
		strLang.c_str(),
		strNation.c_str(),
		strGdpVer.c_str(),
		strGuid.c_str(),
		strUpdateVersion.c_str());
	LOG_INFO(L"DownLoadConf request path: %s", wcsRequest);
	std::wstring strConf = m_strDir + L"conf";
	CHttpDownloader1 downloader(wcsRequest, strConf);

	DeleteFile(strConf.c_str());
	if (!downloader.Download())
	{
		return false;
	}



	std::ifstream inf;
	inf.open(strConf.c_str());
	std::string strText;
	std::getline(inf, strText, (char)EOF);

	if (strText == "1" || strText == "0")
		return false;

	std::wstring wstrText = strings::s2ws(strText, CP_UTF8);
	std::wstring wstrTextDecode;
	if (!GdpBase64Decode(wstrText, wstrTextDecode))
		return false;
	std::string strTextDecode = strings::
		s(wstrTextDecode, CP_UTF8);
	LOG_INFO(L"strTextDecode: %s ", strTextDecode.c_str());
	Json::Value valRoot;
	Json::Reader reader;
	if (!reader.parse(strTextDecode,valRoot))
		return false;

	m_strFielVer = strings::s2ws(valRoot["file_ver"].asString(),CP_UTF8);
	m_strUrl = strings::s2ws(valRoot["file_url"].asString(),CP_UTF8);
	m_strMd5 = strings::s2ws(valRoot["file_md5"].asString(),CP_UTF8);

	return true;

	//EndFun
}

bool CGdpUpdate::DownPacket()
{
	//BeginFun
	if (m_strUrl.empty() || m_strMd5.empty() || m_strFielVer.empty())
		return false;

	for (int i = 0; i < 3; i++)
	{
		CHttpDownloader1 downloader(m_strUrl, m_strUpdateExe);
		DeleteFile(m_strUpdateExe.c_str());
		if (downloader.Download())
		{
			//md5
			wchar_t szMD5[100] = {0};
			std::transform(m_strMd5.begin(), m_strMd5.end(), m_strMd5.begin(), tolower);
			if (Utilities::MD5::md5File(m_strUpdateExe.c_str(), szMD5) && m_strMd5 == szMD5)
				return true;
			else
				return false;
		}
		Sleep(20000);
	}
	return false;

	//EndFun
}

bool CGdpUpdate::GdpBase64Decode(std::wstring& strInput, std::wstring& strOutput)
{
	//BeginFun

	if (strInput.length() <= 4)
	{
		return false;
	}

	unsigned int nsize = strInput.length();
	wchar_t* szFileName = new wchar_t[nsize+1];
	::StrCpyW(szFileName,strInput.c_str());
	::StrTrimW(szFileName, L" \r\t");
	strInput = szFileName;

	std::wstring strNew = strInput.substr(2, strInput.length() - 4);

	int iLen = strInput.length();
	int iNum[4];
	iNum[0] = _wtoi(strInput.substr(0, 1).c_str());
	iNum[1] = _wtoi(strInput.substr(1, 1).c_str());
	if (iNum[0] == iNum[1])
	{
		return false;
	}
	iNum[2] = _wtoi(strInput.substr(iLen - 1, 1).c_str());
	iNum[3] = _wtoi(strInput.substr(iLen - 2, 1).c_str());

	if (iNum[2] == iNum[3])
	{
		return false;
	}

	if (iNum[0] < iNum[1])
	{
		strNew.erase(iNum[0] - 1, 1);
		strNew.erase(iNum[1] - 2, 1);
	}
	else
	{
		strNew.erase(iNum[1] - 1, 1);
		strNew.erase(iNum[0] - 2, 1);
	}

	iLen = strNew.length();

	if (iNum[2] < iNum[3])
	{
		strNew.erase(iLen - iNum[3], 1);
		strNew.erase(iLen - iNum[2] - 1, 1);
	}
	else
	{
		strNew.erase(iLen - iNum[2], 1);
		strNew.erase(iLen - iNum[3] - 1, 1);
	}

	unsigned long uLen = 10240;
	unsigned char* cBuf = new unsigned char[uLen];
	memset(cBuf, 0, 10240);

	bool bRet = Utilities::CeBase64::Decode(strings::ws2s(strNew), cBuf, &uLen);

	if (bRet)
	{
		std::string str = (char*)cBuf;
		strOutput = strings::s2ws(str);
	}
	else
	{
		strOutput = L"";
	}

	delete cBuf;
	return bRet;

	//EndFun
}
