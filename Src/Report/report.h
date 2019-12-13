#pragma once
#include <queue>
//#include <stack>
#include <windows.h>
#include <mutex>

#include "Utilities/Singleton.h"

namespace report
{
	//const std::wstring REPORT_URL = L"http://xa.xingcloud.com";

	enum Event_Index
	{
		Work_Notify =0,
		Stop_Notify =1
	};

	class CReportManager
	{
	public:
		HRESULT Start();
		void Stop();
		void AddTask(std::wstring& wsrequest);
		bool IsEmpty();
		HRESULT ThreadProc();
	private:
		bool SendRequest(const std::wstring &strRequest);
		bool CheckNetworkAlive(const std::wstring& sCheckUrl);

		bool ExecTask();
	private:
		std::wstring  m_userAgent;
		HANDLE        m_hSendThread;
		bool          m_bStarted;
		bool          m_bWorking;
		HANDLE        m_hEvents[2];//0: notify to work event ,1: notify thread to exit event.


		std::queue<std::wstring> m_queueTasks; //Main thread push task in it.
		std::queue<std::wstring> m_sendTasks;  //Get tasks from queue tasks, decrease the lock time.

		std::mutex               m_mutexLock;
	private:
		CReportManager();
		~CReportManager();

		DECLARE_SINGLETON_CLASS(CReportManager);
	};

	typedef Singleton<CReportManager> SendReport;
}

