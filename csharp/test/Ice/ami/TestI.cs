// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using Test;

public class TestI : TestIntfDisp_
{
    public TestI()
    {
    }

    override public void
    op(Ice.Current current)
    {
    }

    override public int
    opWithResult(Ice.Current current)
    {
        return 15;
    }

    override public void
    opWithUE(Ice.Current current)
    {
        throw new TestIntfException();
    }

    override public void
    opWithPayload(byte[] seq, Ice.Current current)
    {
    }

    override public void
    opBatch(Ice.Current current)
    {
        lock(this)
        {
            ++_batchCount;
            System.Threading.Monitor.Pulse(this);
        }
    }

    override public int
    opBatchCount(Ice.Current current)
    {
        lock(this)
        {
            return _batchCount;
        }
    }

    override public bool
    waitForBatch(int count, Ice.Current current)
    {
        lock(this)
        {
            while(_batchCount < count)
            {
                System.Threading.Monitor.Wait(this, 5000);
            }
            bool result = count == _batchCount;
            _batchCount = 0;
            return result;
        }
    }

    override public void
    close(bool force, Ice.Current current)
    {
        current.con.close(force);
    }

    override public void
    shutdown(Ice.Current current)
    {
        current.adapter.getCommunicator().shutdown();
    }

    override public bool
    supportsFunctionalTests(Ice.Current current)
    {
        return false;
    }

    private int _batchCount;
}

public class TestControllerI : TestIntfControllerDisp_
{
    override public void
    holdAdapter(Ice.Current current)
    {
        _adapter.hold();
    }

    override public void
    resumeAdapter(Ice.Current current)
    {
        _adapter.activate();
    }

    public
    TestControllerI(Ice.ObjectAdapter adapter)
    {
        _adapter = adapter;
    }

    private Ice.ObjectAdapter _adapter;
}
