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
            if (current.Operation == "op" || current.Operation == "opVoid")
            {
                TestHelper.Assert(request.Context.Count == 1);
                TestHelper.Assert(request.Context["MyCtx"] == "hello");

                if (current.Operation == "opVoid")
                {
                    request.Context.Clear();
                }
            }
            else
            {
                TestHelper.Assert(request.Context.Count == 0);
            }

            if (current.Operation != "opVoid" && current.Operation != "shutdown")
            {
                request.Context["Intercepted"] = "1";
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
            TestHelper.Assert(current.Context.Count == 2);
            TestHelper.Assert(current.Context.ContainsKey("Intercepted") || current.Context.ContainsKey("Direct"));
            return x;
        }

        public void OpVoid(Current current)
        {
            if (current.Context.Count == 2)
            {
                TestHelper.Assert(current.Context["MyCtx"] == "hello");
                TestHelper.Assert(current.Context.ContainsKey("Direct"));
            }
            else
            {
                TestHelper.Assert(current.Context.Count == 0);
            }
        }

        public (int, string) OpReturnOut(int x, Current current)
        {
            TestHelper.Assert(current.Context.Count == 1);
            TestHelper.Assert(current.Context.ContainsKey("Intercepted") || current.Context.ContainsKey("Direct"));
            return (x, $"value={x}");
        }

        public void OpOneway(int x, Current current)
        {
            TestHelper.Assert(current.Context.Count == 1);
            TestHelper.Assert(current.Context.ContainsKey("Intercepted") || current.Context.ContainsKey("Direct"));
            TestHelper.Assert(x == 42);
        }

        public void OpMyError(Current current)
        {
            TestHelper.Assert(current.Context.Count == 1);
            TestHelper.Assert(current.Context.ContainsKey("Intercepted") || current.Context.ContainsKey("Direct"));
            throw new MyError(42);
        }

        public void OpObjectNotExistException(Current current)
        {
            TestHelper.Assert(current.Context.Count == 1);
            TestHelper.Assert(current.Context.ContainsKey("Intercepted") || current.Context.ContainsKey("Direct"));
            throw new ObjectNotExistException(current);
        }
        public ITestIntfPrx OpNewProxy(Current current)
        {
            TestHelper.Assert(current.Context.Count == 1);
            TestHelper.Assert(current.Context.ContainsKey("Intercepted") || current.Context.ContainsKey("Direct"));
            return current.Adapter.CreateProxy(current.Identity, ITestIntfPrx.Factory).Clone(
                encoding: current.Encoding);
        }

        public void Shutdown(Current current) => current.Communicator.ShutdownAsync();
    }
}
