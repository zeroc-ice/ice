//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef TEST_I_H
#define TEST_I_H

#include <Test.h>
#include <TestHelper.h>
#include <vector>
#include <ServerLocator.h>

class ServerManagerI final : public Test::ServerManager
{
public:
    ServerManagerI(const ServerLocatorRegistryPtr&, const Ice::InitializationData&);

    void startServer(const Ice::Current&) final;
    void shutdown(const Ice::Current&) final;

private:
    std::vector<Ice::CommunicatorPtr> _communicators;
    ServerLocatorRegistryPtr _registry;
    Ice::InitializationData _initData;
    int _nextPort;
};

class HelloI final : public Test::Hello
{
public:
    void sayHello(const Ice::Current&) final;
};

class TestI final : public Test::TestIntf
{
public:
    TestI(const Ice::ObjectAdapterPtr&, const Ice::ObjectAdapterPtr&, const ServerLocatorRegistryPtr&);

    void shutdown(const Ice::Current&) final;
    std::optional<Test::HelloPrx> getHello(const Ice::Current&) final;
    std::optional<Test::HelloPrx> getReplicatedHello(const Ice::Current&) final;
    void migrateHello(const Ice::Current&) final;

private:
    Ice::ObjectAdapterPtr _adapter1;
    Ice::ObjectAdapterPtr _adapter2;
    ServerLocatorRegistryPtr _registry;
};

#endif
