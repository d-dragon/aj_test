#ifndef _TEST_WORKER_H
#define _TEST_WORKER_H

#include "AlljoynClient.h"
#include <string>

using namespace std;

class TestWorker {

	public:
		TestWorker(char *interface);
		~TestWorker();
		QStatus startAlljoynClient();
		QStatus stopAlljoynClient();
		QStatus restartAlljoynClient();
		QStatus executeTestItem(string testItem, size_t numArg, string tiArg[]);

	private:
		string serviceInterface;
		AlljoynClient *ajClient;

};

#endif
