// Copyright (c) ZeroC, Inc.

#pragma once

#include "Test.ice"

module Test
{
    //
    // This interface is a hack that allows us to invoke the opClassAndUnknownOptional operation
    // on the server and pass an optional argument. This isn't necessary in other language
    // mappings where the public stream API is available.
    //
    interface Initial2
    {
        void opClassAndUnknownOptional(A p, optional(1) VarStruct ovs);

        void opVoid(optional(1) int a, optional(2) string v);
    }
}
