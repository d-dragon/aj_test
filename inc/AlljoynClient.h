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
class AlljoynClient
{
	private:
		BusAttachment* mainBus;
		SessionId sessionId;

		struct AboutInfo
		{
			string busName;
			SessionPort port;
			AboutObjectDescription objectDescription;
		};

		class AlljoynClientSessionListener : public SessionListener{
			public:
			AlljoynClientSessionListener();
			~AlljoynClientSessionListener();
			void SessionLost(SessionId sessionId, SessionLostReason reason);
		};

		class AlljoynClientAboutListener : public AboutListener
		{
			public:
				AlljoynClientAboutListener();
				~AlljoynClientAboutListener();
				void Announced(const char* busName, uint16_t version, SessionPort port, const MsgArg& objectDescriptionArg, const MsgArg& aboutDataArg);
				AboutInfo aboutInfo;
				// BusAttachment* dupMainBus;
		};
		
		class RemoteBusObject : public BusObject
		{
			public:
				RemoteBusObject(BusAttachment& bus, const char* path,const char* interface, ProxyBusObject proxyObject, SessionId sessionId);
				~RemoteBusObject();
				void GetBinarySignalHandler(const InterfaceDescription::Member* member, const char* srcPath, Message& msg);
				void SetBinarySignalHandler(const InterfaceDescription::Member* member, const char* srcPath, Message& msg);
				QStatus SendSignal(const char* methodName, size_t numArg);
				
			private:
				const InterfaceDescription::Member** remoteSignalMember;
				SessionId remoteSessionId;
    			size_t num_member;
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
};
#endif