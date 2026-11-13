// Copyright (c) ZeroC, Inc.

#pragma once

["python:identifier:generated.test.Ice.blobject.Test"]
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
