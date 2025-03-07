// Copyright (c) ZeroC, Inc.
    
#pragma once
    
module Test
{
    exception BadEncodingException {}
<<<<<<< Updated upstream
=======
        
>>>>>>> Stashed changes
    interface MyObject
    {
        ["cpp:type:wstring"] string widen(string msg) throws BadEncodingException;
        string narrow(["cpp:type:wstring"] string wmsg);
<<<<<<< Updated upstream

=======
            
>>>>>>> Stashed changes
        void shutdown();
    }
}
