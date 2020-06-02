//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

[[3.7]]

#include <Test.ice>

// 'Test.ice' and 'TestAMD.ice' need to generate code into separate namespaces, but with identical type-ids. So we use
// the 'cs:namespace' metadata here to place the AMD code into a separate AMD namespace, instead of another module;
// Which would result in differing type-ids.
[cs:namespace:ZeroC.Ice.Test]
module Slicing::Exceptions
{

exception UnknownDerived : Base
{
    string ud;
}

exception UnknownIntermediate : Base
{
   string ui;
}

exception UnknownMostDerived1 : KnownIntermediate
{
   string umd1;
}

exception UnknownMostDerived2 : UnknownIntermediate
{
   string umd2;
}

class SPreservedClass : BaseClass
{
    string spc;
}

exception SPreserved1 : KnownPreservedDerived
{
    BaseClass p1;
}

exception SPreserved2 : SPreserved1
{
    BaseClass p2;
}

exception ServerPrivateException
{
    string spe;
}

}
