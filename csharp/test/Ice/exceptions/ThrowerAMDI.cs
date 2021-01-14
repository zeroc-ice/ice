// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Threading;
using System.Threading.Tasks;
using ZeroC.Test;

namespace ZeroC.Ice.Test.Exceptions
{
    public sealed class Forwarder : IObject
    {
        private IObjectPrx _target;

        ValueTask<OutgoingResponseFrame> IObject.DispatchAsync(
            IncomingRequestFrame request,
            Current current,
            CancellationToken cancel)
            => _target.ForwardAsync(request, current.IsOneway, cancel: cancel);

        internal Forwarder(IObjectPrx target) => _target = target;
    }

    public class AsyncThrower : IAsyncThrower
    {
        // 20KB is over the configured 10KB message size max.
        public ValueTask<ReadOnlyMemory<byte>> SendAndReceiveAsync(
            byte[] seq,
            Current current,
            CancellationToken cancel) =>
            new ValueTask<ReadOnlyMemory<byte>>(new byte[1024 * 20]);

        public ValueTask ShutdownAsync(Current current, CancellationToken cancel)
        {
            current.Communicator.ShutdownAsync();
            return default;
        }

        public ValueTask<bool> SupportsAssertExceptionAsync(Current current, CancellationToken cancel) => new(false);

        public ValueTask ThrowAasAAsync(int a, Current current, CancellationToken cancel) => throw new A(a);

        public ValueTask ThrowAorDasAorDAsync(int a, Current current, CancellationToken cancel)
        {
            if (a > 0)
            {
                throw new A(a);
            }
            else
            {
                throw new D(a);
            }
        }

        public ValueTask ThrowBasAAsync(int a, int b, Current current, CancellationToken cancel)
        {
            var s = new TaskCompletionSource<object>();
            s.SetException(new B(a, b));
            return new ValueTask(s.Task);
        }

        public ValueTask ThrowBasBAsync(int a, int b, Current current, CancellationToken cancel) => throw new B(a, b);

        public ValueTask ThrowCasAAsync(int a, int b, int c, Current current, CancellationToken cancel) =>
            throw new C(a, b, c);

        public ValueTask ThrowCasBAsync(int a, int b, int c, Current current, CancellationToken cancel) =>
            throw new C(a, b, c);

        public ValueTask ThrowCasCAsync(int a, int b, int c, Current current, CancellationToken cancel) =>
            throw new C(a, b, c);

        public ValueTask ThrowLocalExceptionAsync(Current current, CancellationToken cancel) =>
            throw new ConnectionClosedException();

        public ValueTask ThrowNonIceExceptionAsync(Current current, CancellationToken cancel) => throw new Exception();

        public ValueTask ThrowAssertExceptionAsync(Current current, CancellationToken cancel)
        {
            TestHelper.Assert(false);
            return default;
        }

        public ValueTask ThrowLocalExceptionIdempotentAsync(Current current, CancellationToken cancel) =>
            throw new ConnectionClosedException();

        // Not supported with C# AMD API
        public ValueTask ThrowAfterResponseAsync(Current current, CancellationToken cancel) => default;

        // Not supported with C# AMD API
        public ValueTask ThrowAfterExceptionAsync(Current current, CancellationToken cancel) => throw new A();

        public ValueTask ThrowAConvertedToUnhandledAsync(Current current, CancellationToken cancel) =>
            throw new A() { ConvertToUnhandled = true };
    }
}
