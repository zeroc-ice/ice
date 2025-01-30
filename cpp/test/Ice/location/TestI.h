// Copyright (c) ZeroC, Inc.

#ifndef TEST_I_H
#define TEST_I_H

#include "ServerLocator.h"
#include "Test.h"

#include <vector>

class ServerManagerI final : public Test::ServerManager
{
public:
    ServerManagerI(ServerLocatorRegistryPtr, Ice::InitializationData);

    void startServer(const Ice::Current&) final;
    void shutdown(const Ice::Current&) final;

private:
    std::vector<Ice::CommunicatorPtr> _communicators;
    ServerLocatorRegistryPtr _registry;
    Ice::InitializationData _initData;
    int _nextPort{1};
};

class HelloI final : public Test::Hello
{
public:
    void sayHello(const Ice::Current&) final;
};

class TestI final : public Test::TestIntf
{
public:
    TestI(Ice::ObjectAdapterPtr, Ice::ObjectAdapterPtr, ServerLocatorRegistryPtr);

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
