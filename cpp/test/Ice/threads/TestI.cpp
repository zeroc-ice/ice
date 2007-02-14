// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <TestI.h>
#include <TestCommon.h>
#include <IceUtil/StaticMutex.h>

using namespace std;

static bool
idsEqual(IceUtil::ThreadControl::ID id1, IceUtil::ThreadControl::ID id2)
{
#ifdef _WIN32
    return id1 == id2;
#else
    return pthread_equal(id1, id2) != 0;
#endif
}

ThreadHookI::ThreadHookI() :
    _nextThreadId(0)
{
}

void
ThreadHookI::start()
{
    IceUtil::Mutex::Lock lock(*this);
    test(getThreadNumInternal() == -1);
    IceUtil::ThreadControl::ID id = IceUtil::ThreadControl().id();
    _threads.push_back(ThreadList::value_type(id, _nextThreadId));
    _nextThreadId++;
}

void
ThreadHookI::stop()
{
    IceUtil::Mutex::Lock lock(*this);
    IceUtil::ThreadControl::ID id = IceUtil::ThreadControl().id();
    for(ThreadList::iterator p = _threads.begin(); p != _threads.end(); ++p)
    {
        if(idsEqual(p->first, id))
        {
            _threads.erase(p);
            return;
        }
    }
    test(false);
}

Ice::Int
ThreadHookI::getThreadNum() const
{
    IceUtil::Mutex::Lock lock(*this);
    return getThreadNumInternal();
}

int
ThreadHookI::activeThreads() const
{
    IceUtil::Mutex::Lock lock(*this);
    return static_cast<int>(_threads.size());
}

Ice::Int
ThreadHookI::getThreadNumInternal() const
{
    IceUtil::ThreadControl::ID id = IceUtil::ThreadControl().id();

    for(ThreadList::const_iterator p = _threads.begin(); p != _threads.end(); ++p)
    {
        if(idsEqual(p->first, id))
        {
            return p->second;
        }
    }

    return -1;
}

AdapterI::AdapterI(const ThreadHookIPtr& threadHook, bool threadPerConnection) :
    _threadHook(threadHook),
    _threadPerConnection(threadPerConnection)
{
}

bool
AdapterI::isThreadPerConnection(const Ice::Current&)
{
    return _threadPerConnection;
}

Ice::Int
AdapterI::getThreadId(const Ice::Current&)
{
    Ice::Int tid = _threadHook->getThreadNum();
    test(tid != -1);
    return tid;
}

void
AdapterI::reset(const Ice::Current&)
{
    _waiting = false;
    _notified = false;
}

Ice::Int
AdapterI::waitForWakeup(Ice::Int timeout, bool& notified, const Ice::Current&)
{
    Ice::Int tid = _threadHook->getThreadNum();
    test(tid != -1);

    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);

    _waiting = true;
    notifyAll();
    test(!_notified);
    timedWait(IceUtil::Time::milliSeconds(timeout));
    notified = _notified;

    return tid;
}

Ice::Int
AdapterI::wakeup(const Ice::Current&)
{
    Ice::Int tid = _threadHook->getThreadNum();
    test(tid != -1);

    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);

    while(!_waiting)
    {
        wait();
    }
    _notified = true;
    notifyAll();

    return tid;
}

void
AdapterI::callSelf(const Test::AdapterPrx& proxy, const Ice::Current&)
{
    //
    // Make two invocations using different thread-per-connection settings.
    // In both cases, the invocations must be collocated.
    //

    proxy->ice_ping();
    try
    {
        //
        // A collocated invocation must not have a connection.
        //
        proxy->ice_getConnection();
        test(false);
    }
    catch(const Ice::CollocationOptimizationException&)
    {
        // Expected.
    }

    Test::AdapterPrx proxy2 =
        Test::AdapterPrx::uncheckedCast(proxy->ice_threadPerConnection(!proxy->ice_isThreadPerConnection()));
    proxy2->ice_ping();
    try
    {
        //
        // A collocated invocation must not have a connection.
        //
        proxy2->ice_getConnection();
        test(false);
    }
    catch(const Ice::CollocationOptimizationException&)
    {
        // Expected.
    }
}

ServerI::ServerI(const Ice::CommunicatorPtr& communicator)
{
    _hook = new ThreadHookI;

    Ice::CommunicatorPtr comm;
    Ice::ObjectAdapterPtr adapter;
    Ice::InitializationData id;
    Test::AdapterPrx proxy;

    id.threadHook = _hook;

    Ice::Identity ident;

    //
    // Create an adapter that uses the communicator's thread pool.
    //
    id.properties = communicator->getProperties()->clone();
    id.properties->setProperty("Ice.ThreadPerConnection", "0");
    id.properties->setProperty("Ice.ThreadPool.Server.SizeMax", "2");
    id.properties->setProperty("Ice.ThreadPool.Server.SizeWarn", "0");
    comm = Ice::initialize(id);
    adapter = comm->createObjectAdapterWithEndpoints("Adapter1", "default");
    ident = comm->stringToIdentity("adapter1");
    proxy = Test::AdapterPrx::uncheckedCast(adapter->add(new AdapterI(_hook, false), ident));
    adapter->activate();
    _adapters.push_back(proxy);

    //
    // Create an adapter that uses the communicator's default setting for thread-per-connection.
    //
    id.properties = communicator->getProperties()->clone();
    id.properties->setProperty("Ice.ThreadPerConnection", "1");
    id.properties->setProperty("Ice.ThreadPool.Server.SizeMax", "0");
    id.properties->setProperty("Ice.ThreadPool.Server.SizeWarn", "0");
    comm = Ice::initialize(id);
    adapter = comm->createObjectAdapterWithEndpoints("Adapter2", "default");
    ident = comm->stringToIdentity("adapter2");
    proxy = Test::AdapterPrx::uncheckedCast(adapter->add(new AdapterI(_hook, true), ident));
    adapter->activate();
    _adapters.push_back(proxy);

    //
    // Create an adapter that uses a private thread pool.
    //
    id.properties = communicator->getProperties()->clone();
    id.properties->setProperty("Ice.ThreadPerConnection", "1");
    id.properties->setProperty("Adapter3.ThreadPool.SizeMax", "2");
    id.properties->setProperty("Adapter3.ThreadPool.SizeWarn", "0");
    comm = Ice::initialize(id);
    adapter = comm->createObjectAdapterWithEndpoints("Adapter3", "default");
    ident = comm->stringToIdentity("adapter3");
    proxy = Test::AdapterPrx::uncheckedCast(adapter->add(new AdapterI(_hook, false), ident));
    adapter->activate();
    _adapters.push_back(proxy);

    //
    // Create an adapter that uses an explicit setting for thread-per-connection.
    //
    id.properties = communicator->getProperties()->clone();
    id.properties->setProperty("Ice.ThreadPerConnection", "0");
    id.properties->setProperty("Ice.ThreadPool.Server.SizeMax", "2");
    id.properties->setProperty("Ice.ThreadPool.Server.SizeWarn", "0");
    id.properties->setProperty("Adapter4.ThreadPerConnection", "1");
    comm = Ice::initialize(id);
    adapter = comm->createObjectAdapterWithEndpoints("Adapter4", "default");
    ident = comm->stringToIdentity("adapter4");
    proxy = Test::AdapterPrx::uncheckedCast(adapter->add(new AdapterI(_hook, true), ident));
    adapter->activate();
    _adapters.push_back(proxy);
}

Test::AdapterSeq
ServerI::getAdapters(const Ice::Current&)
{
    return _adapters;
}

void
ServerI::ping(const Ice::Current&)
{
}

void
ServerI::shutdown(const Ice::Current& current)
{
    for(Test::AdapterSeq::iterator p = _adapters.begin(); p != _adapters.end(); ++p)
    {
        (*p)->ice_getCommunicator()->destroy();
    }
    test(_hook->activeThreads() == 0);
    current.adapter->getCommunicator()->shutdown();
}
