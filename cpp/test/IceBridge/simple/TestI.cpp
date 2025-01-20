// Copyright (c) ZeroC, Inc.

#include "TestI.h"
#include "Ice/Ice.h"
#include "TestHelper.h"

using namespace std;
using namespace Test;

namespace
{
    Ice::Identity callbackId{"callback", ""}; // NOLINT(cert-err58-cpp)
}

void
MyClassI::callCallbackAsync(function<void()> response, function<void(exception_ptr)> error, const Ice::Current& current)
{
    checkConnection(current.con);
    auto prx = current.con->createProxy<CallbackPrx>(callbackId);

    prx->pingAsync(
        [response = std::move(response)]() { response(); },
        [error = std::move(error)](exception_ptr e) { error(e); });
}

void
MyClassI::getCallbackCountAsync(
    function<void(int)> response,
    function<void(exception_ptr)> error,
    const Ice::Current& current)
{
    checkConnection(current.con);
    auto prx = current.con->createProxy<CallbackPrx>(callbackId);

    prx->getCountAsync(
        [response = std::move(response)](int count) { response(count); },
        [error = std::move(error)](exception_ptr e) { error(e); });
}

void
MyClassI::incCounter(int expected, const Ice::Current& current)
{
    checkConnection(current.con);

    {
        lock_guard<mutex> lg(_lock);
        if (_counter + 1 != expected)
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
    while (_counter != value)
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
    if (forceful)
    {
        current.con->abort();
    }
    else
    {
        current.con->close(nullptr, nullptr);
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
    current.con->createProxy<CallbackPrx>(callbackId)->datagram();
}

void
MyClassI::getCallbackDatagramCountAsync(
    function<void(int)> response,
    function<void(exception_ptr)> error,
    const Ice::Current& current)
{
    checkConnection(current.con);
    auto prx = current.con->createProxy<CallbackPrx>(callbackId);

    prx->getDatagramCountAsync(
        [response = std::move(response)](int count) { response(count); },
        [error = std::move(error)](auto e) { error(e); });
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
MyClassI::checkConnection(const shared_ptr<Ice::Connection>& con)
{
    lock_guard<mutex> lg(_lock);
    if (_connections.find(con) == _connections.end())
    {
        _connections.insert(make_pair(con, 0));
        con->setCloseCallback([self = shared_from_this()](const auto& c) { self->removeConnection(c); });
    }
}
