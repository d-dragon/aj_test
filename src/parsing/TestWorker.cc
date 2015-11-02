/*testworker class*/
#include <stdio.h>
#include <iostream>
#include "TestWorker.h"
#include "OnboardingTest.h"

using namespace std;
TestWorker::TestWorker(char *interface){

	serviceInterface.assign(interface);
	ajClient = NULL;
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
	return status;
}

QStatus TestWorker::executeTestItem(string testItem, size_t numArg, string tiArg[]){

	
	QStatus status;

	cout << "executeTestItem: " << testItem << endl;
	for(size_t i = 0; i < numArg; i++){

		cout << "argument " << i << ": " << tiArg[i] << endl;
	}

	if(!testItem.compare("onboarding")){

		printf("procesing onboarding test\n");

	}else{

		printf("processing signal test\n");
		ajClient->SendRequestSignal(testItem.c_str(), numArg, tiArg);

	}
	sleep(3);
}
