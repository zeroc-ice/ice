//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include "IceUtil/IceUtil.h"
#include "Ice/Ice.h"
#include <TestI.h>

using namespace std;
using namespace Ice;
using namespace Test;

namespace
{
    string toString(int value)
    {
        ostringstream os;
        os << value;
        return os.str();
    }
}

optional<RemoteObjectAdapterPrx>
RemoteCommunicatorI::createObjectAdapter(int timeout, int close, int heartbeat, const Current& current)
{
    Ice::CommunicatorPtr com = current.adapter->getCommunicator();
    Ice::PropertiesPtr properties = com->getProperties();
    string protocol = properties->getPropertyWithDefault("Ice.Default.Protocol", "tcp");
    string opts;
    if (protocol != "bt")
    {
        opts = " -h \"" + properties->getPropertyWithDefault("Ice.Default.Host", "127.0.0.1") + "\"";
    }

    string name = generateUUID();
    if (timeout >= 0)
    {
        properties->setProperty(name + ".ACM.Timeout", toString(timeout));
    }
    if (close >= 0)
    {
        properties->setProperty(name + ".ACM.Close", toString(close));
    }
    if (heartbeat >= 0)
    {
        properties->setProperty(name + ".ACM.Heartbeat", toString(heartbeat));
    }
    properties->setProperty(name + ".ThreadPool.Size", "2");
    ObjectAdapterPtr adapter = com->createObjectAdapterWithEndpoints(name, protocol + opts);

    return RemoteObjectAdapterPrx(current.adapter->addWithUUID(make_shared<RemoteObjectAdapterI>(adapter)));
}

void
RemoteCommunicatorI::shutdown(const Ice::Current& current)
{
    current.adapter->getCommunicator()->shutdown();
}

RemoteObjectAdapterI::RemoteObjectAdapterI(const Ice::ObjectAdapterPtr& adapter)
    : _adapter(adapter),
      _testIntf(TestIntfPrx(_adapter->add(make_shared<TestI>(), stringToIdentity("test"))))
{
    _adapter->activate();
}

optional<TestIntfPrx>
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
    catch (const ObjectAdapterDeactivatedException&)
    {
    }
}

void
TestI::sleep(int delay, const Ice::Current&)
{
    unique_lock lock(_mutex);
    _condition.wait_for(lock, chrono::seconds(delay));
}

void
TestI::sleepAndHold(int delay, const Ice::Current& current)
{
    unique_lock lock(_mutex);
    current.adapter->hold();
    _condition.wait_for(lock, chrono::seconds(delay));
}

void
TestI::interruptSleep(const Ice::Current&)
{
    lock_guard lock(_mutex);
    _condition.notify_all();
}

void
TestI::startHeartbeatCount(const Ice::Current& current)
{
    _callback = make_shared<HeartbeatCallbackI>();
    HeartbeatCallbackIPtr callback = _callback;
    current.con->setHeartbeatCallback([callback](Ice::ConnectionPtr connection)
                                      { callback->heartbeat(std::move(connection)); });
}

void
TestI::waitForHeartbeatCount(int count, const Ice::Current&)
{
    assert(_callback);
    _callback->waitForCount(count);
}
