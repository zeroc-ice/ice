//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

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
}

interface ServerManager
{
    void startServer();
    void shutdown();
}

interface Hello
{
    void sayHello();
}

interface TestIntf
{
    void shutdown();

    Hello* getHello();

    void migrateHello();
}

}
