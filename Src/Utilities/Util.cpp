#include "Util.h"
#include <windows.h>
#include "eOSVer.h"
#include <tchar.h>
#include <shlwapi.h> 
#include <ShlObj.h>
#include "GetCRC32.h"
#include "strings/StringsHelper.h"
//#include "GetCRC32.h"

namespace Util{
	namespace help
	{
		std::wstring GetProgramRunDir(HMODULE hDllModule)
		{   
			wchar_t exeFullPath[MAX_PATH];
			std::wstring strPath = L"";

			GetModuleFileName(hDllModule,exeFullPath,MAX_PATH);
			strPath=(std::wstring)exeFullPath;
			int pos = strPath.find_last_of('\\', strPath.length());
			return strPath.substr(0, pos+1);
		}

	    bool ExecuteWait(LPCWSTR lpszProcessName, LPCWSTR lpszCmdLine, LPCWSTR lpCurrentDirectory, bool bForceAdmin, bool bWaitProcess,bool bIsshow,DWORD dwTimeOut)
		{
			BOOL bRet = FALSE;
			HANDLE hProcess = NULL;
			TCHAR szCmd[MAX_PATH * 2] = {0};

			SHELLEXECUTEINFO sei = {sizeof(SHELLEXECUTEINFO)};

			sei.fMask = SEE_MASK_NOCLOSEPROCESS;
			sei.hwnd = NULL;
			Utilities::CeOSVer osVer;
			if (!osVer.IsVistaOrLater())
			{
				sei.lpVerb = L"open";
			}
			else
				sei.lpVerb = bForceAdmin ? L"runas" : L"open";

			sei.lpFile = lpszProcessName;
			sei.lpParameters = (LPWSTR)(LPCWSTR)lpszCmdLine;
			bIsshow ? sei.nShow = SW_SHOWNORMAL : sei.nShow = SW_HIDE;
			
			sei.lpDirectory = lpCurrentDirectory;

			bRet = ::ShellExecuteEx(&sei);

			hProcess = sei.hProcess;

			if (bRet)
			{
				if (bWaitProcess)
				{
					::WaitForSingleObject(hProcess, dwTimeOut);
				}
				::CloseHandle(hProcess);
			}

			return bRet ? true : false;
		}

		BOOL MyCopyFile(LPCTSTR lpExistingFileName,LPCTSTR lpNewFileName,BOOL bFailIfExists)
		{
		
			if(CopyFile(lpExistingFileName,lpNewFileName,bFailIfExists))
				return TRUE;
		
			TCHAR tcTempFileName[MAX_PATH*2]={'\0'};
			_tcscpy_s(tcTempFileName,lpNewFileName);
			_tcscat_s(tcTempFileName,L".Tmp");
		
			// 方法一：
			// 原有的文件改名，拷贝文件，删除改名后的原有的文件(重新启动删除)
			while(1)
			{
		
				if(!DeleteFile(tcTempFileName) && _taccess(tcTempFileName,0)==0)
				{
					break;
				}
		
				if(!MoveFile(lpNewFileName,tcTempFileName) && _taccess(lpNewFileName,0)==0)
				{
					break;
				}
		
				if(!CopyFile(lpExistingFileName,lpNewFileName,bFailIfExists))
				{
					break;
				}
		
				if(!MoveFileEx(tcTempFileName,NULL,MOVEFILE_DELAY_UNTIL_REBOOT))
				{
		
				}
		
		
				return TRUE;
			}
		
			// 方法二：
			// 新的文件拷贝成备份文件，重新启动后替换
		
			if(!CopyFile(lpExistingFileName,tcTempFileName,FALSE))
			{
		
				return FALSE;
			}
		
			if(!MoveFileEx(tcTempFileName,lpNewFileName,MOVEFILE_REPLACE_EXISTING | MOVEFILE_DELAY_UNTIL_REBOOT))
			{
		
				return FALSE;
			}
			else
			{
				return TRUE;
			}

			return FALSE;
		}

		bool ReadKeyValueW(HKEY hKey,LPCWSTR lpSubKey,LPCWSTR lpValueName,DWORD dwType,LPBYTE lpValue,DWORD& dwLength,DWORD dwDesired)
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

				if(RegQueryValueExW(hKEY,lpValueName,NULL,&dwType,lpValue,&dwLength)!=ERROR_SUCCESS)
					goto ENDPOS;
			}


			bRet=true;
ENDPOS:
			if(hKEY!=NULL)
				RegCloseKey(hKEY);

			return bRet;
		}

		void DelSelf(std::wstring &strOutFileDir)
		{
			//采用批处理
			SHELLEXECUTEINFO ExeInfo;
			TCHAR     ExePath[MAX_PATH] = {0};
			TCHAR     ParamPath[MAX_PATH] = {0};
			TCHAR     ComposePath[MAX_PATH] = {0};

			GetModuleFileName(NULL,ExePath,MAX_PATH);
			{
				strOutFileDir=(std::wstring)ExePath;
				int pos = strOutFileDir.find_last_of('\\', strOutFileDir.length());
				strOutFileDir=  strOutFileDir.substr(0, pos);
			}

			GetShortPathName(ExePath,ExePath,MAX_PATH);

			GetEnvironmentVariable(_T("COMSPEC"),ComposePath,MAX_PATH);

			_tcscpy_s(ParamPath,_T("/c ping 127.0.0.1 -n 2 > nul && del /s/q "));
			std::wstring delFiles =strOutFileDir+L"*.*";
			_tcscat_s(ParamPath,delFiles.c_str());
			_tcscat_s(ParamPath, _T(" rd /s/q "));
			_tcscat_s(ParamPath,strOutFileDir.c_str());
			_tcscat_s(ParamPath,_T(" > nul"));

			ZeroMemory(&ExeInfo,sizeof(ExeInfo));
			ExeInfo.cbSize = sizeof(ExeInfo);
			ExeInfo.hwnd = 0;  
			ExeInfo.lpVerb = _T("Open");    
			ExeInfo.lpFile = ComposePath;    
			ExeInfo.lpParameters = ParamPath; 
			ExeInfo.nShow = SW_HIDE;     
			ExeInfo.fMask = SEE_MASK_NOCLOSEPROCESS; 

			if (ShellExecuteEx(&ExeInfo))
			{
				SetPriorityClass(ExeInfo.hProcess,IDLE_PRIORITY_CLASS);
				//设置本程序进程基本为实时执行，快速退出。
				SetPriorityClass(GetCurrentProcess(),REALTIME_PRIORITY_CLASS);
				SetThreadPriority(GetCurrentThread(),THREAD_PRIORITY_TIME_CRITICAL);
				//通知资源管理器，本程序删除
				SHChangeNotify(SHCNE_DELETE,SHCNF_PATH,ExePath,NULL);
			}
		}

		bool DeleteDirectory(std::wstring srcDir)
		{
			HANDLE	handle; 
			WIN32_FIND_DATA    FindFileData;
			std::wstring srcTempDir=L"";

			if(srcDir[srcDir.length() - 1] == '\\')
			{
				srcDir = srcDir.substr(0,srcDir.length()-1);
			}

			srcTempDir = srcDir;
			srcTempDir.append(L"\\*");

			handle = FindFirstFile(srcTempDir.c_str(),&FindFileData); 
			if(handle == INVALID_HANDLE_VALUE) 
			{ 
				return  false; 
			}

			srcTempDir  = (srcDir  + L"\\");

			while(true) 
			{ 
				if(!FindNextFile(handle,&FindFileData))
				{
					break; 
				}

				if(wcscmp(FindFileData.cFileName,L".") != 0 && wcscmp(FindFileData.cFileName,L"..") !=0) 
				{ 
					if((FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY) 
					{ 
						DeleteDirectory(srcTempDir + FindFileData.cFileName);
					} 
					else 
					{ 
						DeleteFile((srcTempDir + FindFileData.cFileName).c_str());
					} 
				} 
			} 

			FindClose(handle);
			RemoveDirectory(srcDir.c_str());

			return true;
		}

		void DelUninstallInfo(const std::wstring& keyName)
		{
			HKEY hkey = NULL;
			int iRet=0;

			iRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE,L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall",0,KEY_ALL_ACCESS,&hkey);

			if (iRet == ERROR_SUCCESS)
			{
				SHDeleteKeyW(hkey,keyName.c_str());
				RegCloseKey(hkey);
			}
		}

		void DeleteInstallFiles(std::wstring strDeleteDir)
		{
			char szTempDir[MAX_PATH] ={0};

			std::string strBatValue ="@echo uninstalling..\n\
									 :begin  \n \
									 del /s/q %1\*.* \n \
									 rd /s/q %1   \n\
									 if not exist %1 goto end \n\
									 goto begin \n\
									 :end";
			DWORD dwLen = GetTempPathA(MAX_PATH, szTempDir);

			sprintf_s(szTempDir,"%suninstall.bat",szTempDir);
			//if (PathFileExists(szTempDir))
			{
				HANDLE hOutFile = ::CreateFileA(szTempDir,GENERIC_WRITE, FILE_SHARE_READ, NULL,
					CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

				DWORD dwWrite =0;
				WriteFile(hOutFile,strBatValue.c_str(), strBatValue.length(), &dwWrite, NULL);
				FlushFileBuffers(hOutFile);

				CloseHandle(hOutFile);
				hOutFile = NULL;

				if (PathFileExistsA(szTempDir))
				{
					ShellExecuteA(NULL,"runas",szTempDir,strings::ws2s(strDeleteDir).c_str(),NULL,SW_HIDE);
				}
			}
		}

		//std::wstring GetThirdRunDir(std::wstring strExtra /* = L"MircosoftStudio" */)
		//{
		//	wchar_t szThirdDir[MAX_PATH] ={0};
		//	DWORD dwLen = GetTempPathW(MAX_PATH, szThirdDir);

		//	std::wstring strThirdDir = szThirdDir;
		//	strThirdDir.append(strExtra);

		//	return strThirdDir;
		//	//swprintf_s(szThirdDir,L"%s")
		//}

		//比较版本字符串大小 返回值相等为0 ，大于 为1 小于 为0
		int CompareVersion(const std::wstring strOld,const std::wstring strNew)
		{
			int nRtn = -1;
			std::string strFir = strings::ws2s(strOld);
			std::string strSec = strings::ws2s(strNew);

			strFir = strings::replace_all_distinct(strFir,".","");
			strSec =strings::replace_all_distinct(strSec,".","");


			int nFir = atoi(strFir.c_str());
			int nSec =atoi(strSec.c_str());

			if (nFir > nSec)
			{
				nRtn = 1;
			}
			else if (nFir == nSec)
			{
				nRtn = 0;
			}
			else
			{
				nRtn = -1;
			}

			return nRtn;
		}

		void GetFilesFullnameInDir(const std::wstring &strSourDir, 
			std::wstring strExtenal,
			std::vector<std::wstring> &filesPathVector,
			BOOL isIncludeSubDir) 
		{

			std::wstring strCurDir = strings::Trim(strSourDir,L"\\");
			WIN32_FIND_DATA FindFileData;
			HANDLE hFind = ::FindFirstFile((strCurDir + L"\\*"+strExtenal).c_str(), &FindFileData);
			if (hFind == INVALID_HANDLE_VALUE)
			{
				return;
			}
			do
			{
				if (!(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
				{
					std::wstring filePath = strCurDir  +L"\\"+ FindFileData.cFileName;
					filesPathVector.push_back(filePath);
				}
				else if (isIncludeSubDir 
					&& _tcscmp(FindFileData.cFileName,_T(".")) != 0 
					&& _tcscmp(FindFileData.cFileName,_T("..")) != 0)
				{
					GetFilesFullnameInDir(strCurDir +L"\\"+ FindFileData.cFileName,strExtenal,filesPathVector,isIncludeSubDir);
				}
			}while (FindNextFileW(hFind, &FindFileData) != 0);
		}

		void DeleteFilesInDirectory(const std::wstring &strSourDir, BOOL isIncludeSubDir) 
		{

			std::wstring strCurDir = strings::Trim(strSourDir,L"\\");
			WIN32_FIND_DATA FindFileData;
			HANDLE hFind = ::FindFirstFile((strCurDir + L"\\*.*").c_str(), &FindFileData);
			if (hFind == INVALID_HANDLE_VALUE)
			{
				return;
			}
			do
			{
				if (!(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
				{
					std::wstring filePath = strCurDir  +L"\\"+ FindFileData.cFileName;
					DeleteFile(filePath.c_str());
				}
				else if (isIncludeSubDir 
					&& _tcscmp(FindFileData.cFileName,_T(".")) != 0 
					&& _tcscmp(FindFileData.cFileName,_T("..")) != 0)
				{
					DeleteFilesInDirectory(strCurDir +L"\\"+ FindFileData.cFileName,isIncludeSubDir);
				}
			}while (FindNextFileW(hFind, &FindFileData) != 0);
		}

		__declspec(naked) DWORD GetCycleCount()
		{
			__asm
			{
				_emit 0x0F
					_emit 0x31
					ret
			}
		}

		BYTE AKL_GetRandomByte(DWORD dwRandom)
		{
			BYTE bSeed=(BYTE)(dwRandom & 0xFF);
			bSeed=(BYTE)((bSeed+0x07) | (((dwRandom >>8) & 0xFF)+0xD5));
			bSeed=(BYTE)(bSeed & ((dwRandom >>16) & 0xFF));
			bSeed=(BYTE)((bSeed & 0x55) ^ ((dwRandom >>24) & 0xFF));

			return bSeed;
		}

		BOOL AKL_RandomTreatByByte(PBYTE phead, DWORD dwNum, BYTE bRandom)
		{
			if(phead==NULL)
				return FALSE;

			for(DWORD i=0;i<dwNum;i++)
			{
				phead[i]^=bRandom;
				bRandom=(BYTE)(bRandom+i);
			}

			return TRUE;
		}

		BOOL AKL_RandomTreatByDword(PBYTE phead, DWORD dwNum, DWORD dwRandom)
		{
			return AKL_RandomTreatByByte(phead,dwNum,AKL_GetRandomByte(dwRandom));
		}

		BOOL ReadDataFileW(PVOID *ppBuf, DWORD *pLength, const wchar_t * const ptcPath, DWORD dwExtendLength)
		{
			HANDLE hFile=CreateFileW(ptcPath,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
			if(hFile==INVALID_HANDLE_VALUE)
				return FALSE;

			DWORD dwLength=GetFileSize(hFile,NULL);
			if(dwLength<=0)
			{
				CloseHandle(hFile);
				return FALSE;
			}

			BYTE *pBuf=new BYTE[dwLength+dwExtendLength];
			if(pBuf==NULL)
			{
				CloseHandle(hFile);
				return FALSE;
			}

			DWORD dwRead=0;
			DWORD dwRet;

			while(dwRead<dwLength)
			{
				if(!ReadFile(hFile,pBuf+dwRead,dwLength-dwRead,&dwRet,NULL))
					break;
				if(dwRet>=0)
					dwRead+=dwRet;
			}
			CloseHandle(hFile);

			if(dwRead<dwLength)
			{
				delete[] pBuf;
				return FALSE;
			}

			*ppBuf=pBuf;
			*pLength=dwLength;

			return TRUE;
		}

		BOOL WriteDataFileW(PVOID pBuf, DWORD dwLength, const wchar_t * const ptcPath)
		{
			SetFileAttributesW(ptcPath,FILE_ATTRIBUTE_NORMAL);

			HANDLE hFile=CreateFileW(ptcPath,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
			if(hFile==INVALID_HANDLE_VALUE)
			{
				DWORD dwErr = GetLastError();
				return FALSE;
			}

			DWORD dwRet;
			BOOL bRet=WriteFile(hFile,(PBYTE)pBuf,dwLength,&dwRet,NULL);

			CloseHandle(hFile);

			return bRet;
		}

		BOOL TreatReadSDFileHead(PVOID pDataBuf, PDWORD pDataLength, PDWORD pLastVersion, DWORD dwAllocLength)
		{
			DWORD dwHeadLen=sizeof(SD_FILEHEADEX);

			if(pDataBuf==NULL || pDataLength==NULL || *pDataLength<dwHeadLen)
				return FALSE;

			PSD_FILEHEADEX pHead=(PSD_FILEHEADEX)pDataBuf;
			if(pHead->m_dwLength+dwHeadLen!=*pDataLength)
				return FALSE;

			if(pLastVersion!=NULL)
				memcpy(pLastVersion,pHead->m_dwVersion,sizeof(pHead->m_dwVersion));

			*pDataLength=pHead->m_dwLength;

			if(pHead->m_dwLength>0)
			{
				DWORD dwCRC32= utils::CRC::GetCRC32(pHead->m_dwLength,(PBYTE)pDataBuf+dwHeadLen);
				if(dwCRC32!=pHead->m_dwCRC32)
					return FALSE;

				AKL_RandomTreatByDword((PBYTE)pDataBuf+dwHeadLen,*pDataLength,pHead->m_dwSeed);
				memmove((PBYTE)pDataBuf,(PBYTE)pDataBuf+dwHeadLen,*pDataLength);
			}

			return TRUE;
		}

		BOOL TreatWriteSDFileHead(PVOID pDataBuf, DWORD dwDataLength, PVOID *ppNewBuf, PDWORD pNewLength, PDWORD pdwVerison)
		{
			DWORD dwHeadLen=sizeof(SD_FILEHEADEX);

			if(pDataBuf==NULL || ppNewBuf==NULL || pNewLength==NULL)
				return FALSE;

			PBYTE pBuf=new BYTE[dwDataLength+dwHeadLen];
			if(pBuf==NULL)
				return FALSE;

			PSD_FILEHEADEX pHead=(PSD_FILEHEADEX)pBuf;

			if(pdwVerison==NULL)
				ZeroMemory(pHead->m_dwVersion,sizeof(pHead->m_dwVersion));
			else
				memcpy(pHead->m_dwVersion,pdwVerison,sizeof(pHead->m_dwVersion));

			pHead->m_dwLength=dwDataLength;

			if(pHead->m_dwLength>0)
			{
				pHead->m_dwSeed=GetCycleCount();

				memmove(pBuf+dwHeadLen,pDataBuf,pHead->m_dwLength);
				AKL_RandomTreatByDword((PBYTE)pBuf+dwHeadLen,pHead->m_dwLength,pHead->m_dwSeed);

				pHead->m_dwCRC32= utils::CRC::GetCRC32(pHead->m_dwLength,pBuf+dwHeadLen);
			}

			*ppNewBuf=pBuf;
			*pNewLength=pHead->m_dwLength+dwHeadLen;

			return TRUE;
		}

		bool ReadSoftDictFileW(wchar_t *ptcPath, PVOID *ppBuf, DWORD *pLength, DWORD *pdwVersion, DWORD dwExtendLength)
		{
			if(ReadDataFileW(ppBuf,pLength,ptcPath,dwExtendLength))
			{
				if(TreatReadSDFileHead(*ppBuf,pLength,pdwVersion,dwExtendLength))
					return true;
				else
				{
					if(*ppBuf!=NULL)
					{
						delete[] *ppBuf;
						*ppBuf=NULL;
					}

					return false;
				}
			}

			return false;
		}

		bool WriteSoftDictFileW(wchar_t *ptcPath, PVOID pBuf, DWORD dwLength, DWORD *pdwVersion)
		{
			PBYTE pNewBuf=NULL;
			DWORD dwNewLength=0;

			if(!TreatWriteSDFileHead(pBuf,dwLength,(PVOID*)&pNewBuf,&dwNewLength,pdwVersion))
				return false;

			if(!WriteDataFileW(pNewBuf,dwNewLength,ptcPath))
			{
				delete[] pNewBuf;
				return false;
			}

			delete[] pNewBuf;

			return true;
		}

		bool MyGetFileSize(std::wstring sFilePath, int* nSize)
		{
			HANDLE hFile = ::CreateFile(sFilePath.c_str(), FILE_READ_EA, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);

			if (hFile != INVALID_HANDLE_VALUE)  
			{
				*nSize = ::GetFileSize(hFile, NULL);
				CloseHandle(hFile);
				return true;
			}

			return false;
		}

	}

}