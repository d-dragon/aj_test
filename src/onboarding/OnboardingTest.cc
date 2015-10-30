/*
*   Source file: OnboardingTest.cc
*/

#include <iostream>
#include <iomanip>
#include <signal.h>
#include <set>
#include <alljoyn/AboutData.h>
#include <alljoyn/AboutListener.h>
#include <alljoyn/AboutObjectDescription.h>

#include <alljoyn/BusAttachment.h>

#include <alljoyn/version.h>
#include <qcc/Log.h>
#include <AJInitializer.h>
#include "OnboardingTest.h"
//using namespace services;
/*
    Constructor and Destructors
*/
OnboardingTest::OnboardingTest(std::string input[]){
    ajInitial = new AJInitializer();
    if (ajInitial->Initialize() != ER_OK) {
        std::cout << "Unable to Initialize AllJoyn " << std::endl;
    }
    *interfaces         = {"org.alljoyn.Onboarding"};
    mbusAttachment      = NULL;
    mcbFlags            = 0;
    GetInfoFromString(input);
}
OnboardingTest::OnboardingTest(long flags){
// Initialize AllJoyn
    ajInitial = new AJInitializer();
    if (ajInitial->Initialize() != ER_OK) {
        std::cout << "Unable to Initialize AllJoyn " << std::endl;
    }
    *interfaces={"org.alljoyn.Onboarding"};
    mbusAttachment      = NULL;
    mcbFlags    = flags;
}

OnboardingTest::~OnboardingTest(){
#if DBGE
    std::cout<< "OnboardingTest::~OnboardingTest" << std::endl;
#endif
	delete ajInitial;
}

// Create bus attachment

QStatus OnboardingTest::CreateBusAttachment(ajn::services::OBInfo connectionInput){
    QStatus status;
    mbusAttachment = new BusAttachment("OnboardingClient", true);

    status = mbusAttachment->Start();
    if (ER_OK == status) {
        std::cout << "BusAttachment started." << std::endl;
    } else {
        std::cout << "Unable to start BusAttachment. Status: " << QCC_StatusText(status) << std::endl;
        return ER_FAIL;
    }

    status = mbusAttachment->Connect();
    if (ER_OK == status) {
        std::cout << "Daemon Connect succeeded." << std::endl;
    } else {
        std::cout << "Failed to connect daemon. Status: " << QCC_StatusText(status) << std::endl;
        return ER_FAIL;

    }

    srpKeyXListener = new SrpKeyXListener();
    status = mbusAttachment->EnablePeerSecurity("ALLJOYN_SRP_KEYX ALLJOYN_ECDHE_PSK", srpKeyXListener, "/.alljoyn_keystore/central.ks", true);

    if (ER_OK == status) {
        std::cout << "EnablePeerSecurity called." << std::endl;
    } else {
        std::cout << "ERROR - EnablePeerSecurity call FAILED with status " << QCC_StatusText(status) << std::endl;
        return ER_FAIL;
    }

    announceHandler = new AboutListenerHandlerImpl(mbusAttachment, mcbFlags, connectionInput);
    mbusAttachment->RegisterAboutListener(*announceHandler);

    status = mbusAttachment->WhoImplements(interfaces, sizeof(interfaces) / sizeof(interfaces[0]));
    if (ER_OK == status) {
        std::cout << "WhoImplements called." << std::endl;
    } else {
        std::cout << "ERROR - WhoImplements failed." << std::endl;
    }
    return ER_OK;
}


QStatus OnboardingTest::FinishBusAttachment(){
    mbusAttachment->CancelWhoImplements(interfaces, sizeof(interfaces) / sizeof(interfaces[0]));
    mbusAttachment->UnregisterAboutListener(*announceHandler);

    mbusAttachment->Stop();
    delete mbusAttachment;
    delete srpKeyXListener;
    delete announceHandler;
    return ER_OK;
}

void OnboardingTest::GetInfoFromString(std::string input[]){
    int i;
    if (!input[0].empty()) mWifiConnectionInfo.SSID.assign(input[0].c_str());
    if (!input[1].empty()) mWifiConnectionInfo.passcode.assign(input[1].c_str());
    for ( i = 0; i< sizeof (smWifiAuthType)/sizeof(struct WifiAuthenticationType); i++)
    {
        if (input[2].compare(smWifiAuthType[i].Name) == 0){
            mWifiConnectionInfo.authType = smWifiAuthType[i].Val;
            break;
        }
    }
    if (input[3].compare("on") == 0) mcbFlags |= GET_VERSION;
    if (input[4].compare("on") == 0) mcbFlags |= GET_STATE;
    if (input[5].compare("on") == 0) mcbFlags |= GET_LAST_ERR;
    if (input[6].compare("on") == 0) mcbFlags |= GET_SCAN_INFO;
    if (input[7].compare("on") == 0) mcbFlags |= CONF_AND_CONNECT_WIFI;
    if (input[8].compare("on") == 0) mcbFlags |= OFF_BOARD_FROM;
}   

// Create bus attachment

QStatus OnboardingTest::CreateBusAttachment(){
    QStatus status;
    mbusAttachment = new BusAttachment("OnboardingClient", true);

    status = mbusAttachment->Start();
    if (ER_OK == status) {
        std::cout << "BusAttachment started." << std::endl;
    } else {
        std::cout << "Unable to start BusAttachment. Status: " << QCC_StatusText(status) << std::endl;
        return ER_FAIL;
    }

    status = mbusAttachment->Connect();
    if (ER_OK == status) {
        std::cout << "Daemon Connect succeeded." << std::endl;
    } else {
        std::cout << "Failed to connect daemon. Status: " << QCC_StatusText(status) << std::endl;
        return ER_FAIL;

    }

    srpKeyXListener = new SrpKeyXListener();
    status = mbusAttachment->EnablePeerSecurity("ALLJOYN_SRP_KEYX ALLJOYN_ECDHE_PSK", srpKeyXListener, "/.alljoyn_keystore/central.ks", true);

    if (ER_OK == status) {
        std::cout << "EnablePeerSecurity called." << std::endl;
    } else {
        std::cout << "ERROR - EnablePeerSecurity call FAILED with status " << QCC_StatusText(status) << std::endl;
        return ER_FAIL;
    }

    announceHandler = new AboutListenerHandlerImpl(mbusAttachment, mcbFlags, mWifiConnectionInfo);
    mbusAttachment->RegisterAboutListener(*announceHandler);

    status = mbusAttachment->WhoImplements(interfaces, sizeof(interfaces) / sizeof(interfaces[0]));
    if (ER_OK == status) {
        std::cout << "WhoImplements called." << std::endl;
    } else {
        std::cout << "ERROR - WhoImplements failed." << std::endl;
    }
    return ER_OK;
}