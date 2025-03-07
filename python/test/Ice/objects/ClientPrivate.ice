// Copyright (c) ZeroC, Inc.

#pragma once

module Test
{
    class Empty
    {
    }
<<<<<<< Updated upstream

    class AlsoEmpty
    {
    }

=======

    class AlsoEmpty
    {
    }

>>>>>>> Stashed changes
    interface UnexpectedObjectExceptionTest
    {
        Empty op();
    }
<<<<<<< Updated upstream

    //
    // Remaining definitions are here to ensure that the generated code compiles.
    //

=======

    //
    // Remaining definitions are here to ensure that the generated code compiles.
    //

>>>>>>> Stashed changes
    class COneMember
    {
        Empty e;
    }
<<<<<<< Updated upstream

=======

>>>>>>> Stashed changes
    class CTwoMembers
    {
        Empty e1;
        Empty e2;
    }
<<<<<<< Updated upstream

=======

>>>>>>> Stashed changes
    exception EOneMember
    {
        Empty e;
    }
<<<<<<< Updated upstream

=======

>>>>>>> Stashed changes
    exception ETwoMembers
    {
        Empty e1;
        Empty e2;
    }
<<<<<<< Updated upstream

=======

>>>>>>> Stashed changes
    struct SOneMember
    {
        Empty e;
    }
<<<<<<< Updated upstream

=======

>>>>>>> Stashed changes
    struct STwoMembers
    {
        Empty e1;
        Empty e2;
    }
<<<<<<< Updated upstream

=======

>>>>>>> Stashed changes
    dictionary<int, COneMember> DOneMember;
    dictionary<int, CTwoMembers> DTwoMembers;
}
