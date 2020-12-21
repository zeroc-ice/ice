// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Threading;
using System.Threading.Tasks;

namespace ZeroC.Ice.Test.Binding
{
    public class RemoteObjectAdapter : IAsyncRemoteObjectAdapter
    {
        private readonly ObjectAdapter _adapter;
        private readonly ITestIntfPrx _testIntf;

        public RemoteObjectAdapter(ObjectAdapter adapter)
        {
            _adapter = adapter;
            _testIntf = _adapter.Add("test", new TestIntf(), ITestIntfPrx.Factory);
        }

        public ValueTask<ITestIntfPrx> GetTestIntfAsync(Current current, CancellationToken cancel) =>
            new(_testIntf);

        public async ValueTask DeactivateAsync(Current current, CancellationToken cancel) =>
            await _adapter.DisposeAsync();
    }
}
