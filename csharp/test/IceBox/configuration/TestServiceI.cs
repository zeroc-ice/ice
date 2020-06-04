//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using ZeroC.Ice;
using ZeroC.IceBox;

namespace ZeroC.IceBox.Test.Configuration
{
    public class TestService : IService
    {
        public void Start(string name, Communicator communicator, string[] args)
        {
            ObjectAdapter adapter = communicator.CreateObjectAdapter(name + "OA");
            adapter.Add("test", new TestIntf(args));
            adapter.Activate();
        }

        public void Stop()
        {
        }
    }
}
