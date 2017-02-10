#include <stdio.h>
#include <signal.h>
#include <cstdarg>
#include <string.h>

#include "AlljoynClient.h"

using namespace ajn;
using namespace std;

AboutData *AlljoynClient::mAboutData = NULL;
string AlljoynClient::mRefDevID = "";

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

    mAboutData  =   NULL;
    mRefDevID   = "";
}

AlljoynClient::~AlljoynClient(){

    if (NULL != mAboutData){
        delete mAboutData;
    }
	AlljoynClientByeBye();


}
/****************************************************/
AlljoynClient::AlljoynClientSessionListener::AlljoynClientSessionListener(){

}
AlljoynClient::AlljoynClientSessionListener::~AlljoynClientSessionListener(){

}

void AlljoynClient::AlljoynClientSessionListener::SessionLost(SessionId sessionId, SessionLostReason reason){
	printf("SessionLost sessionId = %u, Reason = %d\n", sessionId, reason);
}


void AlljoynClient::RemoteBusObject::RegisterCBFunc(void (*cbfunc)(int respFlag, const char *respMsg, const char *srcPath, const char *member)){
    CBFunc = cbfunc;
}
				

AlljoynClient::RemoteBusObject::RemoteBusObject(BusAttachment& bus, const char* path,const char* interface, ProxyBusObject proxyObject, SessionId sessionId, string busName) : BusObject(path), remoteSessionId(sessionId), numMember(0), remoteSignalMember(NULL), remoteBusName(busName) {

	QStatus status;

    CBFunc = NULL;
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
    //printf("Response message: %s\n", msg->GetArg(0)->v_string.str);
    if (CBFunc != NULL){
        CBFunc(1, msg->GetArg(0)->v_string.str, srcPath, member->name.c_str());
    }

}

QStatus AlljoynClient::RemoteBusObject::SendSignal(const char* methodName, size_t numArg, MsgArg args[]){

	QStatus status;
	printf("send signal %s, sessionId: %u\n", methodName, remoteSessionId);
	uint8_t flags = 0;
	
	for(size_t i = 0; i < numMember; i++) {

		// printf("remoteSignalMember: %s message\n", remoteSignalMember[i]->name.c_str());
		if(strcmp(remoteSignalMember[i]->name.c_str(), methodName) == 0) {

			assert(&remoteSignalMember[i]);
			status = Signal(remoteBusName.c_str(), 0, *remoteSignalMember[i], args, numArg ,0, flags, NULL);
			if (ER_OK != status) {

				printf("Send Signal failed status (%s)\n", QCC_StatusText(status));
			}
			printf("Send Signal return status (%s)\n", QCC_StatusText(status));
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

    char *devID;
    QStatus ret;
    //if ((mRefDevID != "") && (mAboutData == NULL))
    if (mAboutData == NULL)
    {
        mAboutData = new AboutData(aboutDataArg);
        ret = mAboutData->GetDeviceId(&devID);
        if (ER_OK != ret)
        {
            return;
        }
        cout << "Find out dev id :"<< devID <<std::endl;
		/*
        if ( 0 != strcmp (mRefDevID.c_str(),devID ))
        {
            delete mAboutData;
            mAboutData = NULL;
            return;
        }
		*/
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
    } else {
		AboutData aboutData(aboutDataArg);
		char* appName;
		char* deviceName;

		cout << "\nReceived About signal:";
		cout << "\n BusName          : " << busName << endl;

		if (ER_OK == aboutData.GetAppName(&appName)) {
			cout << " Application Name : " << appName << endl;
		}
		if (ER_OK == aboutData.GetDeviceName(&deviceName)) {
			cout << " Device Name      : " << deviceName << endl;
		}

		cout << endl;
	}
}
QStatus AlljoynClient::AnnounceClientAboutData()
{
	QStatus status;
	clientAboutObj = new AboutObj(*mainBus);
	
	GUID128 appId;
	printf("appId: %s\n", appId.ToString().c_str());
	status = clientAboutData.SetAppId(appId.ToString().c_str());

	clientAboutData.SetDefaultLanguage("en");
    char buf[64];
    gethostname(buf, sizeof(buf));
    status = clientAboutData.SetDeviceName(buf);

    GUID128 deviceId;
    status = clientAboutData.SetDeviceId(deviceId.ToString().c_str());
	printf("deviceId: %s\n", deviceId.ToString().c_str());
    status = clientAboutData.SetAppName("Venus Alljoyn Client");
    status = clientAboutData.SetManufacturer("VEriKsystems");
    status = clientAboutData.SetModelNumber("1");
    status = clientAboutData.SetDescription("Venus test tool");
    status = clientAboutData.SetDateOfManufacture("2017-02-10");
    status = clientAboutData.SetSoftwareVersion("0.1");
    status = clientAboutData.SetHardwareVersion("0.0.1");
    status = clientAboutData.SetSupportUrl("https://veriksystems.com/");

	if (!clientAboutData.IsValid()) {
		printf("Invalid aboutData\n");
		return ER_FAIL;
	}
	status = clientAboutObj->Announce(25, clientAboutData);
	if (ER_OK != status) {
		printf("error %s::%d status: %s\n", __FUNCTION__, __LINE__, QCC_StatusText(status));
	}
	return status;
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
		printf("BusAttachment connected! Bus name: %s\n", mainBus->GetUniqueName().c_str());
	} else {
		printf("start BusAttachment failed, reason %s!\n", QCC_StatusText(status));
		return status;
	}

	status = InitSecurity2Infra();
	if (ER_OK != status) {
		printf("init security-2.0 infra failed\n");
		return status;
	}

	SessionOpts opts;
	SessionPort port = 25;
	status = mainBus->BindSessionPort(port, opts, sessionPortListener);

	if (ER_OK != (status = AnnounceClientAboutData())) {
		printf("announce client about data failed\n");
		return status;
	}
	pcl->WaitForClaimedState();

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

QStatus AlljoynClient::GetTargetDeviceID(char **devID){
    if (mAboutData == NULL) return ER_FAIL;
    return (mAboutData->GetDeviceId(devID));
}

QStatus AlljoynClient::SetRefTargetDeviceID(const char* refDevID){
    QStatus status = ER_OK;
    mRefDevID      = refDevID;

    return status;
}
QStatus AlljoynClient::ConnectServiceProvider(const char* interface){

	QStatus status;

#ifdef DEBUG_ENABLED
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
		delete interfaces;
	}
	delete [] paths;
#endif

	if (aboutListener.aboutInfo.port == 0) {

		printf("Service provider not found\n");
		return ER_FAIL;
	}
	printf("Remote bus name: %s, SessionPort: %u\n", aboutListener.aboutInfo.busName.c_str(), aboutListener.aboutInfo.port);
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
	printf("proxyObject  path: %s\n", path);
	proxyObject = new ProxyBusObject(*mainBus, aboutListener.aboutInfo.busName.c_str(), aboutListener.aboutInfo.busName.c_str(), path, sessionId, true);
	status = proxyObject->IntrospectRemoteObject();
	if (ER_OK != status) {
		
		printf("Failed to introspect proxy remote object\n");
		return status;
	}

	remoteObject = new RemoteBusObject(*mainBus, path, interface, *proxyObject, sessionId, aboutListener.aboutInfo.busName);
	status = mainBus->RegisterBusObject(*remoteObject, true);
	if(ER_OK == status) {
		printf("BusAttachment RegisterBusObject success\n");
	}
	mainBus->CancelWhoImplements(VENUS_INTF);
	return status;
}

void AlljoynClient::AlljoynClientByeBye(){

	QStatus status;
	cout << "Alljoyn Client is stopping..." << endl;
	status = mainBus->LeaveJoinedSession(sessionId);
	if(status != ER_OK){
		cout << "left joined session failed" << endl;
	}
	cout << "unregistering AboutListener" << endl;
	mainBus->UnregisterAboutListener(aboutListener);

	cout << "unregistering BusObject" << endl;
	if(remoteObject != NULL){

		mainBus->UnregisterBusObject(*remoteObject);
	}
	mainBus->UnbindSessionPort(CLIENT_APP_SESSION_PORT);

	cout << "deallocating objects" << endl;
	delete remoteObject;
	delete proxyObject;
	delete pcl;
	delete clientAboutObj;
	delete authListener;

	cout << "disconnecting alljoyn bus" << endl;
	mainBus->Disconnect();
	mainBus->Stop();

	delete mainBus;

	AllJoynRouterShutdown();
	AllJoynShutdown();
	cout << "*********************************************************************" << endl;
	cout << "******************** Alljoyn Client bye bye :) **********************" << endl;
	cout << "*********************************************************************" << endl;
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

QStatus AlljoynClient::SendRequestSignal(const char* signalName, size_t numArg, string signalArgs[] ){

	QStatus status;

	MsgArg args[numArg];

	for(size_t i = 0; i < numArg; i++){
		args[i].Set("s", signalArgs[i].c_str());
	}
	cout << "alljoyn client send signal " << signalName << endl; 
	status = remoteObject->SendSignal(signalName, numArg, args);
	return status;
}


void AlljoynClient::ClientPCL::PolicyChanged()
{
	lock.Lock();
	QStatus status;
	PermissionConfigurator::ApplicationState appState;
	if (ER_OK == (status = ba.GetPermissionConfigurator().GetApplicationState(appState))) {
		if (PermissionConfigurator::ApplicationState::CLAIMED == appState) {
			qcc::Sleep(250);
            // Upon a policy update, existing connections are invalidated
            // and one needs to make them valid again.
            if (ER_OK != (status = ba.SecureConnectionAsync(nullptr, true))) {
                fprintf(stderr, "Attempt to secure the connection - status (%s)\n",
                        QCC_StatusText(status));
            }
            sem.Signal();
		}
	} else {
        fprintf(stderr, "Failed to GetApplicationState - status (%s)\n", QCC_StatusText(status));
    }
    lock.Unlock();

}

QStatus AlljoynClient::ClientPCL::WaitForClaimedState()
{
	lock.Lock();
	PermissionConfigurator::ApplicationState appState;

	QStatus status = ba.GetPermissionConfigurator().GetApplicationState(appState);
	if (ER_OK != status) {
		fprintf(stderr, "Failed to GetApplicationState - status (%s)\n",
				QCC_StatusText(status));
		lock.Unlock();
		return status;
	}

	if (PermissionConfigurator::ApplicationState::CLAIMED == appState) {
		printf("Already claimed !\n");
		lock.Unlock();
		return ER_OK;
	}

	printf("Waiting to be claimed...\n");
	status = sem.Wait(lock);
	if (ER_OK != status) {
		lock.Unlock();
		return status;
	}

	printf("Claimed !\n");
	lock.Unlock();
	return ER_OK;
}

QStatus AlljoynClient::InitSecurity2Infra()
{
	QStatus status;
	pcl = new ClientPCL(*mainBus);
	
    authListener = new DefaultECDHEAuthListener();
	status = mainBus->EnablePeerSecurity(KEYX_ECDHE_DSA " " KEYX_ECDHE_NULL " " KEYX_ECDHE_PSK, authListener, nullptr, false, pcl);
    if (ER_OK != status) {
        printf("Failed to EnablePeerSecurity - status (%s)\n", QCC_StatusText(status));
        return status;
	}

    PermissionPolicy::Rule manifestRule;
    manifestRule.SetInterfaceName(VENUS_INTF);

	PermissionPolicy::Rule::Member member;
	member.SetMemberName("*");
	member.SetMemberType(PermissionPolicy::Rule::Member::SIGNAL);
	member.SetActionMask(PermissionPolicy::Rule::Member::ACTION_OBSERVE | 
			PermissionPolicy::Rule::Member::ACTION_PROVIDE);
	manifestRule.SetMembers(1, &member);
	status = mainBus->GetPermissionConfigurator().SetPermissionManifest(&manifestRule, 1);
    if (ER_OK != status) {
        printf("Failed to SetPermissionManifest - status (%s)\n", QCC_StatusText(status));
        return status;
    }
	return status;
}
