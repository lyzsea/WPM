#include "FileVersion.h"

namespace utils{namespace FileVersion{
	
VS_FIXEDFILEINFO GetFixedFileInfo(const std::wstring& strFile)
{
	int iVerInfoSize;  
	char *pBuf;  
	VS_FIXEDFILEINFO   *pVsInfo = NULL;  
	VS_FIXEDFILEINFO verInfo = {0};
	unsigned int iFileInfoSize = sizeof(VS_FIXEDFILEINFO);  

	iVerInfoSize = GetFileVersionInfoSize((LPCWSTR)strFile.c_str(),NULL);   

	if(iVerInfoSize!= 0)  
	{     
		pBuf = new char[iVerInfoSize];  
		if(GetFileVersionInfo((LPCWSTR)strFile.c_str(), 0, iVerInfoSize, pBuf))     
		{     
			iFileInfoSize = sizeof(VS_FIXEDFILEINFO);
			if(VerQueryValue(pBuf, L"\\", (void**)&pVsInfo, &iFileInfoSize) && pVsInfo)     
			{    
				verInfo = *pVsInfo;
// 				strVer = to_wstring(HIWORD(pVsInfo->dwFileVersionMS));
// 				strVer += L".";
// 				strVer += to_wstring(LOWORD(pVsInfo->dwFileVersionMS));
// 				strVer += L".";
// 				strVer += to_wstring(HIWORD(pVsInfo->dwFileVersionLS));
// 				strVer += L".";
// 				strVer += to_wstring(LOWORD(pVsInfo->dwFileVersionLS));
			}
		}
		delete pBuf;
	}
	return verInfo;
}

std::wstring GetString( const std::wstring& strFile, const std::wstring& strName )
{
	std::wstring strVal;
	int iVerInfoSize;  
	char *pBuf;  

	unsigned int iFileInfoSize = sizeof(VS_FIXEDFILEINFO);  

	iVerInfoSize = GetFileVersionInfoSize(strFile.c_str(),NULL);   

	if(iVerInfoSize!= 0)  
	{     
		pBuf = new char[iVerInfoSize];  
		if(GetFileVersionInfo(strFile.c_str(), 0, iVerInfoSize, pBuf))     
		{     
			UINT nSize = 0;
			unsigned short int *pValue = NULL;
			std::wstring strLangCode;
			if(VerQueryValue(pBuf, L"\\VarFileInfo\\Translation", (void**)&pValue, &nSize) && pValue) 
			{
				wchar_t  sz[100];
				swprintf_s(sz, 100, L"%04x", pValue[0]);
				strLangCode = sz;
				swprintf_s(sz, 100, L"%04x", pValue[1]);
				strLangCode += sz;
			}
			if (!strLangCode.empty())
			{
				std::wstring strPath = L"\\StringFileInfo\\";
				strPath += strLangCode;
				strPath += L"\\";
				strPath += strName;//L"\\ProductName";
				wchar_t* psValue = NULL;
				nSize = 0;
				if(VerQueryValue(pBuf, strPath.c_str(), (void**)&psValue, &nSize) && psValue) 
				{
					strVal = psValue;
				}
			}
		}     
		delete pBuf;     
	}     
	return strVal;     
}


bool SetString( const std::wstring& strFile, const std::wstring& strName, const std::wstring& strValue )
{
	bool bSuccess = false;
	int iVerInfoSize;  
	char *pBuf;  

	unsigned int iFileInfoSize = sizeof(VS_FIXEDFILEINFO);  
	iVerInfoSize = GetFileVersionInfoSize(strFile.c_str(),NULL);   
	if(iVerInfoSize!= 0)  
	{     
		pBuf = new char[iVerInfoSize];  
		if(GetFileVersionInfo(strFile.c_str(), 0, iVerInfoSize, pBuf))     
		{     
			HANDLE handle = BeginUpdateResource((LPCWSTR)strFile.c_str(), FALSE);

			UINT nSize = 0;
			unsigned short int *pwLang = NULL;
			std::wstring strLangCode;
			if(VerQueryValue(pBuf, L"\\VarFileInfo\\Translation", (void**)&pwLang, &nSize) && pwLang) 
			{
				wchar_t  sz[100];
				swprintf_s(sz, 100, L"%04x", pwLang[0]);
				strLangCode = sz;
				swprintf_s(sz, 100, L"%04x", pwLang[1]);
				strLangCode += sz;
			}
			if (!strLangCode.empty())
			{
				std::wstring strPath = L"\\StringFileInfo\\";
				strPath += strLangCode;
				strPath += L"\\";
				strPath += strName;//L"\\ProductName";
				wchar_t* psValue = NULL;
				nSize = 0;
				if(VerQueryValue(pBuf, strPath.c_str(), (void**)&psValue, &nSize) && psValue) 
				{
					if (psValue && std::wstring(psValue).length() >= strValue.length())
					{
						lstrcpyn(psValue, (LPCWSTR)strValue.c_str(), strValue.length() + 1);
						bSuccess = true;
					}
					else {
						//assert(0); //原来就有name字段，且原字段值的长度要足够长
					}
				}
			}

			WORD dwLang = 0;
			if (pwLang)
				dwLang = pwLang[1];

			//SUBLANG_SYS_DEFAULT
			//SUBLANG_CHINESE_SIMPLIFIED 
			WORD lang  = MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_SIMPLIFIED );

			if(!UpdateResource(handle,RT_VERSION,MAKEINTRESOURCE(VS_VERSION_INFO), lang, (LPVOID)pBuf, iVerInfoSize))
				bSuccess = false; 
			else if(!EndUpdateResource(handle,FALSE))  
				bSuccess = false;
		}  
		
		delete pBuf;     
	}     
	return bSuccess;
}

}}