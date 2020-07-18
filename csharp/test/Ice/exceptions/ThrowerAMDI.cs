//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Threading.Tasks;
using Test;

namespace ZeroC.Ice.Test.Exceptions
{
    public class ThrowerAsync : IThrowerAsync
    {
        public ThrowerAsync()
        {
        }

        public ValueTask ShutdownAsync(Current current)
        {
            current.Adapter.Communicator.ShutdownAsync();
            return new ValueTask(Task.CompletedTask);
        }

        public ValueTask<bool> SupportsAssertExceptionAsync(Current current) =>
            new ValueTask<bool>(false);

        public ValueTask ThrowAasAAsync(int a, Current current) => throw new A(a);

        public ValueTask ThrowAorDasAorDAsync(int a, Current current)
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

        public ValueTask ThrowBasAAsync(int a, int b, Current current)
        {
            //throw new B(a, b);
            var s = new TaskCompletionSource<object>();
            s.SetException(new B(a, b));
            return new ValueTask(s.Task);
        }

        public ValueTask ThrowBasBAsync(int a, int b, Current current) => throw new B(a, b);

        public ValueTask ThrowCasAAsync(int a, int b, int c, Current current) => throw new C(a, b, c);

        public ValueTask ThrowCasBAsync(int a, int b, int c, Current current) => throw new C(a, b, c);

        public ValueTask ThrowCasCAsync(int a, int b, int c, Current current) => throw new C(a, b, c);

        public ValueTask ThrowUndeclaredAAsync(int a, Current current) => throw new A(a);

        public ValueTask ThrowUndeclaredBAsync(int a, int b, Current current) => throw new B(a, b);

        public ValueTask ThrowUndeclaredCAsync(int a, int b, int c, Current current) => throw new C(a, b, c);

        public ValueTask ThrowLocalExceptionAsync(Current current) => throw new ConnectionTimeoutException();

        public ValueTask ThrowNonIceExceptionAsync(Current current) => throw new Exception();

        public ValueTask ThrowAssertExceptionAsync(Current current)
        {
            TestHelper.Assert(false);
            return new ValueTask(Task.CompletedTask);
        }

        // 20KB is over the configured 10KB message size max.
        public ValueTask<ReadOnlyMemory<byte>> ThrowMemoryLimitExceptionAsync(byte[] seq, Current current) =>
            new ValueTask<ReadOnlyMemory<byte>>(new byte[1024 * 20]);

        public ValueTask ThrowLocalExceptionIdempotentAsync(Current current) =>
            throw new ConnectionTimeoutException();

        // Only supported with callback based AMD API
        public ValueTask ThrowAfterResponseAsync(Current current) =>
            new ValueTask(Task.CompletedTask); //throw new Exception();

        // Only supported with callback based AMD API
        public ValueTask ThrowAfterExceptionAsync(Current current) => throw new A(); //throw new Exception();

        public ValueTask ThrowAConvertedToUnhandledAsync(Current current)
        {
            var a = new A();
            a.ConvertToUnhandled = true;
            throw a;
        }
    }
}
