// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Diagnostics;
using Test;

public class ThrowerI : ThrowerDisp_
{
    public ThrowerI()
    {
    }

    public override void shutdownAsync(Action response, Action<Exception> exception, Ice.Current current)
    {
        current.adapter.getCommunicator().shutdown();
        response();
    }

    public override void
    supportsUndeclaredExceptionsAsync(Action<bool> response, Action<Exception> exception, Ice.Current current)
    {
        response(true);
    }

    public override void
    supportsAssertExceptionAsync(Action<bool> response, Action<Exception> exception, Ice.Current current)
    {
        response(false);
    }

    public override void throwAasAAsync(int a, Action response, Action<Exception> exception, Ice.Current current)
    {
        exception(new A(a));
    }

    public override void throwAorDasAorDAsync(int a, Action response, Action<Exception> exception, Ice.Current current)
    {
        if(a > 0)
        {
            exception(new A(a));
        }
        else
        {
            exception(new D(a));
        }
    }

    public override void
    throwBasAAsync(int a, int b, Action response, Action<Exception> exception, Ice.Current current)
    {
        exception(new B(a, b));
    }

    public override void
    throwBasBAsync(int a, int b, Action response, Action<Exception> exception, Ice.Current current)
    {
        exception(new B(a, b));
    }

    public override void
    throwCasAAsync(int a, int b, int c, Action response, Action<Exception> exception, Ice.Current current)
    {
        exception(new C(a, b, c));
    }

    public override void
    throwCasBAsync(int a, int b, int c, Action response, Action<Exception> exception, Ice.Current current)
    {
        exception(new C(a, b, c));
    }

    public override void
    throwCasCAsync(int a, int b, int c, Action response, Action<Exception> exception, Ice.Current current)
    {
        exception(new C(a, b, c));
    }

    public override void
    throwUndeclaredAAsync(int a, Action response, Action<Exception> exception, Ice.Current current)
    {
        exception(new A(a));
    }

    public override void
    throwUndeclaredBAsync(int a, int b, Action response, Action<Exception> exception, Ice.Current current)
    {
        exception(new B(a, b));
    }

    public override void
    throwUndeclaredCAsync(int a, int b, int c, Action response, Action<Exception> exception, Ice.Current current)
    {
        exception(new C(a, b, c));
    }

    public override void
    throwLocalExceptionAsync(Action response, Action<Exception> exception, Ice.Current current)
    {
        exception(new Ice.TimeoutException());
    }

    public override void
    throwNonIceExceptionAsync(Action response, Action<Exception> exception, Ice.Current current)
    {
        throw new Exception();
    }

    public override void
    throwAssertExceptionAsync(Action response, Action<Exception> exception, Ice.Current current)
    {
        Debug.Assert(false);
    }

    public override void
    throwMemoryLimitExceptionAsync(byte[] seq, Action<byte[]> response, Action<Exception> exception,
                                   Ice.Current current)
    {
        response(new byte[1024 * 20]); // 20KB is over the configured 10KB message size max.
    }

    public override void
    throwLocalExceptionIdempotentAsync(Action response, Action<Exception> exception, Ice.Current current)
    {
        throw new Ice.TimeoutException();
    }
    
    public override void
    throwAfterResponseAsync(Action response, Action<Exception> exception, Ice.Current current)
    {
        response();
        throw new Exception();
    }

    public override void
    throwAfterExceptionAsync(Action response, Action<Exception> exception, Ice.Current current)
    {
        exception(new A());
        throw new Exception();
    }
}
