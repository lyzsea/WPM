#if !defined(__ISAFE_PROGRAMLOG_H__)
#define __ISAFE_PROGRAMLOG_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <windows.h>

namespace Dump{namespace log{

	#ifdef __cplusplus
		extern "C"
		{
	#endif

			// 参数定义同InitProgramLog
			void DeleteProgramLog(LPCTSTR pProjectName=NULL,LPCTSTR pLogFilePath=NULL);

			// InitProgramLog函数的参数定义
			// DWORD dwMask的定义
	#define LOG_MASK_ALL		0xFFFFFFFF

			// DWORD dwFlag的定义
	#define LOG_DELETE_MASK		0x0000000F	// 控制日志的大小
	#define LOG_DELETE_NO		0x00000000	// 不自动删除日志文件，dwMaxLogFileSize/dwMaxReserveSize参数无任何意义
	#define LOG_DELETE_INIT		0x00000001	// 在初始化的时候根据文件大小来删除日志文件
	#define LOG_DELETE_CLOSE	0x00000002	// 在关闭的时候根据文件大小来删除日志文件
	#define LOG_DELETE_WRITE	0x00000004	// 在写日志的时候根据文件大小来删除日志文件

	#define LOG_RELEASE			0x80000000	// Release版本也输出日志，默认只有Debug版本输出日志
	#define LOG_SAFE			0x40000000	// 日志文件是否加密
	#define LOG_PROJECTNAME		0x20000000	// 日志文件是否显示项目名称
	#define LOG_THREADID		0x10000000	// 显示线程ID
	#define LOG_TIME			0x08000000	// 日志文件是否显示时间戳

	#define FLAG_INIT			(LOG_DELETE_NO | LOG_PROJECTNAME | LOG_THREADID | LOG_TIME)

			#define __LOG_ENABLE__

	#ifdef __LOG_ENABLE__

			// DWORD dwMask 掩码，控制哪些日志需要真正写入
			// DWORD dwFlag 标志位
			// DWORD dwLogBufSize 日志缓冲区的大小，0为不使用缓冲，-1为使用默认缓冲大小(100K)，其它为实际缓冲区的大小
			// DWORD dwMaxLogFileSize 日志文件的最大尺寸(单位：KB)，0代表不自动删除日志文件
			// DWORD dwMaxReserveSize 日志文件的最大保留尺寸(单位：KB)，0代表全部删除，不保留
			// LPCTSTR pProjectName 项目名称，如果为NULL，取exe文件名
			// LPCTSTR pLogFilePath 日志文件路径，可以为绝对路径和相对路径，如果为相对路径，前面添加exe所在的目录，如果为NULL，则为exe所在的目录+log\\+pProjectName.log

			void InitProgramLog(DWORD dwMask=LOG_MASK_ALL,DWORD dwFlag=FLAG_INIT,DWORD dwLogBufSize=-1,DWORD dwMaxLogFileSize=0,DWORD dwMaxReserveSize=0,LPCTSTR pProjectName=NULL,LPCTSTR pLogFilePath=NULL);
			void CloseProgramLog();

			void FlushProgramLog();	// 把缓存的日志写入文件

			void WriteProgramLogString(DWORD dwMask,LPCTSTR lpszLogText);
			void WriteProgramLogBin(DWORD dwMask,LPCTSTR lpszFront,LPCTSTR lpszBack,LPCTSTR lpszBuf,DWORD uBufLength);
			void WriteProgramLog(DWORD dwMask,LPCTSTR lpszFormat,...);
			void WriteProgramLogNoMask(LPCTSTR lpszFormat,...);

	#define WriteProgramLogStringNoMask(lpszLogText)						ISAFE_WriteProgramLogString(ISAFE_LOG_MASK_ALL,lpszLogText)
	#define WriteProgramLogBinNoMask(lpszFront,lpszBack,lpszBuf,nBufLength)	ISAFE_WriteProgramLogBin(ISAFE_LOG_MASK_ALL,lpszFront,lpszBack,lpszBuf,nBufLength)

	#else	// __LOG_ENABLE__

	#define InitProgramLog														__noop
	#define CloseProgramLog()													__noop
	#define FlushProgramLog()													__noop
	#define WriteProgramLogString(dwMask,lpszLogText)							__noop
	#define WriteProgramLogBin(dwMask,lpszFront,lpszBack,lpszBuf,nBufLength)	__noop
	#define WriteProgramLog														__noop
	#define WriteProgramLogNoMask												__noop

	#define WriteProgramLogStringNoMask(lpszLogText)							__noop
	#define WriteProgramLogBinNoMask(lpszFront,lpszBack,lpszBuf,nBufLength)		__noop

	#endif	// __LOG_ENABLE__

	#ifdef __cplusplus
		}
	#endif

}}


	
#endif // !defined(__ISAFE_PROGRAMLOG_H__)
