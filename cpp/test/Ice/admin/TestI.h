// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef TEST_I_H
#define TEST_I_H

#include <Test.h>
#include <Ice/NativePropertiesAdmin.h>

class RemoteCommunicatorI : virtual public Test::RemoteCommunicator,
                            virtual public Ice::PropertiesAdminUpdateCallback,
                            public IceUtil::Monitor<IceUtil::Mutex>
{
public:

    RemoteCommunicatorI(const Ice::CommunicatorPtr&);

    virtual Ice::ObjectPrx getAdmin(const Ice::Current&);
    virtual Ice::PropertyDict getChanges(const Ice::Current&);

    virtual void print(const std::string&, const Ice::Current&);
    virtual void trace(const std::string&, const std::string&, const Ice::Current&);
    virtual void warning(const std::string&, const Ice::Current&);
    virtual void error(const std::string&, const Ice::Current&);

    virtual void shutdown(const Ice::Current&);
    virtual void waitForShutdown(const Ice::Current&);
    virtual void destroy(const Ice::Current&);

    virtual void updated(const Ice::PropertyDict&);

private:

    Ice::CommunicatorPtr _communicator;
    Ice::PropertyDict _changes;
    bool _called;
};
typedef IceUtil::Handle<RemoteCommunicatorI> RemoteCommunicatorIPtr;

class RemoteCommunicatorFactoryI : public Test::RemoteCommunicatorFactory
{
public:

    virtual Test::RemoteCommunicatorPrx createCommunicator(const Ice::PropertyDict&, const Ice::Current&);
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
