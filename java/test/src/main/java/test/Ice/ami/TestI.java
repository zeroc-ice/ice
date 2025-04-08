// Copyright (c) ZeroC, Inc.

package test.Ice.ami;

import com.zeroc.Ice.Current;
import com.zeroc.Ice.ObjectNotExistException;

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

    TestI() {
    }

    @Override
    public void op(Current current) {
    }

    @Override
    public int opWithResult(Current current) {
        return 15;
    }

    @Override
    public void opWithUE(Current current) throws TestIntfException {
        throw new TestIntfException();
    }

    @Override
    public void opWithPayload(byte[] seq, Current current) {
    }

    @Override
    public synchronized void opBatch(Current current) {
        ++_batchCount;
        notify();
    }

    @Override
    public synchronized int opBatchCount(Current current) {
        return _batchCount;
    }

    @Override
    public boolean supportsFunctionalTests(Current current) {
        return true;
    }

    @Override
    public boolean supportsBackPressureTests(Current current) {
        return true;
    }

    @Override
    public boolean opBool(boolean b, Current current) {
        return b;
    }

    @Override
    public byte opByte(byte b, Current current) {
        return b;
    }

    @Override
    public short opShort(short s, Current current) {
        return s;
    }

    @Override
    public int opInt(int i, Current current) {
        return i;
    }

    @Override
    public long opLong(long l, Current current) {
        return l;
    }

    @Override
    public float opFloat(float f, Current current) {
        return f;
    }

    @Override
    public double opDouble(double d, Current current) {
        return d;
    }

    @Override
    public CompletionStage<Void> pingBiDirAsync(PingReplyPrx reply, Current current) {
        reply = reply.ice_fixed(current.con);
        boolean expectSuccess = !current.ctx.containsKey("ONE");
        return reply.replyAsync()
            .handle(
                (r, ex) -> {
                    if (expectSuccess) {
                        test(ex == null);
                    } else {
                        test(ex instanceof ObjectNotExistException);
                    }
                    return null; // means success
                });
    }

    @Override
    public synchronized boolean waitForBatch(int count, Current current) {
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
    public void closeConnection(Current current) {
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
    public void abortConnection(Current current) {
        current.con.abort();
    }

    @Override
    public void sleep(int ms, Current current) {
        try {
            Thread.sleep(ms);
        } catch (InterruptedException ex) {
        }
    }

    @Override
    public synchronized CompletionStage<Void> startDispatchAsync(Current current) {
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
    public synchronized void finishDispatch(Current current) {
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
    public synchronized void shutdown(Current current) {
        _shutdown = true;
        if (_pending != null) {
            _pending.complete(null);
            _pending = null;
        }
        current.adapter.getCommunicator().shutdown();
    }

    private int _batchCount;
    private boolean _shutdown;
    private CompletableFuture<Void> _pending;
}
