//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Diagnostics;

namespace Ice
{
    namespace exceptions
    {
        public sealed class ThrowerI : Test.Thrower
        {
            public ThrowerI()
            {
            }

            public void shutdown(Ice.Current current)
            {
                current.adapter.getCommunicator().shutdown();
            }

            public bool supportsUndeclaredExceptions(Ice.Current current)
            {
                return true;
            }

            public bool supportsAssertException(Ice.Current current)
            {
                return false;
            }

            public void throwAasA(int a, Ice.Current current)
            {
                var ex = new Test.A();
                ex.aMem = a;
                throw ex;
            }

            public void throwAorDasAorD(int a, Ice.Current current)
            {
                if (a > 0)
                {
                    var ex = new Test.A();
                    ex.aMem = a;
                    throw ex;
                }
                else
                {
                    var ex = new Test.D();
                    ex.dMem = a;
                    throw ex;
                }
            }

            public void throwBasA(int a, int b, Ice.Current current)
            {
                throwBasB(a, b, current);
            }

            public void throwBasB(int a, int b, Ice.Current current)
            {
                var ex = new Test.B();
                ex.aMem = a;
                ex.bMem = b;
                throw ex;
            }

            public void throwCasA(int a, int b, int c, Ice.Current current)
            {
                throwCasC(a, b, c, current);
            }

            public void throwCasB(int a, int b, int c, Ice.Current current)
            {
                throwCasC(a, b, c, current);
            }

            public void throwCasC(int a, int b, int c, Ice.Current current)
            {
                var ex = new Test.C();
                ex.aMem = a;
                ex.bMem = b;
                ex.cMem = c;
                throw ex;
            }

            public void throwLocalException(Ice.Current current)
            {
                throw new Ice.TimeoutException();
            }

            public void throwNonIceException(Ice.Current current)
            {
                throw new System.Exception();
            }

            public void throwAssertException(Ice.Current current)
            {
                Debug.Assert(false);
            }

            public byte[] throwMemoryLimitException(byte[] seq, Ice.Current current)
            {
                return new byte[1024 * 20]; // 20KB is over the configured 10KB message size max.
            }

            public void throwLocalExceptionIdempotent(Ice.Current current)
            {
                throw new Ice.TimeoutException();
            }

            public void throwUndeclaredA(int a, Ice.Current current)
            {
                var ex = new Test.A();
                ex.aMem = a;
                throw ex;
            }

            public void throwUndeclaredB(int a, int b, Ice.Current current)
            {
                var ex = new Test.B();
                ex.aMem = a;
                ex.bMem = b;
                throw ex;
            }

            public void throwUndeclaredC(int a, int b, int c, Ice.Current current)
            {
                var ex = new Test.C();
                ex.aMem = a;
                ex.bMem = b;
                ex.cMem = c;
                throw ex;
            }

            public void throwAfterResponse(Ice.Current current)
            {
                //
                // Only relevant for AMD.
                //
            }

            public void throwAfterException(Ice.Current current)
            {
                //
                // Only relevant for AMD.
                //
                throw new Test.A();
            }
        }
    }
}
