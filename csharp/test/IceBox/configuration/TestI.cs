// Copyright (c) ZeroC, Inc. All rights reserved.

using System.Collections.Generic;
using System.Threading;
using ZeroC.Ice;

namespace ZeroC.IceBox.Test.Configuration
{
    public class TestIntf : ITestIntf
    {
        private readonly string[] _args;

        public TestIntf(string[] args) => _args = args;

        public string GetProperty(string name, Current current, CancellationToken cancel) =>
            current.Adapter.Communicator.GetProperty(name) ?? "";

        public IEnumerable<string> GetArgs(Current current, CancellationToken cancel) => _args;
    }
}
