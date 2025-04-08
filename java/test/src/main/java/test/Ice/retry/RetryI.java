// Copyright (c) ZeroC, Inc.

package test.Ice.retry;

import com.zeroc.Ice.ConnectionLostException;
import com.zeroc.Ice.Current;

import test.Ice.retry.Test.Retry;

public final class RetryI implements Retry {
    public RetryI() {
    }

    @Override
    public void op(boolean kill, Current current) {
        if (kill) {
            if (current.con != null) {
                current.con.abort();
            } else {
                throw new ConnectionLostException();
            }
        }
    }

    @Override
    public int opIdempotent(int nRetry, Current current) {
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
    public void opNotIdempotent(Current current) {
        throw new ConnectionLostException();
    }

    @Override
    public void sleep(int delay, Current c) {
        while (true) {
            try {
                Thread.sleep(delay);
                break;
            } catch (InterruptedException ex) {
            }
        }
    }

    @Override
    public void shutdown(Current current) {
        current.adapter.getCommunicator().shutdown();
    }

    private int _counter;
}
