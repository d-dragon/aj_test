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

#include "AsyncSessionJoinerCB.h"
#include "SessionListenerImpl.h"
#include <alljoyn/config/ConfigClient.h>
#include <alljoyn/onboarding/OnboardingClient.h>
#include <alljoyn/AboutIconProxy.h>
#include <OnboardingSignalListenerImpl.h>
#include <iostream>
#include <iomanip>
#include <signal.h>
#include <set>

using namespace ajn;
using namespace services;

AsyncSessionJoinerCB::AsyncSessionJoinerCB(const char* name, BusAttachment* inputBusAttment, bool isReset) :
    m_Busname("")
{
    if (name) {
        m_Busname.assign(name);
    }
    busAttment = inputBusAttment;
    mresetConnection = isReset;
}

AsyncSessionJoinerCB::~AsyncSessionJoinerCB()
{

}

void AsyncSessionJoinerCB::JoinSessionCB(QStatus status, SessionId id, const SessionOpts& opts, void* context)
{
    if (status == ER_OK) {
        std::cout << "JoinSessionCB(" << m_Busname.c_str() << ") succeeded with id" << id << std::endl;
 //       if (m_Callback) {
            std::cout << "Calling SessionJoiner Callback" << std::endl;
            sessionJoinedCallback(m_Busname, id);
//        }
    } else {
        std::cout << "JoinSessionCB(" << m_Busname.c_str() << ") failed with status: " << QCC_StatusText(status) << std::endl;
    }

    SessionListenerImpl* listener = (SessionListenerImpl*) context;
    delete listener;
    delete this;
}


/* Callback function
*/

void AsyncSessionJoinerCB::sessionJoinedCallback(qcc::String const& busName, SessionId id)
{
    QStatus status = ER_OK;
    if (busAttment == NULL) {
        return;
    }

    busAttment->EnableConcurrentCallbacks();

    AboutProxy aboutProxy(*busAttment, busName.c_str(), id);

    bool isIconInterface = false;
    bool isConfigInterface = false;
    bool isOnboardingInterface = false;
    /*
        Connecting wifi infor
     */
    OBInfo oBInfo;
    oBInfo.SSID.assign("VEriK2");
    oBInfo.passcode.assign("564572694b73797374656d733130");
    oBInfo.authType = WPA2_TKIP;

    std::cout << std::endl << busName.c_str() << " AboutClient ObjectDescriptions" << std::endl;
    std::cout << "-----------------------------------" << std::endl;

    MsgArg objArg;
    aboutProxy.GetObjectDescription(objArg);
    std::cout << "AboutProxy.GetObjectDescriptions:\n" << objArg.ToString().c_str() << "\n\n" << std::endl;

    AboutObjectDescription objectDescription;
    objectDescription.CreateFromMsgArg(objArg);

    isIconInterface = false;
    isIconInterface = objectDescription.HasInterface("/About/DeviceIcon", "org.alljoyn.Icon");

    if (isIconInterface) {
        std::cout << "The given interface 'org.alljoyn.Icon' is found in a given path '/About/DeviceIcon'" << std::endl;
    } else {
        std::cout << "WARNING - The given interface 'org.alljoyn.Icon' is not found in a given path '/About/DeviceIcon'" << std::endl;
    }

    isConfigInterface = false;
    isConfigInterface = objectDescription.HasInterface("/Config", "org.alljoyn.Config");
    if (isConfigInterface) {
        std::cout << "The given interface 'org.alljoyn.Config' is found in a given path '/Config'" << std::endl;
    } else {
        std::cout << "WARNING - The given interface 'org.alljoyn.Config' is not found in a given path '/Config'" << std::endl;
    }

    isOnboardingInterface = false;
    isOnboardingInterface = objectDescription.HasInterface("/Onboarding", "org.alljoyn.Onboarding");
    if (isOnboardingInterface) {
        std::cout << "The given interface 'org.alljoyn.Onboarding' is found in a given path '/Onboarding'" << std::endl;
    } else {
        std::cout << "WARNING - The given interface 'org.alljoyn.Onboarding' is not found in a given path '/Onboarding'" << std::endl;
    }
    GetAllAboutData(aboutProxy);

    std::cout << "aboutProxy GetVersion " << std::endl;
    std::cout << "-----------------------" << std::endl;

    uint16_t version = 0;
    status = aboutProxy.GetVersion(version);
    if (status != ER_OK) {
        std::cout << "WARNING - Call to getVersion failed " << QCC_StatusText(status) << std::endl;
    } else {
        std::cout << "Version=" << version << std::endl;
    }

    if (isIconInterface) {
        AboutIconProxy aiProxy(*busAttment, busName.c_str(), id);
        AboutIcon aboutIcon;

        std::cout << std::endl << busName.c_str() << " AboutIconProxy GetIcon" << std::endl;
        std::cout << "-----------------------------------" << std::endl;

        status = aiProxy.GetIcon(aboutIcon);
        if (status != ER_OK) {
            std::cout << "WARNING - Call to GetIcon failed: " << QCC_StatusText(status) << std::endl;
            //goto
        }

        std::cout << "url=" << aboutIcon.url.c_str() << std::endl;
        std::cout << "Content size = " << aboutIcon.contentSize << std::endl;
        std::cout << "Content =\t";
        for (size_t i = 0; i < aboutIcon.contentSize; i++) {
            if (i % 8 == 0 && i > 0) {
                std::cout << "\n\t\t";
            }
            std::cout << std::hex << std::uppercase << std::setfill('0') << std::setw(2) << (unsigned int)aboutIcon.content[i]
                      << std::nouppercase << std::dec;

            //std::cout << std::endl;
        }
        std::cout << std::endl;
        std::cout << "Mimetype =\t" << aboutIcon.mimetype.c_str() << std::endl;
        std::cout << std::endl << busName.c_str() << " AboutIcontClient GetVersion" << std::endl;
        std::cout << "-----------------------------------" << std::endl;

        uint16_t version;
        status = aiProxy.GetVersion(version);
        if (status != ER_OK) {
            std::cout << "WARNING - Call to getVersion failed: " << QCC_StatusText(status) << std::endl;
        } else {
            std::cout << "Version=" << version << std::endl;
        }
    }

    ConfigClient* configClient = NULL;
    if (isConfigInterface) {
        configClient = new ConfigClient(*busAttment);

        std::cout << std::endl << busName.c_str() << " ConfigClient GetVersion" << std::endl;
        std::cout << "-----------------------------------" << std::endl;
        int version = 0;
        if ((status = configClient->GetVersion(busName.c_str(), version, id)) == ER_OK) {
            std::cout << "Success GetVersion. Version=" << version << std::endl;
        } else {
            std::cout << "Call to getVersion failed: " << QCC_StatusText(status) << std::endl;
        }

        ConfigClient::Configurations configurations;
        std::cout << std::endl << busName.c_str() << " ConfigClient GetConfigurations" << std::endl;
        std::cout << "-----------------------------------" << std::endl;

        if ((status = configClient->GetConfigurations(busName.c_str(), "en", configurations, id)) == ER_OK) {

            for (ConfigClient::Configurations::iterator it = configurations.begin(); it != configurations.end(); ++it) {
                qcc::String key = it->first;
                ajn::MsgArg value = it->second;
                if (value.typeId == ALLJOYN_STRING) {
                    std::cout << "Key name=" << key.c_str() << " value=" << value.v_string.str << std::endl;
                } else if (value.typeId == ALLJOYN_ARRAY && value.Signature().compare("as") == 0) {
                    std::cout << "Key name=" << key.c_str() << " values: ";
                    const MsgArg* stringArray;
                    size_t fieldListNumElements;
                    status = value.Get("as", &fieldListNumElements, &stringArray);
                    for (unsigned int i = 0; i < fieldListNumElements; i++) {
                        char* tempString;
                        stringArray[i].Get("s", &tempString);
                        std::cout << tempString << " ";
                    }
                    std::cout << std::endl;
                }
            }
        } else {
            std::cout << std::endl << "Call to GetConfigurations failed: " << QCC_StatusText(status) << std::endl;
        }
    }

    OnboardingClient* onboardingClient = NULL;
    OnboardingSignalListenerImpl* signalListener = new OnboardingSignalListenerImpl();

    if (isOnboardingInterface) {
        onboardingClient = new    OnboardingClient(*busAttment, *signalListener);

        std::cout << std::endl << busName.c_str() << " OnboardingClient GetVersion" << std::endl;
        std::cout << "-----------------------------------" << std::endl;
        int version = 0;
        if ((status = onboardingClient->GetVersion(busName.c_str(), version, id)) == ER_OK) {
            std::cout << "Version=" << version << std::endl;
        } else {
            std::cout << "Call to GetVersion failed " << QCC_StatusText(status) << std::endl;
        }


        std::cout << std::endl << busName.c_str() << " OnboardingClient GetState" << std::endl;
        std::cout << "-----------------------------------" << std::endl;
        short int state = 0;
        if ((status = onboardingClient->GetState(busName.c_str(), state, id)) == ER_OK) {
            std::cout << "GetState=" << state << std::endl;
        } else {
            std::cout << "Call to GetState failed " << QCC_StatusText(status) << std::endl;
        }

        std::cout << std::endl << busName.c_str() << " OnboardingClient GetLastError" << std::endl;
        std::cout << "-----------------------------------" << std::endl;

        OBLastError lastError = { 0, "" };
        if ((status = onboardingClient->GetLastError(busName.c_str(), lastError, id)) == ER_OK) {
            std::cout << "OBLastError code=" << lastError.validationState << " message= " << lastError.message.c_str() << std::endl;
        } else {
            std::cout << "Call to GetLastError failed " << QCC_StatusText(status) << std::endl;
        }
        if ( true != mresetConnection )
        {
            std::cout << std::endl << busName.c_str() << " OnboardingClient GetScanInfo" << std::endl;
            std::cout << "-----------------------------------" << std::endl;
            unsigned short age = 0;
            OnboardingClient::ScanInfos scanInfos;
            if ((status = onboardingClient->GetScanInfo(busName.c_str(), age, scanInfos, id)) == ER_OK) {
                for (OnboardingClient::ScanInfos::iterator it = scanInfos.begin(); it != scanInfos.end(); ++it) {
                    std::cout << "Network  SSID=" << it->SSID.c_str() << " authType=" << it->authType << std::endl;
                }
            } else {
                std::cout << "Call to GetScanInfo failed " << QCC_StatusText(status) << std::endl;
            }

            std::cout << std::endl << busName.c_str() << " OnboardingClient ConfigureWiFi" << std::endl;
            std::cout << "-----------------------------------" << std::endl;

            short resultStatus;

            if ((status = onboardingClient->ConfigureWiFi(busName.c_str(), oBInfo, resultStatus, id)) == ER_OK) {
                std::cout << "Call to ConfigureWiFi succeeded " << std::endl;
            } else {
                std::cout << "Call to ConfigureWiFi failed " << QCC_StatusText(status) << std::endl;
            }

            if ((status = onboardingClient->ConnectTo(busName.c_str(), id)) == ER_OK) {
                std::cout << "Call to ConnectTo succeeded " << std::endl;
            } else {
                std::cout << "Call to ConnectTo failed " << QCC_StatusText(status) << std::endl;
            }
        }else{ 
            // Call OffBoard to reset connection.

           if ((status = onboardingClient->OffboardFrom(busName.c_str(), id)) == ER_OK) {
               std::cout << "Call to OffboardFrom succeeded " << std::endl;
           } else {
               std::cout << "Call to OffboardFrom failed " << QCC_StatusText(status) << std::endl;
           }
       }

    }

    status = busAttment->LeaveSession(id);
    std::cout << "Leaving session id = " << id << " with " << busName.c_str() << " status: " << QCC_StatusText(status) << std::endl;

    if (configClient) {
        delete configClient;
        configClient = NULL;
    }

    if (onboardingClient) {
        delete onboardingClient;
        onboardingClient = NULL;
    }

    if (signalListener) {
        delete signalListener;
        signalListener = NULL;
    }
}


void AsyncSessionJoinerCB::GetAboutData(AboutData& aboutData, const char* language)
{
    size_t count = aboutData.GetFields();

    const char** fields = new const char*[count];
    aboutData.GetFields(fields, count);

    for (size_t i = 0; i < count; ++i) {
        std::cout << "\tKey: " << fields[i];

        MsgArg* tmp;
        aboutData.GetField(fields[i], tmp, language);
        std::cout << "\t";
        if (tmp->Signature() == "s") {
            const char* tmp_s;
            tmp->Get("s", &tmp_s);
            std::cout << tmp_s;
        } else if (tmp->Signature() == "as") {
            size_t las;
            MsgArg* as_arg;
            tmp->Get("as", &las, &as_arg);
            for (size_t j = 0; j < las; ++j) {
                const char* tmp_s;
                as_arg[j].Get("s", &tmp_s);
                std::cout << tmp_s << " ";
            }
        } else if (tmp->Signature() == "ay") {
            size_t lay;
            uint8_t* pay;
            tmp->Get("ay", &lay, &pay);
            for (size_t j = 0; j < lay; ++j) {
                std::cout << std::hex << static_cast<int>(pay[j]) << " ";
            }
        } else {
            std::cout << "User Defined Value\tSignature: " << tmp->Signature().c_str();
        }
        std::cout << std::endl;
    }
    delete [] fields;
    std::cout << std::endl;
}

void AsyncSessionJoinerCB::GetAllAboutData(AboutProxy& aboutProxy)
{
    MsgArg aArg;
    QStatus status = aboutProxy.GetAboutData(NULL, aArg);
    if (status == ER_OK) {
        std::cout << "*********************************************************************************" << std::endl;
        std::cout << "GetAboutData: (Default Language)" << std::endl;
        AboutData aboutData(aArg);
        GetAboutData(aboutData, NULL);
        size_t lang_num;
        lang_num = aboutData.GetSupportedLanguages();
        // If the lang_num == 1 we only have a default language
        if (lang_num > 1) {
            const char** langs = new const char*[lang_num];
            aboutData.GetSupportedLanguages(langs, lang_num);
            char* defaultLanguage;
            aboutData.GetDefaultLanguage(&defaultLanguage);
            // print out the AboutData for every language but the
            // default it has already been printed.
            for (size_t i = 0; i < lang_num; ++i) {
                if (strcmp(defaultLanguage, langs[i]) != 0) {
                    status = aboutProxy.GetAboutData(langs[i], aArg);
                    if (ER_OK == status) {
                        aboutData.CreatefromMsgArg(aArg, langs[i]);
                        std::cout <<  "GetAboutData: (" << langs[i] << ")" << std::endl;
                        GetAboutData(aboutData, langs[i]);
                    }
                }
            }
            delete [] langs;
        }
        std::cout << "*********************************************************************************" << std::endl;
    }
}
