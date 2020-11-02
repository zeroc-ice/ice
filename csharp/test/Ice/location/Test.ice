//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

#include <Ice/Locator.ice>

[[suppress-warning(reserved-identifier)]]

module ZeroC::Ice::Test::Location
{

interface TestLocatorRegistry : ::Ice::LocatorRegistry
{
    // Allow remote addition of objects to the locator registry.
    void addObject(Object obj);
}

interface TestLocator : ::Ice::Locator
{
    //
    // Returns the number of request on the locator interface.
    //
    idempotent int getRequestCount();
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
