////////////////////////////////////////////////////////////////////////////////
// MyProgramLog.cpp: implementation of the CMyProgramLog class.
//
//////////////////////////////////////////////////////////////////////
#define _CRT_SECURE_NO_WARNINGS 
#define _CRT_NON_CONFORMING_SWPRINTFS

#include <windows.h>

#include <TCHAR.h>
#include <stdio.h>
#include <time.h>
#include <io.h>

#include "ProgramLog.h"

namespace Dump{namespace log{
#ifndef FLAG_ISSET
#define FLAG_ISSET(flag,value)					(((flag) & (value))==(value))
#endif

#define MAX_LOGPATHLENGTH		520
#define MAX_LOG_BUFFER			(1024*100)
#define MAX_LOG_SINGLEBUFFER	(1024*10)

	// LPCTSTR pLogFilePath 日志文件路径，可以为绝对路径和相对路径，如果为相对路径，前面添加exe所在的目录，如果为NULL，则为exe所在的目录+pProjectName.log
	static void GetProgramLogPath(LPCTSTR pProjectName,LPCTSTR pLogFilePath,TCHAR* lpszPathOut)
	{
		if(pProjectName==NULL && pLogFilePath==NULL)
			return;

		TCHAR path_buffer[MAX_LOGPATHLENGTH];
		if(::GetModuleFileName(NULL,path_buffer,MAX_LOGPATHLENGTH)==0)
			return;

		TCHAR drive[_MAX_DRIVE];
		TCHAR dir[_MAX_DIR];
		TCHAR fname[_MAX_FNAME];
		TCHAR ext[_MAX_EXT];
		_tsplitpath_s(path_buffer, drive, _MAX_DRIVE, dir, _MAX_DIR,
			fname, _MAX_FNAME, ext, _MAX_EXT);

		if(pLogFilePath==NULL)
		{
			_tcscpy_s(lpszPathOut,MAX_LOGPATHLENGTH,drive);
			_tcscat_s(lpszPathOut,MAX_LOGPATHLENGTH,dir);
			_tcscat_s(lpszPathOut,MAX_LOGPATHLENGTH,_T("log\\"));
			_tcscat_s(lpszPathOut,MAX_LOGPATHLENGTH,pProjectName);
			_tcscat_s(lpszPathOut,MAX_LOGPATHLENGTH,_T(".LOG"));
		}
		else if(_tcslen(pLogFilePath)>=2 && (pLogFilePath[1]==_T(':') || (pLogFilePath[0]==_T('\\') && pLogFilePath[1]==_T('\\'))))
		{
			_tcscpy_s(lpszPathOut,MAX_LOGPATHLENGTH,pLogFilePath);
		}
		else
		{
			_tcscpy_s(lpszPathOut,MAX_LOGPATHLENGTH,drive);
			_tcscat_s(lpszPathOut,MAX_LOGPATHLENGTH,dir);
			_tcscat_s(lpszPathOut,MAX_LOGPATHLENGTH,pLogFilePath);
		}
	}

	void DeleteProgramLog(LPCTSTR pProjectName,LPCTSTR pLogFilePath)
	{
		if(pProjectName==NULL && pLogFilePath==NULL)
			return;

		TCHAR tcLogFilePath[MAX_LOGPATHLENGTH];
		GetProgramLogPath(pProjectName,pLogFilePath,tcLogFilePath);

		SetFileAttributes(tcLogFilePath,FILE_ATTRIBUTE_NORMAL);
		DeleteFile(tcLogFilePath);
	}

#ifdef __LOG_ENABLE__

	//////////////////////////////////////////////////////////////////////
	// Construction/Destruction
	//////////////////////////////////////////////////////////////////////

	class CISAFE_SafeCS  
	{
		LPCRITICAL_SECTION pcs;
	public:
		CISAFE_SafeCS(LPCRITICAL_SECTION lpcs)
		{
			pcs=lpcs;
			EnterCriticalSection(pcs);
		}

		~CISAFE_SafeCS()
		{
			LeaveCriticalSection(pcs);
		}
	};

	typedef struct tagISAFE_PROGRAMLOGPARAM
	{
		BOOL m_bInit;								// 是否正确初始化
		DWORD m_dwMask;								// 掩码
		DWORD m_dwFlag;								// 标志位
		DWORD m_dwMaxLogFileSize;					// 日志文件的最大尺寸(单位：KB)，0代表不自动删除日志文件
		DWORD m_dwMaxReserveSize;					// 日志文件的最大保留尺寸(单位：KB)，0代表全部删除，不保留
		DWORD m_dwInitTickCount;					// 初始时间
		CRITICAL_SECTION m_CS;						// 同步对象
		TCHAR *m_Buffer;							// 缓冲区
		DWORD m_dwBufferPos;						// 缓冲区当前位置
		DWORD m_dwMaxBufferSize;					// 缓冲区最大值
		TCHAR m_strLogFilePath[MAX_LOGPATHLENGTH];	// 日志文件路径
		TCHAR m_strProjectName[MAX_LOGPATHLENGTH];	// 项目名称
	}ISAFE_PROGRAMLOGPARAM,*PISAFE_PROGRAMLOGPARAM;

	static ISAFE_PROGRAMLOGPARAM m_GlobalParam={0};

#define LOG_Init				(m_GlobalParam.m_bInit)
#define LOG_dwMask				(m_GlobalParam.m_dwMask)
#define LOG_dwFlag				(m_GlobalParam.m_dwFlag)
#define LOG_dwMaxLogFileSize	(m_GlobalParam.m_dwMaxLogFileSize)
#define LOG_dwMaxReserveSize	(m_GlobalParam.m_dwMaxReserveSize)
#define LOG_dwInitTickCount		(m_GlobalParam.m_dwInitTickCount)
#define LOG_CS					(m_GlobalParam.m_CS)
#define LOG_Buffer				(m_GlobalParam.m_Buffer)
#define LOG_dwBufferPos			(m_GlobalParam.m_dwBufferPos)
#define LOG_dwMaxBufferSize		(m_GlobalParam.m_dwMaxBufferSize)
#define LOG_strLogFilePath		(m_GlobalParam.m_strLogFilePath)
#define LOG_strProjectName		(m_GlobalParam.m_strProjectName)

	static void ISAFE_WriteFile(HANDLE hFile,LPVOID pBuf,DWORD dwLength)
	{
		DWORD dwRet=0;
		WriteFile(hFile,pBuf,dwLength,&dwRet,NULL);
	}

	static void ISAFE_SafeWriteFile(HANDLE hFile,LPVOID pBuf,DWORD dwLength)
	{
		LPBYTE p=(LPBYTE)pBuf;
		BYTE bSeed=0x5A;

		for(DWORD i=0;i<dwLength;i++)
		{
			if(p[i]!=0 && p[i]!=bSeed)
				p[i]^=bSeed;
		}

		ISAFE_WriteFile(hFile,p,dwLength);
	}

	static void ISAFE_WriteFileBuf(HANDLE hFile,LPVOID pBuf,DWORD dwBufLength)
	{
		if(FLAG_ISSET(LOG_dwFlag,LOG_SAFE))
			ISAFE_SafeWriteFile(hFile,pBuf,dwBufLength);
		else
			ISAFE_WriteFile(hFile,pBuf,dwBufLength);
	}

#define ISAFE_WriteFileString(hFile,lpszLog)	ISAFE_WriteFileBuf(hFile,(LPBYTE)(lpszLog),(lpszLog)==NULL ? 0 : _tcslen(lpszLog)*sizeof(TCHAR))

	static void ISAFE_TruncLogFile(HANDLE hFile)
	{
		DWORD dwFileSize=GetFileSize(hFile,NULL);

		if(LOG_dwMaxLogFileSize!=0 && dwFileSize>LOG_dwMaxLogFileSize)
		{
			// 文件大小满足要求
			if(LOG_dwMaxReserveSize>0 && LOG_dwMaxReserveSize<LOG_dwMaxLogFileSize)
			{
				// 保留部分数据
				SetFilePointer(hFile,dwFileSize-LOG_dwMaxReserveSize,0,FILE_BEGIN);

				BYTE *pBuf=new BYTE[LOG_dwMaxReserveSize];
				if(pBuf==NULL)
				{
					SetFilePointer(hFile,0,0,FILE_END);
					return;
				}

				DWORD dwRead=0;
				DWORD dwRet;

				while(dwRead<LOG_dwMaxReserveSize)
				{
					if(!ReadFile(hFile,pBuf+dwRead,LOG_dwMaxReserveSize-dwRead,&dwRet,NULL))
						break;
					if(dwRet>=0)
						dwRead+=dwRet;
				}

				if(dwRead<LOG_dwMaxReserveSize)
				{
					delete[] pBuf;
					SetFilePointer(hFile,0,0,FILE_END);
					return;
				}

				SetFilePointer(hFile,0,0,FILE_BEGIN);

#ifdef _UNICODE
				WORD format=0xFEFF;			// little-endian UNICODE
				ISAFE_WriteFileBuf(hFile,(LPBYTE)&format,sizeof(WORD));
#endif	// _UNICODE

				ISAFE_WriteFileBuf(hFile,pBuf,LOG_dwMaxReserveSize);

				delete[] pBuf;
			}
			else
			{
				SetFilePointer(hFile,0,0,FILE_BEGIN);

#ifdef _UNICODE
				WORD format=0xFEFF;			// little-endian UNICODE
				ISAFE_WriteFileBuf(hFile,(LPBYTE)&format,sizeof(WORD));
#endif	// _UNICODE
			}

			SetEndOfFile(hFile);
		}
	}

	static void ISAFE_WriteLogFile(DWORD dwFlag,LPCTSTR lpszLog)
	{
		if(!LOG_Init)
			return;

		SetFileAttributes(LOG_strLogFilePath,FILE_ATTRIBUTE_NORMAL);

		HANDLE hFile=CreateFile(LOG_strLogFilePath,GENERIC_WRITE | GENERIC_READ,0,NULL,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
		if(hFile==INVALID_HANDLE_VALUE)
			return;

		if((dwFlag & LOG_dwFlag & LOG_DELETE_INIT)==LOG_DELETE_INIT ||
			(dwFlag & LOG_dwFlag & LOG_DELETE_CLOSE)==LOG_DELETE_CLOSE ||
			(dwFlag & LOG_dwFlag & LOG_DELETE_WRITE)==LOG_DELETE_WRITE)
		{
			ISAFE_TruncLogFile(hFile);
		}

		SetFilePointer(hFile,0,0,FILE_END);

#ifdef _UNICODE
		DWORD dwFileSize=GetFileSize(hFile,NULL);
		if(dwFileSize==0)
		{
			WORD format=0xFEFF;			// little-endian UNICODE
			ISAFE_WriteFileBuf(hFile,(LPBYTE)&format,sizeof(WORD));
		}
#endif	// _UNICODE

		ISAFE_WriteFileString(hFile,lpszLog);

		CloseHandle(hFile);
	}

	BOOL MyCreateDirectory(LPCTSTR lpszPath)
	{
		TCHAR tcPath[MAX_PATH];
		_tcscpy_s(tcPath,MAX_PATH,lpszPath);

		for(DWORD i=0;i<_tcslen(tcPath);i++)
		{
			if(tcPath[i]==_T('/'))
				tcPath[i]=_T('\\');
		}

		TCHAR *p=tcPath+3;

		while(TRUE)
		{
			p=_tcsstr(p,_T("\\"));

			if(p==NULL)
			{
//			CreateDirectory(strPath,NULL);
				break;
			}
			else
			{
				*p=_T('\0');
				CreateDirectory(tcPath,NULL);
				*p=_T('\\');
				p++;
			}
		}

		return TRUE;
	}

	void InitProgramLog(DWORD dwMask,DWORD dwFlag,DWORD dwLogBufSize,DWORD dwMaxLogFileSize,DWORD dwMaxReserveSize,LPCTSTR pProjectName,LPCTSTR pLogFilePath)
	{
		if(LOG_Init)
			return;

#if !defined(_DEBUG)
		if(!FLAG_ISSET(dwFlag,LOG_RELEASE))
		{
			ZeroMemory(&m_GlobalParam,sizeof(m_GlobalParam));
			LOG_Init=FALSE;
			return;
		}
#endif

		if(dwMaxReserveSize>=dwMaxLogFileSize)
			dwMaxReserveSize=0;

		TCHAR tcProgramFilePath[MAX_LOGPATHLENGTH]={0};
		if(::GetModuleFileName(NULL,tcProgramFilePath,MAX_LOGPATHLENGTH)==0)
			return;

		TCHAR drive[_MAX_DRIVE];
		TCHAR dir[_MAX_DIR];
		TCHAR fname[_MAX_FNAME];
		TCHAR ext[_MAX_EXT];
		_tsplitpath_s(tcProgramFilePath,drive, _MAX_DRIVE, dir, _MAX_DIR, 
			fname, _MAX_FNAME, ext, _MAX_EXT);

		ZeroMemory(&m_GlobalParam,sizeof(m_GlobalParam));
		LOG_dwMask=dwMask;
		LOG_dwFlag=dwFlag;
		LOG_dwMaxLogFileSize=dwMaxLogFileSize;
		LOG_dwMaxReserveSize=dwMaxReserveSize;
		LOG_dwInitTickCount=GetTickCount();

		if(pProjectName==NULL)
			_tcscpy_s(LOG_strProjectName,MAX_LOGPATHLENGTH,fname);
		else
			_tcscpy_s(LOG_strProjectName,MAX_LOGPATHLENGTH,pProjectName);

		GetProgramLogPath(LOG_strProjectName,pLogFilePath,LOG_strLogFilePath);


#if !defined(_DEBUG)
		if (FLAG_ISSET(dwFlag,LOG_RELEASE))
		{
			MyCreateDirectory(LOG_strLogFilePath);
		}
#else
		MyCreateDirectory(LOG_strLogFilePath);
#endif


		InitializeCriticalSection(&LOG_CS);

		if(dwLogBufSize==0)
		{
			LOG_Buffer=NULL;
			LOG_dwBufferPos=0;
			LOG_dwMaxBufferSize=0;
		}
		else
		{
			if(dwLogBufSize==(DWORD)-1)
				LOG_dwMaxBufferSize=MAX_LOG_BUFFER;
			else
			{
				LOG_dwMaxBufferSize=dwLogBufSize;
				if(LOG_dwMaxBufferSize>10*1024*1024)
					LOG_dwMaxBufferSize=10*1024*1024;
			}
			LOG_Buffer=new TCHAR[LOG_dwMaxBufferSize];
			LOG_dwBufferPos=0;
			if(LOG_Buffer==NULL)
				return;
		}

		LOG_Init=TRUE;

		CISAFE_SafeCS msc(&LOG_CS);

		TCHAR tcBuf[MAX_LOG_SINGLEBUFFER] = {0};
		size_t len = _tcslen(tcBuf);
		_stprintf_s(tcBuf, (MAX_LOG_SINGLEBUFFER-len), _T("\r\n\r\n"));

		if(FLAG_ISSET(LOG_dwFlag,LOG_PROJECTNAME))
			_stprintf_s(tcBuf+_tcslen(tcBuf), (MAX_LOG_SINGLEBUFFER-_tcslen(tcBuf)), _T("Project Name: %s\r\n"),LOG_strProjectName);

		_stprintf_s(tcBuf+_tcslen(tcBuf), (MAX_LOG_SINGLEBUFFER-_tcslen(tcBuf)), _T("Project Path<%u.%u>: %s\r\n"),GetCurrentProcessId(),GetCurrentThreadId(),tcProgramFilePath);

		if(FLAG_ISSET(LOG_dwFlag,LOG_TIME))
		{
			SYSTEMTIME st;
			GetLocalTime(&st);
			_stprintf_s(tcBuf+_tcslen(tcBuf), (MAX_LOG_SINGLEBUFFER-_tcslen(tcBuf)), _T("Start Time: %10d %04d-%02d-%02d %02d:%02d:%02d\r\n"),LOG_dwInitTickCount,
				st.wYear,st.wMonth,st.wDay,st.wHour,st.wMinute,st.wSecond);
		}

		_stprintf_s(tcBuf+_tcslen(tcBuf), (MAX_LOG_SINGLEBUFFER-_tcslen(tcBuf)), _T("\r\n"));

		ISAFE_WriteLogFile(LOG_DELETE_INIT | LOG_DELETE_WRITE,tcBuf);
	}

	void CloseProgramLog()
	{
		if(LOG_Init)
		{
			CISAFE_SafeCS msc(&LOG_CS);

			if(LOG_Buffer!=NULL && LOG_dwBufferPos>0)
			{
				ISAFE_WriteLogFile(LOG_DELETE_WRITE,LOG_Buffer);
				LOG_dwBufferPos=0;
			}

			TCHAR tcProgramFilePath[MAX_LOGPATHLENGTH]={0};
			if(::GetModuleFileName(NULL,tcProgramFilePath,MAX_LOGPATHLENGTH)==0)
				return;

			TCHAR tcBuf[MAX_LOG_SINGLEBUFFER] = {0};

			if(FLAG_ISSET(LOG_dwFlag,LOG_PROJECTNAME))
				_stprintf_s(tcBuf+_tcslen(tcBuf), (MAX_LOG_SINGLEBUFFER-_tcslen(tcBuf)), _T("\r\nProject Name: %s\r\n"),LOG_strProjectName);

			_stprintf_s(tcBuf+_tcslen(tcBuf), (MAX_LOG_SINGLEBUFFER-_tcslen(tcBuf)), _T("Project Path<%u.%u>: %s\r\n"),GetCurrentProcessId(),GetCurrentThreadId(),tcProgramFilePath);

			if(FLAG_ISSET(LOG_dwFlag,LOG_TIME))
			{
				SYSTEMTIME st;
				GetLocalTime(&st);
				_stprintf_s(tcBuf+_tcslen(tcBuf),(MAX_LOG_SINGLEBUFFER-_tcslen(tcBuf)),_T("End Time: %10d %04d-%02d-%02d %02d:%02d:%02d\r\n"),GetTickCount(),
					st.wYear,st.wMonth,st.wDay,st.wHour,st.wMinute,st.wSecond);
			}

			ISAFE_WriteLogFile(LOG_DELETE_CLOSE | LOG_DELETE_WRITE,tcBuf);

			if(LOG_Buffer!=NULL)
			{
				delete[] LOG_Buffer;
				LOG_Buffer=NULL;
			}
			LOG_dwBufferPos=0;
		}

		if(LOG_Init)
		{
			DeleteCriticalSection(&LOG_CS);
			LOG_Init=FALSE;
		}
	}

	void FlushProgramLog()	// 把缓存的日志写入文件
	{
		if(!LOG_Init)
			return;

		CISAFE_SafeCS msc(&LOG_CS);

		ISAFE_WriteLogFile(LOG_DELETE_WRITE,LOG_Buffer);
		LOG_dwBufferPos=0;
	}

	void WriteProgramLogString(DWORD dwMask,LPCTSTR lpszLogText)
	{
		if(!LOG_Init)
			return;

		if(!FLAG_ISSET(LOG_dwMask,dwMask))
			return;

		TCHAR tcBuf[MAX_LOG_SINGLEBUFFER] = {0};
		//tcBuf[0]=_T('\0');

		if(FLAG_ISSET(LOG_dwFlag,LOG_TIME))
		{
			DWORD uTick=GetTickCount();
			_stprintf_s(tcBuf+_tcslen(tcBuf),(MAX_LOG_SINGLEBUFFER-_tcslen(tcBuf)),_T("%10d %10.3f "),uTick,((double)uTick-LOG_dwInitTickCount)/CLOCKS_PER_SEC);
		}

		if(FLAG_ISSET(LOG_dwFlag,LOG_PROJECTNAME) && FLAG_ISSET(LOG_dwFlag,LOG_TIME))
		{
			if(FLAG_ISSET(LOG_dwFlag,LOG_THREADID))
				_stprintf_s(tcBuf+_tcslen(tcBuf),(MAX_LOG_SINGLEBUFFER-_tcslen(tcBuf)),_T("%s<%u>:: "),LOG_strProjectName,GetCurrentThreadId());
			else
				_stprintf_s(tcBuf+_tcslen(tcBuf),(MAX_LOG_SINGLEBUFFER-_tcslen(tcBuf)),_T("%s:: "),LOG_strProjectName);
		}

		_stprintf_s(tcBuf+_tcslen(tcBuf),(MAX_LOG_SINGLEBUFFER-_tcslen(tcBuf)),_T("%s\r\n"),lpszLogText);

		CISAFE_SafeCS msc(&LOG_CS);

		if(LOG_Buffer!=NULL)
		{
			if(LOG_dwBufferPos+_tcslen(tcBuf)*sizeof(TCHAR)<LOG_dwMaxBufferSize)
			{
				memcpy(LOG_Buffer+LOG_dwBufferPos,tcBuf,_tcslen(tcBuf)*sizeof(TCHAR));
				LOG_dwBufferPos+=_tcslen(tcBuf);
				LOG_Buffer[LOG_dwBufferPos]=_T('\0');
			}
			else
			{
				ISAFE_WriteLogFile(LOG_DELETE_WRITE,LOG_Buffer);
				LOG_dwBufferPos=0;

				ISAFE_WriteLogFile(LOG_DELETE_WRITE,tcBuf);
			}
		}
		else
			ISAFE_WriteLogFile(LOG_DELETE_WRITE,tcBuf);
	}

	inline TCHAR Hex(char x)
	{
		if(x>=0 && x<=9)
			return x+_T('0');
		else if(x>=10 && x<=16)
			return x-10+_T('A');

		return _T(' ');
	}

	void WriteProgramLogBin(DWORD dwMask,LPCTSTR lpszFront,LPCTSTR lpszBack,LPCTSTR lpszBuf,DWORD uBufLength)
	{
		if(!LOG_Init)
			return;

		if(!FLAG_ISSET(LOG_dwMask,dwMask))
			return;

		TCHAR tcBuf[MAX_LOG_SINGLEBUFFER];
		tcBuf[0]=_T('\0');

		if(lpszFront!=NULL)
		{
			_tcscat_s(tcBuf,MAX_LOG_SINGLEBUFFER,lpszFront);
		}

		if((lpszBuf!=NULL) && (uBufLength>0))
		{
			TCHAR *pText;

			DWORD uNewBufLen;
#ifdef _UNICODE
			uNewBufLen=uBufLength*5;
#else
			uNewBufLen=uBufLength*3;
#endif	// _UNICODE

			pText=(TCHAR*)malloc(uNewBufLen*sizeof(TCHAR));
			if(pText==NULL)
				return;

			DWORD uPos;
			for(DWORD i=0;i<uBufLength;i++)
			{
#ifdef _UNICODE
				uPos=i*5;
				pText[uPos+0]=Hex((lpszBuf[i]>>12) & 0x0F);
				pText[uPos+1]=Hex((lpszBuf[i]>>8) & 0x0F);
				pText[uPos+2]=Hex((lpszBuf[i]>>4) & 0x0F);
				pText[uPos+3]=Hex(lpszBuf[i] & 0x0F);
				pText[uPos+4]=_T(' ');
#else
				uPos=i*3;
				pText[uPos+0]=Hex((lpszBuf[i]>>4) & 0x0F);
				pText[uPos+1]=Hex(lpszBuf[i] & 0x0F);
				pText[uPos+2]=_T(' ');
#endif	// _UNICODE
			}

#ifdef _UNICODE
			pText[uPos+4]=_T('\0');
#else
			pText[uPos+2]=_T('\0');
#endif	// _UNICODE

			_tcscat_s(tcBuf,MAX_LOG_SINGLEBUFFER,pText);

			free(pText);
		}

		if(lpszBack!=NULL)
		{
			_tcscat_s(tcBuf,MAX_LOG_SINGLEBUFFER,lpszBack);
		}

		WriteProgramLogString(dwMask,tcBuf);
	}

	void WriteProgramLog(DWORD dwMask,LPCTSTR lpszFormat,...)
	{
		if(!LOG_Init)
			return;

		if(!FLAG_ISSET(LOG_dwMask,dwMask))
			return;

		TCHAR tcBuf[MAX_LOG_SINGLEBUFFER];

		va_list arglist;
		va_start(arglist,lpszFormat);

		_vstprintf_s(tcBuf,MAX_LOG_SINGLEBUFFER,lpszFormat,arglist);

		WriteProgramLogString(dwMask,tcBuf);
	}

	void WriteProgramLogNoMask(LPCTSTR lpszFormat,...)
	{
		if(!LOG_Init)
			return;

		TCHAR tcBuf[MAX_LOG_SINGLEBUFFER];

		va_list arglist;
		va_start(arglist,lpszFormat);

		_vstprintf_s(tcBuf,MAX_LOG_SINGLEBUFFER,lpszFormat,arglist);

		WriteProgramLogString(-1,tcBuf);
	}

#endif	// __ISAFE_LOG_ENABLE__
}}


