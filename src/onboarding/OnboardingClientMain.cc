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

static volatile sig_atomic_t quit = false;
static void CDECL_CALL SigIntHandler(int sig)
{
    quit = true;
}

int main(int argc, char **argv){
    QStatus ret;
    OBInfo oBInfo;
    int retcode, val;
//    oBInfo.SSID.assign("VEriK2");
//    oBInfo.passcode.assign("564572694b73797374656d733130");
//    oBInfo.authType = WPA2_TKIP;
    bool isReset = false;
    while (1)
    {
      static struct option long_options[] =
        {
          /* These options don’t set a flag.
             We distinguish them by their indices. */
          {"SSID",  required_argument,       0, 's'},
          {"pass",  required_argument,       0, 'p'},
          {"authtype",  required_argument, 0, 'a'},
          {"resetwifi",  required_argument, 0, 'r'},
        {0, 0, 0, 0}
        };
      /* getopt_long stores the option index here. */
      int option_index = 0;

      retcode = getopt_long (argc, argv, "s:p:a:r:",
                       long_options, &option_index);

      /* Detect the end of the options. */
      if (retcode == -1)
        break;

      switch (retcode)
        {
        case 's':
            std::cout << "SSID: " << optarg<<std::endl;
            oBInfo.SSID.assign(optarg);
            break;

        case 'p':
            std::cout << "pass: " << optarg<<std::endl;
            oBInfo.passcode.assign(optarg);
            break;

        case 'a':
            std::cout << "authType: " << optarg<<std::endl;
            val = atoi(optarg);
            if (val<= WPS && val >= WPA2_AUTO){
               oBInfo.authType=(OBAuthType)val; 
            }
            else{
                std::cout <<"Invalid authType, exit program";
                exit(1);
            }
            break;

        case 'r':
          std::cout << "resetwifi: " << optarg <<std::endl;
            if (1 == atoi(optarg)){
                isReset = true;
            }
          break;

        default:
          abort ();
        }
    }

    std::cout << oBInfo.SSID.c_str() << ":" << oBInfo.passcode.c_str() << ":" <<oBInfo.authType<<std::endl;
    OnboardingTest *onboardingTestApp;
    /* Install SIGINT handler so Ctrl + C deallocates memory properly */
    signal(SIGINT, SigIntHandler);

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
    std::cout << "the end";
    delete onboardingTestApp;
//        delete inputParsing;
    return 0;
}
