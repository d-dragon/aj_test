/*
*   Source file: OnboardingTest.cc
*/

#include <iostream>
#include <iomanip>
#include <signal.h>
#include <set>
//#include <alljoyn/services_common/LogModulesNames.h>

#include <alljoyn/AboutData.h>
#include <alljoyn/AboutListener.h>
#include <alljoyn/AboutObjectDescription.h>

#include <alljoyn/BusAttachment.h>

#include <alljoyn/version.h>
#include <qcc/Log.h>
#include <AJInitializer.h>
#include "OnboardingTest.h"

using namespace ajn;
//using namespace services;
/*
    Constructor and Destructors
*/

OnboardingTest::OnboardingTest(){
// Initialize AllJoyn
    ajInitial = new AJInitializer();
    if (ajInitial->Initialize() != ER_OK) {
        std::cout << "Unable to Initialize AllJoyn " << std::endl;
    }
    *interfaces={"org.alljoyn.Onboarding"};
    mbusAttachment =  NULL;
}

OnboardingTest::~OnboardingTest(){
	delete ajInitial;
}

// Create bus attachment

QStatus OnboardingTest::CreateBusAttachment(){
    QStatus status;
    mbusAttachment = new BusAttachment("OnboardingClient", true);

    status = mbusAttachment->Start();
    if (status == ER_OK) {
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

    announceHandler = new AboutListenerHandlerImpl(mbusAttachment);
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