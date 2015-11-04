#ifndef _TEST_WORKER_H
#define _TEST_WORKER_H

#include "AlljoynClient.h"
#include <string>
#include <vector>
using namespace std;

class TestWorker {
	struct TestCaseInfo
		{
			string Signal;
			string Type;
			string ID;
			vector<string> DeviceList;
		};
	public:

		TestWorker(char *interface);
		~TestWorker();
		QStatus startAlljoynClient();
		QStatus stopAlljoynClient();
		QStatus restartAlljoynClient();
		QStatus executeTestItem(string testItem, size_t numArg, string tiArg[]);
		static void TIRespMonitor(int respFlag, const char *respMsg, const char *srcPath, const char *member);
		static int exportStuffToFile(const char* content);

	private:
		struct TestCaseInfo mTestCaseInfo;
		static int signalRespFlag;
		static string *mRespMsg;
		string serviceInterface;
		AlljoynClient *ajClient;
		static string reportFile;

		void  generateReportFileName();
		int ParseRespondedMsg();
};

#endif
