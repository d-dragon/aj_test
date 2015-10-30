#ifndef ONBOARDINGTEST_H_
#define ONBOARDINGTEST_H_

#include <alljoyn/Status.h>
#include <AboutListenerHandlerImpl.h>
#include <SrpKeyXListener.h>
#include <AJInitializer.h>
#include "common_def.h"
#define DBGE 1
using namespace ajn;
using namespace services;

class OnboardingTest{

public:
	/*
		Intialize and Destroy AJ initialization
		cbFlgs: list cb function will be called
	*/
	OnboardingTest(long cbFlgs);
	/*
		Get wifi info, cb function via input[]
	*/
	OnboardingTest(std::string input[]);
	~OnboardingTest();
	/*
		This function need initialize from constructor OnboardingTest(long);
	*/
	QStatus CreateBusAttachment(ajn::services::OBInfo oBInfo);

	/*
		This function need initialize from constructor OnboardingTest(std::string input[]);
	*/
	QStatus CreateBusAttachment();

	QStatus FinishBusAttachment();

private:
	// Convert input into suitable value of wifi info, cb functions
	void GetInfoFromString(std::string[]);
	ajn::BusAttachment* mbusAttachment;
	AboutListenerHandlerImpl* announceHandler;
	SrpKeyXListener* srpKeyXListener;
	const char* interfaces[];

	AJInitializer *ajInitial;
	long mcbFlags;
	ajn::services::OBInfo mWifiConnectionInfo;
	struct WifiAuthenticationType smWifiAuthType[10]={
	    {"WPA2_AUTO",   WPA2_AUTO},                     //!< WPA2_AUTO authentication
	    {"WPA_AUTO",    WPA_AUTO},                      //!< WPA_AUTO authentication
	    {"ANY",         ANY},                           //!< ANY authentication
	    {"OPEN",        OPEN},                          //!< OPEN authentication
	    {"WEP",         WEP},                           //!< WEP authentication
	    {"WPA_TKIP",    WPA_TKIP},                      //!< WPA_TKIP authentication
	    {"WPA_CCMP",    WPA_CCMP},                      //!< WPA_CCMP authentication
	    {"WPA2_TKIP",   WPA2_TKIP},                     //!<WPA2_TKIP authentication
	    {"WPA2_CCMP",   WPA2_CCMP},                     //!<WPA2_CCMP authentication
	    {"WPS",         WPS}                            //!<WPS authentication
	};
};

#endif /* ONBOARDINGTEST_H_ */
