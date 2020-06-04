//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

[[3.7]]

#include <Test.ice>

// Because this test has both AMD and non-AMD Slice definitions with identical type-ids, we need to separate them
// into different namespaces. As a result of this, this module also has to be placed in it's namespace with metadata
// instead of nested modules (which would affect type-id).
[cs:namespace:ZeroC.Ice.Test]
module Slicing::Exceptions
{

class PreservedClass : BaseClass
{
    string pc;
}

exception Preserved1 : KnownPreservedDerived
{
    BaseClass p1;
}

exception Preserved2 : Preserved1
{
    BaseClass p2;
}

exception ClientPrivateException
{
    string cpe;
}

}
