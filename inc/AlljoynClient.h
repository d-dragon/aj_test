#ifndef _ALLJOYN_CLIENT_H
#define _ALLJOYN_CLIENT_H

#include <alljoyn/AboutData.h>
#include <alljoyn/AboutObj.h>
#include <alljoyn/AboutListener.h>
#include <alljoyn/AboutObjectDescription.h>
#include <alljoyn/BusObject.h>
#include <alljoyn/AboutProxy.h>
#include <alljoyn/BusAttachment.h>
#include <alljoyn/Init.h>
#include <alljoyn/MsgArg.h>
#include <alljoyn/Session.h>
#include <alljoyn/SessionListener.h>
#include <alljoyn/InterfaceDescription.h>
#include <alljoyn/PermissionConfigurator.h>
#include <alljoyn/PermissionPolicy.h>
#include <qcc/GUID.h>
#include <qcc/Mutex.h>
#include <qcc/Condition.h>
#include <qcc/Thread.h>
#include <string>

#define KEYX_ECDHE_NULL "ALLJOYN_ECDHE_NULL"
#define KEYX_ECDHE_PSK "ALLJOYN_ECDHE_PSK"
#define KEYX_ECDHE_DSA "ALLJOYN_ECDHE_ECDSA"
#define VENUS_INTF "com.verik.bus.VENUS_CONTROL"
#define VENUS_SIGNAL_MEMBER "*"
#define VENUS_SIGNAL_MATCH_RULE  "type='signal',interface='" VENUS_INTF "',member='" VENUS_SINAL_MEMBER "'"
#define CLIENT_APP_SESSION_PORT 26969 

using namespace ajn;
using namespace std;
using namespace qcc;

/**
 *  AlljoynClient is main class provide some abilities:
 *		- Attach to specific alljoyn bus
 *		- Get about data from service privider by provided interface
 *		- Join session that provider is listening
 *		- Send/receive signal to/from service provider
 *		- Send/receive method to/from service provider
 */

typedef void(*fptr)(int , const char*, const char*, const char*);
class AlljoynClient
{
	private:
		BusAttachment* mainBus;
		SessionId sessionId;

		/**
		*	Store provider configuration which used to communication with remote service
		*/
		struct AboutInfo
		{
			string busName;
			SessionPort port;
			AboutObjectDescription objectDescription;
		};

		/**
		*	Class inform session related events
		*/
		class AlljoynClientSessionListener : public SessionListener{
			public:
			AlljoynClientSessionListener();
			~AlljoynClientSessionListener();
			void SessionLost(SessionId sessionId, SessionLostReason reason);
		};

		/* Session port listener. */
		class AlljoynclientSessionPortListener :
			public SessionPortListener {
				bool AcceptSessionJoiner(SessionPort sessionPort, const char* joiner, const SessionOpts& opts)
				{
					QCC_UNUSED(opts);
					QCC_UNUSED(joiner);
					QCC_UNUSED(sessionPort);

					return true; // Always accept sessions regardless.
				}
			};
		/**
		*	Class is responsible for receiving announced data
		*/
		class AlljoynClientAboutListener : public AboutListener
		{
			public:
				AlljoynClientAboutListener();
				~AlljoynClientAboutListener();
				void Announced(const char* busName, uint16_t version, SessionPort port, const MsgArg& objectDescriptionArg, const MsgArg& aboutDataArg);
				AboutInfo aboutInfo;
		};
		
		/**
		*	Class is use information from about data to 
		*/
		class RemoteBusObject : public BusObject
		{
			public:
				RemoteBusObject(BusAttachment& bus, const char* path,const char* interface, ProxyBusObject proxyObject, SessionId sessionId, string busName);
				~RemoteBusObject();
				void SignalHandler(const InterfaceDescription::Member* member, const char* srcPath, Message& msg);
				QStatus SendSignal(const char* methodName, size_t numArg, MsgArg args[]);
                void RegisterCBFunc(void (*cbfunc)(int respFlag, const char *respMsg, const char *srcPath, const char *member));
				
			private:
                fptr CBFunc;
				const InterfaceDescription::Member** remoteSignalMember;
				SessionId remoteSessionId;
				string remoteBusName;
    			size_t numMember;
		};

		class ClientPCL : public PermissionConfigurationListener {

			public:
				ClientPCL(BusAttachment& bus) : ba(bus) { };

				void PolicyChanged();

				QStatus WaitForClaimedState();

			private:
				BusAttachment& ba;
				Mutex lock;
				Condition sem;
		};
		RemoteBusObject* remoteObject;
		ProxyBusObject* proxyObject;
		ClientPCL *pcl;
		DefaultECDHEAuthListener* authListener;
		AlljoynClientAboutListener aboutListener;
		AlljoynClientSessionListener sessionListener;
		AlljoynclientSessionPortListener sessionPortListener;
		static AboutData *mAboutData;
		AboutData clientAboutData;
		AboutObj* clientAboutObj;
		static string mRefDevID;

		void AlljoynClientByeBye();
		QStatus InitSecurity2Infra();
		QStatus AnnounceClientAboutData();
	public:
       
		AlljoynClient();
		~AlljoynClient();
		QStatus InitAlljoynClient(const char* interface);
		QStatus ConnectServiceProvider(const char* interface);
		/*
			This method must be execute before InitAlljoynClient
		*/
		QStatus SetRefTargetDeviceID(const char* refDevID);
		QStatus GetTargetDeviceID(char **devID);
		QStatus SendRequestSignal(const char* signalName, size_t numArg, ... );
		QStatus SendRequestSignal(const char* signalName, size_t numArg, string signalArgs[]);
		QStatus CallMethod(const char* methodName, size_t numArg, ... );
        void RegisterCB(void (*cbfunc)(int respFlag, const char *respMsg, const char *srcPath, const char *member)) { remoteObject->RegisterCBFunc(cbfunc);};
        
};
#endif
