// Copyright (c) ZeroC, Inc.
    
#pragma once
    
module NoNamespace
{
    class C1
    {
        int i;
    }
<<<<<<< Updated upstream

=======
        
>>>>>>> Stashed changes
    class C2 extends C1
    {
        long l;
    }
<<<<<<< Updated upstream

=======
        
>>>>>>> Stashed changes
    exception E1
    {
        int i;
    }
<<<<<<< Updated upstream

=======
        
>>>>>>> Stashed changes
    exception E2 extends E1
    {
        long l;
    }
<<<<<<< Updated upstream

=======
        
>>>>>>> Stashed changes
    exception notify /* Test keyword escape. */
    {
        int i;
    }
}
