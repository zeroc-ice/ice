// Copyright (c) ZeroC, Inc.
    
#pragma once
    
#include "Ice/Locator.ice"
    
module Test
{
    interface TestLocatorRegistry extends ::Ice::LocatorRegistry
    {
        //
        // Allow remote addition of objects to the locator registry.
        //
        void addObject(Object* obj);
    }
<<<<<<< Updated upstream

=======
        
>>>>>>> Stashed changes
    interface TestLocator extends ::Ice::Locator
    {
        //
        // Returns the number of request on the locator interface.
        //
        ["cpp:const"] idempotent int getRequestCount();
    }
<<<<<<< Updated upstream

=======
        
>>>>>>> Stashed changes
    interface ServerManager
    {
        void startServer();
        void shutdown();
    }
<<<<<<< Updated upstream

=======
        
>>>>>>> Stashed changes
    interface Hello
    {
        void sayHello();
    }
<<<<<<< Updated upstream

    interface TestIntf
    {
        void shutdown();

        Hello* getHello();

        Hello* getReplicatedHello();

=======
        
    interface TestIntf
    {
        void shutdown();
            
        Hello* getHello();
            
        Hello* getReplicatedHello();
            
>>>>>>> Stashed changes
        void migrateHello();
    }
}
