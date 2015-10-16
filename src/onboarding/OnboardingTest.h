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
	OnboardingTest();
	~OnboardingTest();
	/*
		Create Bus Attachment
	*/
	QStatus CreateBusAttachment();
	QStatus FinishBusAttachment();

private:
	ajn::BusAttachment* mbusAttachment;
	AboutListenerHandlerImpl* announceHandler;
	SrpKeyXListener* srpKeyXListener;
	const char* interfaces[];

	AJInitializer *ajInitial;

};

#endif /* ONBOARDINGTEST_H_ */