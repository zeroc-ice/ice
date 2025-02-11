// Copyright (c) ZeroC, Inc.

#pragma once

[["java:package:test.Ice.packagemd"]]
module Test1
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

["java:identifier:_notify"]
exception notify
{
    int i;
}
}
