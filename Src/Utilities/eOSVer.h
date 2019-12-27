/************************************************************************/
/* 
    Date : 10/29/2012
	File : WINOSVER.H
    11
       
	Version : 1.0
	Copyright
*/
/************************************************************************/  

#pragma once
#include <windows.h>
#include <tchar.h>

namespace Utilities{

#define WINMOD_UNKNOWN_OS   0x00000000
#define WINMOD_XP_SP0       0x05010000
#define WINMOD_XP_SP1       0x05010100
#define WINMOD_XP_SP2       0x05010200
#define WINMOD_XP_SP3       0x05010300
#define WINMOD_2K3_SP0      0x05020000
#define WINMOD_2K3_SP1      0x05020100
#define WINMOD_2K3_SP2      0x05020200
#define WINMOD_VISTA_SP0    0x06000000
#define WINMOD_VISTA_SP1    0x06000100
#define WINMOD_VISTA_SP2    0x06000200
#define WINMOD_WIN7_SP0     0x06010000

class CeOSVer
{
public:

    enum {
        em_OS_MajorVer_Win7     = 6,
        em_OS_MajorVer_Vista    = 6,
        em_OS_MajorVer_Win2k3   = 5,
        em_OS_MajorVer_WinXP    = 5,

        em_OS_MinorVer_Win7     = 1,

        em_OS_MinorVer_Win2k3   = 2,
        em_OS_MinorVer_WinXP    = 1,
    };

    DWORD GetOSAndSP()
    {
        OSVERSIONINFOEX osver;
        memset(&osver, 0, sizeof(OSVERSIONINFOEX));
        osver.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
        BOOL br = GetVersionEx((OSVERSIONINFO*) &osver);
        if (!br)
            return 0;

        
        DWORD dwOSAndSP = 0;
        dwOSAndSP |= (LOBYTE(osver.dwMajorVersion)      << 24);
        dwOSAndSP |= (LOBYTE(osver.dwMinorVersion)      << 16);
        dwOSAndSP |= (LOBYTE(osver.wServicePackMajor)   << 8);
        return dwOSAndSP;
    }

    BOOL IsX86()
    {
        SYSTEM_INFO sysInfo;
        memset(&sysInfo, 0, sizeof(sysInfo));
        GetSystemInfo(&sysInfo);

        if (PROCESSOR_ARCHITECTURE_INTEL == sysInfo.wProcessorArchitecture)
            return TRUE;

        return FALSE;
    }

	typedef BOOL (WINAPI *LPFN_ISWOW64PROCESS)(HANDLE, PBOOL);

	bool IsWow64()
	{
		LPFN_ISWOW64PROCESS fnIsWow64Process = (LPFN_ISWOW64PROCESS)GetProcAddress(GetModuleHandle(L"kernel32"),"IsWow64Process");

		BOOL bIsWow64 = false;

		if (NULL != fnIsWow64Process)
		{
			if (!fnIsWow64Process(GetCurrentProcess(),&bIsWow64))
			{
				// Error handle
				return false;
			}
		}

		return  bIsWow64 ? true : false;
	}

    BOOL IsVista()
    {
        return 0 == CompareMajor(em_OS_MajorVer_Vista);
    }

    BOOL IsWin7OrLater()
    {
        return 0 <= CompareVersion(em_OS_MajorVer_Win7, em_OS_MinorVer_Win7);
    }

    BOOL IsVistaOrLater()
    {
        return 0 <= CompareMajor(em_OS_MajorVer_Vista);
    }

    BOOL IsWinXPOrLater()
    {
        return 0 <= CompareVersion(em_OS_MajorVer_WinXP, em_OS_MinorVer_WinXP);
    }

    BOOL IsWin2k3()
    {
        return 0 == CompareVersion(em_OS_MajorVer_Win2k3, em_OS_MinorVer_Win2k3);
    }


    /**
    * @retval   >0  current OS is greater than compared version
    * @retval   <0  current OS is less than compared version
    * @retval   0   current OS is equal to compared version
    */
    int CompareVersion(DWORD dwMajorVer, DWORD dwMinorVer)
    {
        OSVERSIONINFO osInfo;
        osInfo.dwOSVersionInfoSize = sizeof(osInfo);
        ::GetVersionEx(&osInfo);

        if (osInfo.dwMajorVersion > dwMajorVer)
        {
            return 1;
        }
        else if (osInfo.dwMajorVersion < dwMajorVer)
        {
            return -1;
        }

        return osInfo.dwMinorVersion - dwMinorVer;
    }


    int CompareMajor(DWORD dwMajorVer)
    {
        OSVERSIONINFO osInfo;
        osInfo.dwOSVersionInfoSize = sizeof(osInfo);
        ::GetVersionEx(&osInfo);

        return osInfo.dwMajorVersion - dwMajorVer;
    }

	bool getSystemVersion(DWORD& dwMajor,DWORD& dwMinor,DWORD& dwSP)
	{
		OSVERSIONINFOEX osvi;

		ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
		osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

		if(!(GetVersionEx ((OSVERSIONINFO *) &osvi)))
		{
			osvi.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
			if (!GetVersionEx((OSVERSIONINFO *) &osvi))
			{
				return false; 
			}
		}

		dwMajor = osvi.dwMajorVersion;
		dwMinor = osvi.dwMinorVersion;
		dwSP	= osvi.wServicePackMajor;

		return true;
	}


	bool IsAfterVistaSp1()
	{
		DWORD dwMajor=0,dwMinor=0,dwSP=0;
		getSystemVersion(dwMajor,dwMinor,dwSP);
		if (dwMajor >= 6)
		{
			if (dwMinor >0)
			{
				return true;
			}
			else if (dwMinor == 0)
			{
				if (dwSP >= 1)
				{
					return true;
				}
			}
		}

		return false;

	}



	typedef int (WINAPI *pRtlGetVersion)(PRTL_OSVERSIONINFOW lpVersionInformation);

#ifndef NT_PRODUCT_TYPE
	typedef enum _NT_PRODUCT_TYPE  
	{
		NtProductWinNt = 1,
		NtProductLanManNt,
		NtProductServer
	}NT_PRODUCT_TYPE,*PNT_PRODUCT_TYPE;
#endif	// NT_PRODUCT_TYPE

	typedef BOOL (WINAPI *pRtlGetNtProductType)(PNT_PRODUCT_TYPE NtProductType);  

	ULONG GetRealProcAddress(HANDLE hModule, char *lpProcName)
	{
		PIMAGE_DOS_HEADER       DosHeader  = NULL;
		PIMAGE_NT_HEADERS       NtHeader   = NULL;
		PIMAGE_DATA_DIRECTORY   ExportsDir = NULL;
		PIMAGE_EXPORT_DIRECTORY Exports    = NULL;
		PIMAGE_SECTION_HEADER   SectionHeader = NULL;
		PULONG Functions = NULL;
		PSHORT Ordinals  = NULL;
		PULONG Names     = NULL;
		ULONG ProcAddr   = 0;
		ULONG i=0, NumOfNames=0, iOrd=0, nSize=0;

		if (!hModule || !lpProcName)
			return 0;

		DosHeader=(PIMAGE_DOS_HEADER)hModule;
		NtHeader=(PIMAGE_NT_HEADERS)((char*)hModule+DosHeader->e_lfanew);

		if (!NtHeader)
			return 0;

		ExportsDir=NtHeader->OptionalHeader.DataDirectory+
			IMAGE_DIRECTORY_ENTRY_EXPORT;

		if (!ExportsDir)
			return 0;

		SectionHeader = IMAGE_FIRST_SECTION(NtHeader);	


		Exports = (PIMAGE_EXPORT_DIRECTORY)((char*)hModule + ExportsDir->VirtualAddress);
		Functions = (PULONG)(Exports->AddressOfFunctions  + (char*)hModule);
		Ordinals = (PSHORT)(Exports->AddressOfNameOrdinals  + (char*)hModule);
		Names = (PULONG)(Exports->AddressOfNames + (char*)hModule);


		NumOfNames=Exports->NumberOfNames;
		nSize=ExportsDir->Size;

		for(i=0;i<NumOfNames;i++)
		{
			iOrd=Ordinals[i];

			if(_stricmp((char*)hModule+ Names[i] ,lpProcName)==0)
			{
				return (ULONG)(Functions[iOrd] + NtHeader->OptionalHeader.ImageBase);
			}
		}

		return 0;
	}

	BOOL MyGetVersion(PRTL_OSVERSIONINFOEXW pOSVersion)
	{
		if(pOSVersion==NULL)
			return FALSE;

		BOOL bRet=FALSE;

		static pRtlGetVersion pGetVersion=NULL;
		static pRtlGetNtProductType pGetNtProductType=NULL;

#ifndef STATUS_SUCCESS
#define STATUS_SUCCESS	0
#endif

		__try
		{
			memset(pOSVersion,0,sizeof(RTL_OSVERSIONINFOEXW));
			pOSVersion->dwOSVersionInfoSize=sizeof(RTL_OSVERSIONINFOEXW);

			if(pGetVersion==NULL || pGetNtProductType==NULL)
			{
				//	OS_TEST
				//			HMODULE hmod=GetModuleHandle(_T("ntdll.dll"));
				//			if(hmod==NULL)
				//				return FALSE;

				HMODULE hmod=GetModuleHandle(_T("ntdll.dll"));
				if(hmod==NULL)
				{
					DWORD dwError=GetLastError();
					_stprintf_s((TCHAR*)pOSVersion,pOSVersion->dwOSVersionInfoSize,_T("3_%X"),dwError);
					return FALSE;
				}
				//	OS_TEST			
				pGetVersion=(pRtlGetVersion)GetRealProcAddress(hmod,"RtlGetVersion");
				pGetNtProductType=(pRtlGetNtProductType)GetRealProcAddress(hmod,"RtlGetNtProductType");

				if(pGetVersion==NULL)
					return FALSE;
			}

			bRet=(pGetVersion((PRTL_OSVERSIONINFOW)pOSVersion)==STATUS_SUCCESS);
			//	OS_TEST
			if(!bRet)
			{
				DWORD dwError=GetLastError();
				_stprintf_s((TCHAR*)pOSVersion,pOSVersion->dwOSVersionInfoSize,_T("3_%X"),dwError);
			}
			//	OS_TEST

			if(pGetNtProductType!=NULL)
			{
				NT_PRODUCT_TYPE ProductType=NtProductWinNt;
				if(pGetNtProductType(&ProductType))
					pOSVersion->wProductType=ProductType;
			}
		}
		__except(EXCEPTION_EXECUTE_HANDLER)
		{
			return bRet;
		}

		return bRet;
	}


	BOOL GDP_GetVersionExW(LPOSVERSIONINFOW lpVersionInfo)
	{


		if(lpVersionInfo==NULL || (lpVersionInfo->dwOSVersionInfoSize!=sizeof(OSVERSIONINFOW) && lpVersionInfo->dwOSVersionInfoSize!=sizeof(OSVERSIONINFOEXW)))
		{
			if(!GetVersionExW(lpVersionInfo))
			{
				DWORD dwError=GetLastError();
				_stprintf_s((TCHAR*)lpVersionInfo,lpVersionInfo->dwOSVersionInfoSize,_T("1_%X"),dwError);

				return FALSE;
			}

			return TRUE;
		}
		//	OS_TEST

		RTL_OSVERSIONINFOEXW rtl_osvi;
		rtl_osvi.dwOSVersionInfoSize=sizeof(RTL_OSVERSIONINFOEXW);
		if(MyGetVersion(&rtl_osvi))
		{
			memmove(lpVersionInfo,&rtl_osvi,lpVersionInfo->dwOSVersionInfoSize);

			return TRUE;
		}

		//	OS_TEST
		//	return GetVersionExW(lpVersionInfo);

		if(!GetVersionExW(lpVersionInfo))
		{
			DWORD dwError=GetLastError();
			_stprintf_s((TCHAR*)lpVersionInfo,lpVersionInfo->dwOSVersionInfoSize,_T("2_%X"),dwError);

			return FALSE;
		}

		return TRUE;
		//	OS_TEST
	}

	#define MIN_MINIDESCLENGTH	20
	// lpDesc：存储描述的缓冲区
	// dwDescLength：存储描述的缓冲区的长度，要求不少于20个字符
	BOOL GetMiniOSVersionDesc(LPWSTR lpDesc,DWORD dwDescLength)
	{
		if(lpDesc==NULL || dwDescLength<MIN_MINIDESCLENGTH)
			return FALSE;

		BOOL bOsVersionInfoEx=TRUE;

		OSVERSIONINFOEXW osvi;
		osvi.dwOSVersionInfoSize=sizeof(OSVERSIONINFOEXW);
		if(!GDP_GetVersionExW((LPOSVERSIONINFOW)&osvi))
		{
			osvi.dwOSVersionInfoSize=sizeof(OSVERSIONINFOW);
			if(!GDP_GetVersionExW((LPOSVERSIONINFOW)&osvi))
				return FALSE;

			bOsVersionInfoEx=FALSE;
		}

#define BUFSIZE 20
		typedef void (WINAPI *PGNSI)(LPSYSTEM_INFO);
		typedef BOOL (WINAPI *PGPI)(DWORD, DWORD, DWORD, DWORD, PDWORD);

		SYSTEM_INFO si;
		PGNSI pGNSI;

		pGNSI = (PGNSI) GetProcAddress(
			GetModuleHandle(TEXT("kernel32.dll")), 
			"GetNativeSystemInfo");
		if(NULL != pGNSI)
			pGNSI(&si);
		else GetSystemInfo(&si);

#define SM_SERVERR2									89

#define VER_SUITE_STORAGE_SERVER					0x00002000
#define VER_SUITE_COMPUTE_SERVER					0x00004000

		_tcscpy_s(lpDesc,dwDescLength,_T("win"));

		switch (osvi.dwPlatformId)
		{
			// Test for the Windows NT product family.
		case VER_PLATFORM_WIN32_NT:

			if(osvi.dwMajorVersion == 6)
			{
				if (osvi.dwMinorVersion == 3 )
				{
					_tcscat_s(lpDesc,dwDescLength,_T("8_1"));
				}

				if (osvi.dwMinorVersion == 2 )
				{
					_tcscat_s(lpDesc,dwDescLength,_T("8"));
				}

				if (osvi.dwMinorVersion == 1 )
				{
					if( osvi.wProductType == VER_NT_WORKSTATION )
						_tcscat_s(lpDesc,dwDescLength,_T("7"));
					else
						_tcscat_s(lpDesc,dwDescLength,_T("2008_r2"));
				}

				if (osvi.dwMinorVersion == 0 )
				{
					if( osvi.wProductType == VER_NT_WORKSTATION )
						_tcscat_s(lpDesc,dwDescLength,_T("vista"));
					else
						_tcscat_s(lpDesc,dwDescLength,_T("2008"));
				}
			}

			if(osvi.dwMajorVersion>=6)
			{
				if( si.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_AMD64 )
					_tcscat_s(lpDesc,dwDescLength,_T("_x64"));
				else if (si.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_INTEL )
					_tcscat_s(lpDesc,dwDescLength,_T("_x86"));
			}

			else if ( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 2 )
			{
				if( GetSystemMetrics(SM_SERVERR2) )
					_tcscat_s(lpDesc,dwDescLength,_T( "2003_r2"));
				else if ( osvi.wSuiteMask==VER_SUITE_STORAGE_SERVER )
					_tcscat_s(lpDesc,dwDescLength,_T( "2003"));
				else if( osvi.wProductType == VER_NT_WORKSTATION &&
					si.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_AMD64)
				{
					_tcscat_s(lpDesc,dwDescLength,_T( "xp_x64"));
				}
				else
					_tcscat_s(lpDesc,dwDescLength,_T("2003"));

				// Test for the server type.
				if ( osvi.wProductType != VER_NT_WORKSTATION )
				{
					if ( si.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_IA64 )
					{
						_tcscat_s(lpDesc,dwDescLength,_T("_ia64"));
					}

					else if ( si.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_AMD64 )
					{
						_tcscat_s(lpDesc,dwDescLength,_T("_x64"));
					}

					else
					{
						_tcscat_s(lpDesc,dwDescLength,_T("_x86"));
					}
				}
			}

			else if ( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 1 )
			{
				_tcscat_s(lpDesc,dwDescLength,_T("xp"));
			}

			else if ( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 0 )
			{
				_tcscat_s(lpDesc,dwDescLength,_T("2000"));

			}
		}

		return TRUE;

#undef SM_SERVERR2

#undef VER_SUITE_STORAGE_SERVER
#undef VER_SUITE_COMPUTE_SERVER

#undef BUFSIZE
	}
};

}