// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.interrupt;

public class TestI extends test.Ice.interrupt.Test._TestIntfDisp
{
    TestI(TestControllerI controller)
    {
        _controller = controller;
    }

    @Override
    public void
    op(Ice.Current current)
    {
    }

    @Override
    public void
    opIdempotent(Ice.Current current)
    {
        throw new Ice.UnknownException();
    }

    @Override
    public void
    sleep(int to, Ice.Current current)
        throws test.Ice.interrupt.Test.InterruptedException
    {
        _controller.addUpcallThread();
        try
        {
            Thread.sleep(to);
        }
        catch(InterruptedException ex)
        {
            throw new test.Ice.interrupt.Test.InterruptedException();
        }
        finally
        {
            _controller.removeUpcallThread();
        }
    }

    @Override
    public void
    opWithPayload(byte[] seq, Ice.Current current)
    {
    }

    @Override
    public void
    shutdown(Ice.Current current)
    {
        current.adapter.getCommunicator().shutdown();
    }

    private TestControllerI _controller;
}
