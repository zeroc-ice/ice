// Copyright (c) ZeroC, Inc.

package test.Ice.metrics;

import com.zeroc.Ice.Current;
import com.zeroc.Ice.ObjectNotExistException;
import com.zeroc.Ice.ObjectPrx;
import com.zeroc.Ice.SyscallException;

import test.Ice.metrics.Test.Metrics;
import test.Ice.metrics.Test.UserEx;

public final class MetricsI implements Metrics {
    public MetricsI() {
    }

    @Override
    public void op(Current current) {
    }

    @Override
    public void fail(Current current) {
        current.con.abort();
    }

    @Override
    public void opWithUserException(Current current) throws UserEx {
        throw new UserEx();
    }

    @Override
    public void opWithRequestFailedException(Current current) {
        throw new ObjectNotExistException();
    }

    @Override
    public void opWithLocalException(Current current) {
        throw new SyscallException(null);
    }

    @Override
    public void opWithUnknownException(Current current) {
        throw new IllegalArgumentException();
    }

    @Override
    public void opByteS(byte[] bs, Current current) {
    }

    @Override
    public ObjectPrx getAdmin(Current current) {
        return current.adapter.getCommunicator().getAdmin();
    }

    @Override
    public void shutdown(Current current) {
        current.adapter.getCommunicator().shutdown();
    }
}
