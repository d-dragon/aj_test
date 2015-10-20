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

#include "AboutListenerHandlerImpl.h"

#include <SessionListenerImpl.h>
#include <AsyncSessionJoinerCB.h>

using namespace ajn;

AboutListenerHandlerImpl::AboutListenerHandlerImpl( BusAttachment *inputBusAttment, bool isReset,ajn::services::OBInfo conInfo)
{
    busAttment      = inputBusAttment;
    mresetConnection = isReset;
    
    oBInfo.SSID.assign(conInfo.SSID.c_str());
    oBInfo.passcode.assign(conInfo.passcode.c_str());
    oBInfo.authType = conInfo.authType;
}

AboutListenerHandlerImpl::~AboutListenerHandlerImpl()
{
}

void AboutListenerHandlerImpl::Announced(const char* busName, uint16_t version, SessionPort port, const MsgArg& objectDescriptionArg, const MsgArg& aboutDataArg)
{
    std::cout << "*********************************************************************************" << std::endl;
    std::cout << "Announce signal discovered" << std::endl;
    std::cout << "\tFrom bus " << busName << std::endl;
    std::cout << "\tAbout version " << version << std::endl;
    std::cout << "\tSessionPort " << port << std::endl;
    std::cout << "\tObjectDescription" << std::endl << objectDescriptionArg.ToString().c_str() << std::endl;
    std::cout << "\tAboutData:" << std::endl << aboutDataArg.ToString().c_str() << std::endl;
    std::cout << "*********************************************************************************" << std::endl;

    std::cout << "AboutListenerHandlerImpl::Announced()" << std::endl;
//    if (m_BasicCallback) {
        std::cout << "Calling AnnounceHandler Callback" << std::endl;
        announceBasicHandlerCallback(busName, port);
//    }

/*    if (m_FullCallback) {
        std::cout << "Calling AnnounceHandler Callback" << std::endl;
        AboutData aboutData;
        aboutData.CreatefromMsgArg(aboutDataArg);
        AboutObjectDescription aboutObjectDescription;
        aboutObjectDescription.CreateFromMsgArg(objectDescriptionArg);
        m_FullCallback(busName, version, port, aboutObjectDescription, aboutData);
    }*/
}

void AboutListenerHandlerImpl::announceBasicHandlerCallback(qcc::String const& busName, unsigned short port)
{
    std::cout << "announceHandlerCallback " << busName.c_str() << " " << port << std::endl;
    std::set<qcc::String>::iterator searchIterator = handledAnnouncements.find(qcc::String(busName));
    if (searchIterator == handledAnnouncements.end()) {
        handledAnnouncements.insert(busName);

        SessionOpts opts(SessionOpts::TRAFFIC_MESSAGES, false, SessionOpts::PROXIMITY_ANY, TRANSPORT_ANY);
        SessionListenerImpl* sessionListener = new SessionListenerImpl(busName);
        AsyncSessionJoinerCB* joincb = new AsyncSessionJoinerCB(busName.c_str(), busAttment, mresetConnection, oBInfo);

        QStatus status = busAttment->JoinSessionAsync(busName.c_str(), port, sessionListener, opts, joincb,
                                                         sessionListener);

        if (status != ER_OK) {
            std::cout << "Unable to JoinSession with " << busName.c_str() << std::endl;
        }
    } else {
        std::cout << busName.c_str()  << " has already been handled" << std::endl;
    }
}

