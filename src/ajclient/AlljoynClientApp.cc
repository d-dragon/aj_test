#include <stdio.h>
#include <string>
#include "AlljoynClient.h"

using namespace std;

struct SignalInfo{

	string signalName;
	int numArg;
	
}SignalInfo;

static struct SignalInfo SignalInfoList[] = {
	{"add_devices", 1}, \
	{"get_binary", 4}, \
	{"list_devices", 1}, \
	{"set_binary", 5}, \
	{"update_firmware", 1}, \
	{"set_rule", 4}, \
	{"get_rule", 1}, \
	{"rule_actions", 6}
};

int GetSignalNumArg(string signalName){

	int numSignalInfo = (sizeof SignalInfoList) / (sizeof SignalInfo);

	for (int i = 0; i < numSignalInfo; ++i)
	{
		if( SignalInfoList[i].signalName.compare(signalName) == 0) {

		}
	}
}

int help(){

	int numSignalInfo = (sizeof SignalInfoList) / (sizeof SignalInfo);
	printf("Supported Features:\n");
	for (int i = 0; i < numSignalInfo; ++i)
	{
		cout << "\t" << i <<":"<< SignalInfoList[i].signalName << endl;
	}
	printf("Press number to take corresponding action!!!\n");
}
int main()
{
	QStatus status;

	/*******************************/

	help();
	int funcIndex;
	cout << "Choose Index of Feature:";
	cin >> funcIndex;
	cout << "Input " << SignalInfoList[funcIndex].numArg << "arguments of " << SignalInfoList[funcIndex].signalName << endl;
	string args[funcIndex];
	for (int i = 0; i < SignalInfoList[funcIndex].numArg; i++) {
		cout << "Argument" << i + 1 << ":";
		cin >> args[i];
		cout << args[i] << endl;
	}

	return 1;
	/*******************************/


	printf("*********Start Alljoyn Client app **********\n");
	AlljoynClient* ajClient = new AlljoynClient();
	status = ajClient->InitAlljoynClient("com.verik.bus.VENUS_BOARD");

	if(ER_OK != status) {
		printf("init alljoyn client failed\n");
		return -1;
	}
	sleep(1);
	status = ajClient->ConnectServiceProvider("com.verik.bus.VENUS_BOARD");
	if (ER_OK == status) {


		/*ajClient->SendRequestSignal("add_devices", 1, "zwave");
		sleep(4);*/
/*		ajClient->SendRequestSignal("list_devices", 1, "zwave");
		sleep(2);
		
		ajClient->SendRequestSignal("get_binary", 4, "zwave", "08", "" , "");
		sleep(2);
		
		ajClient->SendRequestSignal("set_binary", 5, "zwave", "08", "0", "", "");
		sleep(2);
		
		ajClient->SendRequestSignal("update_firmware", 1, "arg1");
		sleep(2);*/
		
/*		ajClient->SendRequestSignal("set_rule", 4, "time", "rule1", "15:40" , "zwave;08;1");
		sleep(2);
		
		ajClient->SendRequestSignal("get_rule", 1, "all");
		sleep(2);
		*/
		ajClient->SendRequestSignal("rule_actions", 6, "update", "time", "rule1", "rule2", "16:00;1,2,3,4,5", "zwave;33D5;0;01;0104");

	}
	// sleep(3);
	while(1);
	printf("App terminated\n");
	// delete ajClient;	
	return 0;
}