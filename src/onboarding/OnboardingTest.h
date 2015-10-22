#ifndef ONBOARDINGTEST_H_
#define ONBOARDINGTEST_H_

#include <alljoyn/Status.h>
#include <AboutListenerHandlerImpl.h>
#include <SrpKeyXListener.h>
#include <AJInitializer.h>
#define DBGE 1
class OnboardingTest{

public:
	/*
	Intialize and Destroy AJ initialization
	*/
	OnboardingTest(long);
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
	long mcbFlags;

};

#endif /* ONBOARDINGTEST_H_ */
