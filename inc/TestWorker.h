#ifndef _TEST_WORKER_H
#define _TEST_WORKER_H

#include "AlljoynClient.h"
#include <string>
#include <vector>

#define OK 0
#define ERROR -1


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
		int startAlljoynClient();
		int stopAlljoynClient();
		int restartAlljoynClient();
		int executeTestItem(string testItem, size_t numArg, string tiArg[]);
		static void TIRespMonitor(int respFlag, const char *respMsg, const char *srcPath, const char *member);
		static int exportStuffToFile(const char* content);

	private:
		// Store all base infor of Test case and list all Devices.
		struct TestCaseInfo mTestCaseInfo;
		static int signalRespFlag;
		static string *mRespMsg;
		string serviceInterface;
		AlljoynClient *ajClient;
		static string reportFile;

		void  generateReportFileName();
		int ParseRespondedMsg();
		void GetDevIDFromList();
		void UpdateDevIDOfTC(string*);
		int GetIndexByCondition(string);
};

#endif
