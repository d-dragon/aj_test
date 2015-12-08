/*testworker class*/
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <ctime>
#include <string>
#include <string.h>
#include "TestWorker.h"
#include "OnboardingTest.h"
#include "JsonParser.h"

#define TIME_OUT 100
#define DEVICE_INDEX 1
#define ASSOCIATED_INDEX 5

using namespace std;

int TestWorker::signalRespFlag;
string TestWorker::reportFile;
string *TestWorker::mRespMsg = NULL;
vector<struct DeviceInfo> TestWorker::mDeviceList;
string TestWorker::htmlResultContent;
vector<string> TestWorker::mLogPool;

string ReplaceAll(string str, const string& from, const string& to) {
	size_t start_pos = 0;
	while((start_pos = str.find(from, start_pos)) != string::npos) {
		str.replace(start_pos, from.length(), to);
		start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
	}
	return str;
}


struct TestWorker::TestItemInfo TestWorker::mTestItemInfo;
struct TestWorker::ResponseMessageInfo TestWorker::respmsg;

TestWorker::TestWorker(const char *interface){

	serviceInterface.assign(interface);
	ajClient = NULL;
	signalRespFlag = 0;
	mRespMsg = NULL;
	mLogPool.reserve(LEN_512B);
	mDeviceList.clear(); 
	generateReportFileName();
}
TestWorker::~TestWorker(){

}

int TestWorker::startAlljoynClient(const char *serviceId){

	QStatus status;
	char *targetDevId;
	ajClient = new	AlljoynClient();

	ajClient->SetRefTargetDeviceID(serviceId);
	cout << "connect to service provider: " << serviceId << endl;
	status = ajClient->InitAlljoynClient(serviceInterface.c_str());
	if(ER_OK != status){

		printf("init alljoyn client failed\n");
		return ERROR;
	}
	sleep(10);

	status = ajClient->GetTargetDeviceID(&targetDevId);
	if(ER_OK != status){

		printf("Get targetDevId failed\n");
		return ERROR;
	}
	LOGCXX("Compare DEV ID : "<< targetDevId);
	if (0 != strcmp(serviceId, targetDevId)){
		//deallocate ajClient will be handled by caller (call StopAlljoynClient) 
		return ERROR;
	}

	status = ajClient->ConnectServiceProvider(serviceInterface.c_str());
	if(ER_OK != status){

		printf("connect to service provider failed\n");
		return ERROR;
	}
	ajClient->RegisterCB(TIRespMonitor);
	LOGCXX("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
	return OK;
}

void TestWorker::StopAlljoynClient(){

	delete ajClient;
	mTestItemInfo.Signal = "";
}

int TestWorker::executeTestItem(string testItem, size_t numArg, string tiArg[], TestItemInfo *test_item_info){

	
	QStatus status;
	int timeout = 0;
	OnboardingTest *onboardingTestApp;
	signalRespFlag = 0;
	static unsigned int last_log_index = 0;
	
	cout << "execute test item: " << testItem << endl;

	mTestItemInfo.Signal.clear();
	mTestItemInfo.Type.clear();
	
	htmlResultContent.clear();
	//Save infor of Test Case
	mTestItemInfo.Signal.assign(testItem);
	mTestItemInfo.Type.assign(tiArg[0]);
	mTestItemInfo.StartLogIndex = last_log_index;

	for(size_t i = 0; i < numArg; i++){

		cout << "argument " << i << ": " << tiArg[i] << endl;
	}

	if(!testItem.compare("onboarding")){

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
	 
	}else if(!testItem.compare("listen_notification")){
	

			int time;
			time = stoi(tiArg[2]);

			cout << "listen notification in: " <<  time << "s" << endl;
			htmlResultContent.append("<tr><th>Result</th>");
			sleep(time);
			htmlResultContent.append("</tr></table><br>");
	}else{
		if ( numArg > 1 ){
			UpdateDevIDOfTC(tiArg);
		}

		ajClient->SendRequestSignal(testItem.c_str(), numArg, tiArg);

		cout << "call ResponseAnalyst" << endl;
		ResponseAnalyst();
		ParseRespondedMsg();

		
	
	}

	last_log_index = (unsigned int)mLogPool.size();
	mTestItemInfo.EndLogIndex = last_log_index - 1;
	
	cout << "Start log at: " << mTestItemInfo.StartLogIndex << endl;
	cout << "End Log at: " << mTestItemInfo.EndLogIndex << endl;
	cout << "Matched Log at: " << mTestItemInfo.MatchedLogIndex << endl;
	
	test_item_info = &mTestItemInfo;
	
	signalRespFlag = 0;
	respmsg.message.clear();
	respmsg.srcpath.clear();
	respmsg.signalname.clear();
	return status;
}

void TestWorker::ResponseAnalyst(){

	int timeout = 0;

	while((signalRespFlag != 1) && (timeout < 300)){
		usleep(100000);
		timeout++;
		if(timeout == 300){
			cout << "Test item execution timeout!!!" << endl;
			//TODO - need implement action for this case
			htmlResultContent.append("<tr><th>Result</th><td colspan=\"2\">");
			htmlResultContent.append("Test item execution timeout!!!");
			htmlResultContent.append("</td></tr></table><br>");

		}

	}
	if(timeout < 300){
		//received response message from callback
		//parse and analyse json message then export 
		
		if((mTestItemInfo.Type.compare("zwave") == 0) && (respmsg.signalname.compare("notify") != 0 )){
			string status;
			string reason;
			JsonParser parser(NULL, NULL, NULL, NULL);

			parser.JSONGetObjectValue(&respmsg.message,"status", &status);
			htmlResultContent.append("<tr><th>Result</th><td colspan=\"2\">");
			if(status.length() > 0){

				cout << "status " << status << endl;
				htmlResultContent.append(status.c_str());
				if(status.compare("failed") == 0){
					parser.JSONGetObjectValue(&respmsg.message, "reason", &reason);
					if(reason.length() > 0){
						htmlResultContent.append(" | Reason: ");
						htmlResultContent.append(reason.c_str());
					}
				}
			}else{

				htmlResultContent.append("Response message has no execution status");
			}

			htmlResultContent.append("</td></tr>");
		}else if(mTestItemInfo.Type.compare("upnp") == 0){


			respmsg.message = ReplaceAll(respmsg.message, "\n", "<br><br>");
			htmlResultContent.append("<tr><th>Result</th><td colspan=\"2\">");

			htmlResultContent.append(respmsg.message.c_str());
			htmlResultContent.append("</td></tr>");
		}

		htmlResultContent.append("<tr><th>Response Message</th><td colspan=\"2\">");
		htmlResultContent.append(respmsg.message.c_str());
		htmlResultContent.append("</td></tr></table><br>");
	}

}
void TestWorker::TIRespMonitor(int respFlag, const char *respMsg, const char *srcPath, const char *member){

	string tmpStr;
	tmpStr.assign(respMsg);
	mLogPool.push_back(tmpStr);
	cout << "Message: " << respMsg << endl; 
	// TO DO
	if (0 != mTestItemInfo.Signal.compare(member))
	{
		if((0 == mTestItemInfo.Signal.compare("listen_notification")) && (strcmp(member, "notify") == 0)){
			//export the result to report file	
			tmpStr = ReplaceAll(tmpStr, "\n", "<br><br>");
			htmlResultContent.append("<td colspan=\"2\">");
			htmlResultContent.append(tmpStr.c_str());
			htmlResultContent.append("</td>");
			mTestItemInfo.MatchedLogIndex = ((unsigned int)mLogPool.size() - 1);
		}
		return;
	}else{

		respmsg.message.assign(respMsg);
		respmsg.srcpath.assign(srcPath);
		respmsg.signalname.assign(member);
		mTestItemInfo.MatchedLogIndex = ((unsigned int)mLogPool.size() - 1);
		signalRespFlag = respFlag;
	}
}


int TestWorker::exportStuffToFile(const char* content){

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
	ret =  mTestItemInfo.Signal.compare("list_devices");
	if (0 == ret)
	{
		// Clear list dev iD before add it again
		LOGCXX("TestWorker::ParseRespondedMsg condition");
		mDeviceList.clear();
		if (respmsg.message.length() > 0){
			GetDevIDFromList();	
		}
	}


	return ret;
}
	/*
	Function: 	support user update device ID of test case, base on index of test case
	Input: 		String from test case
	Output: 	arg with ID of device will be changed corresponding with index
				if wrong index: the first device will be fill instead.
	*/
void TestWorker::UpdateDevIDOfTC(string arg[]){
	LOGCXX("TestWorker::UpdateDevIDOfTC: "<< mTestItemInfo.Signal.c_str());
	char *cstring, *saveptr;
	int cnt = 0;
	int pos;
	string tempdevId, actions, newIDValue, stringsuffix, *saveActionInput;

	saveActionInput = NULL;

	if (mDeviceList.size() == 0){
		LOGCXX("There is no Dev ID was recorded");
		return;
	}

	/*
		Handle normal signals
	*/
	if ((mTestItemInfo.Signal.compare("set_binary") 	== 0) ||
		(mTestItemInfo.Signal.compare("get_binary") 	== 0) ||
		(mTestItemInfo.Signal.compare("read_spec") 		== 0) ||
		(mTestItemInfo.Signal.compare("write_spec") 	== 0) ||
		(mTestItemInfo.Signal.compare("read_s_spec") 	== 0) ||
		(mTestItemInfo.Signal.compare("remove_device") 	== 0) ||
		(mTestItemInfo.Signal.compare("write_s_spec") 	== 0)
		)
	{
		if(arg[DEVICE_INDEX].find("ID") == std::string::npos){
			// Keep the input from user
			return;
		}

		//replace device id such as configured (specific device id or getting device id from its index of device list) 
		int index;
		if(mConfig.altDeviceId.length() > 0){
			arg[DEVICE_INDEX].assign(mConfig.altDeviceId); 
		}else{
			index = stoi(mConfig.deviceIndex);
			newIDValue = mDeviceList.at(index).ID;
			arg[DEVICE_INDEX].assign(newIDValue);
			LOGCXX("Update new Dev ID: "<< arg[1].c_str()); 
		}
		newIDValue.clear();

		//in case of sensor set association with another device, update associated device id
		if((mTestItemInfo.Signal.compare("write_spec")	 == 0) ||
				(mTestItemInfo.Signal.compare("write_s_spec")	== 0)){

			//if A_ID is defined in test suite, it will be replaced by users  configuration. If not, kepp the defined test suite value
			if(arg[ASSOCIATED_INDEX].find("ID") != string::npos){ 				if(mConfig.altAssDevId.length() > 0){
					arg[ASSOCIATED_INDEX].assign(mConfig.altAssDevId);
				}else{

					index = stoi(mConfig.associationDevIndex);
					newIDValue = mDeviceList.at(index).ID;
					arg[ASSOCIATED_INDEX].assign(newIDValue);
				}
			}
			newIDValue.clear();
		}

	}

	/*
		Handle actions 
	*/
	if(mTestItemInfo.Signal.compare("set_rule") 		== 0) {
		saveActionInput = &arg[3];
	}
	if(mTestItemInfo.Signal.compare("rule_actions") 	== 0){
		saveActionInput = &arg[5];
	}
	if (saveActionInput != NULL){
		if(saveActionInput->find("ID") == std::string::npos){
			// Keep the input from user
			return;
		}
		LOGCXX("Action input "<< saveActionInput->c_str());
		actions.assign(*saveActionInput);
		for ( cstring 	= strtok_r((char*)actions.c_str(), ";",&saveptr); cstring; cstring = strtok_r(NULL, ";", &saveptr)) {
			if (cnt++ == 1){
				LOGCXX(cstring);
				tempdevId.assign(cstring);
				break;
			}
		}
		// Save the first occurent
		pos 			= actions.find(tempdevId);
		LOGCXX("Temporary dev id: " << tempdevId.c_str());
		//Replace 
		newIDValue 		= mDeviceList.at(GetIndexByCondition(tempdevId)).ID;
		stringsuffix	= saveActionInput->substr(pos+tempdevId.length(), saveActionInput->length()- (pos+tempdevId.length()));
		actions 		= saveActionInput->substr(0,pos) + ";" + newIDValue + ";" + stringsuffix;
		saveActionInput->assign(actions);
		LOGCXX("Update new Dev ID -> Updated actions: "<< *saveActionInput);
	}
	/*
		Handle other
	*/
}
	/*
	Function: 	Get device IDs from response of list_devices signal on Zwave
	Input: 		responded msg from list_devices
	Output: 	save all device order in this list: mTestItemInfo.DeviceList
	*/

void TestWorker::GetDevIDFromList(){ 
#if 0
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
				mTestItemInfo.DeviceList.push_back(std::string(devID));
				LOGCXX("Found device ID: ["<<mTestItemInfo.DeviceList.back().c_str()<<"]");
				break;
			}
			cnt++;
		}
	}
#endif
	JsonParser::GetDevIDInJSMsg(&respmsg.message, &mDeviceList);
	LOGCXX("number of devices "<< mDeviceList.size());

}
	/*
	Function: 		Get index of Dev by parsing input
	Input format: 	"ID index"
					index is an integer value
	Return:			valid index.
	*/

int TestWorker::GetIndexByCondition(string condition){
	int ret = 0;
	int found = -1, cnt;
	char *index, *saveptr;
	/*
		Condition format: "ID index"
		ID: string
		index: integer
	*/
	LOGCXX("TestWorker::GetIndexByCondition condition: " << condition);
	found = condition.find("ID");
	if (found != std::string::npos){
		cnt = 0;
		for ( index = strtok_r((char*)condition.c_str(), " ", &saveptr); index; index = strtok_r(NULL, " ", &saveptr)){
			if (cnt++ == 1){
				ret = atoi(index);
				LOGCXX("Found index [" <<ret<<"]");
				break;
			}
		}
	}

	/*Cover out of range*/

	if ((ret < 0) || (ret >= mDeviceList.size())){
		LOGCXX("Invalid index (out of range), set it to first dev index with ID: "<< mDeviceList.front().ID);
		ret = 0;
	}
	return ret;
}

string TestWorker::GetPoolEleValue(int pool_index) {

	return mLogPool[pool_index];
}

void TestWorker::ClearLogPool() {
	mLogPool.clear();
}

unsigned int TestWorker::GetPoolSize() {
	return (unsigned int) mLogPool.size();
}
