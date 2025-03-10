// Copyright (c) ZeroC, Inc.

namespace Ice.proxy
{


        namespace AMD
        {
            public sealed class MyDerivedClassI : Test.MyDerivedClassDisp_, Ice.Object
            {
                public MyDerivedClassI()
                {
                }

                public override Task<Ice.ObjectPrx> echoAsync(Ice.ObjectPrx obj, Ice.Current c)
                {
                    return Task.FromResult(obj);
                }

                public override Task shutdownAsync(Ice.Current current)
                {
                    current.adapter.getCommunicator().shutdown();
                    return Task.CompletedTask;
                }

                public override Task<Dictionary<string, string>> getContextAsync(Ice.Current current)
                {
                    return Task.FromResult(_ctx);
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
    }

