// Copyright (c) ZeroC, Inc.
    
#pragma once
    
module Test
{
    interface Echo
    {
        void setConnection();
        void startBatch();
        void flushBatch();
        void shutdown();
        bool supportsCompress();
    }
<<<<<<< Updated upstream

=======
        
>>>>>>> Stashed changes
    interface MyObject
    {
        string getName();
        void shutdown();
    }
}
