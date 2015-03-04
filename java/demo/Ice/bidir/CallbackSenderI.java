// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import Demo.*;

class CallbackSenderI extends _CallbackSenderDisp implements java.lang.Runnable
{
    CallbackSenderI(Ice.Communicator communicator)
    {
        _communicator = communicator;
    }

    synchronized public void
    destroy()
    {
        System.out.println("destroying callback sender");
        _destroy = true;

        this.notify();
    }

    synchronized public void
    addClient(Ice.Identity ident, Ice.Current current)
    {
        System.out.println("adding client `" + _communicator.identityToString(ident) + "'");

        Ice.ObjectPrx base = current.con.createProxy(ident);
        CallbackReceiverPrx client = CallbackReceiverPrxHelper.uncheckedCast(base);
        _clients.add(client);
    }

    public void
    run()
    {
        int num = 0;
        while(true)
        {
            java.util.List<CallbackReceiverPrx> clients;
            synchronized(this)
            {
                try
                {
                    this.wait(2000);
                }
                catch(java.lang.InterruptedException ex)
                {
                }

                if(_destroy)
                {
                    break;
                }

                clients = new java.util.ArrayList<CallbackReceiverPrx>(_clients);
            }

            if(!clients.isEmpty())
            {
                ++num;

                for(CallbackReceiverPrx p : clients)
                {
                    try
                    {
                        p.callback(num);
                    }
                    catch(Exception ex)
                    {
                        System.out.println("removing client `" + _communicator.identityToString(p.ice_getIdentity()) +
                                           "':");
                        ex.printStackTrace();

                        synchronized(this)
                        {
                            _clients.remove(p);
                        }
                    }
                }
            }
        }
    }

    private Ice.Communicator _communicator;
    private boolean _destroy = false;
    private java.util.List<CallbackReceiverPrx> _clients = new java.util.ArrayList<CallbackReceiverPrx>();
}
