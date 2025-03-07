// Copyright (c) ZeroC, Inc.
    
#pragma once
    
module Test
{
    module MA
    {
        interface IA
        {
            IA* iaop(IA* p);
        }
    }
<<<<<<< Updated upstream

=======
        
>>>>>>> Stashed changes
    module MB
    {
        interface IB1 extends MA::IA
        {
            IB1* ib1op(IB1* p);
        }
<<<<<<< Updated upstream

=======
            
>>>>>>> Stashed changes
        interface IB2 extends MA::IA
        {
            IB2* ib2op(IB2* p);
        }
    }
<<<<<<< Updated upstream

=======
        
>>>>>>> Stashed changes
    module MA
    {
        interface IC extends MB::IB1, MB::IB2
        {
            IC* icop(IC* p);
        }
    }
<<<<<<< Updated upstream

=======
        
>>>>>>> Stashed changes
    interface Initial
    {
        void shutdown();
        MA::IA* iaop();
        MB::IB1* ib1op();
        MB::IB2* ib2op();
        MA::IC* icop();
    }
}
