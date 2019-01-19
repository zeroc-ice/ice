//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef TEST_I_H
#define TEST_I_H

#include <Test.h>
#include <TestHelper.h>
#include <Ice/NativePropertiesAdmin.h>

class RemoteCommunicatorI : public virtual Test::RemoteCommunicator,
#ifndef ICE_CPP11_MAPPING
                            public virtual Ice::PropertiesAdminUpdateCallback,
#endif
                            public IceUtil::Monitor<IceUtil::Mutex>
{
public:

    RemoteCommunicatorI(const Ice::CommunicatorPtr&);

    virtual Ice::ObjectPrxPtr getAdmin(const Ice::Current&);
    virtual Ice::PropertyDict getChanges(const Ice::Current&);

    virtual void addUpdateCallback(const Ice::Current&);
    virtual void removeUpdateCallback(const Ice::Current&);

    virtual void print(ICE_IN(std::string), const Ice::Current&);
    virtual void trace(ICE_IN(std::string), ICE_IN(std::string), const Ice::Current&);
    virtual void warning(ICE_IN(std::string), const Ice::Current&);
    virtual void error(ICE_IN(std::string), const Ice::Current&);

    virtual void shutdown(const Ice::Current&);
    virtual void waitForShutdown(const Ice::Current&);
    virtual void destroy(const Ice::Current&);

    virtual void updated(const Ice::PropertyDict&);

private:

    Ice::CommunicatorPtr _communicator;
    Ice::PropertyDict _changes;

#ifdef ICE_CPP11_MAPPING
    std::function<void()> _removeCallback;
#else
    bool _hasCallback;
#endif
};
ICE_DEFINE_PTR(RemoteCommunicatorIPtr, RemoteCommunicatorI);

class RemoteCommunicatorFactoryI : public Test::RemoteCommunicatorFactory
{
public:

    virtual Test::RemoteCommunicatorPrxPtr createCommunicator(ICE_IN(Ice::PropertyDict), const Ice::Current&);
    virtual void shutdown(const Ice::Current&);
};

class TestFacetI : public Test::TestFacet
{
public:

    virtual void op(const Ice::Current&)
    {
    }
};

#endif
