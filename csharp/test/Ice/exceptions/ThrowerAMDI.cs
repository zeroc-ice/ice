// Copyright (c) ZeroC, Inc.

using System.Diagnostics;

namespace Ice.exceptions.AMD;

public class ThrowerI : Test.ThrowerDisp_
{
    public ThrowerI()
    {
    }

    public override Task
    shutdownAsync(Current current)
    {
        current.adapter.getCommunicator().shutdown();
        return Task.CompletedTask;
    }

    public override Task<bool>
    supportsUndeclaredExceptionsAsync(Current current) => Task.FromResult(true);

    public override Task<bool>
    supportsAssertExceptionAsync(Current current) => Task.FromResult(false);

    public override Task
    throwAasAAsync(int a, Current current) => throw new Test.A(a);

    public override Task
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

    public override Task
    throwBasAAsync(int a, int b, Current current)
    {
        var s = new TaskCompletionSource<object>();
        s.SetException(new Test.B(a, b));
        return s.Task;
    }

    public override Task
    throwBasBAsync(int a, int b, Current current) => throw new Test.B(a, b);

    public override Task
    throwCasAAsync(int a, int b, int c, Current current) => throw new Test.C(a, b, c);

    public override Task
    throwCasBAsync(int a, int b, int c, Current current) => throw new Test.C(a, b, c);

    public override Task
    throwCasCAsync(int a, int b, int c, Current current) => throw new Test.C(a, b, c);

    public override Task
    throwUndeclaredAAsync(int a, Current current) => throw new Test.A(a);

    public override Task
    throwUndeclaredBAsync(int a, int b, Current current) => throw new Test.B(a, b);

    public override Task
    throwUndeclaredCAsync(int a, int b, int c, Current current) => throw new Test.C(a, b, c);

    public override Task
    throwLocalExceptionAsync(Current current) => throw new TimeoutException();

    public override Task
    throwNonIceExceptionAsync(Current current) => throw new System.Exception();

    public override Task
    throwAssertExceptionAsync(Current current)
    {
        Debug.Assert(false);
        return Task.CompletedTask;
    }

    // 20KB is over the configured 10KB message size max.
    public override Task<byte[]>
    throwMemoryLimitExceptionAsync(byte[] seq, Current current) => Task.FromResult(new byte[1024 * 20]);

    public override Task
    throwLocalExceptionIdempotentAsync(Current current) => throw new TimeoutException();

    public override Task throwDispatchExceptionAsync(byte replyStatus, Current current) =>
        throw new DispatchException((ReplyStatus)replyStatus);

    // Only supported with callback based AMD API
    public override Task
    throwAfterResponseAsync(Current current) => Task.CompletedTask;

    // Only supported with callback based AMD API
    public override Task
    throwAfterExceptionAsync(Current current) => throw new Test.A();
}
