#ifndef _TEST_WORKER_H
#define _TEST_WORKER_H

#include "common_def.h"
#include "AlljoynClient.h"
#include <string>
#include <vector>

#define OK 0
#define ERROR -1
#define LEN_128B 128
#define LEN_256B 256
#define LEN_512B 512

using namespace std;

class TestWorker {

	public:

		struct InputConfiguration{
			string serviceId;
			string deviceIndex;
			string altDeviceId;
			string deviceType;
			string associationDevIndex;
			string altAssDevId;
		};

		struct ResponseMessageInfo
		{
			string message;
			string srcpath;
			string signalname;
		};
		struct InputConfiguration mConfig;
		static string htmlResultContent;
		TestWorker(const char *interface);
		~TestWorker();
		int startAlljoynClient(const char *serviceId);
		void StopAlljoynClient();
		int restartAlljoynClient();
		int executeTestItem(string testItem, size_t numArg, string tiArg[], TestItemInfo **test_item_info, TestItem *t_test_item);
		static void TIRespMonitor(int respFlag, const char *respMsg, const char *srcPath, const char *member);
		static int exportStuffToFile(const char* content);

		/* Each worker has a Log Pool which store all response message 
		 * Worker will immediately push log into pool while callback invoked
		 * There are interfaces that manipulate Log Pool
		 */
		string GetPoolEleValue(int pool_index);
		void ClearLogPool();
		unsigned int GetPoolSize();
	private:
		// Store all base infor of Test case and list all Devices.
		static struct TestItemInfo mTestItemInfo;
		static struct ResponseMessageInfo respmsg;
		static vector<struct DeviceInfo> mDeviceList;
		static vector<string> mLogPool;
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
