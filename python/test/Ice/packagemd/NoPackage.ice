//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

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

exception def /* Test keyword escape. */
{
    int i;
}
}
