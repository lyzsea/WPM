/************************************************************************/
/* 
    Date : 2/28/2013
	File : EREGOPT.H
    Author : Eadwin Li
    Function :  
	Version : 1.0
	Copyright (c) 2012 - 2013 iWinPhone Studio
*/
/************************************************************************/  

#pragma once

#include <windows.h>
#include <string>

#ifndef LPCBYTE
#define LPCBYTE const LPBYTE
#endif // LPCBYTE

namespace Utilities{namespace Reg{

	class CeRegOptUtilities
	{
	public:
		CeRegOptUtilities(){}
		~CeRegOptUtilities(){}

		static HKEY GethKey(std::wstring& skey);

		static bool ReadKeyValueW(HKEY hKey,LPCWSTR lpSubKey,
			LPCWSTR lpValueName,DWORD dwType,LPBYTE lpValue,
			DWORD& dwLength,DWORD dwDesired);
		static bool CeRegOptUtilities::ReadKeyOrValueW(HKEY hKey,
			LPCWSTR lpSubKey,LPCWSTR lpValueName,
			DWORD dwType,LPWSTR lpValue,DWORD& dwLength,
			DWORD dwDesired);
		static bool WriteKeyValueW(HKEY hKey,LPCWSTR lpSubKey,
			LPCWSTR lpValueName,DWORD dwType,LPCBYTE lpValue,
			DWORD& dwLength,DWORD dwDesired);
		static bool DeleteKeyValueW(HKEY hKey,LPCWSTR lpSubKey,
			LPCWSTR lpValueName,DWORD dwDesired);
		static bool GetRegValueAllW(HKEY hRootKey, LPCWSTR lpszKeyName, 
			LPCWSTR lpszValueName, LPDWORD lpdwType, 
			LPBYTE lpData, LPDWORD lpcbData,DWORD dwDesired);

	};

}}