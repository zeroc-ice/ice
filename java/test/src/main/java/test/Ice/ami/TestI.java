// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.ami;

import test.Ice.ami.Test.CloseMode;
import test.Ice.ami.Test.TestIntf;
import test.Ice.ami.Test.TestIntfException;

import java.util.concurrent.CompletionStage;
import java.util.concurrent.CompletableFuture;

public class TestI implements TestIntf
{
    TestI()
    {
    }

    @Override
    public void op(com.zeroc.Ice.Current current)
    {
    }

    @Override
    public int opWithResult(com.zeroc.Ice.Current current)
    {
        return 15;
    }

    @Override
    public void opWithUE(com.zeroc.Ice.Current current)
        throws TestIntfException
    {
        throw new TestIntfException();
    }

    @Override
    public void opWithPayload(byte[] seq, com.zeroc.Ice.Current current)
    {
    }

    @Override
    public synchronized void opBatch(com.zeroc.Ice.Current current)
    {
        ++_batchCount;
        notify();
    }

    @Override
    public synchronized int opBatchCount(com.zeroc.Ice.Current current)
    {
        return _batchCount;
    }

    @Override
    public boolean supportsAMD(com.zeroc.Ice.Current current)
    {
        return true;
    }

    @Override
    public boolean supportsFunctionalTests(com.zeroc.Ice.Current current)
    {
        return true;
    }

    @Override
    public boolean opBool(boolean b, com.zeroc.Ice.Current current)
    {
        return b;
    }

    @Override
    public byte opByte(byte b, com.zeroc.Ice.Current current)
    {
        return b;
    }

    @Override
    public short opShort(short s, com.zeroc.Ice.Current current)
    {
        return s;
    }

    @Override
    public int opInt(int i, com.zeroc.Ice.Current current)
    {
        return i;
    }

    @Override
    public long opLong(long l, com.zeroc.Ice.Current current)
    {
        return l;
    }

    @Override
    public float opFloat(float f, com.zeroc.Ice.Current current)
    {
        return f;
    }

    @Override
    public double opDouble(double d, com.zeroc.Ice.Current current)
    {
        return d;
    }

    @Override
    public synchronized boolean
    waitForBatch(int count, com.zeroc.Ice.Current current)
    {
        while(_batchCount < count)
        {
            try
            {
                wait(5000);
            }
            catch(InterruptedException ex)
            {
            }
        }
        boolean result = count == _batchCount;
        _batchCount = 0;
        return result;
    }

    @Override
    public void
    close(CloseMode mode, com.zeroc.Ice.Current current)
    {
        current.con.close(com.zeroc.Ice.ConnectionClose.valueOf(mode.value()));
    }

    @Override
    public void
    sleep(int ms, com.zeroc.Ice.Current current)
    {
        try
        {
            Thread.sleep(ms);
        }
        catch(InterruptedException ex)
        {
        }
    }

    @Override
    public synchronized CompletionStage<Void>
    startDispatchAsync(com.zeroc.Ice.Current current)
    {
        CompletableFuture<Void> f = new CompletableFuture<>();
        _pending.add(f);
        return f;
    }

    @Override
    public synchronized void
    finishDispatch(com.zeroc.Ice.Current current)
    {
        for(CompletableFuture<Void> f : _pending)
        {
            f.complete(null);
        }
        _pending.clear();
    }

    @Override
    public synchronized void
    shutdown(com.zeroc.Ice.Current current)
    {
        //
        // Just in case a request arrived late.
        //
        for(CompletableFuture<Void> f : _pending)
        {
            f.complete(null);
        }
        current.adapter.getCommunicator().shutdown();
    }

    private int _batchCount;
    private java.util.List<CompletableFuture<Void>> _pending = new java.util.LinkedList<>();
}
