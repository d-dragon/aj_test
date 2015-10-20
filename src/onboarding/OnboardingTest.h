#ifndef ONBOARDINGTEST_H_
#define ONBOARDINGTEST_H_

#include <alljoyn/Status.h>
#include <AboutListenerHandlerImpl.h>
#include <SrpKeyXListener.h>
#include <AJInitializer.h>
class OnboardingTest{

public:
	/*
	Intialize and Destroy AJ initialization
	*/
	OnboardingTest(bool isReset);
	~OnboardingTest();
	/*
		Create Bus Attachment
	*/
	QStatus CreateBusAttachment(ajn::services::OBInfo oBInfo);
	QStatus FinishBusAttachment();

private:
	ajn::BusAttachment* mbusAttachment;
	AboutListenerHandlerImpl* announceHandler;
	SrpKeyXListener* srpKeyXListener;
	const char* interfaces[];

	AJInitializer *ajInitial;
	bool mresetConnection;

};

#endif /* ONBOARDINGTEST_H_ */