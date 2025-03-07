// Copyright (c) ZeroC, Inc.

#pragma once

module Test
{
    exception UE
    {
    }
<<<<<<< Updated upstream

=======

>>>>>>> Stashed changes
    interface Hello
    {
        void sayHello(int delay);
        int add(int s1, int s2);
        void raiseUE()
<<<<<<< Updated upstream
        throws UE;
=======
            throws UE;
>>>>>>> Stashed changes
        void shutdown();
    }
}
