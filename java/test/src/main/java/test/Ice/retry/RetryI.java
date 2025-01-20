// Copyright (c) ZeroC, Inc.

package test.Ice.retry;

import com.zeroc.Ice.ConnectionLostException;

import test.Ice.retry.Test.Retry;

public final class RetryI implements Retry {
    public RetryI() {}

    @Override
    public void op(boolean kill, com.zeroc.Ice.Current current) {
        if (kill) {
            if (current.con != null) {
                current.con.abort();
            } else {
                throw new ConnectionLostException();
            }
        }
    }

    @Override
    public int opIdempotent(int nRetry, com.zeroc.Ice.Current current) {
        if (nRetry < 0) {
            _counter = 0;
            return 0;
        }

        if (nRetry > _counter) {
            ++_counter;
            throw new ConnectionLostException();
        }

        int counter = _counter;
        _counter = 0;
        return counter;
    }

    @Override
    public void opNotIdempotent(com.zeroc.Ice.Current current) {
        throw new ConnectionLostException();
    }

    @Override
    public void sleep(int delay, com.zeroc.Ice.Current c) {
        while (true) {
            try {
                Thread.sleep(delay);
                break;
            } catch (InterruptedException ex) {
            }
        }
    }

    @Override
    public void shutdown(com.zeroc.Ice.Current current) {
        current.adapter.getCommunicator().shutdown();
    }

    private int _counter;
}
