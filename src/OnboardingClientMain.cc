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
#include "common_def.h"
#include <getopt.h>

using namespace ajn;
using namespace services;
#define TIME_OUT 100


bool printHelpContent(OBInfo, long);
void doOnboarding(OBInfo oBInfo, long flags);


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
        LOGCXX("\nCurent test case num: "<< i+1 << std::endl);
		cbFlgs = 0;
		parsingClient->GetWifiConfiguration(&wifiInfo, &cbFlgs, i);
        if(printHelpContent(wifiInfo, cbFlgs))
        {
            LOGCXX("....Exit app");
            break;
        }

        doOnboarding(wifiInfo, cbFlgs );
        
	}
	delete parsingClient;
	return 0;
}

bool printHelpContent(OBInfo wifiInfo, long flg){
    bool quit = true;
    char input = 'q';
    do {
        if ((flg & OFF_BOARD_FROM) != OFF_BOARD_FROM) // not enable offboard function
        {
            LOGCXX("\nPlease make sure your PC is connected to CERES AP");
            if ((flg & CONF_AND_CONNECT_WIFI) == CONF_AND_CONNECT_WIFI){
                if (wifiInfo.SSID.empty())
                    LOGCXX("\nAre you sure without SSID? for this test case?" );
                else
                    LOGCXX("SSID: "<< wifiInfo.SSID.c_str() << (flg & OFF_BOARD_FROM)) ;
                if (wifiInfo.passcode.empty())
                    LOGCXX("\nPasscode is empty, are you sure?");
            }
        }else
        {
            LOGCXX("\nPlease make suere your PC is connected to the AP which CERES is connected to");
            LOGCXX("\nthen we do the offboard function");
        }
        


        LOGCXX("\nPlease do configuration before continue \nWhen you're ready, press 'y' to confinue or 'q' to exit program\n...");
        input = std::cin.get();
        switch(input){
            case 'y':
                quit = true;
                break;
            case 'q':
                return true;
            default:
                quit = false;
                break;
        }   
    }
    while (quit != true);
    return false;
}
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
