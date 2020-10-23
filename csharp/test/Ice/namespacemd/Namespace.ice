//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

[cs:namespace(ZeroC.Ice.Test.NamespaceMD)]
module WithNamespace
{
class C1
{
    int i;
}

class C2 : C1
{
    long l;
}

exception E1
{
    int i;
}

exception E2 : E1
{
    long l;
}
}
