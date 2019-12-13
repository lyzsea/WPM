#pragma once
#include "stdafx.h"
#include "cmdAnalyze/Cmdline.h"
#include "Utilities//Singleton.h"

class CConfigure
{
public:
	CConfigure(void);
	~CConfigure(void);
	void Init(const wchar_t* szCmdLine);

	std::wstring m_strGuid;
	std::wstring m_strVer;
	std::wstring m_strPtid;
	std::wstring m_strChannel;
// 	std::wstring m_strOuterPtId0;
// 	std::wstring m_strOuterPtId1;
	std::wstring m_strOemId;
 	std::wstring m_strNation;
 	std::wstring m_strLang;
	bool m_bSilence;
	bool m_bIsWow64;
	typedef enum CMD_ACTION
	{
		CMDACTION_NONE = 0
		,CMDACTION_INSTALL
		,CMDACTION_UNINSTALL
		,CMDACTION_KILL
		,CMDACTION_START
		,CMDACTION_STOP
	};
	//CMD_ACTION m_cmdAction;
	BOOL	m_bDebug;

	void GetFileInfo( const std::wstring strFileName, std::wstring& strVer, std::wstring& strProductName );
	bool GetSvcInfoFromReg(const std::wstring& strItem, std::wstring& strItemValue);
	bool SetSvcInfoFromReg(const std::wstring& strItem, std::wstring& strItemValue);
	bool CreateInfoRegKey();
};

typedef Singleton<CConfigure> GConfigInfo;

