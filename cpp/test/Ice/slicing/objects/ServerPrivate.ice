// Copyright (c) ZeroC, Inc.

#pragma once

#include "Test.ice"

module Test
{
    class SBSUnknownDerived extends SBase
    {
        string sbsud;
    }
<<<<<<< Updated upstream

=======

>>>>>>> Stashed changes
    class SUnknown
    {
        string su;
        SUnknown cycle;
    }
<<<<<<< Updated upstream

=======

>>>>>>> Stashed changes
    class D2 extends B
    {
        string sd2;
        B pd2;
    }
<<<<<<< Updated upstream

=======

>>>>>>> Stashed changes
    class D4 extends B
    {
        B p1;
        B p2;
    }
<<<<<<< Updated upstream

=======

>>>>>>> Stashed changes
    exception UnknownDerivedException extends BaseException
    {
        string sude;
        D2 pd2;
    }
<<<<<<< Updated upstream

=======

>>>>>>> Stashed changes
    class MyClass
    {
        int i;
    }
<<<<<<< Updated upstream

=======

>>>>>>> Stashed changes
    class PSUnknown extends Preserved
    {
        string psu;
        PNode graph;
        MyClass cl;
    }
<<<<<<< Updated upstream

=======

>>>>>>> Stashed changes
    class PSUnknown2 extends Preserved
    {
        PBase pb;
    }
}
