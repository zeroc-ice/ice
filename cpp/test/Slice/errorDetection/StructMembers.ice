// Copyright (c) ZeroC, Inc.

module Test
{
    struct s1               // Illegal empty struct
    {
    }
<<<<<<< Updated upstream

=======

>>>>>>> Stashed changes
    struct s2
    {
        long l;             // One member, OK
    }
<<<<<<< Updated upstream

=======

>>>>>>> Stashed changes
    struct s4
    {
        ["Hi"] long l;    // One member with metadata, OK
    }
<<<<<<< Updated upstream

=======

>>>>>>> Stashed changes
    struct s5               // Two members, OK
    {
        long l;
        byte b;
    }
<<<<<<< Updated upstream

=======

>>>>>>> Stashed changes
    struct s6
    {                       // Two members with metadata, OK
        ["Hi"]    long l;
              byte b;
    }
<<<<<<< Updated upstream

=======

>>>>>>> Stashed changes
    struct s7               // Two members with metadata, OK
    {
        ["Hi"]    long l;
        ["there"] byte b;
    }
<<<<<<< Updated upstream

=======

>>>>>>> Stashed changes
    struct s8
    {
        ["hi"] long ;     // Missing data member name
    }
<<<<<<< Updated upstream

=======

>>>>>>> Stashed changes
    struct s9
    {
        ["there"] long    // Missing data member name
    }
<<<<<<< Updated upstream

=======

>>>>>>> Stashed changes
    struct s10
    {
        long ;              // Missing data member name
    }
<<<<<<< Updated upstream

=======

>>>>>>> Stashed changes
    struct s11
    {
        long                // Missing data member name
    }
}
