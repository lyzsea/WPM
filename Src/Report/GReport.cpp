#include "GReport.h"
#include "report.h"

namespace report
{
	HRESULT CGReport::Start()
	{
		return SendReport::instance()->Start();
	}

	void CGReport::Stop()
	{
		SendReport::instance()->Stop();
	}

	void CGReport::AddTask(std::wstring& wsrequest)
	{
		SendReport::instance()->AddTask(wsrequest);
	}

	bool CGReport::IsEmpty()
	{
		return SendReport::instance()->IsEmpty();
	}
}

