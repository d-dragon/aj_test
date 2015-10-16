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

#ifndef ABOUTLISTENERHANDLERSAMPLE_H_
#define ABOUTLISTENERHANDLERSAMPLE_H_
#include <alljoyn/AboutListener.h>
#include <iostream>
#include <iomanip>
#include <signal.h>
#include <set>

#if defined(QCC_OS_GROUP_WINDOWS)
/* Disabling warning C 4100. Function doesnt use all passed in parameters */
#pragma warning(push)
#pragma warning(disable: 4100)
#endif


/**
 * class AnnounceHandlerImpl
 */
class AboutListenerHandlerImpl : public ajn::AboutListener {

  public:

    /**
     * Announced - announced callback
     * @param busName
     * @param version
     * @param port
     * @param objectDescriptionArg
     * @param aboutDataArg
     */
    virtual void Announced(const char* busName, uint16_t version, ajn::SessionPort port, const ajn::MsgArg& objectDescriptionArg, const ajn::MsgArg& aboutDataArg);
    /**
     * AnnounceHandlerImpl
     * @param basicCallback
     * @param fullCallback
     */
    AboutListenerHandlerImpl(ajn::BusAttachment *);

    /**
     * ~AnnounceHandlerImpl
     */
    ~AboutListenerHandlerImpl();

  private:

    void announceBasicHandlerCallback(qcc::String const& busName, unsigned short port);

    std::set<qcc::String> handledAnnouncements;

    ajn::BusAttachment *busAttment;
};

#endif /* ABOUTLISTENERHANDLERSAMPLE_H_ */
