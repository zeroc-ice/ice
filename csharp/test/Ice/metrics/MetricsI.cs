// Copyright (c) ZeroC, Inc.

using Test;

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

public sealed class MetricsI : MetricsDisp_
{
    public override void op(Ice.Current current)
    {
    }

    public override void fail(Ice.Current current) => current.con.abort();

    public override void opWithUserException(Ice.Current current) => throw new UserEx();

    public override void opWithRequestFailedException(Ice.Current current) => throw new Ice.ObjectNotExistException();

    public override void opWithLocalException(Ice.Current current) => throw new Ice.SyscallException(message: null);

    public override void opWithUnknownException(Ice.Current current) => throw new ArgumentOutOfRangeException();

    public override void opByteS(byte[] bs, Ice.Current current)
    {
    }

    public override Ice.ObjectPrx getAdmin(Ice.Current current) => current.adapter.getCommunicator().getAdmin();

    public override void shutdown(Ice.Current current) => current.adapter.getCommunicator().shutdown();
}
