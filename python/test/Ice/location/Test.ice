// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#pragma once

#include <Ice/Locator.ice>

module Test
{

interface TestLocatorRegistry extends ::Ice::LocatorRegistry
{
    //
    // Allow remote addition of objects to the locator registry.
    //
    void addObject(Object* obj);
};

interface TestLocator extends ::Ice::Locator
{
    //
    // Returns the number of request on the locator interface.
    //
    ["cpp:const"] idempotent int getRequestCount();
};

interface ServerManager
{
    void startServer();
    void shutdown();
};

interface Hello
{
    void sayHello();
};

interface TestIntf
{
    void shutdown();

    Hello* getHello();

    Hello* getReplicatedHello();

    void migrateHello();
};

};
