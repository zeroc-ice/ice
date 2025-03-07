// Copyright (c) ZeroC, Inc.
    
#pragma once
    
#include "Glacier2/Session.ice"
    
module Test
{
    interface Session extends Glacier2::Session
    {
        ["amd"] void destroyFromClient();
<<<<<<< Updated upstream

=======
            
>>>>>>> Stashed changes
        void shutdown();
    }
}
