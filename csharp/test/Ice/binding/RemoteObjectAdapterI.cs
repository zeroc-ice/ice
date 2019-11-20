//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Ice.binding.Test;

namespace Ice
{
    namespace binding
    {
        public class RemoteObjectAdapterI : Test.RemoteObjectAdapter
        {
            public RemoteObjectAdapterI(Ice.ObjectAdapter adapter)
            {
                _adapter = adapter;
                _testIntf = TestIntfPrxHelper.uncheckedCast(_adapter.Add(new TestI(), Util.stringToIdentity("test")));
                _adapter.activate();
            }

            public Test.TestIntfPrx
            getTestIntf(Ice.Current current)
            {
                return _testIntf;
            }

            public void
            deactivate(Ice.Current current)
            {
                try
                {
                    _adapter.destroy();
                }
                catch (Ice.ObjectAdapterDeactivatedException)
                {
                }
            }

            private Ice.ObjectAdapter _adapter;
            private Test.TestIntfPrx _testIntf;
        }
    }
}
