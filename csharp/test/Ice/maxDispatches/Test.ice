// Copyright (c) ZeroC, Inc.
    
#pragma once
    
["cs:namespace:Ice.maxDispatches"]
module Test
{
    interface TestIntf
    {
        ["amd"] void op();
        int resetMaxConcurrentDispatches();
        void shutdown();
    }
        
    interface Responder
    {
        void start();
        void stop();
        int pendingResponseCount();
    }
}
