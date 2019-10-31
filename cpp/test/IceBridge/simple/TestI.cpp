//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Ice.h>
#include <TestI.h>
#include <TestHelper.h>

using namespace std;
using namespace Test;

namespace
{

Ice::Identity callbackId = { "callback" , "" };

}

void
MyClassI::callCallbackAsync(function<void()> response,
                            function<void(exception_ptr)> error,
                            const Ice::Current& current)
{
    checkConnection(current.con);
    auto prx = Ice::uncheckedCast<CallbackPrx>(current.con->createProxy(callbackId));

    prx->pingAsync([response = move(response)](){ response(); },
                   [error = move(error)](exception_ptr e){ error(e); });
}

void
MyClassI::getCallbackCountAsync(function<void(int)> response,
                                function<void(exception_ptr)> error,
                                const Ice::Current& current)
{
    checkConnection(current.con);
    auto prx = Ice::uncheckedCast<CallbackPrx>(current.con->createProxy(callbackId));

    prx->getCountAsync([response = move(response)](int count){ response(count); },
                       [error = move(error)](exception_ptr e){ error(e); });
}

void
MyClassI::incCounter(int expected, const Ice::Current& current)
{
    checkConnection(current.con);

    {
        lock_guard<mutex> lg(_lock);
        if(_counter + 1 != expected)
        {
            cout << _counter << " " << expected << endl;
        }
        test(++_counter == expected);
    }
    _condVar.notify_all();
}

void
MyClassI::waitCounter(int value, const Ice::Current&)
{
    unique_lock<mutex> lock(_lock);
    while(_counter != value)
    {
        _condVar.wait(lock);
    }
}

int
MyClassI::getConnectionCount(const Ice::Current& current)
{
    checkConnection(current.con);
    return static_cast<int>(_connections.size());
}

string
MyClassI::getConnectionInfo(const Ice::Current& current)
{
    checkConnection(current.con);
    return current.con->toString();
}

void
MyClassI::closeConnection(bool forceful, const Ice::Current& current)
{
    checkConnection(current.con);
    if(forceful)
    {
        current.con->close(Ice::ConnectionClose::Forcefully);
    }
    else
    {
        current.con->close(Ice::ConnectionClose::Gracefully);
    }
}

void
MyClassI::datagram(const Ice::Current& current)
{
    checkConnection(current.con);
    test(current.con->getEndpoint()->getInfo()->datagram());
    ++_datagramCount;
}

int
MyClassI::getDatagramCount(const Ice::Current& current)
{
    checkConnection(current.con);
    return _datagramCount;
}

void
MyClassI::callDatagramCallback(const Ice::Current& current)
{
    checkConnection(current.con);
    test(current.con->getEndpoint()->getInfo()->datagram());
    Ice::uncheckedCast<CallbackPrx>(current.con->createProxy(callbackId))->datagram();
}

void
MyClassI::getCallbackDatagramCountAsync(function<void(int)> response,
                                        function<void(exception_ptr)> error,
                                        const Ice::Current& current)
{
    checkConnection(current.con);
    auto prx = Ice::uncheckedCast<CallbackPrx>(current.con->createProxy(callbackId));

    prx->getDatagramCountAsync([response = move(response)](int count){ response(count); },
                               [error = move(error)](auto e){ error(e); });
}

int
MyClassI::getHeartbeatCount(const Ice::Current& current)
{
    checkConnection(current.con);

    lock_guard<mutex> lg(_lock);
    return _connections[current.con];
}

void
MyClassI::enableHeartbeats(const Ice::Current& current)
{
    checkConnection(current.con);
    current.con->setACM(1, Ice::nullopt, Ice::ACMHeartbeat::HeartbeatAlways);
}

void
MyClassI::shutdown(const Ice::Current& current)
{
    checkConnection(current.con);
    current.adapter->getCommunicator()->shutdown();
}

void
MyClassI::removeConnection(const shared_ptr<Ice::Connection>& con)
{
    lock_guard<mutex> lg(_lock);
    _connections.erase(con);
}

void
MyClassI::incHeartbeatCount(const shared_ptr<Ice::Connection>& con)
{
    lock_guard<mutex> lg(_lock);
    auto p = _connections.find(con);
    if(p == _connections.end())
    {
        return;
    }
    ++p->second;
}

void
MyClassI::checkConnection(const shared_ptr<Ice::Connection>& con)
{
    lock_guard<mutex> lg(_lock);
    if(_connections.find(con) == _connections.end())
    {
        _connections.insert(make_pair(con, 0));
        con->setCloseCallback([self = shared_from_this()](const auto& c){ self->removeConnection(c); });
        con->setHeartbeatCallback([self = shared_from_this()](const auto& c){ self->incHeartbeatCount(c); });
    }
}
