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

    // Regression test for issue #5480: slice2js must not emit a '?' suffix for an optional
    // parameter that is followed by a required parameter (this would generate invalid
    // TypeScript that fails to compile, TS1016). This interface is compiled only for the JS
    // client; its generated .d.ts is type-checked by the build, which is what exercises the bug.
    interface OptionalParamOrder
    {
        int opOptionalFirstParam(optional(1) int p1, int p2);
    }
}
