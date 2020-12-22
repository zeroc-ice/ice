// Copyright (c) ZeroC, Inc. All rights reserved.

using ZeroC.Ice;

namespace ZeroC.IceBox.Test.Configuration
{
    public class TestService : IService
    {
        public void Start(string name, Communicator communicator, string[] args)
        {
            ObjectAdapter adapter = communicator.CreateObjectAdapter(name + "OA");
            adapter.Add("test", new TestIntf(args));
            adapter.ActivateAsync().GetAwaiter().GetResult(); // TODO: temporary
        }

        public void Stop()
        {
        }
    }
}
