//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

#include <Ice/Locator.ice>

["objc:prefix:TestLocation"]
module Test
{

interface TestLocatorRegistry extends ::Ice::LocatorRegistry
{
    //
    // Allow remote addition of objects to the locator registry.
    //
    void addObject(Object* obj);
}

interface TestLocator extends ::Ice::Locator
{
    //
    // Returns the number of request on the locator interface.
    //
    ["cpp:const"] idempotent int getRequestCount();
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

    Hello* getReplicatedHello();

    void migrateHello();
}

}
