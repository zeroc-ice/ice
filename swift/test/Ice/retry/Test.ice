// Copyright (c) ZeroC, Inc.
#pragma once
    
module Test
{
    interface Retry
    {
        void op(bool kill);
<<<<<<< Updated upstream

        idempotent int opIdempotent(int c);
        void opNotIdempotent();

=======
            
        idempotent int opIdempotent(int c);
        void opNotIdempotent();
            
>>>>>>> Stashed changes
        idempotent void shutdown();
    }
}
