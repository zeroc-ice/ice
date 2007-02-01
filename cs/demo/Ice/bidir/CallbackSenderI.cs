// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
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
        _num = 0;
        _clients = new ArrayList();
    }

    public void destroy()
    {
        lock(this)
        {
            System.Console.Out.WriteLine("destroying callback sender");
            _destroy = true;
            
            System.Threading.Monitor.Pulse(this);
        }
    }

    public override void addClient(Ice.Identity ident, Ice.Current current)
    {
        lock(this)
        {
            System.Console.Out.WriteLine("adding client `" + _communicator.identityToString(ident) + "'");

            Ice.ObjectPrx @base = current.con.createProxy(ident);
            CallbackReceiverPrx client = CallbackReceiverPrxHelper.uncheckedCast(@base);
            _clients.Add(client);
        }
    }

    public void Run()
    {
        lock(this)
        {
            while(!_destroy)
            {
                System.Threading.Monitor.Wait(this, 2000);
                
                if(!_destroy && _clients.Count != 0)
                {
                    ++_num;

                    ArrayList toRemove = new ArrayList();
                    foreach(CallbackReceiverPrx c in _clients)
                    {
                        try
                        {
                            c.callback(_num);
                        }
                        catch(Ice.LocalException ex)
                        {
                            Console.Error.WriteLine("removing client `" +
                                                    _communicator.identityToString(c.ice_getIdentity()) + "':\n" + ex);
                            toRemove.Add(c);
                        }
                    }
                    foreach(CallbackReceiverPrx c in toRemove)
                    {
                        _clients.Remove(c);
                    }
                }
            }
        }
    }

    private Ice.Communicator _communicator;
    private bool _destroy;
    private int _num;
    private ArrayList _clients;
}
