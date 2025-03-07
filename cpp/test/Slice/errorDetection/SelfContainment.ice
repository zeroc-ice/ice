// Copyright (c) ZeroC, Inc.

module Test
{
    struct x
    {
        int i;
        x j;        // Error
    }
<<<<<<< Updated upstream

=======

>>>>>>> Stashed changes
    class y
    {
        int i;
        y j;        // OK
    }
}
