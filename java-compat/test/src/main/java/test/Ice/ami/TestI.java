// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

package test.Ice.ami;

import test.Ice.ami.Test._TestIntfDisp;
import test.Ice.ami.Test.AMD_TestIntf_startDispatch;
import test.Ice.ami.Test.CloseMode;
import test.Ice.ami.Test.TestIntfException;
import test.Ice.ami.Test.PingReplyPrx;
import test.Ice.ami.Test.PingReplyPrxHelper;

public class TestI extends _TestIntfDisp
{
    TestI()
    {
    }

    @Override
    public void
    op(Ice.Current current)
    {
    }

    @Override
    public int
    opWithResult(Ice.Current current)
    {
        return 15;
    }

    @Override
    public void
    opWithUE(Ice.Current current)
        throws TestIntfException
    {
        throw new TestIntfException();
    }

    @Override
    public void
    opWithPayload(byte[] seq, Ice.Current current)
    {
    }

    @Override
    public synchronized void
    opBatch(Ice.Current current)
    {
        ++_batchCount;
        notify();
    }

    @Override
    public synchronized int
    opBatchCount(Ice.Current current)
    {
        return _batchCount;
    }

    @Override
    public boolean supportsAMD(Ice.Current current)
    {
        return true;
    }

    @Override
    public boolean supportsFunctionalTests(Ice.Current current)
    {
        return true;
    }

    @Override
    public boolean opBool(boolean b, Ice.Current current)
    {
        return b;
    }

    @Override
    public byte opByte(byte b, Ice.Current current)
    {
        return b;
    }

    @Override
    public short opShort(short s, Ice.Current current)
    {
        return s;
    }

    @Override
    public int opInt(int i, Ice.Current current)
    {
        return i;
    }

    @Override
    public long opLong(long l, Ice.Current current)
    {
        return l;
    }

    @Override
    public float opFloat(float f, Ice.Current current)
    {
        return f;
    }

    @Override
    public double opDouble(double d, Ice.Current current)
    {
        return d;
    }

    @Override
    public void pingBiDir(PingReplyPrx reply, Ice.Current current)
    {
        reply = PingReplyPrxHelper.uncheckedCast(reply.ice_fixed(current.con));
        Ice.AsyncResult result = reply.begin_reply();
        reply.end_reply(result);
    }

    @Override
    public synchronized boolean
    waitForBatch(int count, Ice.Current current)
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
    close(CloseMode mode, Ice.Current current)
    {
        current.con.close(Ice.ConnectionClose.valueOf(mode.value()));
    }

    @Override
    public void
    sleep(int ms, Ice.Current current)
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
    public synchronized void
    startDispatch_async(AMD_TestIntf_startDispatch cb, Ice.Current current)
    {
        if(_shutdown)
        {
            // Ignore, this can occur with the forcefull connection close test, shutdown can be dispatch
            // before start dispatch.
            cb.ice_response();
            return;
        }
        else if(_pending != null)
        {
            _pending.ice_response();
        }
        _pending = cb;
    }

    @Override
    public synchronized void
    finishDispatch(Ice.Current current)
    {
        if(_shutdown)
        {
            return;
        }
        else if(_pending != null) // Pending might not be set yet if startDispatch is dispatch out-of-order
        {
            _pending.ice_response();
            _pending = null;
        }
    }

    @Override
    public synchronized void
    shutdown(Ice.Current current)
    {
        _shutdown = true;
        if(_pending != null)
        {
            _pending.ice_response();
            _pending = null;
        }
        current.adapter.getCommunicator().shutdown();
    }

    private int _batchCount;
    private boolean _shutdown = false;
    private AMD_TestIntf_startDispatch _pending = null;
}
