// Copyright (c) ZeroC, Inc.
#pragma once
    
#include "Ice/Context.ice"
    
module Test
{
    interface TestIntf
    {
        void shutdown();
<<<<<<< Updated upstream

        Ice::Context getEndpointInfoAsContext();

=======
            
        Ice::Context getEndpointInfoAsContext();
            
>>>>>>> Stashed changes
        Ice::Context getConnectionInfoAsContext();
    }
}
