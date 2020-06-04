//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Threading.Tasks;
using Test;

namespace ZeroC.Ice.Test.AMD.Exceptions
{
    public class Thrower : IThrower
    {
        public Thrower()
        {
        }

        public ValueTask
        shutdownAsync(Current current)
        {
            current.Adapter.Communicator.Shutdown();
            return new ValueTask(Task.CompletedTask);
        }

        public ValueTask<bool>
        supportsAssertExceptionAsync(Current current) => new ValueTask<bool>(false);

        public ValueTask
        throwAasAAsync(int a, Current current) => throw new A(a);

        public ValueTask
        throwAorDasAorDAsync(int a, Current current)
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

        public ValueTask
        throwBasAAsync(int a, int b, Current current)
        {
            //throw new B(a, b);
            var s = new TaskCompletionSource<object>();
            s.SetException(new B(a, b));
            return new ValueTask(s.Task);
        }

        public ValueTask
        throwBasBAsync(int a, int b, Current current) => throw new B(a, b);

        public ValueTask
        throwCasAAsync(int a, int b, int c, Current current) => throw new C(a, b, c);

        public ValueTask
        throwCasBAsync(int a, int b, int c, Current current) => throw new C(a, b, c);

        public ValueTask
        throwCasCAsync(int a, int b, int c, Current current) => throw new C(a, b, c);

        public ValueTask
        throwUndeclaredAAsync(int a, Current current) => throw new A(a);

        public ValueTask
        throwUndeclaredBAsync(int a, int b, Current current) => throw new B(a, b);

        public ValueTask
        throwUndeclaredCAsync(int a, int b, int c, Current current) => throw new C(a, b, c);

        public ValueTask
        throwLocalExceptionAsync(Current current) => throw new ConnectionTimeoutException();

        public ValueTask
        throwNonIceExceptionAsync(Current current) => throw new Exception();

        public ValueTask
        throwAssertExceptionAsync(Current current)
        {
            TestHelper.Assert(false);
            return new ValueTask(Task.CompletedTask);
        }

        public ValueTask<ReadOnlyMemory<byte>>
        throwMemoryLimitExceptionAsync(byte[] seq, Current current) =>
            new ValueTask<ReadOnlyMemory<byte>>(new byte[1024 * 20]); // 20KB is over the configured 10KB message size max.

        public ValueTask
        throwLocalExceptionIdempotentAsync(Current current) =>
            throw new ConnectionTimeoutException();

        public ValueTask
        throwAfterResponseAsync(Current current)
        {
            // Only supported with callback based AMD API
            return new ValueTask(Task.CompletedTask);
            //throw new Exception();
        }

        public ValueTask
        throwAfterExceptionAsync(Current current)
        {
            // Only supported with callback based AMD API
            throw new A();
            //throw new Exception();
        }

        public ValueTask throwAConvertedToUnhandledAsync(Current current)
        {
            var a = new A();
            a.ConvertToUnhandled = true;
            throw a;
        }
    }
}
