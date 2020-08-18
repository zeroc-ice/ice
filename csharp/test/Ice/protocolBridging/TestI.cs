//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Linq;
using System.Threading.Tasks;
using Test;
using ZeroC.Ice;

namespace ZeroC.Ice.Test.ProtocolBridging
{
    public sealed class Forwarder : IObject
    {
        private IObjectPrx _target;

        ValueTask<OutgoingResponseFrame> IObject.DispatchAsync(IncomingRequestFrame request, Current current)
        {
            if (current.Operation != "shutdown")
            {
                TestHelper.Assert(current.Context["MyCtx"] == "hello");
            }

            return _target.ForwardAsync(current.IsOneway, request);
        }

        internal Forwarder(IObjectPrx target) => _target = target;
    }

    public sealed class TestI : ITestIntf
    {
        public int Op(int x, Current current)
        {
            TestHelper.Assert(current.Context["MyCtx"] == "hello");
            return x;
        }

        public (int, string) OpReturnOut(int x, Current current)
        {
            TestHelper.Assert(current.Context["MyCtx"] == "hello");
            return (x, $"value={x}");
        }

        public void OpOneway(int x, Current current)
        {
            TestHelper.Assert(current.Context["MyCtx"] == "hello");
            TestHelper.Assert(x == 42);
        }

        public void OpMyError(Current current)
        {
            TestHelper.Assert(current.Context["MyCtx"] == "hello");
            throw new MyError(42);
        }

        public void OpObjectNotExistException(Current current)
        {
            TestHelper.Assert(current.Context["MyCtx"] == "hello");
            throw new ObjectNotExistException(current);
        }
        public ITestIntfPrx OpNewProxy(Current current)
        {
            TestHelper.Assert(current.Context["MyCtx"] == "hello");
            return current.Adapter.CreateProxy(current.Identity, ITestIntfPrx.Factory).Clone(
                encoding: current.Encoding);
        }

        public void Shutdown(Current current) => current.Communicator.ShutdownAsync();
    }
}
