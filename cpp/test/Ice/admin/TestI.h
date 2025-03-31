// Copyright (c) ZeroC, Inc.

#ifndef TEST_I_H
#define TEST_I_H

#include "Test.h"
#include "TestHelper.h"

class RemoteCommunicatorI final : public Test::RemoteCommunicator
{
public:
    RemoteCommunicatorI(Ice::CommunicatorPtr);

    std::optional<Ice::ObjectPrx> getAdmin(const Ice::Current&) final;
    Ice::PropertyDict getChanges(const Ice::Current&) final;

    void addUpdateCallback();
    void addUpdateCallback(const Ice::Current&) final;
    void removeUpdateCallback(const Ice::Current&) final;

    void print(std::string, const Ice::Current&) final;
    void trace(std::string, std::string, const Ice::Current&) final;
    void warning(std::string, const Ice::Current&) final;
    void error(std::string, const Ice::Current&) final;

    void shutdown(const Ice::Current&) final;
    void waitForShutdown(const Ice::Current&) final;
    void destroy(const Ice::Current&) final;

    void updated(const Ice::PropertyDict&);

private:
    Ice::CommunicatorPtr _communicator;
    Ice::PropertyDict _changes;

    std::function<void()> _removeCallback;
    std::mutex _mutex;
};
using RemoteCommunicatorIPtr = std::shared_ptr<RemoteCommunicatorI>;

class RemoteCommunicatorFactoryI final : public Test::RemoteCommunicatorFactory
{
public:
    std::optional<Test::RemoteCommunicatorPrx> createCommunicator(Ice::PropertyDict, const Ice::Current&) final;
    void shutdown(const Ice::Current&) final;
};

class TestFacetI final : public Test::TestFacet
{
public:
    void op(const Ice::Current&) final {}
};

#endif
