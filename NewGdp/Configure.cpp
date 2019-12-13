#include "stdafx.h"
#include "Configure.h"
#include "Utilities/FileVersion.h"
#include "Utilities/uid.h"
#include "Utilities/eOSVer.h"
#include "Utilities/eRegOpt.h"
#include "Utilities/NationLanguage.h"
#include "../src/Safe/ConvStr.h"

CConfigure::CConfigure(void)
	: m_bSilence(false)
	, m_strOemId(CMDLINE_PARAM_NAME_OEMID_DEFAULT)
	, m_bIsWow64(false)
	//, m_cmdAction(CMDACTION_NONE)
	, m_bDebug(false)
{
}

CConfigure::~CConfigure(void)
{
}

void CConfigure::Init( const wchar_t* szCmdLine )
{
	//BeginFun
	
	
	//CreateInfoRegKey();


	NationLanguage::GetNationName(m_strNation);
	NationLanguage::GetLangName(m_strLang);

//guid
/*	wchar_t szGuid[MAX_PATH] = {0};
	UID::help::getGUID(szGuid, MAX_PATH);  //这里需要判断返回的SzGuid 如果等于VMwareXVirtualXIDEXHardXDrive_ 或者里面包含 VMware  直接返回可在对在放的版本中进行。
	                                       //增加得到当前映像文件地址,并调用deltefile如果成功则推出，失败则继续执行。
	                                       //后续会增加对各大杀软的沙盒的判断。	*/
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

	Utilities::CeOSVer ov;
	m_bIsWow64 = ov.IsWow64();
	//
	//
	TCHAR szModuleFileName[MAX_PATH] = {0};
	::GetModuleFileName(NULL, szModuleFileName, MAX_PATH);
	std::wstring strModuleFileName = szModuleFileName;

	std::wstring strProductName;
	GetFileInfo(strModuleFileName, m_strVer, strProductName);

	Utilities::String::CCmdLine _CmdLine;


	if (g_strGuid.empty())
	{
		g_strGuid = std::w2a(g_szGuid);
	}

	m_strGuid = g_szGuid;

	if(_CmdLine.Analyze(szCmdLine) == false)
	{
		m_bSilence = true;
		return;
	}

/*
	//oemid
	if (_CmdLine.HasParam(CMDLINE_PARAM_NAME_OEMID))
	{
		m_strOemId = _CmdLine[CMDLINE_PARAM_NAME_OEMID];
	}
	//ptid
	if(!GetSvcInfoFromReg(REG_STRING_PTID, m_strPtid))
	{
		if (_CmdLine.HasParam(CMDLINE_PARAM_NAME_PTID))
		{
			std::wstring strTemp =  _CmdLine[CMDLINE_PARAM_NAME_PTID];
			size_t nIndex = 0;
			if ( (nIndex = strTemp.find(L";") ) != std::wstring::npos)
			{
				m_strPtid = strTemp.substr(nIndex+1);
			}
			else
			{
				m_strPtid  = strTemp;
			}

			SetSvcInfoFromReg(REG_STRING_PTID, m_strPtid);
		}
		else
		{
			m_strPtid = L"wpmvt";
		}
	}*/
	m_strPtid = L"wpmvt";



	
	//EndFun
}

void  CConfigure::GetFileInfo( const std::wstring strFileName, std::wstring& strVer, std::wstring& strProductName )
{     
	//BeginFun

	VS_FIXEDFILEINFO   ver = {0};
	ver = utils::FileVersion::GetFixedFileInfo(strFileName);  

	strVer = std::to_wstring(HIWORD(ver.dwFileVersionMS));
	strVer += L".";
	strVer += std::to_wstring(LOWORD(ver.dwFileVersionMS));
	strVer += L".";
	strVer += std::to_wstring(HIWORD(ver.dwFileVersionLS));
	strVer += L".";
	strVer += std::to_wstring(LOWORD(ver.dwFileVersionLS));

	strProductName = utils::FileVersion::GetString(strFileName, FILE_ATTR_PRODUCTNAME);

	return;   

	//EndFun
}  

bool CConfigure::GetSvcInfoFromReg(const std::wstring& strItem, std::wstring& strItemValue)
{
	//BeginFun

	strItemValue = L"";
	DWORD dwDesired = 0;
	DWORD dwType = REG_SZ;
	std::wstring szSubKey = REG_SUBKEY_GDP;
	wchar_t strData[3*MAX_PATH] = {0};
	DWORD dwcbData = 3*MAX_PATH*sizeof(wchar_t);


	if (m_bIsWow64)
	{
		dwDesired |= KEY_WOW64_32KEY;
	}

	if(!Utilities::Reg::CeRegOptUtilities::ReadKeyValueW(HKEY_LOCAL_MACHINE,szSubKey.c_str(),
		strItem.c_str(),dwType,(LPBYTE)strData,dwcbData,dwDesired))
		return false;

	strItemValue = strData;
	return true;

	//EndFun
}

bool CConfigure::SetSvcInfoFromReg(const std::wstring& strItem, std::wstring& strItemValue)
{
	//BeginFun

	DWORD dwDesired = 0;
	DWORD dwType = REG_SZ;
	std::wstring szSubKey = REG_SUBKEY_GDP;


	DWORD dwcbData = (strItemValue.length()+1)*sizeof(wchar_t);

	if (m_bIsWow64)
	{
		dwDesired |= KEY_WOW64_32KEY;
	}

	return Utilities::Reg::CeRegOptUtilities::WriteKeyValueW(HKEY_LOCAL_MACHINE,szSubKey.c_str(),
		strItem.c_str(),dwType,(LPBYTE)strItemValue.c_str(),dwcbData,dwDesired);	

	EndFun
}

bool CConfigure::CreateInfoRegKey()
{
	//BeginFun

	bool bRet=false;
	DWORD dwDesired = 0;
	DWORD dwType = REG_SZ;
	HKEY hkey = NULL;
	DWORD dwDisposition = 0;
	std::wstring szSubKey = REG_SUBKEY_GDP;

	if (m_bIsWow64)
	{
		dwDesired |= KEY_WOW64_32KEY;
	}

	do 
	{
		if (::RegCreateKeyExW(HKEY_LOCAL_MACHINE,szSubKey.c_str(),0,NULL,REG_OPTION_NON_VOLATILE,
			KEY_ALL_ACCESS|dwDesired,NULL,&hkey,&dwDisposition) !=ERROR_SUCCESS)
			break;

		bRet = true;

	} while (false);

	if(hkey!=NULL)
		RegCloseKey(hkey);

	return bRet;

	//EndFun
}
