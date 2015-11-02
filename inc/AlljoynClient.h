#ifndef _ALLJOYN_CLIENT_H
#define _ALLJOYN_CLIENT_H

#include <alljoyn/AboutData.h>
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
#include <string>

using namespace ajn;
using namespace std;

/**
 *  AlljoynClient is main class provide some abilities:
 *		- Attach to specific alljoyn bus
 *		- Get about data from service privider by provided interface
 *		- Join session that provider is listening
 *		- Send/receive signal to/from service provider
 *		- Send/receive method to/from service provider
 */

typedef void(*fptr)(int);
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
				RemoteBusObject(BusAttachment& bus, const char* path,const char* interface, ProxyBusObject proxyObject, SessionId sessionId);
				~RemoteBusObject();
				void SignalHandler(const InterfaceDescription::Member* member, const char* srcPath, Message& msg);
				QStatus SendSignal(const char* methodName, size_t numArg, MsgArg args[]);
                void RegisterCBFunc(void (*cbfunc)(int));
				
			private:
                fptr CBFunc;
				const InterfaceDescription::Member** remoteSignalMember;
				SessionId remoteSessionId;
    			size_t numMember;
		};

		RemoteBusObject* remoteObject;
		ProxyBusObject* proxyObject;
		AlljoynClientAboutListener aboutListener;
		AlljoynClientSessionListener sessionListener;
	public:
       
		AlljoynClient();
		~AlljoynClient();
		QStatus InitAlljoynClient(const char* interface);
		QStatus ConnectServiceProvider(const char* interface);
		QStatus SendRequestSignal(const char* signalName, size_t numArg, ... );
		QStatus CallMethod(const char* methodName, size_t numArg, ... );
        void RegisterCB(void (*cbfunc)(int)) { remoteObject->RegisterCBFunc(cbfunc);};
        
};
#endif
