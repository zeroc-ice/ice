// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using Demo;
using System.Threading;

public class DiscoverReplyI : DiscoverReplyDisp_
{
    public override void
    reply(Ice.ObjectPrx obj, Ice.Current current)
    {
        lock(this)
        {
            if(_obj == null)
            {
                _obj = obj;
            }
            Monitor.Pulse(this);
        }
    }

    public Ice.ObjectPrx
    waitReply(long timeout)
    {
        lock(this)
        {
            long end = System.DateTime.Now.Ticks / 1000 + timeout;
            while(_obj == null)
            {
                int delay = (int)(end - System.DateTime.Now.Ticks / 1000);
                if(delay > 0)
                {
                    Monitor.Wait(this, delay);
                }
                else
                {
                    break;
                }
            }
            return _obj;
        }
    }

    private Ice.ObjectPrx _obj;
}
