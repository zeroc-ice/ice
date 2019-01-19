//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

module Test
{

exception UE
{
}

interface Hello
{
    void sayHello(int delay);
    int add(int s1, int s2);
    void raiseUE()
        throws UE;
    void shutdown();
}

}
