// Copyright (c) ZeroC, Inc.
    
#pragma once
    
module Test
{
    interface Empty
    {
    }
<<<<<<< Updated upstream

=======
        
>>>>>>> Stashed changes
    interface A
    {
        string callA();
    }
<<<<<<< Updated upstream

=======
        
>>>>>>> Stashed changes
    interface B extends A
    {
        string callB();
    }
<<<<<<< Updated upstream

=======
        
>>>>>>> Stashed changes
    interface C extends A
    {
        string callC();
    }
<<<<<<< Updated upstream

=======
        
>>>>>>> Stashed changes
    interface D extends B, C
    {
        string callD();
    }
<<<<<<< Updated upstream

=======
        
>>>>>>> Stashed changes
    interface E
    {
        string callE();
    }
<<<<<<< Updated upstream

=======
        
>>>>>>> Stashed changes
    interface F extends E
    {
        string callF();
    }
<<<<<<< Updated upstream

=======
        
>>>>>>> Stashed changes
    interface G
    {
        void shutdown();
        string callG();
    }
<<<<<<< Updated upstream

=======
        
>>>>>>> Stashed changes
    interface H extends G
    {
        string callH();
    }
<<<<<<< Updated upstream

=======
        
>>>>>>> Stashed changes
    interface Echo
    {
        void setConnection();
        void startBatch();
        void flushBatch();
        void shutdown();
    }
}
