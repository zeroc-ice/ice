//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using Test;
using System.Threading.Tasks;

namespace Ice
{
    namespace exceptions
    {
        namespace AMD
        {
            public class ThrowerI : Test.IThrower
            {
                public ThrowerI()
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
                throwAasAAsync(int a, Current current) => throw new Test.A(a);

                public ValueTask
                throwAorDasAorDAsync(int a, Current current)
                {
                    if (a > 0)
                    {
                        throw new Test.A(a);
                    }
                    else
                    {
                        throw new Test.D(a);
                    }
                }

                public ValueTask
                throwBasAAsync(int a, int b, Current current)
                {
                    //throw new B(a, b);
                    var s = new TaskCompletionSource<object>();
                    s.SetException(new Test.B(a, b));
                    return new ValueTask(s.Task);
                }

                public ValueTask
                throwBasBAsync(int a, int b, Current current) => throw new Test.B(a, b);

                public ValueTask
                throwCasAAsync(int a, int b, int c, Current current) => throw new Test.C(a, b, c);

                public ValueTask
                throwCasBAsync(int a, int b, int c, Current current) => throw new Test.C(a, b, c);

                public ValueTask
                throwCasCAsync(int a, int b, int c, Current current) => throw new Test.C(a, b, c);

                public ValueTask
                throwUndeclaredAAsync(int a, Current current) => throw new Test.A(a);

                public ValueTask
                throwUndeclaredBAsync(int a, int b, Current current) => throw new Test.B(a, b);

                public ValueTask
                throwUndeclaredCAsync(int a, int b, int c, Current current) => throw new Test.C(a, b, c);

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

                public ValueTask<byte[]>
                throwMemoryLimitExceptionAsync(byte[] seq, Current current) =>
                    new ValueTask<byte[]>(new byte[1024 * 20]); // 20KB is over the configured 10KB message size max.

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
                    throw new Test.A();
                    //throw new Exception();
                }

                public ValueTask throwAConvertedToUnhandledAsync(Current current)
                {
                    var a = new Test.A();
                    a.ConvertToUnhandled = true;
                    throw a;
                }
            }
        }
    }
}
