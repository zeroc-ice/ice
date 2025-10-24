// Copyright (c) ZeroC, Inc.

using Test;

namespace Ice.metrics;

public sealed class ControllerI(Ice.ObjectAdapter adapter) : ControllerDisp_
{
    public override void hold(Ice.Current current)
    {
        _adapter.hold();
        _adapter.waitForHold();
    }

    public override void resume(Ice.Current current) => _adapter.activate();

    private readonly Ice.ObjectAdapter _adapter = adapter;
}

public sealed class MetricsI : AsyncMetricsDisp_
{
    public override Task opAsync(Ice.Current current) => Task.CompletedTask;

    public override Task failAsync(Ice.Current current)
    {
        current.con.abort();
        return Task.CompletedTask;
    }

    public override Task opWithUserExceptionAsync(Ice.Current current) => throw new UserEx();

    public override Task opWithRequestFailedExceptionAsync(Ice.Current current) =>
        throw new Ice.ObjectNotExistException();

    public override Task opWithLocalExceptionAsync(Ice.Current current) =>
        throw new Ice.SyscallException(message: null);

    public override Task opWithUnknownExceptionAsync(Ice.Current current) =>
        throw new ArgumentOutOfRangeException(nameof(current));

    public override Task opByteSAsync(byte[] bs, Ice.Current current) => Task.CompletedTask;

    public override Task<Ice.ObjectPrx> getAdminAsync(Ice.Current current) =>
        Task.FromResult(current.adapter.getCommunicator().getAdmin());

    public override Task shutdownAsync(Ice.Current current)
    {
        current.adapter.getCommunicator().shutdown();
        return Task.CompletedTask;
    }
}
