// Copyright (c) ZeroC, Inc.
    
#pragma once
    
#include "Test.ice"
    
module Test
{
    exception UnknownDerived extends Base
    {
        string ud;
    }
<<<<<<< Updated upstream

=======
        
>>>>>>> Stashed changes
    exception UnknownIntermediate extends Base
    {
        string ui;
    }
<<<<<<< Updated upstream

=======
        
>>>>>>> Stashed changes
    exception UnknownMostDerived1 extends KnownIntermediate
    {
        string umd1;
    }
<<<<<<< Updated upstream

=======
        
>>>>>>> Stashed changes
    exception UnknownMostDerived2 extends UnknownIntermediate
    {
        string umd2;
    }
}
