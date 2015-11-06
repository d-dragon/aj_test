/*testworker class*/
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <ctime>
#include <string>
#include <string.h>
#include "TestWorker.h"
#include "OnboardingTest.h"
#define TIME_OUT 100

using namespace std;
int TestWorker::signalRespFlag;
string TestWorker::reportFile;
string *TestWorker::mRespMsg = NULL;

TestWorker::TestWorker(char *interface){

	serviceInterface.assign(interface);
	ajClient = NULL;
	signalRespFlag = 0;
	mRespMsg = NULL;
	generateReportFileName();
}
TestWorker::~TestWorker(){

}

int TestWorker::startAlljoynClient(){

	QStatus status;
	ajClient = new	AlljoynClient();

	status = ajClient->InitAlljoynClient(serviceInterface.c_str());
	if(ER_OK != status){

		printf("init alljoyn client failed\n");
		return ERROR;
	}
	sleep(2);
	status = ajClient->ConnectServiceProvider(serviceInterface.c_str());
	if(ER_OK != status){

		printf("connect to service provider failed\n");
		return ERROR;
	}
	ajClient->RegisterCB(TIRespMonitor);
	return OK;
}

int TestWorker::executeTestItem(string testItem, size_t numArg, string tiArg[]){

	
	QStatus status;
	OnboardingTest *onboardingTestApp;
	int timeout = 0;

	cout << "executeTestItem: " << testItem << endl;
	for(size_t i = 0; i < numArg; i++){

		cout << "argument " << i << ": " << tiArg[i] << endl;
	}

	if(!testItem.compare("onboarding")){

		std::cout<<"procesing onboarding test\n";
		onboardingTestApp = new OnboardingTest(tiArg);
		status = onboardingTestApp->CreateBusAttachment();
		if ( ER_OK != status )
		{
			onboardingTestApp->FinishBusAttachment();
			return status;
		}
		while (timeout++ < TIME_OUT) {
		   usleep(100 * 1000);
		}
		onboardingTestApp->FinishBusAttachment();
	 
	}else if(!testItem.compare("listennotification")){
	
		if(!tiArg[0].compare("upnp")){

			int time;
			time = stoi(tiArg[1]);

			cout << "listen notification in: " <<  time << "s" << endl;
			sleep(time);
		}
	}else{
		//Save infor of Test Case
		mTestCaseInfo.Signal.assign(testItem.c_str());
		mTestCaseInfo.Type.assign(tiArg[0]);
		if ( numArg > 2 ){
//			mTestCaseInfo.ID.assign(tiArg[1]);
			UpdateDevIDOfTC(tiArg);
			LOGCXX("Update device ID: "<<tiArg[1] );
		}

		printf("processing signal test\n");
		ajClient->SendRequestSignal(testItem.c_str(), numArg, tiArg);
		while((signalRespFlag != 1) && (timeout < 300)){
			usleep(100000);
			timeout++;
			if(timeout == 300){
				cout << "Test item execution timeout!!!" << endl;
				//TODO - need implement action for this case
				exportStuffToFile("<tr><th>Result</th><td colspan=\"2\">");
				exportStuffToFile("Test item execution timeout!!!");
				exportStuffToFile("</td></tr></table><br>");

			}

		}
		ParseRespondedMsg();
		signalRespFlag = 0;
	
	}
}

void TestWorker::TIRespMonitor(int respFlag, const char *respMsg, const char *srcPath, const char *member){

	mRespMsg = new string(respMsg);
	cout << "Received Message: " << respMsg << endl;
	//TODO - export the result to file
	
	exportStuffToFile("<tr><th>Result</th><td colspan=\"2\">");
	exportStuffToFile(respMsg);
	exportStuffToFile("</td></tr></table><br>");
	signalRespFlag = respFlag;
}


int TestWorker::exportStuffToFile(const char* content){

	cout << "export content to file" << endl;
	//TODO - auto generate file name
	fstream outFile;
	outFile.open(reportFile.c_str(), fstream::out | fstream::app);
	if(!outFile.is_open()){
		cout << "can not open file" << endl;
		return -1;
	}

	outFile << content;
	outFile.close();
	return 0;

}

void TestWorker::generateReportFileName(){

	time_t rawTime;
	struct tm *timeInfo;
	char timeBuff[128];
	char reportName[256];

	time(&rawTime);
	timeInfo = localtime(&rawTime);

	strftime(timeBuff, 128, "%d-%m-%Y_%I:%M:%S", timeInfo);
	sprintf(reportName, "%s%s%s%s", "output/", "report-", timeBuff, ".html");
	reportFile.assign(reportName);

}

int TestWorker::ParseRespondedMsg(){
	// TODO: make a structure of signal name 
	int ret = 0;
	LOGCXX("TestWorker::ParseRespondedMsg");
	ret =  mTestCaseInfo.Signal.compare("list_devices");
	if ((0 == ret) && (0 == mTestCaseInfo.Type.compare("zwave")))
	{
		if ( NULL != mRespMsg ){
			GetDevIDFromList();	
		}
	}


	return ret;
}

void TestWorker::UpdateDevIDOfTC(string arg[]){
	LOGCXX("TestWorker::UpdateDevIDOfTC: "<< mTestCaseInfo.Signal.c_str());
	char *cstring, *saveptr;
	int cnt = 0;
	int pos;
	if (mTestCaseInfo.DeviceList.size() == 0){
		LOGCXX("There is no Dev ID was recorded");
		return;
	}
	if (mTestCaseInfo.Signal.compare("set_binary") == 0){ 
//		if (0 == condition.compare("from_adddevice_output")){
			arg[1].assign(mTestCaseInfo.DeviceList.front());
			LOGCXX("Update output: "<< arg[1].c_str()); 
//		}
	}	
	if(mTestCaseInfo.Signal.compare("get_binary") == 0){
		arg[1].assign(mTestCaseInfo.DeviceList.front());
		LOGCXX("Update output: "<< arg[1].c_str()); 
	}
	if(mTestCaseInfo.Signal.compare("set_rule") == 0) {
		string tempdevId, actions;
		LOGCXX("arg [3]: "<< arg[3].c_str());
		actions.assign(arg[3]);
		for ( cstring = strtok_r((char*)actions.c_str(), ";",&saveptr); cstring; cstring = strtok_r(NULL, ";", &saveptr)) {
			if (cnt++ == 1){
				LOGCXX(cstring);
				tempdevId.assign(cstring);
				break;
			}
		}
		// Save the first occurent
		pos = actions.find(tempdevId);
		LOGCXX("Temp deviD: " << tempdevId.c_str());
		//Replace 
		actions = arg[3].substr(0,pos) + ";" + mTestCaseInfo.DeviceList.front() + ";" + arg[3].substr(pos+tempdevId.length(), arg[3].length()- (pos+tempdevId.length()));
		LOGCXX("New actions string: " << actions); 
		arg[3].assign(actions);
	}
}


void TestWorker::GetDevIDFromList(){ 
	char *devInfo, *info,*devID,  *tmp, *tmp1, *tmp2;
	int cnt = 0;
	char *listDevs =(char *) mRespMsg->c_str();
	for ( devInfo = strtok_r(listDevs, "\n", &tmp); devInfo; devInfo = strtok_r( NULL, "\n", &tmp ) ){
		cnt = 0;
		// devInfo store Responded Msg from CRESS of only 1 dev zwave
		for ( info = strtok_r(devInfo, "|", &tmp1 ); info; info = strtok_r(NULL, "|", &tmp1) ){
			if ( cnt == 3 ) // Position of Dev ID in response message
			{
				devID = strtok_r(info, " ", &tmp2);				
				mTestCaseInfo.DeviceList.push_back(std::string(devID));
				LOGCXX("Found device ID: ["<<mTestCaseInfo.DeviceList.back().c_str()<<"]");
				break;
			}
			cnt++;
		}
	}
	delete mRespMsg;
	mRespMsg = NULL;

}
