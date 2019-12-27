/********************************************************************
	Created:	21-4-2007   16:53
	FileName: 	UTL_FileTime.cpp
	Author:		ChenQingMing
	
	purpose:	
*********************************************************************/
#include "UTL_FileTime.H"
#include <tchar.h>
#include <shlwapi.h>
#include <stdio.h>
bool GetFileModifyTime(LPCTSTR lpFile,FILETIME & modFT)
{
	WIN32_FIND_DATA wfd;
	HANDLE hFind = FindFirstFile(lpFile,&wfd);
	if(hFind==INVALID_HANDLE_VALUE)
		return false;
	modFT = wfd.ftLastWriteTime;
	FindClose(hFind);
	return true;
}

bool FileTimeToString(const FILETIME & ft,LPTSTR lpTimeString, int nSize)
{//"Wed, 07 Jun 2006 19:35:34 GMT"
	TCHAR * szWeekArr[] ={_T("Sun"),_T("Mon"),_T("Tue"),_T("Wed"),_T("Thu"),_T("Fri"),_T("Sat")};
	TCHAR * szMonArr[] ={_T(""),_T("Jan"),_T("Feb"),_T("Mar"),_T("Apr"),_T("May"),_T("Jun"),_T("Jul"),_T("Aug"),_T("Sep"),_T("Oct"),_T("Nov"),_T("Dec")};
	SYSTEMTIME st;
	FileTimeToSystemTime(&ft,&st);
	_sntprintf_s(lpTimeString, nSize, _TRUNCATE, _T("%s, %02d %s %d %02d:%02d:%02d GMT"),szWeekArr[st.wDayOfWeek],st.wDay,szMonArr[st.wMonth],st.wYear,st.wHour,st.wMinute,st.wSecond);
	return true;
}

bool TimeStringToFileTime(LPCTSTR lpTimeString,FILETIME & ft)
{//"Wed, 07 Jun 2006 19:35:34 GMT"
	TCHAR * szWeekArr[] ={_T("Sun"),_T("Mon"),_T("Tue"),_T("Wed"),_T("Thu"),_T("Fri"),_T("Sat")};
	TCHAR * szMonArr[] ={_T(""),_T("Jan"),_T("Feb"),_T("Mar"),_T("Apr"),_T("May"),_T("Jun"),_T("Jul"),_T("Aug"),_T("Sep"),_T("Oct"),_T("Nov"),_T("Dec")};

	try
	{
		SYSTEMTIME st={0};
		TCHAR szWeek[6],szMon[6], szTime[3][10];
		TCHAR szDay[10], szYear[10];
		int nOkCount = _stscanf_s(lpTimeString,_T("%[^, \t]%*[, \t]%[^, \t]%*[, \t]%[^, \t]%*[, \t]%[^, \t]%*[, \t]%[^:]:%[^:]:%[^, \t]"),szWeek, szDay, szMon, szYear, szTime[0],szTime[1],szTime[2]);
		if(7 != nOkCount)
			return false;
        int i = 0;
		for(i=0;i<sizeof(szWeekArr)/sizeof(szWeekArr[0]);i++)
		{
			if(StrCmpI(szWeekArr[i],szWeek) == 0)
			{
				st.wDayOfWeek = i;
				break;
			}
		}
		
		if(i >= 7)
			return false;
		
        int j = 1;
		for(j=1;j<sizeof(szMonArr)/sizeof(szMonArr[0]);j++)
		{
			if(StrCmpI(szMonArr[j],szMon) == 0)
			{
				st.wMonth = j;
				break;
			}
		}
		if(j >= 13)
			return false;
        st.wYear = (SHORT)_ttol(szYear);
        st.wDay = (SHORT)_ttol(szDay);

		st.wHour = (SHORT)_ttol(szTime[0]);
		st.wMinute = (SHORT)_ttol(szTime[1]);
		st.wSecond = (SHORT)_ttol(szTime[2]);
		return !!SystemTimeToFileTime(&st, &ft);
	}catch(...){}
	return false;
}

std::string ws2s( const std::wstring& wide, UINT CodePage )
{
	int wide_length = static_cast<int>(wide.length());
	if (wide_length == 0)
		return std::string();

	// Compute the length of the buffer we'll need.
	int charcount = WideCharToMultiByte(CodePage, 0, wide.data(), wide_length,
		NULL, 0, NULL, NULL);
	if (charcount == 0)
		return std::string();

	std::string mb;
	mb.resize(charcount);
	WideCharToMultiByte(CodePage, 0, wide.data(), wide_length,
		&mb[0], charcount, NULL, NULL);

	return mb;
}

std::wstring s2ws( const std::string& mb,UINT CodePage)
{
	if (mb.empty())
		return std::wstring();

	int mb_length = static_cast<int>(mb.length());
	// Compute the length of the buffer.
	int charcount = MultiByteToWideChar(CodePage, 0,
		mb.data(), mb_length, NULL, 0);
	if (charcount == 0)
		return std::wstring();

	std::wstring wide;
	wide.resize(charcount);
	MultiByteToWideChar(CodePage, 0, mb.data(), mb_length, &wide[0], charcount);

	return wide;
}
