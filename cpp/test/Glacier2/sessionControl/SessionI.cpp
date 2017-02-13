// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <SessionI.h>
#include <TestCommon.h>

using namespace std;
using namespace Test;

class DestroyCB : virtual public IceUtil::Shared
{
public:

    DestroyCB(const Test::AMD_Session_destroyFromClientPtr& cb) : _cb(cb)
    {
    }

    void 
    response()
    {
        _cb->ice_response();
    }

    void
    exception(const IceUtil::Exception&)
    {
        test(false);
    }

private:

    Test::AMD_Session_destroyFromClientPtr _cb;
};

typedef IceUtil::Handle<DestroyCB> DestroyCBPtr;

Glacier2::SessionPrx
SessionManagerI::create(const string& userId, const Glacier2::SessionControlPrx& sessionControl,
                        const Ice::Current& current)
{
    if(userId == "rejectme")
    {
        throw Glacier2::CannotCreateSessionException("");
    }
    if(userId == "localexception")
    {
        throw Ice::ObjectNotExistException(__FILE__, __LINE__);
    }
    return Glacier2::SessionPrx::uncheckedCast(current.adapter->addWithUUID(new SessionI(sessionControl)));
}

SessionI::SessionI(const Glacier2::SessionControlPrx& sessionControl) :
    _sessionControl(sessionControl)
{
    assert(sessionControl);
}

void
SessionI::destroyFromClient_async(const Test::AMD_Session_destroyFromClientPtr& cb, const Ice::Current&)
{
    DestroyCBPtr asyncCB = new DestroyCB(cb);
    Glacier2::Callback_SessionControl_destroyPtr amiCB = Glacier2::newCallback_SessionControl_destroy(asyncCB, 
                                                 &DestroyCB::response, 
                                                 &DestroyCB::exception);
    _sessionControl->begin_destroy(amiCB);
}

void
SessionI::shutdown(const Ice::Current& current)
{
    current.adapter->getCommunicator()->shutdown();
}

void
SessionI::destroy(const Ice::Current& current)
{
    current.adapter->remove(current.id);
}
