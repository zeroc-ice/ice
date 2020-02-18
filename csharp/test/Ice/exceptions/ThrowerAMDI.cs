//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Diagnostics;
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
                shutdownAsync(Ice.Current current)
                {
                    current.Adapter.Communicator.Shutdown();
                    return new ValueTask(Task.CompletedTask);
                }

                public ValueTask<bool>
                supportsUndeclaredExceptionsAsync(Ice.Current current)
                {
                    return new ValueTask<bool>(true);
                }

                public ValueTask<bool>
                supportsAssertExceptionAsync(Ice.Current current)
                {
                    return new ValueTask<bool>(false);
                }

                public ValueTask
                throwAasAAsync(int a, Ice.Current current)
                {
                    throw new Test.A(a);
                }

                public ValueTask
                throwAorDasAorDAsync(int a, Ice.Current current)
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
                throwBasAAsync(int a, int b, Ice.Current current)
                {
                    //throw new B(a, b);
                    var s = new TaskCompletionSource<object>();
                    s.SetException(new Test.B(a, b));
                    return new ValueTask(s.Task);
                }

                public ValueTask
                throwBasBAsync(int a, int b, Ice.Current current)
                {
                    throw new Test.B(a, b);
                }

                public ValueTask
                throwCasAAsync(int a, int b, int c, Ice.Current current)
                {
                    throw new Test.C(a, b, c);
                }

                public ValueTask
                throwCasBAsync(int a, int b, int c, Ice.Current current)
                {
                    throw new Test.C(a, b, c);
                }

                public ValueTask
                throwCasCAsync(int a, int b, int c, Ice.Current current)
                {
                    throw new Test.C(a, b, c);
                }

                public ValueTask
                throwUndeclaredAAsync(int a, Ice.Current current)
                {
                    throw new Test.A(a);
                }

                public ValueTask
                throwUndeclaredBAsync(int a, int b, Ice.Current current)
                {
                    throw new Test.B(a, b);
                }

                public ValueTask
                throwUndeclaredCAsync(int a, int b, int c, Ice.Current current)
                {
                    throw new Test.C(a, b, c);
                }

                public ValueTask
                throwLocalExceptionAsync(Ice.Current current)
                {
                    throw new Ice.TimeoutException();
                }

                public ValueTask
                throwNonIceExceptionAsync(Ice.Current current)
                {
                    throw new System.Exception();
                }

                public ValueTask
                throwAssertExceptionAsync(Ice.Current current)
                {
                    Debug.Assert(false);
                    return new ValueTask(Task.CompletedTask);
                }

                public ValueTask<byte[]>
                throwMemoryLimitExceptionAsync(byte[] seq, Ice.Current current)
                {
                    return new ValueTask<byte[]>(new byte[1024 * 20]); // 20KB is over the configured 10KB message size max.
                }

                public ValueTask
                throwLocalExceptionIdempotentAsync(Ice.Current current)
                {
                    throw new Ice.TimeoutException();
                }

                public ValueTask
                throwAfterResponseAsync(Ice.Current current)
                {
                    // Only supported with callback based AMD API
                    return new ValueTask(Task.CompletedTask);
                    //throw new Exception();
                }

                public ValueTask
                throwAfterExceptionAsync(Ice.Current current)
                {
                    // Only supported with callback based AMD API
                    throw new Test.A();
                    //throw new Exception();
                }
            }
        }
    }
}
