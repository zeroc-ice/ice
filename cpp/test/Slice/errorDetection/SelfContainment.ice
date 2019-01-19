//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

module Test
{

struct x
{
    int i;
    x j;        // Error
}

class y
{
    int i;
    y j;        // OK
}

}
