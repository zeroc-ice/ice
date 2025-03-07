// Copyright (c) ZeroC, Inc.
    
#ifndef TEST_ICE
#define TEST_ICE
    
#include "Ice/PropertyDict.ice"
    
module Test
{
    interface TestIntf
    {
        void sleep(int ms);
    }
<<<<<<< Updated upstream

    interface RemoteCommunicator
    {
        TestIntf* getObject();

        int getThreadStartCount();
        int getThreadStopCount();

        void destroy();
    }

    interface RemoteCommunicatorFactory
    {
        RemoteCommunicator* createCommunicator(Ice::PropertyDict props);

        void shutdown();
    }
}

=======
        
    interface RemoteCommunicator
    {
        TestIntf* getObject();
            
        int getThreadStartCount();
        int getThreadStopCount();
            
        void destroy();
    }
        
    interface RemoteCommunicatorFactory
    {
        RemoteCommunicator* createCommunicator(Ice::PropertyDict props);
            
        void shutdown();
    }
}
    
>>>>>>> Stashed changes
#endif
