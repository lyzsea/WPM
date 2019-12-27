/************************************************************************/
/* 
    Date : 3/26/2013
	File : ESVCTOKENUTILITIES.CPP
    Author : Eadwin Li
    Function :  
	Version : 1.0
	Copyright (c) 2012 - 2013 iWinPhone Studio
*/
/************************************************************************/  
#include "WinToken.h"

#include <stdio.h>
#include <Userenv.h>
#include <psapi.h>
#include <Sddl.h>
#include "Procprivilege.h"

#include "Utilities/eOSVer.h"


#pragma comment(lib,"Psapi.lib")
#pragma comment(lib,"Userenv.lib")

#define  MAX_PATHEX (MAX_PATH*2)
namespace Utilities{namespace svc{


	HRESULT CeSvcTokenUtilities::ExecutebyToken( 
		const std::wstring& strExePath, 
		const std::wstring& strCmdline, 
		BOOL bShow,
		HANDLE m_hToken,
		DWORD& dwProcessId,DWORD& dwRetCode,bool bWait)

	{
		HRESULT hr = S_OK;
		PROCESS_INFORMATION pi;
		STARTUPINFO         si;
		CProcPrivilege      pri;
		std::wstring strCmdLineDuplicate;
		LPVOID pvEnvironment = NULL;
		BOOL bRet = FALSE;


		if ( NULL == m_hToken )
		{
			hr = E_HANDLE;
			goto Exit0;
		}

		memset( &si, 0, sizeof( si ) );
		memset( &pi, 0, sizeof( pi ) );

		si.cb = sizeof( STARTUPINFO );
		si.lpDesktop = L"winsta0\\default";
		si.wShowWindow = SW_HIDE;

		hr = pri.SetPri( TRUE, SE_ASSIGNPRIMARYTOKEN_NAME );
		if ( FAILED( hr ) )
		{
			goto Exit0;
		}

		hr = pri.SetPri( TRUE, SE_INCREASE_QUOTA_NAME );
		if ( FAILED( hr ) )
		{
			goto Exit0;
		}

		if ( strCmdline == L"" )
		{
			strCmdLineDuplicate = strExePath;
		}
		else
		{
			wchar_t strCmdLine[MAX_PATH*4] = {0};
			swprintf_s(strCmdLine,L"\"%s\" %s",strExePath.c_str(),strCmdline.c_str());
			strCmdLineDuplicate = strCmdLine;
		}


		bRet = CreateEnvironmentBlock( &pvEnvironment, m_hToken, FALSE );
		if ( !bRet )
		{
			pvEnvironment = NULL;
		}

		bRet = CreateProcessAsUserW( 
			m_hToken, 
			NULL, 
			(LPWSTR)strCmdLineDuplicate.c_str(), 
			NULL, 
			NULL, 
			FALSE, 
			NORMAL_PRIORITY_CLASS | CREATE_UNICODE_ENVIRONMENT,
			pvEnvironment,
			NULL,
			&si, 
			&pi
			);

		if ( !bRet )
		{
			hr = HRESULT_FROM_WIN32( GetLastError() );
			//MyWriteProgramLog(_T("CreateProcessAsUser Fail with Code %d"),GetLastError());
		}
Exit0:
		if ( pvEnvironment )
		{
			DestroyEnvironmentBlock(pvEnvironment);
			pvEnvironment = NULL;
		}

		if ( bRet )
		{
			if (bWait)
			{
				WaitForSingleObject(pi.hProcess,INFINITE);
			}
			//else
			{
				dwProcessId = pi.dwProcessId;
				GetExitCodeProcess(pi.hProcess, &dwRetCode);
				if ( pi.hThread )
				{
					CloseHandle( pi.hThread );
				}

				if ( pi.hProcess )
				{
					CloseHandle( pi.hProcess );
				}
			}
			
		}

		return hr;
	}

		BOOL CeSvcTokenUtilities::GetDosDeviceInfo(PDOSDEVICEINFO pInfo,DWORD& dwInfoNum)
		{
			TCHAR szTemp[MAX_PATHEX]={0};
			dwInfoNum=0;

			if (GetLogicalDriveStrings(MAX_PATHEX-1, szTemp)) 
			{
				TCHAR* p = szTemp;

				do 
				{
					pInfo[dwInfoNum].tcDosName[0]=*p;
					pInfo[dwInfoNum].tcDosName[1]=_T(':');
					pInfo[dwInfoNum].tcDosName[2]=_T('\0');

					// Look up each device name
					if (QueryDosDevice(pInfo[dwInfoNum].tcDosName, pInfo[dwInfoNum].tcDeviceName, MAX_PATHEX))
					{
						pInfo[dwInfoNum].tcDosName[2]=_T('\\');
						pInfo[dwInfoNum].tcDosName[3]=_T('\0');

						if(pInfo[dwInfoNum].tcDeviceName[_tcslen(pInfo[dwInfoNum].tcDeviceName)-1]!=_T('\\'))
							_tcscat_s(pInfo[dwInfoNum].tcDeviceName,MAX_PATHEX,_T("\\"));


						dwInfoNum++;
					}

					// Go to the next NULL character.
					while (*p++);
				} while (*p); // end of string
			}

			return TRUE;
		}

		BOOL GetProcessImagePath(HANDLE hProcess,LPTSTR lpszIamge,PDOSDEVICEINFO pInfo,DWORD dwInfoNum)
		{
			typedef DWORD (WINAPI *PGetProcessImageFileName)(
				__in   HANDLE hProcess,
				__out  LPTSTR lpImageFileName,
				__in   DWORD nSize
				);

			TCHAR tcTempPath[MAX_PATHEX] = {0};
			Utilities::CeOSVer osv;
			if(osv.IsWow64())
			{
				static PGetProcessImageFileName m_pGetProcessImageFileName=NULL;
				if(m_pGetProcessImageFileName==NULL)
				{
					TCHAR tcPath[MAX_PATHEX]={0};
					GetSystemDirectory(tcPath,MAX_PATHEX);

					if(tcPath[_tcslen(tcPath)-1]!=_T('\\'))
						_tcscat_s(tcPath,MAX_PATHEX,_T("\\"));

					_tcscat_s(tcPath,MAX_PATHEX,_T("psapi.dll"));

					HINSTANCE hPSAPI=LoadLibrary(tcPath);
					if(hPSAPI!=NULL)
						m_pGetProcessImageFileName=(PGetProcessImageFileName)GetProcAddress(hPSAPI,"GetProcessImageFileNameW");
				}

				if(m_pGetProcessImageFileName==NULL)
					return FALSE;

				if(!m_pGetProcessImageFileName(hProcess,lpszIamge,MAX_PATHEX))
					return FALSE;

				for(DWORD i=0;i<dwInfoNum;i++)
				{
					if(_tcsnicmp(pInfo[i].tcDeviceName,lpszIamge,_tcslen(pInfo[i].tcDeviceName))==0)
					{
						_tcscpy_s(tcTempPath,MAX_PATHEX,pInfo[i].tcDosName);
						_tcscat_s(tcTempPath,lpszIamge+_tcslen(pInfo[i].tcDeviceName));
						_tcscpy_s(lpszIamge,MAX_PATHEX,tcTempPath);
						//_tcscpy_s(lpszIamge,MAX_PATHEX,pInfo[i].tcDosName);
						//memmove(lpszIamge+_tcslen(lpszIamge),lpszIamge+_tcslen(pInfo[i].tcDeviceName),(_tcslen(lpszIamge+_tcslen(pInfo[i].tcDeviceName))+1)*sizeof(TCHAR));

					}
				}

				return TRUE;
			}
			else
			{
				HMODULE hMod;
				DWORD cbNeeded;

				if(EnumProcessModules(hProcess,&hMod,sizeof(hMod),&cbNeeded))
				{
					GetModuleFileNameEx(hProcess,hMod,lpszIamge,MAX_PATHEX);

					return TRUE;
				}
				else
				{
					DWORD dwError=GetLastError();

					return FALSE;
				}
			}

			return TRUE;
		}

		BOOL CeSvcTokenUtilities::GetProcessImage(DWORD dwPID,LPTSTR lpszImage,PDOSDEVICEINFO pInfo,DWORD dwInfoNum)
		{
			lpszImage[0]=_T('\0');
			LPCTSTR lpszHeadPath=_T("\\??\\");

			HANDLE hProcess = OpenProcess( PROCESS_QUERY_INFORMATION |
				PROCESS_VM_READ,
				FALSE, dwPID );

			if ( hProcess )
			{
				if(!GetProcessImagePath(hProcess,lpszImage,pInfo,dwInfoNum))
					return FALSE;

				if(_tcsncmp(lpszImage,lpszHeadPath,_tcslen(lpszHeadPath))==0)
					memmove(lpszImage,lpszImage+4,(_tcslen(lpszImage)-4)*sizeof(TCHAR)+1);

			}
			else
			{
				DWORD dwError=GetLastError();
			}

			CloseHandle(hProcess);

			return TRUE;
		}


		DWORD CeSvcTokenUtilities::GetExplorerIDBySessionID(DWORD dwFlag,DWORD dwSessionID)
		{

			if(dwFlag==0)
				dwFlag=GES_SESSIONONLY;

			if((dwFlag & (dwFlag-1))!=0)
				return 0;

			TCHAR tcExplorerPath[MAX_PATHEX]={_T('\0')};
			GetSystemWindowsDirectory(tcExplorerPath,MAX_PATHEX);
			if(tcExplorerPath[_tcslen(tcExplorerPath)-1]!=_T('\\'))
			{
				_tcscat_s(tcExplorerPath,MAX_PATHEX,_T("\\"));
			}
			_tcscat_s(tcExplorerPath,MAX_PATHEX,_T("Explorer.exe"));

			DWORD aProcesses[1024],cbNeeded,cProcesses;
			TCHAR tcProcessPath[MAX_PATHEX]={0};
			
			if(!EnumProcesses(aProcesses,sizeof(aProcesses),&cbNeeded))
				return TRUE;

			DWORD dwMinorSessionID=-1;
			DWORD dwMinorExplorerID=0;

			cProcesses=cbNeeded/sizeof(DWORD);

			DOSDEVICEINFO info[26]={0};
			DWORD dwInfoNum=0;
			GetDosDeviceInfo(info,dwInfoNum);

			for(DWORD j=0;j<cProcesses;j++)
			{
				GetProcessImage(aProcesses[j],tcProcessPath,info,dwInfoNum);

				if(_tcsnicmp(tcExplorerPath,tcProcessPath,_tcslen(tcExplorerPath))!=0)
					continue;


				if(FLAG_ISSET(dwFlag,GES_FIRST))
					return aProcesses[j];

				DWORD dwThisSessionID=0;
				if(!ProcessIdToSessionId(aProcesses[j],&dwThisSessionID))
					continue;

				if(dwThisSessionID==dwSessionID)
					return aProcesses[j];

				if(FLAG_ISSET(dwFlag,GES_MINORSESSION))
				{
					if(dwThisSessionID<dwMinorSessionID)
					{
						dwMinorSessionID=dwThisSessionID;
						dwMinorExplorerID=aProcesses[j];
					}
				}
			}

			if(FLAG_ISSET(dwFlag,GES_MINORSESSION))
				return dwMinorExplorerID;


			return 0;
		}


		HRESULT ObtainImpersonateToken(DWORD dwProcessId, HANDLE& hTokenObtain,BOOL bNeedAdmin)
		{
			HRESULT hr = S_OK;
			HANDLE hProcess = NULL;
			HANDLE hToken = NULL;
			HANDLE hDupToken = NULL;
			MY_TOKEN_MANDATORY_LABEL *pTIL = NULL;

			hProcess = ::OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, dwProcessId);
			if (!hProcess)
			{
				hr = AtlHresultFromLastError();
				goto Exit0;
			}

			BOOL bRet = ::OpenProcessToken(hProcess, TOKEN_ALL_ACCESS, &hToken);
			if (!bRet)
			{
				hr = AtlHresultFromLastError();
				goto Exit0;
			}

			if (bNeedAdmin)
			{
				DWORD dwBytesNeeded = 0;

				bRet = ::GetTokenInformation(
					hToken,
					(TOKEN_INFORMATION_CLASS)MYTokenIntegrityLevel,
					NULL, 0, &dwBytesNeeded
					);

				pTIL = (MY_TOKEN_MANDATORY_LABEL *)new BYTE[dwBytesNeeded];
				if (!pTIL)
				{
					hr = E_OUTOFMEMORY;
					goto Exit0;
				}

				bRet = ::GetTokenInformation(
					hToken,
					(TOKEN_INFORMATION_CLASS)MYTokenIntegrityLevel,
					pTIL, dwBytesNeeded, &dwBytesNeeded
					);
				if (!bRet || !pTIL)
				{
					hr = AtlHresultFromLastError();
					goto Exit0;
				}

				SID* pSid = static_cast<SID*>(pTIL->Label.Sid);
				if (!pSid)
				{
					hr = E_FAIL;
					goto Exit0;
				}

				if (SECURITY_MANDATORY_HIGH_RID != pSid->SubAuthority[0])
				{
					MY_TOKEN_LINKED_TOKEN linkedToken = {0};
					DWORD dwSize = sizeof linkedToken;

					bRet = ::GetTokenInformation(
						hToken,
						(TOKEN_INFORMATION_CLASS)MYTokenLinkedToken,
						&linkedToken,
						dwSize, &dwSize
						);
					if (!bRet)
					{
						hr = AtlHresultFromLastError();
						goto Exit0;
					}

					CloseHandle( hToken );
					hToken = linkedToken.LinkedToken;
					linkedToken.LinkedToken = NULL;
				}
			}

			bRet = ::DuplicateTokenEx(
				hToken, 
				TOKEN_ALL_ACCESS, 
				0, 
				SecurityImpersonation, 
				TokenPrimary, 
				&hDupToken);
			if (!bRet)
			{
				hr = AtlHresultFromLastError();
				goto Exit0;
			}

			if (bNeedAdmin)
			{
				{
					PWSTR szIntegritySid= L"S-1-16-12288"; //high

					PSID  pIntegritySid  = NULL;
					MY_TOKEN_MANDATORY_LABEL til = {0};

					bRet = ConvertStringSidToSid(szIntegritySid, &pIntegritySid);
					if (!bRet)
					{
						hr = AtlHresultFromLastError();
						goto Exit0;
					}

					til.Label.Attributes = SE_GROUP_INTEGRITY;
					til.Label.Sid = pIntegritySid;

					bRet = ::SetTokenInformation(hDupToken, (TOKEN_INFORMATION_CLASS)MYTokenIntegrityLevel,  &til, sizeof(MY_TOKEN_MANDATORY_LABEL) );
					if (!bRet)
					{
						hr = AtlHresultFromLastError();
						goto Exit0;
					}

					if (pIntegritySid)
					{
						::LocalFree((HLOCAL)pIntegritySid);
						pIntegritySid = NULL;
					}
				}
			}
Exit0:
			if ( SUCCEEDED( hr ) )
			{
				hTokenObtain = hDupToken;
			}
			else
			{
				if ( hDupToken )
				{
					CloseHandle( hDupToken );
					hDupToken = NULL;
				}
			}


			if (pTIL)
			{
				delete[] pTIL;
				pTIL = NULL;
			}

			if ( hToken )
			{
				CloseHandle( hToken );
				hToken = NULL;
			}

			if ( hProcess )
			{
				CloseHandle( hProcess );
				hProcess = NULL;
			}

			return hr;
		}


		HRESULT CeSvcTokenUtilities::ObtainExplorerToken(HANDLE& hTokenObtain,BOOL bNeedAdmin /*= TRUE*/)
		{

			HRESULT hr;

			Utilities::CeOSVer osv;
			DWORD dwSessionID=osv.IsVistaOrLater() ? 1 : 0;
			DWORD dwExplorerID=GetExplorerIDBySessionID(GES_MINORSESSION,dwSessionID);

			hr = ObtainImpersonateToken(dwExplorerID, hTokenObtain,bNeedAdmin);
			if (FAILED(hr))
				return E_FAIL;

			return E_FAIL;
			return S_OK;
		}

}}