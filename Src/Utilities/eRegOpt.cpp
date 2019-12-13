/************************************************************************/
/* 
    Date : 2/28/2013
	File : EREGOPT.CPP
    Author : Eadwin Li
    Function :  
	Version : 1.0
	Copyright (c) 2012 - 2013 iWinPhone Studio
*/
/************************************************************************/  

#include "eRegOpt.h"
#include <string>

namespace Utilities{namespace Reg{

	bool CeRegOptUtilities::ReadKeyValueW(HKEY hKey,LPCWSTR lpSubKey,
		LPCWSTR lpValueName,DWORD dwType,LPBYTE lpValue,
		DWORD& dwLength,DWORD dwDesired)
	{
		bool bRet=false;
		HKEY hKEY=NULL;

		do 
		{
			if(RegOpenKeyExW(hKey,lpSubKey,0,KEY_READ|dwDesired,&hKEY)!=ERROR_SUCCESS)
				break;

			if(RegQueryValueExW(hKEY,lpValueName,NULL,&dwType,(LPBYTE)lpValue,&dwLength)!=ERROR_SUCCESS)
				break;

			bRet=true;

		} while (false);

		if(hKEY!=NULL)
			RegCloseKey(hKEY);

		return bRet;
	}


	bool CeRegOptUtilities::WriteKeyValueW(HKEY hKey,LPCWSTR lpSubKey,
		LPCWSTR lpValueName,DWORD dwType,LPCBYTE lpValue,
		DWORD& dwLength,DWORD dwDesired)
	{
		bool bRet=false;
		HKEY hKEY=NULL;

		do 
		{
			if(RegOpenKeyExW(hKey,lpSubKey,0,KEY_READ|KEY_WRITE|dwDesired,&hKEY)!=ERROR_SUCCESS)
				break;

			if(RegSetValueExW(hKEY,lpValueName,NULL,dwType,lpValue,dwLength)!=ERROR_SUCCESS)
				break;

			bRet=true;

		} while (false);


		if(hKEY!=NULL)
			RegCloseKey(hKEY);

		return bRet;
	}


	bool CeRegOptUtilities::DeleteKeyValueW(HKEY hKey,LPCWSTR lpSubKey,
		LPCWSTR lpValueName,DWORD dwDesired)
	{
		bool bRet=false;
		HKEY hKEY=NULL;

		do 
		{
			if(RegOpenKeyExW(hKey,lpSubKey,0,KEY_READ|KEY_WRITE|dwDesired,&hKEY)!=ERROR_SUCCESS)
				break;


			if(RegDeleteValueW(hKEY,lpValueName)!=ERROR_SUCCESS)
				break;

			bRet=true;

		} while (false);

		if(hKEY!=NULL)
			RegCloseKey(hKEY);

		return bRet;
	}


	bool CeRegOptUtilities::GetRegValueAllW(HKEY hRootKey, LPCWSTR lpszKeyName, 
		LPCWSTR lpszValueName, LPDWORD lpdwType, 
		LPBYTE lpData, LPDWORD lpcbData,DWORD dwDesired)
	{
		bool		bReturn = false;
		HKEY		hKey = NULL;
		LONG		lResult = ERROR_SUCCESS;

		do 
		{

			if ( lpData != static_cast<LPBYTE>(0) && lpcbData == static_cast<LPDWORD>(0) )
			{
				bReturn = true;
				break;
			}

			// 打开子键
			hKey = NULL;
			lResult = ::RegOpenKeyExW(hRootKey, lpszKeyName, 0, 
				KEY_QUERY_VALUE | KEY_READ | dwDesired,
				&hKey);
			if (lResult != ERROR_SUCCESS)
			{
				bReturn = false;
				break;
			}

			// 查询键值
			if ( lpData != static_cast<LPBYTE>(0) )
				::memset(lpData, 0, *lpcbData);

			if ( lpdwType != static_cast<LPDWORD>(0) )
				*lpdwType = REG_SZ;

			lResult = ::RegQueryValueExW(hKey,lpszValueName,NULL,lpdwType,(LPBYTE)lpData,
				lpcbData);
			if (lResult != ERROR_SUCCESS)
			{
				bReturn = false;
				break;
			}

			bReturn = true;
		} while (false);

	
		if (hKey != NULL)
			::RegCloseKey(hKey);

		return bReturn;
	}


	HKEY CeRegOptUtilities::GethKey(std::wstring& skey)
	{
		if(skey.compare(L"HKEY_CLASSES_ROOT") == 0)
			return HKEY_CLASSES_ROOT;
		if(skey.compare(L"HKEY_CURRENT_USER") == 0)
			return HKEY_CURRENT_USER;
		if(skey.compare(L"HKEY_LOCAL_MACHINE") == 0)
			return HKEY_LOCAL_MACHINE;
		if(skey.compare(L"HKEY_USERS") == 0)
			return HKEY_USERS;
		if(skey.compare(L"HKEY_CURRENT_CONFIG") == 0)
			return HKEY_CURRENT_CONFIG;

		return 0;
	}

	bool CeRegOptUtilities::ReadKeyOrValueW(HKEY hKey,LPCWSTR lpSubKey,LPCWSTR lpValueName,DWORD dwType,LPWSTR lpValue,DWORD& dwLength,DWORD dwDesired)
	{
		bool bRet=false;

		HKEY hKEY=NULL;

		if (wcslen(lpValueName) < 1)
		{
			if(RegOpenKeyExW(hKey,lpSubKey,0,KEY_READ|dwDesired,&hKEY)!=ERROR_SUCCESS)
				goto ENDPOS;
		}
		else
		{
			if(RegOpenKeyExW(hKey,lpSubKey,0,KEY_QUERY_VALUE|KEY_READ|dwDesired,&hKEY)!=ERROR_SUCCESS)
				goto ENDPOS;

			if(RegQueryValueExW(hKEY,lpValueName,NULL,&dwType,(LPBYTE)lpValue,&dwLength)!=ERROR_SUCCESS)
				goto ENDPOS;
		}


		bRet=true;

ENDPOS:

		if(hKEY!=NULL)
			RegCloseKey(hKEY);

		return bRet;
	}

}}
