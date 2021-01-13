// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.IO;
using System.Threading;
using System.Threading.Tasks;
using ZeroC.Test;

namespace ZeroC.Ice.Test.Operations
{
    public static class OnewaysAMI
    {
        private class CallbackBase
        {
            private bool _called;
            private readonly object _mutex = new();

            public virtual void Check()
            {
                lock (_mutex)
                {
                    while (!_called)
                    {
                        Monitor.Wait(_mutex);
                    }
                    _called = false;
                }
            }

            public virtual void Called()
            {
                lock (_mutex)
                {
                    TestHelper.Assert(!_called);
                    _called = true;
                    Monitor.Pulse(_mutex);
                }
            }

            internal CallbackBase() => _called = false;
        }

        private class Callback : CallbackBase
        {
            public void Sent() => Called();
        }

        internal static void Run(TestHelper helper, IMyClassPrx proxy)
        {
            Communicator communicator = helper.Communicator;
            IMyClassPrx p = proxy.Clone(oneway: true);

            {
                var cb = new Callback();
                p.IcePingAsync(progress: new Progress<bool>(sentSynchronously => cb.Sent()));
                cb.Check();
            }

            bool b = p.IceIsAAsync("::ZeroC::Ice::Test::Operations::MyClass").Result;
            string id = p.IceIdAsync().Result;
            string[] ids = p.IceIdsAsync().Result;

            {
                var cb = new Callback();
                p.OpVoidAsync(progress: new Progress<bool>(sentSynchronously => cb.Sent()));
                cb.Check();
            }

            {
                var cb = new Callback();
                p.OpIdempotentAsync(progress: new Progress<bool>(sentSynchronously => cb.Sent()));
                cb.Check();
            }

            {
                var cb = new Callback();
                p.OpOnewayAsync(progress: new Progress<bool>(sentSynchronously => cb.Sent()));
                cb.Check();
            }

            {
                var cb = new Callback();
                p.OpOnewayMetadataAsync(progress: new Progress<bool>(sentSynchronously => cb.Sent()));
                cb.Check();
            }

            (byte ReturnValue, byte p3) = p.OpByteAsync(0xff, 0x0f).Result;

            if (p.Protocol != Protocol.Ice1)
            {
                Task.Run(async () =>
                {
                    await p.OpSendStream1Async(File.OpenRead("AllTests.cs")).ConfigureAwait(false);
                    await p.OpSendStream2Async("AllTests.cs", File.OpenRead("AllTests.cs")).ConfigureAwait(false);
                }).Wait();
            }
        }
    }
}
