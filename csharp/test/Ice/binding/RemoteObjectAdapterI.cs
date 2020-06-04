//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

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

        public ITestIntfPrx getTestIntf(Current current) => _testIntf;

        public void deactivate(Current current)
        {
            try
            {
                _adapter.Destroy();
            }
            catch (ObjectAdapterDeactivatedException)
            {
            }
        }

        private ObjectAdapter _adapter;
        private ITestIntfPrx _testIntf;
    }
}
