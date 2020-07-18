//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using Test;

namespace ZeroC.Ice.Test.Exceptions
{
    public sealed class Thrower : IThrower
    {
        public void Shutdown(Current current) => current.Adapter.Communicator.ShutdownAsync();

        public bool SupportsAssertException(Current current) => false;

        public void ThrowAasA(int a, Current current) => throw new A(a);

        public void ThrowAorDasAorD(int a, Current current)
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

        public void ThrowBasA(int a, int b, Current current) => ThrowBasB(a, b, current);

        public void ThrowBasB(int a, int b, Current current) => throw new B(a, b);

        public void ThrowCasA(int a, int b, int c, Current current) => ThrowCasC(a, b, c, current);

        public void ThrowCasB(int a, int b, int c, Current current) => ThrowCasC(a, b, c, current);

        public void ThrowCasC(int a, int b, int c, Current current) => throw new C(a, b, c);

        public void ThrowLocalException(Current current) => throw new ConnectionTimeoutException();

        public void ThrowNonIceException(Current current) => throw new System.Exception();

        public void ThrowAssertException(Current current) => TestHelper.Assert(false);

        // 20KB is over the configured 10KB message size max.
        public ReadOnlyMemory<byte> ThrowMemoryLimitException(byte[] seq, Current current) => new byte[1024 * 20];

        public void ThrowLocalExceptionIdempotent(Current current) => throw new ConnectionTimeoutException();

        public void ThrowUndeclaredA(int a, Current current) => throw new A(a);

        public void ThrowUndeclaredB(int a, int b, Current current) => throw new B(a, b);

        public void ThrowUndeclaredC(int a, int b, int c, Current current) => throw new C(a, b, c);

        public void ThrowAfterResponse(Current current)
        {
            //
            // Only relevant for AMD.
            //
        }

        //
        // Only relevant for AMD.
        //
        public void ThrowAfterException(Current current) => throw new A();

        public void ThrowAConvertedToUnhandled(Current current)
        {
            var a = new A();
            a.ConvertToUnhandled = true;
            throw a;
        }
    }
}
