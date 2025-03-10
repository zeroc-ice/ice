// Copyright (c) ZeroC, Inc.

namespace Ice.proxy
{


        public sealed class MyDerivedClassI : Test.MyDerivedClassDisp_, Ice.Object
        {
            public MyDerivedClassI()
            {
            }

            public override Ice.ObjectPrx echo(Ice.ObjectPrx obj, Ice.Current c)
            {
                return obj;
            }

            public override void shutdown(Ice.Current current)
            {
                current.adapter.getCommunicator().shutdown();
            }

            public override Dictionary<string, string> getContext(Ice.Current current)
            {
                return _ctx;
            }

            public bool ice_isA(string s, Ice.Current current)
            {
                _ctx = current.ctx;
                // TODO: call the default implementation when the base(Object) syntax is supported.
                return Array.BinarySearch((this as Ice.Object).ice_ids(current), s) >= 0;
            }

            private Dictionary<string, string> _ctx;
        }
    }

