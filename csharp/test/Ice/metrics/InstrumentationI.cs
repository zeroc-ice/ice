// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Collections.Generic;
using System.Diagnostics;

public class ObserverI : Ice.Instrumentation.Observer
{
    virtual public void 
    reset()
    {
        lock(this)
        {
            total = 0;
            current = 0;
            failedCount = 0;
        }
    }

    public void 
    attach()
    {
        lock(this)
        {
            ++total;
            ++current;
        }
    }
    public void 
    detach()
    {
        lock(this)
        {
            --current;
        }
    }
    public void 
    failed(String s)
    {
        lock(this)
        {
            ++failedCount;
        }
    }

    public int 
    getTotal()
    {
        lock(this)
        {
            return total;
        }
    }

    public int 
    getCurrent()
    {
        lock(this)
        {
            return current;
        }
    }

    public int 
    getFailedCount()
    {
        lock(this)
        {
            return failedCount;
        }
    }

    public int total;
    public int current;
    public int failedCount;
};

public class ChildInvocationObserverI : ObserverI, Ice.Instrumentation.ChildInvocationObserver
{
    override public void 
    reset()
    {
        lock(this)
        {
            base.reset();
            replySize = 0;
        }
    }

    public void
    reply(int s)
    {
        lock(this)
        {
            replySize += s;
        }
    }

    public int replySize;
};

public class RemoteObserverI : ChildInvocationObserverI, Ice.Instrumentation.RemoteObserver
{
};

public class CollocatedObserverI : ChildInvocationObserverI, Ice.Instrumentation.CollocatedObserver
{
};

public class InvocationObserverI : ObserverI , Ice.Instrumentation.InvocationObserver
{
    override public void 
    reset()
    {
        lock(this)
        {
            base.reset();
            retriedCount = 0;
            userExceptionCount = 0;
            if(remoteObserver != null)
            {
                remoteObserver.reset();
            }
            if(collocatedObserver != null)
            {
                collocatedObserver.reset();
            }
        }
    }

    public void 
    retried()
    {
        lock(this)
        {
            ++retriedCount;
        }
    }

    public void 
    userException()
    {
        lock(this)
        {
            ++userExceptionCount;
        }
    }

    public Ice.Instrumentation.RemoteObserver 
    getRemoteObserver(Ice.ConnectionInfo c, Ice.Endpoint e, int a, int b)
    {
        lock(this)
        {
            if(remoteObserver == null)
            {
                remoteObserver = new RemoteObserverI();
                remoteObserver.reset();
            }
            return remoteObserver;
        }
    }

    public Ice.Instrumentation.CollocatedObserver
    getCollocatedObserver(Ice.ObjectAdapter adapter, int a, int b)
    {
        lock(this)
        {
            if(collocatedObserver == null)
            {
                collocatedObserver = new CollocatedObserverI();
                collocatedObserver.reset();
            }
            return collocatedObserver;
        }
    }

    public int userExceptionCount;
    public int retriedCount;

    public RemoteObserverI remoteObserver = null;
    public CollocatedObserverI collocatedObserver = null;
};

public class DispatchObserverI : ObserverI , Ice.Instrumentation.DispatchObserver
{
    override public void 
    reset()
    {
        lock(this)
        {
            base.reset();
            userExceptionCount = 0;
            replySize = 0;
        }
    }

    public void 
    userException()
    {
        lock(this)
        {
            ++userExceptionCount;
        }
    }

    public void 
    reply(int s)
    {
        lock(this)
        {
            replySize += s;
        }
    }

    public int userExceptionCount;
    public int replySize;
};

public class ConnectionObserverI : ObserverI , Ice.Instrumentation.ConnectionObserver
{
    override public void 
    reset()
    {
        lock(this)
        {
            base.reset();
            received = 0;
            sent = 0;
        }
    }

    public void 
    sentBytes(int s)
    {
        lock(this)
        {
            sent += s;
        }
    }

    public void 
    receivedBytes(int s)
    {
        lock(this)
        {
            received += s;
        }
    }
    
    public int sent;
    public int received;
};

public class ThreadObserverI : ObserverI , Ice.Instrumentation.ThreadObserver
{
    override public void 
    reset()
    {
        lock(this)
        {
            base.reset();
            states = 0;
        }
    }

    public void 
    stateChanged(Ice.Instrumentation.ThreadState o, Ice.Instrumentation.ThreadState n)
    {
        lock(this)
        {
            ++states;
        }
    }

    public int states;
};

public class CommunicatorObserverI : Ice.Instrumentation.CommunicatorObserver
{
    public void 
    setObserverUpdater(Ice.Instrumentation.ObserverUpdater u)
    {
        lock(this)
        {
            updater = u;
        }
    }
 
    public Ice.Instrumentation.Observer 
    getConnectionEstablishmentObserver(Ice.Endpoint e, String s)
    {
        lock(this)
        {
            if(connectionEstablishmentObserver == null)
            {
                connectionEstablishmentObserver = new ObserverI();
                connectionEstablishmentObserver.reset();
            }
            return connectionEstablishmentObserver;
        }
    }

 
    public Ice.Instrumentation.Observer 
    getEndpointLookupObserver(Ice.Endpoint e)
    {
        lock(this)
        {
            if(endpointLookupObserver == null)
            {
                endpointLookupObserver = new ObserverI();
                endpointLookupObserver.reset();
            }
            return endpointLookupObserver;
        }
    }
    
    public Ice.Instrumentation.ConnectionObserver 
    getConnectionObserver(Ice.ConnectionInfo c,
                          Ice.Endpoint e,
                          Ice.Instrumentation.ConnectionState s, 
                          Ice.Instrumentation.ConnectionObserver old)
    {
        lock(this)
        {
            Debug.Assert(old == null || old is ConnectionObserverI);
            if(connectionObserver == null)
            {
                connectionObserver = new ConnectionObserverI();
                connectionObserver.reset();
            }
            return connectionObserver;
        }
    }

    public Ice.Instrumentation.ThreadObserver 
    getThreadObserver(String p, String id, Ice.Instrumentation.ThreadState s,
                      Ice.Instrumentation.ThreadObserver old)
    {
        lock(this)
        {
            Debug.Assert(old == null || old is ThreadObserverI);
            if(threadObserver == null)
            {
                threadObserver = new ThreadObserverI();
                threadObserver.reset();
            }
            return threadObserver; 
        }
   }

    public Ice.Instrumentation.InvocationObserver
    getInvocationObserver(Ice.ObjectPrx p, String op, Dictionary<String, String> ctx)
    {
        lock(this)
        {
            if(invocationObserver == null)
            {
                invocationObserver = new InvocationObserverI();
                invocationObserver.reset();
            }
            return invocationObserver;
        }
    }

    public Ice.Instrumentation.DispatchObserver
    getDispatchObserver(Ice.Current current, int s)
    {
        lock(this)
        {
            if(dispatchObserver == null)
            {
                dispatchObserver = new DispatchObserverI();
                dispatchObserver.reset();
            }
            return dispatchObserver;
        }
    }


    /*
    void
    reset()
    {
        lock(this)
        {
            if(connectionEstablishmentObserver != null)
            {
                connectionEstablishmentObserver.reset();
            }
            if(endpointLookupObserver != null)
            {
                endpointLookupObserver.reset();
            }
            if(connectionObserver != null)
            {
                connectionObserver.reset();
            }
            if(threadObserver != null)
            {
                threadObserver.reset();
            }
            if(invocationObserver != null)
            {
                invocationObserver.reset();
            }
            if(dispatchObserver != null)
            {
                dispatchObserver.reset();
            }
        }
    }
    */
    
    protected Ice.Instrumentation.ObserverUpdater updater;

    public ObserverI connectionEstablishmentObserver;
    public ObserverI endpointLookupObserver;
    public ConnectionObserverI connectionObserver;
    public ThreadObserverI threadObserver;
    public InvocationObserverI invocationObserver;
    public DispatchObserverI dispatchObserver;
};

