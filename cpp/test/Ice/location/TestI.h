// **********************************************************************
//
// Copyright (c) 2002
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

#ifndef TEST_I_H
#define TEST_I_H

#include <Test.h>

class ServerManagerI : public ServerManager
{
public:
    ServerManagerI(const Ice::ObjectAdapterPtr&);
    
    virtual void startServer(const Ice::Current&);
    virtual void shutdown(const Ice::Current&);

private:

    Ice::ObjectAdapterPtr _adapter;
    Ice::CommunicatorPtr _serverCommunicator;

};

class HelloI : public Hello
{
public:

    virtual void sayHello(const Ice::Current&);
};

class TestI : public Test
{
public:

    TestI(const Ice::ObjectAdapterPtr&);

    virtual void shutdown(const Ice::Current&);
    virtual HelloPrx getHello(const Ice::Current&);

private:

    Ice::ObjectAdapterPtr _adapter;
};

#endif
