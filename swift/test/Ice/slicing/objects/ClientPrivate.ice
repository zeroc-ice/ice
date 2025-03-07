// Copyright (c) ZeroC, Inc.
#pragma once

#include "Test.ice"

[["swift:class-resolver-prefix:IceSlicingObjectsClient"]]

module Test
{
    class D3 extends B
    {
        string sd3;
        B pd3;
    }
<<<<<<< Updated upstream

=======

>>>>>>> Stashed changes
    class PCUnknown extends PBase
    {
        string pu;
    }
<<<<<<< Updated upstream

=======

>>>>>>> Stashed changes
    class PCDerived extends PDerived
    {
        PBaseSeq pbs;
    }
<<<<<<< Updated upstream

=======

>>>>>>> Stashed changes
    class PCDerived2 extends PCDerived
    {
        int pcd2;
    }
<<<<<<< Updated upstream

=======

>>>>>>> Stashed changes
    class PCDerived3 extends PCDerived2
    {
        Object pcd3;
    }
<<<<<<< Updated upstream

=======

>>>>>>> Stashed changes
    class CompactPCDerived(57) extends CompactPDerived
    {
        PBaseSeq pbs;
    }
}
