#include "stdafx.h"
#include "ReportData.h"
#include "Configure.h"
#include "Safe/StrEncrypt.h"
#include "StrDefine.h"



namespace report
{
	std::wstring strReportType[eReport_Number]= 
	{
		L"xa.geoip",
		L"install",
		L"visit.heartbeat",
		L"uninstall"
	};

	std::wstring XingCloudReport(const eReportList eIndex)
	{
		//BeginFun
		wchar_t wcsRequest[1024] = {0};

		switch (eIndex)
		{
		case eReport_Geoip:
		case eReport_Start:
			{
				swprintf_s(wcsRequest,1024,REPORT_INSTALL_STR,
					GConfigInfo::instance()->m_strGuid.c_str(),
					strReportType[eReport_Geoip].c_str(),
					strReportType[eReport_Start].c_str(),					
					GConfigInfo::instance()->m_strPtid.c_str(),
					GConfigInfo::instance()->m_strNation.c_str(),
					GConfigInfo::instance()->m_strLang.c_str());
			}
			break;
		case eReport_Uninstall:
			{
				swprintf_s(wcsRequest,1024,REPORT_REQUEST_STR,GConfigInfo::instance()->m_strGuid.c_str(),
					strReportType[eIndex].c_str());
			}
			break;
		case eReport_Heart:
			{
				swprintf_s(wcsRequest,1024,REPORT_HEARTBEAT_STR,
					GConfigInfo::instance()->m_strGuid.c_str(),
					GConfigInfo::instance()->m_strPtid.c_str());
			}
			break;
		case  eReport_Heart_FIRST:
			{
				swprintf_s(wcsRequest,1024,REPORT_HEARTBEAT_STR_First,
					GConfigInfo::instance()->m_strGuid.c_str(),
					GConfigInfo::instance()->m_strPtid.c_str(),
					GConfigInfo::instance()->m_strVer.c_str());
			}
			break;
		default:
			break;
		}

		//EndFun

		return wcsRequest;
	}
}
