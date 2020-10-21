//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

[[java:package(test.Ice.packagemd.testpkg)]]

module Test2
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

[java:package(test.Ice.packagemd.modpkg)]
module Test3
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
