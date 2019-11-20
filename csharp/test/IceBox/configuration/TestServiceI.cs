//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using IceBox;
using Test;

class TestServiceI : Service
{
    public void
    start(string name, Ice.Communicator communicator, string[] args)
    {
        Ice.ObjectAdapter adapter = communicator.createObjectAdapter(name + "OA");
        adapter.Add(new TestI(args), "test");
        adapter.Activate();
    }

    public void
    stop()
    {
    }
}
