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
using System.Threading.Tasks;
using Test;

public class ThrowerI : ThrowerDisp_
{
    public ThrowerI()
    {
    }

    public override Task
    shutdownAsync(Ice.Current current)
    {
        current.adapter.getCommunicator().shutdown();
        return null;
    }

    public override Task<bool>
    supportsUndeclaredExceptionsAsync(Ice.Current current)
    {
        return Task.FromResult<bool>(true);
    }

    public override Task<bool>
    supportsAssertExceptionAsync(Ice.Current current)
    {
        return Task.FromResult<bool>(false);
    }

    public override Task
    throwAasAAsync(int a, Ice.Current current)
    {
        throw new A(a);
    }

    public override Task
    throwAorDasAorDAsync(int a, Ice.Current current)
    {
        if(a > 0)
        {
            throw new A(a);
        }
        else
        {
            throw new D(a);
        }
    }

    public override Task
    throwBasAAsync(int a, int b, Ice.Current current)
    {
        //throw new B(a, b);
        var s = new TaskCompletionSource<object>();
        s.SetException(new B(a,b));
        return s.Task;
    }

    public override Task
    throwBasBAsync(int a, int b, Ice.Current current)
    {
        throw new B(a, b);
    }

    public override Task
    throwCasAAsync(int a, int b, int c, Ice.Current current)
    {
        throw new C(a, b, c);
    }

    public override Task
    throwCasBAsync(int a, int b, int c, Ice.Current current)
    {
        throw new C(a, b, c);
    }

    public override Task
    throwCasCAsync(int a, int b, int c, Ice.Current current)
    {
        throw new C(a, b, c);
    }

    public override Task
    throwUndeclaredAAsync(int a, Ice.Current current)
    {
        throw new A(a);
    }

    public override Task
    throwUndeclaredBAsync(int a, int b, Ice.Current current)
    {
        throw new B(a, b);
    }

    public override Task
    throwUndeclaredCAsync(int a, int b, int c, Ice.Current current)
    {
        throw new C(a, b, c);
    }

    public override Task
    throwLocalExceptionAsync(Ice.Current current)
    {
        throw new Ice.TimeoutException();
    }

    public override Task
    throwNonIceExceptionAsync(Ice.Current current)
    {
        throw new Exception();
    }

    public override Task
    throwAssertExceptionAsync(Ice.Current current)
    {
        Debug.Assert(false);
        return null;
    }

    public override Task<byte[]>
    throwMemoryLimitExceptionAsync(byte[] seq, Ice.Current current)
    {
        return Task.FromResult<byte[]>(new byte[1024 * 20]); // 20KB is over the configured 10KB message size max.
    }

    public override Task
    throwLocalExceptionIdempotentAsync(Ice.Current current)
    {
        throw new Ice.TimeoutException();
    }

    public override Task
    throwAfterResponseAsync(Ice.Current current)
    {
        // Only supported with callback based AMD API
        return null;
        //throw new Exception();
    }

    public override Task
    throwAfterExceptionAsync(Ice.Current current)
    {
        // Only supported with callback based AMD API
        throw new A();
        //throw new Exception();
    }
}
