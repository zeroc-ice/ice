//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using Test;
using System.Threading;

namespace Ice.operations
{
    public class OnewaysAMI
    {
        private class CallbackBase
        {
            internal CallbackBase() => _called = false;

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
                    TestHelper.Assert(!_called);
                    _called = true;
                    Monitor.Pulse(this);
                }
            }

            private bool _called;
        }

        private class Callback : CallbackBase
        {
            public void sent() => called();
        }

        internal static void onewaysAMI(TestHelper helper, Test.IMyClassPrx proxy)
        {
            Communicator? communicator = helper.Communicator();
            TestHelper.Assert(communicator != null);
            Test.IMyClassPrx p = proxy.Clone(oneway: true);

            {
                var cb = new Callback();
                p.IcePingAsync(progress: new Progress<bool>(sentSynchronously => cb.sent()));
                cb.check();
            }

            bool b = p.IceIsAAsync("::Test::MyClass").Result;
            string id = p.IceIdAsync().Result;
            string[] ids = p.IceIdsAsync().Result;

            {
                var cb = new Callback();
                p.opVoidAsync(progress: new Progress<bool>(sentSynchronously => cb.sent()));
                cb.check();
            }

            {
                var cb = new Callback();
                p.opIdempotentAsync(progress: new Progress<bool>(sentSynchronously => cb.sent()));
                cb.check();
            }

            {
                var cb = new Callback();
                p.opOnewayAsync(progress: new Progress<bool>(sentSynchronously => cb.sent()));
                cb.check();
            }

            {
                var cb = new Callback();
                p.opOnewayMetadataAsync(progress: new Progress<bool>(sentSynchronously => cb.sent()));
                cb.check();
            }

            (byte ReturnValue, byte p3) = p.opByteAsync(0xff, 0x0f).Result;
        }
    }
}
