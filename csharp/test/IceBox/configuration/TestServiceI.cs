//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using IceBox;
using Test;

class TestService : IService
{
    public void
    start(string name, Ice.Communicator communicator, string[] args)
    {
        Ice.ObjectAdapter adapter = communicator.createObjectAdapter(name + "OA");
        adapter.Add(new TestIntf(args), "test");
        adapter.Activate();
    }

    public void
    stop()
    {
    }
}
