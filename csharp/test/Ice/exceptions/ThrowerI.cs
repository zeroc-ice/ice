//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using Test;

namespace Ice.exceptions
{
    public sealed class Thrower : Test.IThrower
    {
        public void shutdown(Current current) => current.Adapter.Communicator.Shutdown();

        public bool supportsAssertException(Current current) => false;

        public void throwAasA(int a, Current current) => throw new Test.A(a);

        public void throwAorDasAorD(int a, Current current)
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

        public void throwBasA(int a, int b, Current current) => throwBasB(a, b, current);

        public void throwBasB(int a, int b, Current current) => throw new Test.B(a, b);

        public void throwCasA(int a, int b, int c, Current current) => throwCasC(a, b, c, current);

        public void throwCasB(int a, int b, int c, Current current) => throwCasC(a, b, c, current);

        public void throwCasC(int a, int b, int c, Current current) => throw new Test.C(a, b, c);

        public void throwLocalException(Current current) => throw new ConnectionTimeoutException();

        public void throwNonIceException(Current current) => throw new System.Exception();

        public void throwAssertException(Current current) => TestHelper.Assert(false);

        // 20KB is over the configured 10KB message size max.
        public ReadOnlyMemory<byte> throwMemoryLimitException(byte[] seq, Current current) => new byte[1024 * 20];

        public void throwLocalExceptionIdempotent(Current current) => throw new ConnectionTimeoutException();

        public void throwUndeclaredA(int a, Current current) => throw new Test.A(a);

        public void throwUndeclaredB(int a, int b, Current current) => throw new Test.B(a, b);

        public void throwUndeclaredC(int a, int b, int c, Current current) => throw new Test.C(a, b, c);

        public void throwAfterResponse(Current current)
        {
            //
            // Only relevant for AMD.
            //
        }

        //
        // Only relevant for AMD.
        //
        public void throwAfterException(Current current) => throw new Test.A();

        public void throwAConvertedToUnhandled(Current current)
        {
            var a = new Test.A();
            a.ConvertToUnhandled = true;
            throw a;
        }
    }
}
