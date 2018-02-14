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

class RemoteCommunicatorI : public Test::RemoteCommunicator
{
public:

    RemoteCommunicatorI();

    virtual Test::RemoteObjectAdapterPrx createObjectAdapter(const std::string&, const std::string&, 
                                                             const Ice::Current&);
    virtual void deactivateObjectAdapter(const Test::RemoteObjectAdapterPrx&, const Ice::Current&);
    virtual void shutdown(const Ice::Current&);

private:

    int _nextPort;
};

class RemoteObjectAdapterI : public Test::RemoteObjectAdapter
{
public:

    RemoteObjectAdapterI(const Ice::ObjectAdapterPtr&);
    
    virtual Test::TestIntfPrx getTestIntf(const Ice::Current&);
    virtual void deactivate(const Ice::Current&);

private:

    const Ice::ObjectAdapterPtr _adapter;
    const Test::TestIntfPrx _testIntf;
};

class TestI : public Test::TestIntf
{
public:

    virtual std::string getAdapterName(const Ice::Current&);
};

#endif
