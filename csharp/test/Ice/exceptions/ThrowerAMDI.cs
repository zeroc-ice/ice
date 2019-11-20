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
            public class ThrowerI : Test.Thrower
            {
                public ThrowerI()
                {
                }

                public Task
                shutdownAsync(Ice.Current current)
                {
                    current.adapter.getCommunicator().shutdown();
                    return null;
                }

                public Task<bool>
                supportsUndeclaredExceptionsAsync(Ice.Current current)
                {
                    return Task.FromResult(true);
                }

                public Task<bool>
                supportsAssertExceptionAsync(Ice.Current current)
                {
                    return Task.FromResult(false);
                }

                public Task
                throwAasAAsync(int a, Ice.Current current)
                {
                    throw new Test.A(a);
                }

                public Task
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

                public Task
                throwBasAAsync(int a, int b, Ice.Current current)
                {
                    //throw new B(a, b);
                    var s = new TaskCompletionSource<object>();
                    s.SetException(new Test.B(a, b));
                    return s.Task;
                }

                public Task
                throwBasBAsync(int a, int b, Ice.Current current)
                {
                    throw new Test.B(a, b);
                }

                public Task
                throwCasAAsync(int a, int b, int c, Ice.Current current)
                {
                    throw new Test.C(a, b, c);
                }

                public Task
                throwCasBAsync(int a, int b, int c, Ice.Current current)
                {
                    throw new Test.C(a, b, c);
                }

                public Task
                throwCasCAsync(int a, int b, int c, Ice.Current current)
                {
                    throw new Test.C(a, b, c);
                }

                public Task
                throwUndeclaredAAsync(int a, Ice.Current current)
                {
                    throw new Test.A(a);
                }

                public Task
                throwUndeclaredBAsync(int a, int b, Ice.Current current)
                {
                    throw new Test.B(a, b);
                }

                public Task
                throwUndeclaredCAsync(int a, int b, int c, Ice.Current current)
                {
                    throw new Test.C(a, b, c);
                }

                public Task
                throwLocalExceptionAsync(Ice.Current current)
                {
                    throw new Ice.TimeoutException();
                }

                public Task
                throwNonIceExceptionAsync(Ice.Current current)
                {
                    throw new System.Exception();
                }

                public Task
                throwAssertExceptionAsync(Ice.Current current)
                {
                    Debug.Assert(false);
                    return null;
                }

                public Task<byte[]>
                throwMemoryLimitExceptionAsync(byte[] seq, Ice.Current current)
                {
                    return Task.FromResult(new byte[1024 * 20]); // 20KB is over the configured 10KB message size max.
                }

                public Task
                throwLocalExceptionIdempotentAsync(Ice.Current current)
                {
                    throw new Ice.TimeoutException();
                }

                public Task
                throwAfterResponseAsync(Ice.Current current)
                {
                    // Only supported with callback based AMD API
                    return null;
                    //throw new Exception();
                }

                public Task
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
