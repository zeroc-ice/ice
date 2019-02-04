//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

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
