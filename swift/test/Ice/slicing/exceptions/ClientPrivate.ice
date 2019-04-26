//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

#include <Test.ice>

[["swift:class-resolver-prefix:IceSlicingExceptionsClient"]]

module Test
{

class PreservedClass extends BaseClass
{
    string pc;
}

exception Preserved1 extends KnownPreservedDerived
{
    BaseClass p1;
}

exception Preserved2 extends Preserved1
{
    BaseClass p2;
}

}
