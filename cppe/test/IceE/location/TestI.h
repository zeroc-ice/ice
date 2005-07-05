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

#include <IceE/Config.h>

#ifndef ICE_NO_LOCATOR

#include <Test.h>
#include <vector>
#include <ServerLocator.h>

class ServerManagerI : public Test::ServerManager
{
public:

    ServerManagerI(const IceE::ObjectAdapterPtr&, const ServerLocatorRegistryPtr&);
    
    virtual void startServer(const IceE::Current&);
    virtual void shutdown(const IceE::Current&);

private:

    IceE::ObjectAdapterPtr _adapter;
    std::vector<IceE::CommunicatorPtr> _communicators;
    ServerLocatorRegistryPtr _registry;
};

class HelloI : public Test::Hello
{
public:

    virtual void sayHello(const IceE::Current&);
};

class TestI : public Test::TestIntf
{
public:

    TestI(const IceE::ObjectAdapterPtr&, const IceE::ObjectAdapterPtr&, const ServerLocatorRegistryPtr&);

    virtual void shutdown(const IceE::Current&);
    virtual ::Test::HelloPrx getHello(const IceE::Current&);
    virtual void migrateHello(const IceE::Current&);

private:

    IceE::ObjectAdapterPtr _adapter1;
    IceE::ObjectAdapterPtr _adapter2;
    ServerLocatorRegistryPtr _registry;

};

#endif

#endif
