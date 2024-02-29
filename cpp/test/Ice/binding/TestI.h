//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef TEST_I_H
#define TEST_I_H

#include <Test.h>
#include <TestHelper.h>

class RemoteCommunicatorI final : public Test::RemoteCommunicator
{
public:

    RemoteCommunicatorI();

    Test::RemoteObjectAdapterPrxPtr createObjectAdapter(std::string, std::string, const Ice::Current&) final;
    void deactivateObjectAdapter(Test::RemoteObjectAdapterPrxPtr, const Ice::Current&) final;
    void shutdown(const Ice::Current&) final;

private:

    int _nextPort;
};

class RemoteObjectAdapterI final : public Test::RemoteObjectAdapter
{
public:

    RemoteObjectAdapterI(const Ice::ObjectAdapterPtr&);

    Test::TestIntfPrxPtr getTestIntf(const Ice::Current&) final;
    void deactivate(const Ice::Current&) final;

private:

    const Ice::ObjectAdapterPtr _adapter;
    const Test::TestIntfPrxPtr _testIntf;
};

class TestI final : public Test::TestIntf
{
public:

    std::string getAdapterName(const Ice::Current&) final;
};

#endif
