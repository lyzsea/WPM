#pragma once
#include <windows.h>
#include <string>

namespace report
{
	class CGReport
	{
	public:

		static CGReport* Instance()
		{
			static CGReport temp;
			return &temp;
		}


		HRESULT Start();
		void Stop();
		void AddTask(std::wstring& wsrequest);

		bool IsEmpty();
	private:
		CGReport(){};
		~CGReport(){}

	};
}
