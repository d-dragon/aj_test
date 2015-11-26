#ifndef _TEST_WORKER_H
#define _TEST_WORKER_H

#include "AlljoynClient.h"
#include <string>
#include <vector>

#define OK 0
#define ERROR -1


using namespace std;

class TestWorker {

		struct InputConfiguration{
			string serviceId;
			string deviceIndex;
			string altDeviceId;
			string deviceType;
			string associationDevIndex;
			string altAssDevId;
		};
		struct TestCaseInfo
		{
			string Signal;
			string Type;
			string ID;
		};

		struct ResponseMessageInfo
		{
			string message;
			string srcpath;
			string signalname;
		};
	public:

		struct InputConfiguration mConfig;
		static string htmlResultContent;
		TestWorker(const char *interface);
		~TestWorker();
		int startAlljoynClient(const char *serviceId);
		void StopAlljoynClient();
		int restartAlljoynClient();
		int executeTestItem(string testItem, size_t numArg, string tiArg[]);
		static void TIRespMonitor(int respFlag, const char *respMsg, const char *srcPath, const char *member);
		static int exportStuffToFile(const char* content);

	private:
		// Store all base infor of Test case and list all Devices.
		static struct TestCaseInfo mTestCaseInfo;
		static struct ResponseMessageInfo respmsg;
		static vector<struct DeviceInfo> mDeviceList;
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
		void ResponseAnalyst();
};

#endif
