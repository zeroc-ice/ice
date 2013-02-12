// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
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
#if COMPACT
        _m.Lock();
        try
        {
            if(_obj == null)
            {
                _obj = obj;
            }
            _m.Notify();
        }
        finally
        {
            _m.Unlock();
        }
#else
        lock(this)
        {
            if(_obj == null)
            {
                _obj = obj;
            }
            Monitor.Pulse(this);
        }
#endif
    }

    public Ice.ObjectPrx
    waitReply(long timeout)
    {
#if COMPACT
        _m.Lock();
        try
        {
            long end = System.DateTime.Now.Ticks / 1000 + timeout;
            while(_obj == null)
            {
                int delay = (int)(end - System.DateTime.Now.Ticks / 1000);
                if(delay > 0)
                {
                    _m.TimedWait(delay);
                }
                else
                {
                    break;
                }
            }
            return _obj;
        }
        finally
        {
            _m.Unlock();
        }
#else
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
#endif
    }

    private Ice.ObjectPrx _obj;
#if COMPACT
    private readonly IceUtilInternal.Monitor _m = new IceUtilInternal.Monitor();
#endif
}
