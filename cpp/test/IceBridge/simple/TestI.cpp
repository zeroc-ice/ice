// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <TestI.h>
#include <TestCommon.h>

using namespace std;
using namespace Test;

namespace
{

class CloseCallbackI : public Ice::CloseCallback
{
public:

    CloseCallbackI(const MyClassIPtr& cl) : _cl(cl)
    {
    }

    void
    closed(const Ice::ConnectionPtr& connection)
    {
        _cl->removeConnection(connection);
    }

private:

    MyClassIPtr _cl;
};

class HeartbeatCallbackI : public Ice::HeartbeatCallback
{
public:

    HeartbeatCallbackI(const MyClassIPtr& cl) : _cl(cl)
    {
    }

    void
    heartbeat(const Ice::ConnectionPtr& connection)
    {
        _cl->incHeartbeatCount(connection);
    }

private:

    MyClassIPtr _cl;
};

class CallbackI : public IceUtil::Shared
{
public:

    CallbackI(const AMD_MyClass_callCallbackPtr& amdCB) : _amdCB(amdCB)
    {
    }

    void response()
    {
        _amdCB->ice_response();
    }

    void exception(const Ice::Exception& ex)
    {
        _amdCB->ice_exception(ex);
    }

private:

    const AMD_MyClass_callCallbackPtr _amdCB;
};

template<typename T> class GetCount : public IceUtil::Shared
{
public:

    GetCount(const T& amdCB) : _amdCB(amdCB)
    {
    }

    void response(int count)
    {
        _amdCB->ice_response(count);
    }

    void exception(const Ice::Exception& ex)
    {
        _amdCB->ice_exception(ex);
    }

private:

    const T _amdCB;
};
typedef GetCount<AMD_MyClass_getCallbackCountPtr> GetCallbackCount;
typedef GetCount<AMD_MyClass_getCallbackDatagramCountPtr> GetCallbackDatagramCount;

Ice::Identity callbackId = { "callback" , "" };

}

MyClassI::MyClassI() : _datagramCount(0), _counter(0)
{
}

void
MyClassI::callCallback_async(const AMD_MyClass_callCallbackPtr& amdCB, const Ice::Current& c)
{
    checkConnection(c.con);
    Callback_Callback_pingPtr cb = newCallback_Callback_ping(new CallbackI(amdCB),
                                                             &CallbackI::response,
                                                             &CallbackI::exception);
    Ice::uncheckedCast<CallbackPrx>(c.con->createProxy(callbackId))->begin_ping(cb);
}

void
MyClassI::getCallbackCount_async(const AMD_MyClass_getCallbackCountPtr& amdCB, const Ice::Current& c)
{
    checkConnection(c.con);
    Callback_Callback_getCountPtr cb = newCallback_Callback_getCount(new GetCallbackCount(amdCB),
                                                                     &GetCallbackCount::response,
                                                                     &GetCallbackCount::exception);
    Ice::uncheckedCast<CallbackPrx>(c.con->createProxy(callbackId))->begin_getCount(cb);
}

void
MyClassI::incCounter(int expected, const Ice::Current& c)
{
    checkConnection(c.con);

    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(_monitor);
    if(_counter + 1 != expected)
    {
        cout << _counter << " " << expected << endl;
    }
    test(++_counter == expected);
    _monitor.notifyAll();
}

void
MyClassI::waitCounter(int value, const Ice::Current& c)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(_monitor);
    while(_counter != value)
    {
        _monitor.wait();
    }
}

int
MyClassI::getConnectionCount(const Ice::Current& c)
{
    checkConnection(c.con);
    return static_cast<int>(_connections.size());
}

string
MyClassI::getConnectionInfo(const Ice::Current& c)
{
    checkConnection(c.con);
    return c.con->toString();
}

void
MyClassI::closeConnection(bool forceful, const Ice::Current& c)
{
    checkConnection(c.con);
    if(forceful)
    {
        c.con->close(Ice::ConnectionCloseForcefully);
    }
    else
    {
        c.con->close(Ice::ConnectionCloseGracefully);
    }
}

void
MyClassI::datagram(const Ice::Current& c)
{
    checkConnection(c.con);
    test(c.con->getEndpoint()->getInfo()->datagram());
    ++_datagramCount;
}

int
MyClassI::getDatagramCount(const Ice::Current& c)
{
    checkConnection(c.con);
    return _datagramCount;
}

void
MyClassI::callDatagramCallback(const Ice::Current& c)
{
    checkConnection(c.con);
    test(c.con->getEndpoint()->getInfo()->datagram());
    Ice::uncheckedCast<CallbackPrx>(c.con->createProxy(callbackId))->datagram();
}

void
MyClassI::getCallbackDatagramCount_async(const AMD_MyClass_getCallbackDatagramCountPtr& amdCB, const Ice::Current& c)
{
    checkConnection(c.con);
    Callback_Callback_getDatagramCountPtr cb = newCallback_Callback_getDatagramCount(new GetCallbackDatagramCount(amdCB),
                                                                                     &GetCallbackDatagramCount::response,
                                                                                     &GetCallbackDatagramCount::exception);
    Ice::uncheckedCast<CallbackPrx>(c.con->createProxy(callbackId))->begin_getDatagramCount(cb);
}

int
MyClassI::getHeartbeatCount(const Ice::Current& c)
{
    checkConnection(c.con);

    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(_monitor);
    return _connections[c.con];
}

void
MyClassI::enableHeartbeats(const Ice::Current& c)
{
    checkConnection(c.con);
    c.con->setACM(1, IceUtil::None, Ice::HeartbeatAlways);
}

void
MyClassI::shutdown(const Ice::Current& c)
{
    checkConnection(c.con);
    c.adapter->getCommunicator()->shutdown();
}

void
MyClassI::removeConnection(const Ice::ConnectionPtr& c)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(_monitor);
    _connections.erase(c);
}

void
MyClassI::incHeartbeatCount(const Ice::ConnectionPtr& c)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(_monitor);
    map<Ice::ConnectionPtr, int>::iterator p = _connections.find(c);
    if(p == _connections.end())
    {
        return;
    }
    ++p->second;
}

void
MyClassI::checkConnection(const Ice::ConnectionPtr& c)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(_monitor);
    if(_connections.find(c) == _connections.end())
    {
        _connections.insert(make_pair(c, 0));
        c->setCloseCallback(new CloseCallbackI(this));
        c->setHeartbeatCallback(new HeartbeatCallbackI(this));
    }
}
