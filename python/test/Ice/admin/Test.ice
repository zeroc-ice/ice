// Copyright (c) ZeroC, Inc.
    
#ifndef TEST_ICE
#define TEST_ICE
    
#include "Ice/PropertyDict.ice"
    
module Test
{
    interface RemoteCommunicator
    {
        Object* getAdmin();
<<<<<<< Updated upstream

        Ice::PropertyDict getChanges();

        void shutdown();

        void waitForShutdown();

        void destroy();
    }

    interface RemoteCommunicatorFactory
    {
        RemoteCommunicator* createCommunicator(Ice::PropertyDict props);

        void shutdown();
    }

=======
            
        Ice::PropertyDict getChanges();
            
        void shutdown();
            
        void waitForShutdown();
            
        void destroy();
    }
        
    interface RemoteCommunicatorFactory
    {
        RemoteCommunicator* createCommunicator(Ice::PropertyDict props);
            
        void shutdown();
    }
        
>>>>>>> Stashed changes
    interface TestFacet
    {
        void op();
    }
}
<<<<<<< Updated upstream

=======
    
>>>>>>> Stashed changes
#endif
