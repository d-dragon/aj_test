#include <stdio.h>
#include <string>
#include "AlljoynClient.h"

int main()
{
	QStatus status;
	
	printf("*********Start Alljoyn Client app**********\n");
	AlljoynClient* ajClient = new AlljoynClient();
	status = ajClient->InitAlljoynClient("com.verik.bus.VENUS_BOARD");

	if(ER_OK != status) {
		printf("init alljoyn client failed\n");
		return -1;
	}
	sleep(1);
	status = ajClient->ConnectServiceProvider("com.verik.bus.VENUS_BOARD");
	if (ER_OK == status) {
		ajClient->SendRequestSignal("add_devices", 1, "arg1");
		sleep(1);
		ajClient->SendRequestSignal("list_devices", 1, "all");
		sleep(1);
		ajClient->SendRequestSignal("get_binary", 4, "arg1", "arg2", "arg3" , "arg4");
		sleep(1);
		ajClient->SendRequestSignal("set_binary", 5, "arg1", "arg2", "arg3", "arg4", "arg5");
		sleep(1);
		ajClient->SendRequestSignal("update_firmware", 1, "arg1");
		sleep(1);
		ajClient->SendRequestSignal("set_rule", 4, "arg1", "arg2", "arg3" , "arg4");
		sleep(1);
		ajClient->SendRequestSignal("get_rule", 1, "arg1");
		sleep(1);
		ajClient->SendRequestSignal("rule_actions", 6, "arg1", "arg2", "arg3", "arg4", "arg5", "arg6");
	}
	sleep(3);
	printf("App terminated\n");
	// delete ajClient;	
	return 0;
}