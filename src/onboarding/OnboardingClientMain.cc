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
#if 0
#else
    static volatile sig_atomic_t quit = false;
    static void CDECL_CALL SigIntHandler(int sig)
    {
        quit = true;
    }

    int main(int argc, char **argv){
        QStatus ret;
        OBInfo oBInfo;
        // Default value
        oBInfo.SSID.assign("VEriK2");
        oBInfo.passcode.assign("564572694b73797374656d733130");
        oBInfo.authType = WPA2_TKIP;
        std::cout <<  oBInfo.SSID.c_str() <<  oBInfo.passcode.c_str() << oBInfo.authType << "tuanngo main";
//        ParsingModule *inputParsing = new ParsingModule();
        OnboardingTest *onboardingTestApp;
        /* Install SIGINT handler so Ctrl + C deallocates memory properly */
        signal(SIGINT, SigIntHandler);
        bool isReset = false;
        onboardingTestApp = new OnboardingTest(isReset);
        ret = onboardingTestApp->CreateBusAttachment(oBInfo);
        if ( ER_OK != ret )
        {
            onboardingTestApp->FinishBusAttachment();
            delete onboardingTestApp;
            return 1;
        }
        while (!quit) {
        #ifdef _WIN32
                Sleep(100);
        #else
                usleep(100 * 1000);
        #endif
            }
        onboardingTestApp->FinishBusAttachment();
        delete onboardingTestApp;
//        delete inputParsing;
        return 0;
    }
#endif