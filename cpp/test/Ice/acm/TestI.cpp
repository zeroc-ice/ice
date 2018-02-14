// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
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

class ConnectionCallbackI : public Ice::ConnectionCallback, private IceUtil::Monitor<IceUtil::Mutex>
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

private:

    virtual void
    heartbeat(const Ice::ConnectionPtr&)
    {
        Lock sync(*this);
        --_count;
        notifyAll();
    }

    virtual void
    closed(const Ice::ConnectionPtr&)
    {
    }

    int _count;
};
typedef IceUtil::Handle<ConnectionCallbackI> ConnectionCallbackIPtr;

}

RemoteObjectAdapterPrx
RemoteCommunicatorI::createObjectAdapter(int timeout, int close, int heartbeat, const Current& current)
{
    Ice::CommunicatorPtr com = current.adapter->getCommunicator();
    Ice::PropertiesPtr properties = com->getProperties();
    string protocol = properties->getPropertyWithDefault("Ice.Default.Protocol", "tcp");
    string host = properties->getPropertyWithDefault("Ice.Default.Host", "127.0.0.1");

    string name = IceUtil::generateUUID();
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
    ObjectAdapterPtr adapter = com->createObjectAdapterWithEndpoints(name, protocol + " -h \"" + host + "\"");
    return RemoteObjectAdapterPrx::uncheckedCast(current.adapter->addWithUUID(new RemoteObjectAdapterI(adapter)));
}

void
RemoteCommunicatorI::shutdown(const Ice::Current& current)
{
    current.adapter->getCommunicator()->shutdown();
}

RemoteObjectAdapterI::RemoteObjectAdapterI(const Ice::ObjectAdapterPtr& adapter) :
    _adapter(adapter),
    _testIntf(TestIntfPrx::uncheckedCast(_adapter->add(new TestI(),
                                         adapter->getCommunicator()->stringToIdentity("test"))))
{
    _adapter->activate();
}

TestIntfPrx
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
    ConnectionCallbackIPtr callback = new ConnectionCallbackI();
    current.con->setCallback(callback);
    callback->waitForCount(count);
}
