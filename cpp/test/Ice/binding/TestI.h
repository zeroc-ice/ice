// Copyright (c) ZeroC, Inc.

#ifndef TEST_I_H
#define TEST_I_H

#include "Test.h"
#include "TestHelper.h"

class RemoteCommunicatorI final : public Test::RemoteCommunicator
{
public:
    RemoteCommunicatorI();

    std::optional<Test::RemoteObjectAdapterPrx>
    createObjectAdapter(std::string, std::string, const Ice::Current&) final;
    void deactivateObjectAdapter(std::optional<Test::RemoteObjectAdapterPrx>, const Ice::Current&) final;
    void shutdown(const Ice::Current&) final;

private:
    int _nextPort{1};
};

class RemoteObjectAdapterI final : public Test::RemoteObjectAdapter
{
public:
    RemoteObjectAdapterI(Ice::ObjectAdapterPtr);

    std::optional<Test::TestIntfPrx> getTestIntf(const Ice::Current&) final;
    void deactivate(const Ice::Current&) final;

private:
    const Ice::ObjectAdapterPtr _adapter;
    const Test::TestIntfPrx _testIntf;
};

class TestI final : public Test::TestIntf
{
public:
    std::string getAdapterName(const Ice::Current&) final;
};

#endif
