#include <stdio.h>
#include <signal.h>

#include "AlljoynClient.h"

using namespace ajn;
using namespace std;

// Print out the fields found in the AboutData. Only fields with known signatures
// are printed out.  All others will be treated as an unknown field.
//this is temperary function for testing
void printAboutData(AboutData& aboutData, const char* language, int tabNum)
{
    size_t count = aboutData.GetFields();

    const char** fields = new const char*[count];
    aboutData.GetFields(fields, count);

    for (size_t i = 0; i < count; ++i) {
        for (int j = 0; j < tabNum; ++j) {
            printf("\t");
        }
        printf("Key: %s", fields[i]);

        MsgArg* tmp;
        aboutData.GetField(fields[i], tmp, language);
        printf("\t");
        if (tmp->Signature() == "s") {
            const char* tmp_s;
            tmp->Get("s", &tmp_s);
            printf("%s", tmp_s);
        } else if (tmp->Signature() == "as") {
            size_t las;
            MsgArg* as_arg;
            tmp->Get("as", &las, &as_arg);
            for (size_t j = 0; j < las; ++j) {
                const char* tmp_s;
                as_arg[j].Get("s", &tmp_s);
                printf("%s ", tmp_s);
            }
        } else if (tmp->Signature() == "ay") {
            size_t lay;
            uint8_t* pay;
            tmp->Get("ay", &lay, &pay);
            for (size_t j = 0; j < lay; ++j) {
                printf("%02x ", pay[j]);
            }
        } else {
            printf("User Defined Value\tSignature: %s", tmp->Signature().c_str());
        }
        printf("\n");
    }
    delete [] fields;
}

AlljoynClient::AlljoynClient() : mainBus(NULL), remoteObject(NULL), proxyObject(NULL){

	printf("AlljoynClient Contructor\n");
}

AlljoynClient::~AlljoynClient(){

	printf("AlljoynClient Destructor\n");
	mainBus->UnregisterAboutListener(aboutListener);
	delete mainBus;
	AllJoynRouterShutdown();
	AllJoynShutdown();
}
/****************************************************/
AlljoynClient::AlljoynClientSessionListener::AlljoynClientSessionListener(){

}
AlljoynClient::AlljoynClientSessionListener::~AlljoynClientSessionListener(){

}

void AlljoynClient::AlljoynClientSessionListener::SessionLost(SessionId sessionId, SessionLostReason reason){
	printf("SessionLost sessionId = %u, Reason = %d\n", sessionId, reason);
}

AlljoynClient::RemoteBusObject::RemoteBusObject(BusAttachment& bus, const char* path,const char* interface, ProxyBusObject proxyObject, SessionId sessionId) : BusObject(path), remoteSessionId(sessionId), num_member(0){

	QStatus status;

	const InterfaceDescription* remoteInterface = NULL;
	remoteInterface = proxyObject.GetInterface(interface);
	assert(remoteInterface);
	status = AddInterface(*remoteInterface);
	if(ER_OK != status){
		printf("BusObject AddInterface failed, reason: %s\n", QCC_StatusText(status));
	}

	num_member = remoteInterface->GetMembers(NULL, 0);

	if (num_member > 0) {
		// remoteSignalMember = new InterfaceDescription::Member();
		printf("number member of %s: %d\n", interface, (int)num_member);
		remoteInterface->GetMembers(remoteSignalMember, num_member);
		assert(remoteSignalMember);
// #ifdef DEBUG_ENABLE
		for (int i = 0; i < (int)num_member; i++) {
			printf("member %d: %s\n", i, remoteSignalMember[i]->name.c_str());
		}
// #endif
	}

	/**********Register Signal Handler**********/
	status = bus.RegisterSignalHandler(this, static_cast<MessageReceiver::SignalHandler>(&AlljoynClient::RemoteBusObject::GetBinarySignalHandler), remoteSignalMember[1], NULL);
	if (ER_OK == status)
	{
		printf("register SignalHandler for %s\n", remoteSignalMember[1]->name.c_str());
	}
}


void AlljoynClient::RemoteBusObject::GetBinarySignalHandler(const InterfaceDescription::Member* member, const char* srcPath, Message& msg){

	printf("receive signal from %s\n", srcPath);
    QCC_UNUSED(member);
    QCC_UNUSED(srcPath);
    printf("%s: %s\n", msg->GetSender(), msg->GetArg(0)->v_string.str);
}

QStatus AlljoynClient::RemoteBusObject::SendSignal(const char* methodName, size_t numArg){

	QStatus status;
	printf("send signal %s, sessionId: %u\n", methodName, remoteSessionId);
	uint8_t flags = 0;
	assert(&remoteSignalMember[1]);
	MsgArg tmp_arg[4];
	tmp_arg[0].Set("s", "all");
	tmp_arg[1].Set("s", "all");
	tmp_arg[2].Set("s", "all");
	tmp_arg[3].Set("s", "all");

	printf("remoteSignalMember: %s message\n", remoteSignalMember[1]->name.c_str());
	status = Signal(NULL, remoteSessionId, *remoteSignalMember[1], tmp_arg, numArg ,0, flags, NULL);
	if (ER_OK != status) {

		printf("Send Signal failed\n");
	}

	return status;
}
AlljoynClient::RemoteBusObject::~RemoteBusObject(){

}
/*****************************************************/
AlljoynClient::AlljoynClientAboutListener::AlljoynClientAboutListener(){

}
AlljoynClient::AlljoynClientAboutListener::~AlljoynClientAboutListener(){

}

void AlljoynClient::AlljoynClientAboutListener::Announced(const char* busName, uint16_t version, SessionPort port, const MsgArg& objectDescriptionArg, const MsgArg& aboutDataArg){

	aboutInfo.objectDescription.CreateFromMsgArg(objectDescriptionArg);
	aboutInfo.busName = string(busName);
	aboutInfo.port = port;
#ifdef DEBUG_ENABLE
	printf("================================================\n");
	printf("Announce signal discovered\n");
	printf("\tFrom bus %s\n", busName);
	printf("\tAbout version %hu\n", version);
	printf("\tSessionPort %hu\n", port);
	printf("\tObjectDescription:\n");

	printf("\tAboutData\n");
	AboutData aboutData(aboutDataArg);
	printAboutData(aboutData, NULL, 2);
	printf("================================================\n");
#endif
	/*Join to provider session then fetch more announcement data*/
/*	QStatus status;
	if (mainBus != NULL) {
		SessionOpts opts(SessionOpts::TRAFFIC_MESSAGES, false, SessionOpts::PROXIMITY_ANY, TRANSPORT_ANY);
		mainBus->EnableConcurrentCallbacks();
		status = mainBus-> JoinSession(busName, port, &sessionListener, sessionId, opts);
		printf("SessionJoined sessionId = %u, status = %s\n", sessionId, QCC_StatusText(status));
	}*/
}

QStatus AlljoynClient::InitAlljoynClient(const char* interface){

	QStatus status;
	if((status = AllJoynInit()) != ER_OK) {
		printf("AlljoynInit failed\n");
		return status;
	}

	if((status = AllJoynRouterInit()) != ER_OK) {
		printf("init AllJoynRouter failed\n");
		AllJoynShutdown();
		return status;
	}

	mainBus = new BusAttachment("Alljoyn.Client", true);
	if ((status = mainBus->Start()) == ER_OK) {
		printf("BusAttachment started!\n");
	}else{
		printf("start BusAttachment failed, reason %s!\n", QCC_StatusText(status));
		return status;
	}

	if ((status = mainBus->Connect()) == ER_OK) {
		printf("BusAttachment connected!\n");
	}else{
		printf("start BusAttachment failed, reason %s!\n", QCC_StatusText(status));
		return status;
	}

	mainBus->RegisterAboutListener(aboutListener);

	const char* interfaces[] = { interface };
	status = mainBus->WhoImplements(interfaces, sizeof(interfaces) / sizeof(interfaces[0]));
	if(ER_OK == status) {
		printf("WhoImplements called\n");
	}else{
		printf("WhoImplements call FAILED with status %s\n", QCC_StatusText(status));
	}
	return status;
}

QStatus AlljoynClient::ConnectServiceProvider(const char* interface){

	QStatus status;

#ifdef DEBUG_ENABLE
	size_t num_path = aboutListener.aboutInfo.objectDescription.GetPaths(NULL,0);
	const char** paths = new const char*[num_path];
	aboutListener.aboutInfo.objectDescription.GetPaths(paths, num_path);
	for (size_t i = 0; i < num_path; i++) {
		printf("\t\t%s\n", paths[i]);
		size_t num_interface = aboutListener.aboutInfo.objectDescription.GetInterfaces(paths[i], NULL, 0);
		const char** interfaces = new const char*[num_interface];
		aboutListener.aboutInfo.objectDescription.GetInterfaces(paths[i], interfaces, num_interface);
		for (size_t j = 0; j < num_interface; j++) {
			printf("\t\t\t%s\n", interfaces[j]);
		}
	}
	delete paths;
	delete interfaces;
#endif

	printf("busName: %s, SessionPort: %u\n", aboutListener.aboutInfo.busName.c_str(), aboutListener.aboutInfo.port);
	if (mainBus != NULL) {

		SessionOpts opts(SessionOpts::TRAFFIC_MESSAGES, false, SessionOpts::PROXIMITY_ANY, TRANSPORT_ANY);
		mainBus->EnableConcurrentCallbacks();
		status = mainBus->JoinSession(aboutListener.aboutInfo.busName.c_str(), aboutListener.aboutInfo.port, &sessionListener, sessionId, opts);
		if(ER_OK == status) {
			printf("connected to service! sessionId=%u\n", sessionId);
		}
	}

	/*get proxybusobject*/
	const char* path;
	aboutListener.aboutInfo.objectDescription.GetInterfacePaths(interface, &path, 1);
	printf("proxyObject for interface path: %s/%s\n", path, interface);
	proxyObject = new ProxyBusObject(*mainBus, aboutListener.aboutInfo.busName.c_str(), path, sessionId);
	status = proxyObject->IntrospectRemoteObject();
	if (ER_OK != status) {
		
		printf("Failed to introspect proxy remote object\n");
		return status;
	}

	remoteObject = new RemoteBusObject(*mainBus, path, interface, *proxyObject, sessionId);
	status = mainBus->RegisterBusObject(*remoteObject);
	if(ER_OK == status) {
		printf("BusAttachment RegisterBusObject success\n");
	}
	return status;
}

QStatus AlljoynClient::SendRequestSignal(const char* signalName, size_t numArg, ... ){

	QStatus status;
	// size_t numArg = 4;
	status = remoteObject->SendSignal(signalName, numArg);

	return status;
}

int main()
{
	QStatus status;
	printf("start app\n");

	AlljoynClient* ajClient = new AlljoynClient();
	status = ajClient->InitAlljoynClient("com.verik.bus.VENUS_BOARD");

	if(status == ER_OK) {
		printf("init alljoyn client success\n");
	}
	sleep(1);
	status = ajClient->ConnectServiceProvider("com.verik.bus.VENUS_BOARD");
	
	ajClient->SendRequestSignal("get_binary", (size_t)4);

	// delete ajClient;
	return 0;
}