#include <stdio.h>
#include <signal.h>
#include <cstdarg>
#include <string.h>

#include "AlljoynClient.h"

using namespace ajn;
using namespace std;

// Print out the fields found in the AboutData. Only fields with known signatures
// are printed out.  All others will be treated as an unknown field.
//this is temperary function for testing
#ifdef DEBUG_ENABLED
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
#endif

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

AlljoynClient::RemoteBusObject::RemoteBusObject(BusAttachment& bus, const char* path,const char* interface, ProxyBusObject proxyObject, SessionId sessionId) : BusObject(path), remoteSessionId(sessionId), numMember(0), remoteSignalMember(NULL) {

	QStatus status;

	const InterfaceDescription* remoteInterface = NULL;
	remoteInterface = proxyObject.GetInterface(interface);
	assert(remoteInterface);
	status = AddInterface(*remoteInterface);
	if(ER_OK != status){
		printf("BusObject AddInterface failed, reason: %s\n", QCC_StatusText(status));
	}

	numMember = remoteInterface->GetMembers(NULL, 0);

	if (numMember > 0) {
		remoteSignalMember = new const InterfaceDescription::Member*[numMember];
		printf("number member of %s: %d\n", interface, (int)numMember);
		remoteInterface->GetMembers(remoteSignalMember, numMember);
		assert(remoteSignalMember);
#ifdef DEBUG_ENABLE
		for (size_t i = 0; i < numMember; i++) {
			printf("member %u: %s\n", i, remoteSignalMember[i]->name.c_str());
		}
#endif
	}

	/**********Register Signal Handler**********/
	for (size_t i = 0; i < numMember; i++) {
	
		status = bus.RegisterSignalHandler(this, static_cast<MessageReceiver::SignalHandler>(&AlljoynClient::RemoteBusObject::SignalHandler), remoteSignalMember[i], NULL);
		if (ER_OK == status)
		{
			printf("register SignalHandler for %s\n", remoteSignalMember[i]->name.c_str());
		}
	}
}


void AlljoynClient::RemoteBusObject::SignalHandler(const InterfaceDescription::Member* member, const char* srcPath, Message& msg){

	printf("\nReceived signal [%s] from %s\n", member->name.c_str(), srcPath);
    QCC_UNUSED(member);
    QCC_UNUSED(srcPath);
    // printf("%s: %s\n", msg->GetSender(), msg->GetArg(0)->v_string.str);
    printf("Response message: %s\n", msg->GetArg(0)->v_string.str);

}

QStatus AlljoynClient::RemoteBusObject::SendSignal(const char* methodName, size_t numArg, MsgArg args[]){

	QStatus status;
	printf("send signal %s, sessionId: %u\n", methodName, remoteSessionId);
	uint8_t flags = 0;
	
	for(size_t i = 0; i < numMember; i++) {

		// printf("remoteSignalMember: %s message\n", remoteSignalMember[i]->name.c_str());
		if(strcmp(remoteSignalMember[i]->name.c_str(), methodName) == 0) {

			assert(&remoteSignalMember[i]);
			status = Signal(NULL, remoteSessionId, *remoteSignalMember[i], args, numArg ,0, flags, NULL);
			if (ER_OK != status) {

				printf("Send Signal failed\n");
			}
			return status;
		}
	}
	
	printf("request signal <%s> not found\n", methodName);
	return ER_FAIL;
}
AlljoynClient::RemoteBusObject::~RemoteBusObject(){

}
/*****************************************************/
AlljoynClient::AlljoynClientAboutListener::AlljoynClientAboutListener(){

	aboutInfo.port = 0;
}
AlljoynClient::AlljoynClientAboutListener::~AlljoynClientAboutListener(){

}

void AlljoynClient::AlljoynClientAboutListener::Announced(const char* busName, uint16_t version, SessionPort port, const MsgArg& objectDescriptionArg, const MsgArg& aboutDataArg){

	aboutInfo.objectDescription.CreateFromMsgArg(objectDescriptionArg);
	aboutInfo.busName = string(busName);
	aboutInfo.port = port;
#ifdef DEBUG_ENABLED
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

	if (aboutListener.aboutInfo.port == 0) {

		printf("Service provider not found\n");
		return ER_FAIL;
	}
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

	va_list signalArgs;

	va_start(signalArgs, numArg);
	MsgArg args[numArg];

	for (size_t i = 0; i < numArg; i++) {

		args[i].Set("s", va_arg(signalArgs, char*));
	}
	va_end(signalArgs);
	status = remoteObject->SendSignal(signalName, numArg, args);
	return status;
}
