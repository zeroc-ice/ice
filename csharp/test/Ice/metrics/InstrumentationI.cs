//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using System.Diagnostics;

public class Observer : Ice.Instrumentation.IObserver
{
    virtual public void
    reset()
    {
        lock (this)
        {
            total = 0;
            current = 0;
            failedCount = 0;
        }
    }

    public void
    Attach()
    {
        lock (this)
        {
            ++total;
            ++current;
        }
    }
    public void
    Detach()
    {
        lock (this)
        {
            --current;
        }
    }
    public void
    Failed(string s)
    {
        lock (this)
        {
            ++failedCount;
        }
    }

    public int
    getTotal()
    {
        lock (this)
        {
            return total;
        }
    }

    public int
    GetCurrent()
    {
        lock (this)
        {
            return current;
        }
    }

    public int
    GetFailedCount()
    {
        lock (this)
        {
            return failedCount;
        }
    }

    public int total;
    public int current;
    public int failedCount;
};

public class ChildInvocationObserver : Observer, Ice.Instrumentation.IChildInvocationObserver
{
    override public void
    reset()
    {
        lock (this)
        {
            base.reset();
            replySize = 0;
        }
    }

    public void
    Reply(int s)
    {
        lock (this)
        {
            replySize += s;
        }
    }

    public int replySize;
};

public class RemoteObserver : ChildInvocationObserver, Ice.Instrumentation.IRemoteObserver
{
};

public class CollocatedObserver : ChildInvocationObserver, Ice.Instrumentation.ICollocatedObserver
{
};

public class InvocationObserver : Observer, Ice.Instrumentation.IInvocationObserver
{
    override public void
    reset()
    {
        lock (this)
        {
            base.reset();
            retriedCount = 0;
            userExceptionCount = 0;
            if (remoteObserver != null)
            {
                remoteObserver.reset();
            }
            if (collocatedObserver != null)
            {
                collocatedObserver.reset();
            }
        }
    }

    public void
    Retried()
    {
        lock (this)
        {
            ++retriedCount;
        }
    }

    public void
    RemoteException()
    {
        lock (this)
        {
            ++userExceptionCount;
        }
    }

    public Ice.Instrumentation.IRemoteObserver
    GetRemoteObserver(Ice.ConnectionInfo c, Ice.Endpoint e, int a, int b)
    {
        lock (this)
        {
            if (remoteObserver == null)
            {
                remoteObserver = new RemoteObserver();
                remoteObserver.reset();
            }
            return remoteObserver;
        }
    }

    public Ice.Instrumentation.ICollocatedObserver
    GetCollocatedObserver(Ice.ObjectAdapter adapter, int a, int b)
    {
        lock (this)
        {
            if (collocatedObserver == null)
            {
                collocatedObserver = new CollocatedObserver();
                collocatedObserver.reset();
            }
            return collocatedObserver;
        }
    }

    public int userExceptionCount;
    public int retriedCount;

    public RemoteObserver remoteObserver = null;
    public CollocatedObserver collocatedObserver = null;
};

public class DispatchObserver : Observer, Ice.Instrumentation.IDispatchObserver
{
    override public void
    reset()
    {
        lock (this)
        {
            base.reset();
            userExceptionCount = 0;
            replySize = 0;
        }
    }

    public void
    RemoteException()
    {
        lock (this)
        {
            ++userExceptionCount;
        }
    }

    public void
    Reply(int s)
    {
        lock (this)
        {
            replySize += s;
        }
    }

    public int userExceptionCount;
    public int replySize;
};

public class ConnectionObserver : Observer, Ice.Instrumentation.IConnectionObserver
{
    override public void
    reset()
    {
        lock (this)
        {
            base.reset();
            received = 0;
            sent = 0;
        }
    }

    public void
    SentBytes(int s)
    {
        lock (this)
        {
            sent += s;
        }
    }

    public void
    ReceivedBytes(int s)
    {
        lock (this)
        {
            received += s;
        }
    }

    public int sent;
    public int received;
};

public class ThreadObserver : Observer, Ice.Instrumentation.IThreadObserver
{
    override public void
    reset()
    {
        lock (this)
        {
            base.reset();
            states = 0;
        }
    }

    public void
    StateChanged(Ice.Instrumentation.ThreadState o, Ice.Instrumentation.ThreadState n)
    {
        lock (this)
        {
            ++states;
        }
    }

    public int states;
};

public class CommunicatorObserver : Ice.Instrumentation.ICommunicatorObserver
{
    public void
    SetObserverUpdater(Ice.Instrumentation.IObserverUpdater u)
    {
        lock (this)
        {
            updater = u;
        }
    }

    public Ice.Instrumentation.IObserver
    GetConnectionEstablishmentObserver(Ice.Endpoint e, String s)
    {
        lock (this)
        {
            if (connectionEstablishmentObserver == null)
            {
                connectionEstablishmentObserver = new Observer();
                connectionEstablishmentObserver.reset();
            }
            return connectionEstablishmentObserver;
        }
    }

    public Ice.Instrumentation.IObserver
    GetEndpointLookupObserver(Ice.Endpoint e)
    {
        lock (this)
        {
            if (endpointLookupObserver == null)
            {
                endpointLookupObserver = new Observer();
                endpointLookupObserver.reset();
            }
            return endpointLookupObserver;
        }
    }

    public Ice.Instrumentation.IConnectionObserver
    GetConnectionObserver(Ice.ConnectionInfo c,
                          Ice.Endpoint e,
                          Ice.Instrumentation.ConnectionState s,
                          Ice.Instrumentation.IConnectionObserver old)
    {
        lock (this)
        {
            Debug.Assert(old == null || old is ConnectionObserver);
            if (connectionObserver == null)
            {
                connectionObserver = new ConnectionObserver();
                connectionObserver.reset();
            }
            return connectionObserver;
        }
    }

    public Ice.Instrumentation.IThreadObserver
    GetThreadObserver(string p, string id, Ice.Instrumentation.ThreadState s, Ice.Instrumentation.IThreadObserver old)
    {
        lock (this)
        {
            Debug.Assert(old == null || old is ThreadObserver);
            if (threadObserver == null)
            {
                threadObserver = new ThreadObserver();
                threadObserver.reset();
            }
            return threadObserver;
        }
    }

    public Ice.Instrumentation.IInvocationObserver
    GetInvocationObserver(Ice.IObjectPrx? p, string op, IReadOnlyDictionary<string, string> ctx)
    {
        lock (this)
        {
            if (invocationObserver == null)
            {
                invocationObserver = new InvocationObserver();
                invocationObserver.reset();
            }
            return invocationObserver;
        }
    }

    public Ice.Instrumentation.IDispatchObserver
    GetDispatchObserver(Ice.Current current, int s)
    {
        lock (this)
        {
            if (dispatchObserver == null)
            {
                dispatchObserver = new DispatchObserver();
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

    protected Ice.Instrumentation.IObserverUpdater updater;

    public Observer connectionEstablishmentObserver;
    public Observer endpointLookupObserver;
    public ConnectionObserver connectionObserver;
    public ThreadObserver threadObserver;
    public InvocationObserver invocationObserver;
    public DispatchObserver dispatchObserver;
};
