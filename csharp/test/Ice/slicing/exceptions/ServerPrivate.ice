//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

[[3.7]]
[[suppress-warning(reserved-identifier)]]

#include <Test.ice>

module ZeroC::Ice::Test::Slicing::Exceptions
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
