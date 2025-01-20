// Copyright (c) ZeroC, Inc.

#pragma once

["cs:namespace:Ice.namespacemd"]
module WithNamespace
{
class C1
{
    int i;
}

class C2 extends C1
{
    long l;
}

exception E1
{
    int i;
}

exception E2 extends E1
{
    long l;
}
}
