//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Threading.Tasks;
using Test;

public sealed class Controller : IController
{
    public Controller(Func<Ice.ObjectAdapter> factory)
    {
        _factory = factory;
        _adapter = factory();
        _adapter.Activate();
    }

    public void hold(Ice.Current current)
    {
        _adapter.Destroy();
        _adapter = _factory(); // Recreate the adapter without activating it
    }

    public void resume(Ice.Current current) => _adapter.Activate();

    private readonly Func<Ice.ObjectAdapter> _factory;
    private Ice.ObjectAdapter _adapter;
};

public sealed class Metrics : IMetrics
{
    public void op(Ice.Current current)
    {
    }

    public void fail(Ice.Current current) => current.Connection!.Close(Ice.ConnectionClose.Forcefully);

    public void opWithUserException(Ice.Current current) => throw new UserEx();

    public void opWithRequestFailedException(Ice.Current current) => throw new Ice.ObjectNotExistException(current);

    public void opWithLocalException(Ice.Current current) => throw new Ice.InvalidConfigurationException("fake");

    public void opWithUnknownException(Ice.Current current) => throw new ArgumentOutOfRangeException();

    public void opByteS(byte[] bs, Ice.Current current)
    {
    }

    public Ice.IObjectPrx? getAdmin(Ice.Current current) => current.Adapter.Communicator.GetAdmin();

    public void shutdown(Ice.Current current) => current.Adapter.Communicator.Shutdown();
}
