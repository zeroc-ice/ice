// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.timeout;
import test.Ice.timeout.Test._TimeoutDisp;

class TimeoutI extends _TimeoutDisp
{
    @Override
    public void
    op(Ice.Current current)
    {
    }

    @Override
    public void
    sendData(byte[] seq, Ice.Current current)
    {
    }

    @Override
    public void
    sleep(int to, Ice.Current current)
    {
        try
        {
            Thread.sleep(to);
        }
        catch(InterruptedException ex)
        {
            System.err.println("sleep interrupted");
        }
    }
}
