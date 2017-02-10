#include <stdio.h>
#include <string>
#include "AlljoynClient.h"
#include <signal.h>
using namespace std;

struct SignalInfo{

	string signalName;
	int numArg;
	string argHint;
	
}SignalInfo;

static volatile int isTerminating = 1;

static struct SignalInfo SignalInfoList[] = {
	{"add_devices", 1, "(type) -> e.g. (zwave)"}, \
	{"get_binary", 4, "(servicename, id, epnum, priod) -> e.g. (zwave, 08, option, option)"}, \
	{"list_devices", 1, "(type) -> e.g. (all/zwave)"}, \
	{"set_binary", 5, "(servicename, id, value, epnum, proid) -> e.g. (zwave, 08, 1/0, option, option)"}, \
	{"update_firmware", 1, "(url) -> e.g. (ftp://192.168.1.2/firmware.img)"}, \
	{"set_rule", 4, "(ruly_type, rule_name, conditions, actions) -> e.g. (time, rule1, 12:00, zwave;08;1)"}, \
	{"get_rule", 1, "(type) -> e.g. (all)"}, \
	{"rule_actions", 6, "(rule_action, rule_type, rule_name, new_name, new_conditions, new_actions)"}
};

static void SignalHander(int signo)
{
	switch(signo) {
		case SIGINT:
			printf("Received signal %d\n", signo);
			isTerminating = 2;
			break;
	};
	return;
}

static void SetSignalHandler(void) {
	struct sigaction act, old_act;
	act.sa_flags = (SA_NOCLDSTOP | SA_NOCLDWAIT | SA_RESTART | SA_SIGINFO);
	act.sa_handler = SignalHander;

	if (sigaction(SIGSEGV, &act, &old_act)) {
		cout << "sigaction failed... errno: " << errno << endl;
	} else {
		if (old_act.sa_handler == SIG_IGN) {

			cout << "oldact RTMIN: SIGIGN" << endl;
		}
		if (old_act.sa_handler == SIG_DFL) {

			cout << "oldact RTMIN: SIGDFL" << endl;
		}
	}
	if (sigaction(SIGINT, &act, &old_act)) {

		cout << "sigaction failed... errno: " << errno;
	} else {
		if (old_act.sa_handler == SIG_IGN) {

			cout << "oldact RTMIN: SIGIGN" << endl;
		}
		if (old_act.sa_handler == SIG_DFL) {

			cout << "oldact RTMIN: SIGDFL" << endl;
		}
	}




}
static void help(){

	int numSignalInfo = (sizeof SignalInfoList) / (sizeof SignalInfo);
	printf("\n********Supported Features********************************************\n");
	for (int i = 0; i < numSignalInfo; ++i)
	{
		cout << "\t" << i <<":"<< SignalInfoList[i].signalName << SignalInfoList[i].argHint << endl;
	}
	cout << "\t" << "8:quit" << endl;
	printf("Press the number to take corresponding action!!!\n");
	printf("**********************************************************************\n");

}


#define TIMEOUT 10000
int main()
{
	QStatus status;
	int funcIndex;
    int sig_status;
    int cnt = 0;
	SetSignalHandler();
	printf("*********Start Alljoyn Client app **********\n");
	AlljoynClient* ajClient = new AlljoynClient();
	status = ajClient->InitAlljoynClient(VENUS_INTF);

	if(ER_OK != status) {
		printf("init alljoyn client failed\n");
		return -1;
	}
	sleep(2);
	status = ajClient->ConnectServiceProvider(VENUS_INTF);
	if (status == ER_FAIL) {
		return -1;
	}
	cout << "Connected to Venus hub successfully!" << endl;

/*
    ajClient->RegisterCB(signal_handler);
*/
    
	if (ER_OK == status) {
		while(isTerminating != 2){
			/*
            if (isTerminating != 1)
            {
                usleep(100);
                continue;
            }
			*/
            isTerminating = 0;
			help();
			
			cout << "Choose feature index:";
			cin >> funcIndex;
			if (funcIndex > 8 || funcIndex < 0) {
				cout << "Invalid index" << endl;
				continue;
			} else if (funcIndex == 8) {
				cout << "Program exit>>>>>" << endl;
				goto exit;
			}
			cout << "Input " << SignalInfoList[funcIndex].numArg << " arguments of " << SignalInfoList[funcIndex].signalName << endl;
			string args[SignalInfoList[funcIndex].numArg];
			for (int i = 0; i < SignalInfoList[funcIndex].numArg; i++) {
				cout << "Argument" << i + 1 << ":";
				cin >> args[i];
				cout << args[i] << endl;
			}

			switch(SignalInfoList[funcIndex].numArg){

				case 1:

					status = ajClient->SendRequestSignal(SignalInfoList[funcIndex].signalName.c_str(), 1, args[0].c_str());
					if (ER_OK != status) {
						cout << "send signal ["<< SignalInfoList[funcIndex].signalName <<"] failed!" << endl;
					}
					break;
				case 4:
					status = ajClient->SendRequestSignal(SignalInfoList[funcIndex].signalName.c_str(), 4, args[0].c_str(), args[1].c_str(), args[2].c_str(), args[3].c_str());
					if (ER_OK != status) {
						cout << "send signal ["<< SignalInfoList[funcIndex].signalName <<"] failed!" << endl;
					}
					break;
				case 5:
					status = ajClient->SendRequestSignal(SignalInfoList[funcIndex].signalName.c_str(), 5, args[0].c_str(), args[1].c_str(), args[2].c_str(), args[3].c_str(), args[4].c_str());
					if (ER_OK != status) {
						cout << "send signal ["<< SignalInfoList[funcIndex].signalName <<"] failed!" << endl;
					}
					break;
				case 6:
					status = ajClient->SendRequestSignal(SignalInfoList[funcIndex].signalName.c_str(), 6, args[0].c_str(), args[1].c_str(), args[2].c_str(), args[3].c_str(), args[4].c_str(), args[5].c_str());
					if (ER_OK != status) {
						cout << "send signal ["<< SignalInfoList[funcIndex].signalName <<"] failed!" << endl;
					}
					break;
				default:
					break;
			};
			cin.clear();
		};
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
/*		ajClient->SendRequestSignal("rule_actions", 6, "update", "time", "rule1", "rule2", "16:00;1,2,3,4,5", "zwave;33D5;0;01;0104");
*/
	}
exit:
	delete ajClient;	
	printf("App terminated\n");
	return 0;
}
