//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

#include <Test.ice>

module Test
{

//
// This class is a hack that allows us to invoke the opClassAndUnknownOptional operation
// on the server and pass an optional argument. This isn't necessary in other language
// mappings where the public stream API is available.
//
interface Initial2
{
    void opClassAndUnknownOptional(A p);

    void opVoid(tag(1) int? a, tag(2) string? v);
}

}
