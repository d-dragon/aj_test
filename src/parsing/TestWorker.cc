/*testworker class*/
#include <stdio.h>
#include <iostream>
#include "TestWorker.h"
#include "OnboardingTest.h"

using namespace std;
int TestWorker::signalRespFlag;
TestWorker::TestWorker(char *interface){

	serviceInterface.assign(interface);
	ajClient = NULL;
	signalRespFlag = 0;
}
TestWorker::~TestWorker(){

}

QStatus TestWorker::startAlljoynClient(){

	QStatus status;
	ajClient = new	AlljoynClient();

	status = ajClient->InitAlljoynClient(serviceInterface.c_str());
	if(ER_OK != status){

		printf("init alljoyn client failed\n");
		return status;
	}
	sleep(2);
	status = ajClient->ConnectServiceProvider(serviceInterface.c_str());
	if(ER_OK != status){

		printf("connect to service provider failed\n");
	}
	ajClient->RegisterCB(TIRespMonitor);
	return status;
}

QStatus TestWorker::executeTestItem(string testItem, size_t numArg, string tiArg[]){

	
	QStatus status;
	int timeout = 0;

	cout << "executeTestItem: " << testItem << endl;
	for(size_t i = 0; i < numArg; i++){

		cout << "argument " << i << ": " << tiArg[i] << endl;
	}

	if(!testItem.compare("onboarding")){

		printf("procesing onboarding test\n");

	}else{

		printf("processing signal test\n");
		ajClient->SendRequestSignal(testItem.c_str(), numArg, tiArg);
		while((signalRespFlag != 1) && (timeout < 300)){
			usleep(100000);
			timeout++;
			if(timeout == 300){
				cout << "Test item execution timeout!!!" << endl;
				//TODO - need implement action for this case
			}

		}
		signalRespFlag = 0;
	
	}
}

void TestWorker::TIRespMonitor(int respFlag, const char *respMsg, const char *srcPath, const char *member){
//void TestWorker::TIRespMonitor(int respFlag){

	cout << "Received Message: " << respMsg << endl;
	//TODO - export the result to file
	signalRespFlag = respFlag;
}
