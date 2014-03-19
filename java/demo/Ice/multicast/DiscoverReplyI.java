// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import Demo.*;

public class DiscoverReplyI extends _DiscoverReplyDisp
{
    public synchronized void
    reply(Ice.ObjectPrx obj, Ice.Current current)
    {
        if(_obj == null)
        {
            _obj = obj;
        }
        notify();
    }

    public synchronized Ice.ObjectPrx
    waitReply(long timeout)
    {
        long end = System.currentTimeMillis() + timeout;
        while(_obj == null)
        {
            long delay = end - System.currentTimeMillis();
            if(delay > 0)
            {
                try
                {
                    wait(delay);
                }
                catch(java.lang.InterruptedException ex)
                {
                }
            }
            else
            {
                break;
            }
        }
        return _obj;
    }

    private Ice.ObjectPrx _obj;
}
