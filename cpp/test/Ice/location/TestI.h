// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef TEST_I_H
#define TEST_I_H

#include <Test.h>
#include <vector>

class ServerManagerI : public Test::ServerManager
{
public:
    ServerManagerI(const Ice::ObjectAdapterPtr&);
    
    virtual void startServer(const Ice::Current&);
    virtual void shutdown(const Ice::Current&);

private:

    Ice::ObjectAdapterPtr _adapter;
    std::vector<Ice::CommunicatorPtr> _communicators;
};

class HelloI : public Test::Hello
{
public:

    virtual void sayHello(const Ice::Current&);
};

class TestI : public Test::TestIntf
{
public:

    TestI(const Ice::ObjectAdapterPtr&);

    virtual void shutdown(const Ice::Current&);
    virtual ::Test::HelloPrx getHello(const Ice::Current&);

private:

    Ice::ObjectAdapterPtr _adapter;
};

#endif
