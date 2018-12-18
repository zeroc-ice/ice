// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#ifndef TEST_I_H
#define TEST_I_H

#include <Test.h>
#include <TestHelper.h>
#include <vector>
#include <ServerLocator.h>

class ServerManagerI : public Test::ServerManager
{
public:

    ServerManagerI(const ServerLocatorRegistryPtr&, const Ice::InitializationData&);

    virtual void startServer(const Ice::Current&);
    virtual void shutdown(const Ice::Current&);

private:

    std::vector<Ice::CommunicatorPtr> _communicators;
    ServerLocatorRegistryPtr _registry;
    Ice::InitializationData _initData;
    int _nextPort;
};

class HelloI : public Test::Hello
{
public:

    virtual void sayHello(const Ice::Current&);
};

class TestI : public Test::TestIntf
{
public:

    TestI(const Ice::ObjectAdapterPtr&, const Ice::ObjectAdapterPtr&, const ServerLocatorRegistryPtr&);

    virtual void shutdown(const Ice::Current&);
    virtual ::Test::HelloPrxPtr getHello(const Ice::Current&);
    virtual ::Test::HelloPrxPtr getReplicatedHello(const Ice::Current&);
    virtual void migrateHello(const Ice::Current&);

private:

    Ice::ObjectAdapterPtr _adapter1;
    Ice::ObjectAdapterPtr _adapter2;
    ServerLocatorRegistryPtr _registry;

};

#endif
