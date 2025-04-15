// Copyright (c) ZeroC, Inc.

package test.Ice.timeout;

import com.zeroc.Ice.Current;

import test.Ice.timeout.Test.Timeout;

class TimeoutI implements Timeout {
    @Override
    public void op(Current current) {}

    @Override
    public void sendData(byte[] seq, Current current) {}

    @Override
    public void sleep(int to, Current current) {
        try {
            Thread.sleep(to);
        } catch (InterruptedException ex) {
            System.err.println("sleep interrupted");
        }
    }
}
