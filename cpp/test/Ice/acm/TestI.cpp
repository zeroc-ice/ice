// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/IceUtil.h>
#include <Ice/Ice.h>
#include <TestI.h>

using namespace std;
using namespace Ice;
using namespace Test;

namespace
{

string
toString(int value)
{
    ostringstream os;
    os << value;
    return os.str();
}

class HeartbeatCallbackI :
#ifdef ICE_CPP11_MAPPING
                            public enable_shared_from_this<HeartbeatCallbackI>,
#else
                            public Ice::HeartbeatCallback,
#endif
                            private IceUtil::Monitor<IceUtil::Mutex>
{
public:

    void
    waitForCount(int count)
    {
        Lock sync(*this);
        _count = count;
        while(_count > 0)
        {
            wait();
        }
    }

    virtual void
    heartbeat(const Ice::ConnectionPtr&)
    {
        Lock sync(*this);
        --_count;
        notifyAll();
    }

private:

    int _count;
};
ICE_DEFINE_PTR(HeartbeatCallbackIPtr, HeartbeatCallbackI);

}

RemoteObjectAdapterPrxPtr
RemoteCommunicatorI::createObjectAdapter(int timeout, int close, int heartbeat, const Current& current)
{
    Ice::CommunicatorPtr com = current.adapter->getCommunicator();
    Ice::PropertiesPtr properties = com->getProperties();
    string protocol = properties->getPropertyWithDefault("Ice.Default.Protocol", "tcp");
    string opts;
    if(protocol != "bt")
    {
        opts = " -h \"" + properties->getPropertyWithDefault("Ice.Default.Host", "127.0.0.1") + "\"";
    }

    string name = generateUUID();
    if(timeout >= 0)
    {
        properties->setProperty(name + ".ACM.Timeout", toString(timeout));
    }
    if(close >= 0)
    {
        properties->setProperty(name + ".ACM.Close", toString(close));
    }
    if(heartbeat >= 0)
    {
        properties->setProperty(name + ".ACM.Heartbeat", toString(heartbeat));
    }
    properties->setProperty(name + ".ThreadPool.Size", "2");
    ObjectAdapterPtr adapter = com->createObjectAdapterWithEndpoints(name, protocol + opts);

    return ICE_UNCHECKED_CAST(RemoteObjectAdapterPrx, current.adapter->addWithUUID(
                              ICE_MAKE_SHARED(RemoteObjectAdapterI, adapter)));
}

void
RemoteCommunicatorI::shutdown(const Ice::Current& current)
{
    current.adapter->getCommunicator()->shutdown();
}

RemoteObjectAdapterI::RemoteObjectAdapterI(const Ice::ObjectAdapterPtr& adapter) :
    _adapter(adapter),
    _testIntf(ICE_UNCHECKED_CAST(TestIntfPrx, _adapter->add(ICE_MAKE_SHARED(TestI),
                                         stringToIdentity("test"))))
{
    _adapter->activate();
}

TestIntfPrxPtr
RemoteObjectAdapterI::getTestIntf(const Ice::Current&)
{
    return _testIntf;
}

void
RemoteObjectAdapterI::activate(const Ice::Current&)
{
    _adapter->activate();
}

void
RemoteObjectAdapterI::hold(const Ice::Current&)
{
    _adapter->hold();
}

void
RemoteObjectAdapterI::deactivate(const Ice::Current&)
{
    try
    {
        _adapter->destroy();
    }
    catch(const ObjectAdapterDeactivatedException&)
    {
    }
}

void
TestI::sleep(int delay, const Ice::Current& current)
{
    Lock sync(*this);
    timedWait(IceUtil::Time::seconds(delay));
}

void
TestI::sleepAndHold(int delay, const Ice::Current& current)
{
    Lock sync(*this);
    current.adapter->hold();
    timedWait(IceUtil::Time::seconds(delay));
}

void
TestI::interruptSleep(const Ice::Current& current)
{
    Lock sync(*this);
    notifyAll();
}

void
TestI::waitForHeartbeat(int count, const Ice::Current& current)
{
    HeartbeatCallbackIPtr callback = ICE_MAKE_SHARED(HeartbeatCallbackI);
#ifdef ICE_CPP11_MAPPING
    current.con->setHeartbeatCallback([callback](Ice::ConnectionPtr connection)
    {
        callback->heartbeat(move(connection));
    });
#else
    current.con->setHeartbeatCallback(callback);
#endif
    callback->waitForCount(count);
}
