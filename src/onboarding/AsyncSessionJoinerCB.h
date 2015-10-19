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

#ifndef ASYNCSESSIONJOINERCB_H_
#define ASYNCSESSIONJOINERCB_H_

#include <alljoyn/BusAttachment.h>
#include <alljoyn/AboutProxy.h>
#include <alljoyn/onboarding/Onboarding.h>
#if defined(QCC_OS_GROUP_WINDOWS)
/* Disabling warning C 4100. Function doesnt use all passed in parameters */
#pragma warning(push)
#pragma warning(disable: 4100)
#endif

/**
 * class AsyncSessionJoinerCB
 */
class AsyncSessionJoinerCB : public ajn::BusAttachment::JoinSessionAsyncCB {

  public:
    /**
     * Constructor
     * @param name
     * @param callback
     */
    AsyncSessionJoinerCB(const char* name, ajn::BusAttachment* inputBusAttment, bool resetConnection, ajn::services::OBInfo oBInfo);

    /**
     * destructor
     */
    virtual ~AsyncSessionJoinerCB();

    /**
     * JoinSessionCB
     * @param status
     * @param id
     * @param opts
     * @param context
     */
    void JoinSessionCB(QStatus status, ajn::SessionId id, const ajn::SessionOpts& opts, void* context);

    void GetAllAboutData(ajn::AboutProxy& aboutProxy);

  private:
    void sessionJoinedCallback(qcc::String const& busName, ajn::SessionId id);

    void GetAboutData(ajn::AboutData& aboutData, const char* language);

    qcc::String m_Busname;

    ajn::BusAttachment* busAttment;

    bool mresetConnection;

    ajn::services::OBInfo oBInfo;
};

#endif /* ASYNCSESSIONJOINERCB_H_ */
