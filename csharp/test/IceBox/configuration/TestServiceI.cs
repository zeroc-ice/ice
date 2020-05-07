//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using IceBox;

public class TestService : IService
{
    public void Start(string name, Ice.Communicator communicator, string[] args)
    {
        Ice.ObjectAdapter adapter = communicator.CreateObjectAdapter(name + "OA");
        adapter.Add("test", new TestIntf(args));
        adapter.Activate();
    }

    public void Stop()
    {
    }
}
