#pragma once

#include <string>


namespace report
{
	enum eReportList
	{
		eReport_Geoip = 0,
		eReport_Start,
		eReport_Heart,
		eReport_Uninstall,
		eReport_Heart_FIRST,

		eReport_Number
	};

	std::wstring XingCloudReport(const eReportList eIndex);

}
