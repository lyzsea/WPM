#include "report.h"
#include <process.h>
#include "Net/WinHttpClient.h"
#include <Sensapi.h>
#include "Safe/StrEncrypt.h"
#include "StrDefine.h"

#pragma comment(lib,"Sensapi.lib")
#pragma comment (lib,"Wininet.lib")


#ifdef _DEBUG
#include <iostream>
using namespace std;
#endif

//if network is break,loop to wait time.
#define  WATI_NETWORK_ACTIVE 60*1000
#define  RESEND_COUNT        5    //if network is active ,but send failed 5 times ,clear current task queue.

namespace report
{
	CStrEncrypt g_StrFun;
	CReportManager::CReportManager()
	{
		m_bWorking = false;
		m_bStarted = false;
		m_hSendThread = NULL;

		for (int i=0; i <2; ++i)
		{
			m_hEvents[i] = NULL;
		}
	}

	CReportManager::~CReportManager()
	{
		if (m_hSendThread != NULL)
		{
			CloseHandle(m_hSendThread);
		}
	}

	static unsigned __stdcall  ThreadFunc(void* pThis)
	{
		CReportManager* _this = static_cast<CReportManager*>(pThis);
		_this->ThreadProc();

		return 0;
	}

	void CReportManager::AddTask(std::wstring& wsrequest)
	{
		if (!m_bStarted)
		{
			return;
		}
		std::lock_guard<std::mutex> lg(m_mutexLock);

		m_queueTasks.push(wsrequest);

		if (!m_bWorking)
		{
			SetEvent(m_hEvents[Work_Notify]);
		}
	}

	bool CReportManager::SendRequest(const std::wstring &strRequest)
	{
		bool bRtn = false;
		//std::cout<<strings::ws2s(strRequest,CP_UTF8).c_str()<<endl;
		const std::wstring strUserAgent = L"Mozilla/4.0 (compatible; MSIE 9.0; Windows NT 6.1; Trident/5.0) in my heart of heart.";

		WinHttpClient httpReq(strRequest);

		// Send http request, a GET request by default.
		httpReq.SetUserAgent(strUserAgent);

		bRtn = httpReq.SendHttpRequest();
		
		// The response content.
		//std::wstring httpResponseContent = httpReq.GetResponseContent();

		return bRtn;
	}

	bool CReportManager::CheckNetworkAlive(const std::wstring& sCheckUrl)
	{
		DWORD dwCheckType = NETWORK_ALIVE_WAN;

		if (IsNetworkAlive((LPDWORD)&dwCheckType)
			&& InternetCheckConnection(sCheckUrl.c_str(), FLAG_ICC_FORCE_CONNECTION, NULL))
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	bool CReportManager::ExecTask()
	{
		bool bRtn = false;
		//std::lock_guard<std::mutex> lg(m_mutexLock);
		//std::wstring strTopTask =L"";
		m_mutexLock.lock();
		int nGetCount =0;
		while (!m_queueTasks.empty() && nGetCount < 100)
		{
			m_sendTasks.push(m_queueTasks.front());
			m_queueTasks.pop();
			++nGetCount;
		}
		m_mutexLock.unlock();

		bool bNetActive = CheckNetworkAlive(REPROT_XINGCLOUD_URL);

		while (!m_sendTasks.empty())
		{
			if (bNetActive)
			{
				bool bSendRtn = SendRequest(m_sendTasks.front());
				if (bSendRtn)
				{
					std::cout<<strings::ws2s(m_sendTasks.front(),CP_UTF8).c_str()<<endl;
					m_sendTasks.pop();
				}
				else
				{
					int nSendFailCount = 1;
					while (bNetActive =CheckNetworkAlive(REPROT_XINGCLOUD_URL))
					{
						if (SendRequest(m_sendTasks.front()))
						{
#ifdef _DEBUG
std::cout<<strings::ws2s(m_sendTasks.front(),CP_UTF8).c_str()<<endl;
#endif
							m_sendTasks.pop();
							break;
						}
						else
						{
							nSendFailCount++;
							if (nSendFailCount >= RESEND_COUNT)
							{
								while (!m_sendTasks.empty())
								{
#ifdef _DEBUG
									std::cout<<"clear task queue: "<<strings::ws2s(m_sendTasks.front(),CP_UTF8).c_str()<<endl;
#endif
									m_sendTasks.pop();
								}
								break;
							}
							Sleep(200);
						}
					}
				}
			}
			else
			{
				Sleep(WATI_NETWORK_ACTIVE);
				bNetActive = CheckNetworkAlive(REPROT_XINGCLOUD_URL);
			}
		} 

		return bRtn;
	}

	HRESULT CReportManager::ThreadProc()
	{
		while(true)
		{
			DWORD dwRet=WaitForMultipleObjects(2,m_hEvents,FALSE,INFINITE);

			switch (dwRet) 
			{ 
			case WAIT_OBJECT_0 + 0: 
				{
					m_bWorking = true;
					while(!m_queueTasks.empty())
					{
						ExecTask();
					}
					m_bWorking = false;
				}
				break; 

			case WAIT_OBJECT_0 + 1: 
				goto EXIT;
				break; 

			case WAIT_TIMEOUT:
				break;

				// Return value is invalid.
			default: 
				break;

			}

		}
EXIT:
		for (int i =0; i < 2; ++i)
		{
			CloseHandle(m_hEvents[i]);
			m_hEvents[i] = NULL;
		}

		return S_OK;
	}

	HRESULT CReportManager::Start()
	{
		if (m_bStarted)
		{
			return S_FALSE;
		}
		HRESULT hr = S_OK;

		for (int i =0; i< 2; ++i)
		{
			m_hEvents[i] = CreateEvent( 
				NULL,   // default security attributes
				FALSE,  // auto-reset event object
				FALSE,  // initial state is nonsignaled
				NULL);  // unnamed object
		}

		//std::thread thExecTask(ThreadFunc,this);
		//thExecTask.detach();
		unsigned uWorkThreadId;
		m_hSendThread = (HANDLE)_beginthreadex(NULL,0, &ThreadFunc, this, 0, &uWorkThreadId );

		if (NULL != m_hSendThread)
		{
			m_bStarted = true;
		}
		else
		{
			hr = E_FAIL;
			m_bStarted = false;
		}

		return hr;
	}

	bool CReportManager::IsEmpty()
	{
		bool bRtnFlag = false;
		if (m_queueTasks.empty() && m_sendTasks.empty())
		{
			bRtnFlag = true;
		}

		return bRtnFlag;
	}

	void CReportManager::Stop()
	{
		if (!m_bStarted)
			return;

		m_bStarted = FALSE;
		SetEvent(m_hEvents[Stop_Notify]);

		if (WaitForSingleObject(m_hSendThread, 60000) == WAIT_TIMEOUT)
		{
			TerminateThread(m_hSendThread, 0);
			CloseHandle(m_hSendThread);
			m_hSendThread = NULL;
		}

		for (int i =0 ;i < 2; ++i)
		{
			if (m_hEvents[i] != NULL)
			{
				CloseHandle(m_hEvents[i]);
				m_hEvents[i] = NULL;
			}
		}
	}
}

