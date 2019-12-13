#ifndef  _COMMON_UTIL_FILEVERSION_H
#define _COMMON_UTIL_FILEVERSION_H

#include <string>
#include <windows.h>
namespace utils{ namespace FileVersion{

	VS_FIXEDFILEINFO GetFixedFileInfo(const std::wstring& strFile);
	std::wstring GetString( const std::wstring& strFile, const std::wstring& strName);
	bool SetString(const std::wstring& strFile, const std::wstring& strName, const std::wstring& strValue);
	
}}



#endif