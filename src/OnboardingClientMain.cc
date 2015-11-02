/******************************************************************************
* Copyright AllSeen Alliance. All rights reserved.
*
*    Permission to use, copy, modify, and/or distribute this software for any
*    purpose with or without fee is hereby granted, provided that the above
*    copyright notice and this permission notice appear in all copies.
*
*    THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
*    WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
*    MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
*    ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
*    WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
*    ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
*    OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
******************************************************************************/

#include <iostream>
#include <iomanip>
#include <signal.h>
#include <set>
#include <alljoyn/onboarding/Onboarding.h>
#include "OnboardingTest.h"
#include "ParsingModule.h"
#include <getopt.h>

using namespace ajn;
using namespace services;
#define TIME_OUT 100

void doOnboarding(OBInfo oBInfo, long flags){
	QStatus ret;
	int timeout = 0;

	OnboardingTest *onboardingTestApp;
    /* Install SIGINT handler so Ctrl + C deallocates memory properly */
//    signal(SIGINT, SigIntHandler);


	onboardingTestApp = new OnboardingTest(flags);
    ret = onboardingTestApp->CreateBusAttachment(oBInfo);
    if ( ER_OK != ret )
    {

        onboardingTestApp->FinishBusAttachment();
        return;
    }
    while (timeout++ < TIME_OUT) {
		
       usleep(100 * 1000);
    }
    onboardingTestApp->FinishBusAttachment();
    std::cout << "the end";
	return;
}


int main( int argc, char **argv){
	
	int i, num = 0;
	long cbFlgs;

	OBInfo wifiInfo;
	ParsingModule *parsingClient;
	parsingClient = new ParsingModule();
	//Read configuration file
	parsingClient->LoadJSONFromFile("./wifi.json");

	num = parsingClient->GetNumOfWifiConfiguration();
	LOGCXX("Number of setting is: " << num <<std::endl);
	for (i =0; i < num ; i++)
	{
		cbFlgs = 0;
		parsingClient->GetNextWifiConfiguration(&wifiInfo,&cbFlgs);
		LOGCXX("Wifi " << i << " config info: " << wifiInfo.SSID.c_str() << " : "  << wifiInfo.passcode.c_str() <<" : " << wifiInfo.authType << ":" << std::hex << cbFlgs << std::endl);
		doOnboarding(wifiInfo, cbFlgs );
		break;
	}

	delete parsingClient;
	return 0;
}
