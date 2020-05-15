//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace ZeroC.Ice.binding
{
    public class RemoteObjectAdapter : Test.IRemoteObjectAdapter
    {
        public RemoteObjectAdapter(ObjectAdapter adapter)
        {
            _adapter = adapter;
            _testIntf = _adapter.Add("test", new TestIntf(), Test.ITestIntfPrx.Factory);
            _adapter.Activate();
        }

        public Test.ITestIntfPrx getTestIntf(Current current) => _testIntf;

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
        private Test.ITestIntfPrx _testIntf;
    }
}
