// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
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
        _clients.addElement(client);
    }

    public void
    run()
    {
        int num = 0;
        while(true)
        {
            CallbackReceiverPrx[] clients;
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

                clients = new CallbackReceiverPrx[_clients.size()];
                _clients.copyInto((Object[])clients);
            }

            if(clients.length > 0)
            {
                ++num;

                for(int i = 0; i < clients.length; ++i)
                {
                    CallbackReceiverPrx r = (CallbackReceiverPrx)clients[i];
                    try
                    {
                        r.callback(num);
                    }
                    catch(Exception ex)
                    {
                        System.out.println("removing client `" + _communicator.identityToString(r.ice_getIdentity()) +
                                           "':");
                        ex.printStackTrace();

                        synchronized(this)
                        {
                            _clients.removeElement(r);
                        }
                    }
                }
            }
        }
    }

    private Ice.Communicator _communicator;
    private boolean _destroy = false;
    private java.util.Vector _clients = new java.util.Vector();
}
