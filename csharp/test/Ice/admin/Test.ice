// Copyright (c) ZeroC, Inc.
    
#ifndef TEST_ICE
#define TEST_ICE
    
#include "Ice/PropertyDict.ice"
    
["cs:namespace:Ice.admin"]
module Test
{
    interface RemoteCommunicator
    {
        Object* getAdmin();
<<<<<<< Updated upstream

        Ice::PropertyDict getChanges();

=======
            
        Ice::PropertyDict getChanges();
            
>>>>>>> Stashed changes
        //
        // Logger operations
        //
        void print(string message);
        void trace(string category, string message);
        void warning(string message);
        void error(string message);
<<<<<<< Updated upstream

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
