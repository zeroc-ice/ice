// Copyright (c) ZeroC, Inc.
    
#pragma once
    
["cs:namespace:Ice.retry"]
module Test
{
    interface Retry
    {
        void op(bool kill);
<<<<<<< Updated upstream

        idempotent int opIdempotent(int c);
        void opNotIdempotent();

        idempotent void sleep(int delay);

=======
            
        idempotent int opIdempotent(int c);
        void opNotIdempotent();
            
        idempotent void sleep(int delay);
            
>>>>>>> Stashed changes
        idempotent void shutdown();
    }
}
