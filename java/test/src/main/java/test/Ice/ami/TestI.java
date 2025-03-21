// Copyright (c) ZeroC, Inc.

package test.Ice.ami;

import test.Ice.ami.Test.PingReplyPrx;
import test.Ice.ami.Test.TestIntf;
import test.Ice.ami.Test.TestIntfException;

import java.util.concurrent.CompletableFuture;
import java.util.concurrent.CompletionStage;

public class TestI implements TestIntf {
    private static void test(boolean b) {
        if (!b) {
            new Throwable().printStackTrace();
            //
            // Exceptions raised by callbacks are swallowed by CompletableFuture.
            //
            throw new RuntimeException();
        }
    }

    TestI() {}

    @Override
    public void op(com.zeroc.Ice.Current current) {}

    @Override
    public int opWithResult(com.zeroc.Ice.Current current) {
        return 15;
    }

    @Override
    public void opWithUE(com.zeroc.Ice.Current current) throws TestIntfException {
        throw new TestIntfException();
    }

    @Override
    public void opWithPayload(byte[] seq, com.zeroc.Ice.Current current) {}

    @Override
    public synchronized void opBatch(com.zeroc.Ice.Current current) {
        ++_batchCount;
        notify();
    }

    @Override
    public synchronized int opBatchCount(com.zeroc.Ice.Current current) {
        return _batchCount;
    }

    @Override
    public boolean supportsFunctionalTests(com.zeroc.Ice.Current current) {
        return true;
    }

    @Override
    public boolean supportsBackPressureTests(com.zeroc.Ice.Current current) {
        return true;
    }

    @Override
    public boolean opBool(boolean b, com.zeroc.Ice.Current current) {
        return b;
    }

    @Override
    public byte opByte(byte b, com.zeroc.Ice.Current current) {
        return b;
    }

    @Override
    public short opShort(short s, com.zeroc.Ice.Current current) {
        return s;
    }

    @Override
    public int opInt(int i, com.zeroc.Ice.Current current) {
        return i;
    }

    @Override
    public long opLong(long l, com.zeroc.Ice.Current current) {
        return l;
    }

    @Override
    public float opFloat(float f, com.zeroc.Ice.Current current) {
        return f;
    }

    @Override
    public double opDouble(double d, com.zeroc.Ice.Current current) {
        return d;
    }

    @Override
    public CompletionStage<Void> pingBiDirAsync(PingReplyPrx reply, com.zeroc.Ice.Current current) {
        reply = reply.ice_fixed(current.con);
        boolean expectSuccess = !current.ctx.containsKey("ONE");
        return reply.replyAsync()
                .handle(
                        (r, ex) -> {
                            if (expectSuccess) {
                                test(ex == null);
                            } else {
                                test(ex instanceof com.zeroc.Ice.ObjectNotExistException);
                            }
                            return null; // means success
                        });
    }

    @Override
    public synchronized boolean waitForBatch(int count, com.zeroc.Ice.Current current) {
        while (_batchCount < count) {
            try {
                wait(5000);
            } catch (InterruptedException ex) {
            }
        }
        boolean result = count == _batchCount;
        _batchCount = 0;
        return result;
    }

    @Override
    public void closeConnection(com.zeroc.Ice.Current current) {
        // We can't wait for the connection to be closed - this would cause a self dead-lock. So
        // instead we just initiate the closure in the background.
        CompletableFuture.runAsync(
                () -> {
                    try {
                        current.con.close();
                    } catch (Exception e) {
                        // make sure the closure is graceful
                        System.err.println("********** Connection.close failed: " + e);
                        test(false);
                    }
                });
    }

    @Override
    public void abortConnection(com.zeroc.Ice.Current current) {
        current.con.abort();
    }

    @Override
    public void sleep(int ms, com.zeroc.Ice.Current current) {
        try {
            Thread.sleep(ms);
        } catch (InterruptedException ex) {
        }
    }

    @Override
    public synchronized CompletionStage<Void> startDispatchAsync(com.zeroc.Ice.Current current) {
        if (_shutdown) {
            // Ignore, this can occur with the forceful connection close test, shutdown can be
            // dispatch before start dispatch.
            CompletableFuture<Void> v = new CompletableFuture<>();
            v.complete(null);
            return v;
        } else if (_pending != null) {
            _pending.complete(null);
        }
        _pending = new CompletableFuture<>();
        return _pending;
    }

    @Override
    public synchronized void finishDispatch(com.zeroc.Ice.Current current) {
        if (_shutdown) {
            return;
        } else if (_pending
                != null) // Pending might not be set yet if startDispatch is dispatch out-of-order
        {
            _pending.complete(null);
            _pending = null;
        }
    }

    @Override
    public synchronized void shutdown(com.zeroc.Ice.Current current) {
        _shutdown = true;
        if (_pending != null) {
            _pending.complete(null);
            _pending = null;
        }
        current.adapter.getCommunicator().shutdown();
    }

    private int _batchCount;
    private boolean _shutdown = false;
    private CompletableFuture<Void> _pending = null;
}
