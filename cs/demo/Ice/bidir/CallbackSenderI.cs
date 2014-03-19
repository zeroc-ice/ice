// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using Demo;
using System;
using System.Collections;

class CallbackSenderI : CallbackSenderDisp_
{
    public CallbackSenderI(Ice.Communicator communicator)
    {
        _communicator = communicator;
        _destroy = false;
        _clients = new ArrayList();
    }

    public void destroy()
    {
#if COMPACT
        _m.Lock();
        try
        {
            System.Console.Out.WriteLine("destroying callback sender");
            _destroy = true;
            
            _m.Notify();
        }
        finally
        {
            _m.Unlock();
        }
#else
        lock(this)
        {
            System.Console.Out.WriteLine("destroying callback sender");
            _destroy = true;
            
            System.Threading.Monitor.Pulse(this);
        }
#endif
    }

    public override void addClient(Ice.Identity ident, Ice.Current current)
    {
#if COMPACT
        _m.Lock();
        try
        {
            System.Console.Out.WriteLine("adding client `" + _communicator.identityToString(ident) + "'");

            Ice.ObjectPrx @base = current.con.createProxy(ident);
            CallbackReceiverPrx client = CallbackReceiverPrxHelper.uncheckedCast(@base);
            _clients.Add(client);
        }
        finally
        {
            _m.Unlock();
        }
#else
        lock(this)
        {
            System.Console.Out.WriteLine("adding client `" + _communicator.identityToString(ident) + "'");

            Ice.ObjectPrx @base = current.con.createProxy(ident);
            CallbackReceiverPrx client = CallbackReceiverPrxHelper.uncheckedCast(@base);
            _clients.Add(client);
        }
#endif
    }

    public void Run()
    {
        int num = 0;
        while(true)
        {
            ArrayList clients;
#if COMPACT
            _m.Lock();
            try
            {
                _m.TimedWait(2000);
                if(_destroy)
                {
                    break;
                }

                clients = new ArrayList(_clients);
            }
            finally
            {
                _m.Unlock();
            }
#else
            lock(this)
            {
                System.Threading.Monitor.Wait(this, 2000);
                if(_destroy)
                {
                    break;
                }

                clients = new ArrayList(_clients);
            }
#endif

            if(clients.Count > 0)
            {
                ++num;
                foreach(CallbackReceiverPrx c in clients)
                {
                    try
                    {
                        c.callback(num);
                    }
                    catch(Ice.LocalException ex)
                    {
                        Console.Error.WriteLine("removing client `" +
                                                _communicator.identityToString(c.ice_getIdentity()) + "':\n" + ex);

#if COMPACT
                        _m.Lock();
                        try
                        {
                            _clients.Remove(c);
                        }
                        finally
                        {
                            _m.Unlock();
                        }
#else
                        lock(this)
                        {
                            _clients.Remove(c);
                        }
#endif
                    }
                }
            }
        }
    }

    private Ice.Communicator _communicator;
    private bool _destroy;
    private ArrayList _clients;
#if COMPACT
    private readonly IceUtilInternal.Monitor _m = new IceUtilInternal.Monitor();
#endif
}
