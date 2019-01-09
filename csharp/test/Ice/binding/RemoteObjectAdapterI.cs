// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

namespace Ice
{
    namespace binding
    {
        public class RemoteObjectAdapterI : Test.RemoteObjectAdapterDisp_
        {
            public RemoteObjectAdapterI(Ice.ObjectAdapter adapter)
            {
                _adapter = adapter;
                _testIntf = Test.TestIntfPrxHelper.uncheckedCast(_adapter.add(new TestI(),
                                                            Ice.Util.stringToIdentity("test")));
                _adapter.activate();
            }

            public override Test.TestIntfPrx
            getTestIntf(Ice.Current current)
            {
                return _testIntf;
            }

            public override void
            deactivate(Ice.Current current)
            {
                try
                {
                    _adapter.destroy();
                }
                catch(Ice.ObjectAdapterDeactivatedException)
                {
                }
            }

            private Ice.ObjectAdapter _adapter;
            private Test.TestIntfPrx _testIntf;
        }
    }
}
