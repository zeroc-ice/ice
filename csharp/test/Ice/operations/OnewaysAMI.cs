//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Diagnostics;
using System.Threading;

namespace Ice.operations
{
    public class OnewaysAMI
    {
        private static void test(bool b)
        {
            if (!b)
            {
                System.Diagnostics.Debug.Assert(false);
                throw new System.Exception();
            }
        }

        private class CallbackBase
        {
            internal CallbackBase()
            {
                _called = false;
            }

            public virtual void check()
            {
                lock (this)
                {
                    while (!_called)
                    {
                        Monitor.Wait(this);
                    }
                    _called = false;
                }
            }

            public virtual void called()
            {
                lock (this)
                {
                    Debug.Assert(!_called);
                    _called = true;
                    Monitor.Pulse(this);
                }
            }

            private bool _called;
        }

        private class Callback : CallbackBase
        {
            public Callback()
            {
            }

            public void
            sent(bool sentSynchronously)
            {
                called();
            }

            public void noException(Exception ex)
            {
                test(false);
            }
        }

        internal static void onewaysAMI(global::Test.TestHelper helper, Test.IMyClassPrx proxy)
        {
            Communicator communicator = helper.communicator();
            Test.IMyClassPrx p = proxy.Clone(oneway: true);

            {
                Callback cb = new Callback();
                p.IcePingAsync(progress: new Progress<bool>(
                    sentSynchronously =>
                    {
                        cb.sent(sentSynchronously);
                    }));
                cb.check();
            }

            bool b = p.IceIsAAsync("::Test::MyClass").Result;
            string id = p.IceIdAsync().Result;
            string[] ids = p.IceIdsAsync().Result;

            {
                Callback cb = new Callback();
                p.opVoidAsync(progress: new Progress<bool>(
                    sentSynchronously =>
                    {
                        cb.sent(sentSynchronously);
                    }));
                cb.check();
            }

            {
                Callback cb = new Callback();
                p.opIdempotentAsync(progress: new Progress<bool>(
                    sentSynchronously =>
                    {
                        cb.sent(sentSynchronously);
                    }));
                cb.check();
            }

            {
                Callback cb = new Callback();
                p.opOnewayAsync(progress: new Progress<bool>(
                    sentSynchronously =>
                    {
                        cb.sent(sentSynchronously);
                    }));
                cb.check();
            }

            var _ = p.opByteAsync(0xff, 0x0f).Result;
        }
    }
}
