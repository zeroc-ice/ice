//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Collections.Generic;

using ZeroC.Ice;

namespace ZeroC.IceBox.Test.Configuration
{
    public class TestIntf : ITestIntf
    {
        private string[] _args;

        public TestIntf(string[] args) => _args = args;

        public string GetProperty(string name, Current current) => current.Adapter.Communicator.GetProperty(name) ?? "";

        public IEnumerable<string> GetArgs(Current current) => _args;
    }
}
