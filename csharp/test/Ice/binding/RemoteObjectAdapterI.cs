//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;

namespace ZeroC.Ice.Test.Binding
{
    public class RemoteObjectAdapter : IRemoteObjectAdapter
    {
        public RemoteObjectAdapter(ObjectAdapter adapter)
        {
            _adapter = adapter;
            _testIntf = _adapter.Add("test", new TestIntf(), ITestIntfPrx.Factory);
            _adapter.Activate();
        }

        public ITestIntfPrx GetTestIntf(Current current) => _testIntf;

        public void Deactivate(Current current)
        {
            try
            {
                _adapter.Dispose();
            }
            catch (ObjectDisposedException)
            {
            }
        }

        private ObjectAdapter _adapter;
        private ITestIntfPrx _testIntf;
    }
}
