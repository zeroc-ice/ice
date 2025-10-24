// Copyright (c) ZeroC, Inc.

using System.Diagnostics;

namespace Ice.metrics;

public class ObserverI : Ice.Instrumentation.Observer
{
    public virtual void
    reset()
    {
        lock (mutex_)
        {
            total = 0;
            current = 0;
            failedCount = 0;
        }
    }

    public void attach()
    {
        lock (mutex_)
        {
            ++total;
            ++current;
        }
    }

    public void detach()
    {
        lock (mutex_)
        {
            --current;
        }
    }

    public void failed(string exceptionName)
    {
        lock (mutex_)
        {
            ++failedCount;
        }
    }

    public int getTotal()
    {
        lock (mutex_)
        {
            return total;
        }
    }

    public int getCurrent()
    {
        lock (mutex_)
        {
            return current;
        }
    }

    public int getFailedCount()
    {
        lock (mutex_)
        {
            return failedCount;
        }
    }

    public int total;
    public int current;
    public int failedCount;

    protected readonly object mutex_ = new();
}

public class ChildInvocationObserverI : ObserverI, Ice.Instrumentation.ChildInvocationObserver
{
    public override void
    reset()
    {
        lock (mutex_)
        {
            base.reset();
            replySize = 0;
        }
    }

    public void reply(int size)
    {
        lock (mutex_)
        {
            replySize += size;
        }
    }

    public int replySize;
}

public class RemoteObserverI : ChildInvocationObserverI, Ice.Instrumentation.RemoteObserver
{
}

public class CollocatedObserverI : ChildInvocationObserverI, Ice.Instrumentation.CollocatedObserver
{
}

public class InvocationObserverI : ObserverI, Ice.Instrumentation.InvocationObserver
{
    public override void
    reset()
    {
        lock (mutex_)
        {
            base.reset();
            retriedCount = 0;
            userExceptionCount = 0;
            remoteObserver?.reset();
            collocatedObserver?.reset();
        }
    }

    public void
    retried()
    {
        lock (mutex_)
        {
            ++retriedCount;
        }
    }

    public void
    userException()
    {
        lock (mutex_)
        {
            ++userExceptionCount;
        }
    }

    public Ice.Instrumentation.RemoteObserver
    getRemoteObserver(Ice.ConnectionInfo con, Ice.Endpoint endpt, int requestId, int size)
    {
        lock (mutex_)
        {
            if (remoteObserver == null)
            {
                remoteObserver = new RemoteObserverI();
                remoteObserver.reset();
            }
            return remoteObserver;
        }
    }

    public Ice.Instrumentation.CollocatedObserver
    getCollocatedObserver(Ice.ObjectAdapter adapter, int requestId, int size)
    {
        lock (mutex_)
        {
            if (collocatedObserver == null)
            {
                collocatedObserver = new CollocatedObserverI();
                collocatedObserver.reset();
            }
            return collocatedObserver;
        }
    }

    public int userExceptionCount;
    public int retriedCount;

    public RemoteObserverI remoteObserver;
    public CollocatedObserverI collocatedObserver;
}

public class DispatchObserverI : ObserverI, Ice.Instrumentation.DispatchObserver
{
    public override void
    reset()
    {
        lock (mutex_)
        {
            base.reset();
            userExceptionCount = 0;
            replySize = 0;
        }
    }

    public void
    userException()
    {
        lock (mutex_)
        {
            ++userExceptionCount;
        }
    }

    public void
    reply(int size)
    {
        lock (mutex_)
        {
            replySize += size;
        }
    }

    public int userExceptionCount;
    public int replySize;
}

public class ConnectionObserverI : ObserverI, Ice.Instrumentation.ConnectionObserver
{
    public override void
    reset()
    {
        lock (mutex_)
        {
            base.reset();
            received = 0;
            sent = 0;
        }
    }

    public void
    sentBytes(int num)
    {
        lock (mutex_)
        {
            sent += num;
        }
    }

    public void
    receivedBytes(int num)
    {
        lock (mutex_)
        {
            received += num;
        }
    }

    public int sent;
    public int received;
}

public class ThreadObserverI : ObserverI, Ice.Instrumentation.ThreadObserver
{
    public override void
    reset()
    {
        lock (mutex_)
        {
            base.reset();
            states = 0;
        }
    }

    public void
    stateChanged(Ice.Instrumentation.ThreadState oldState, Ice.Instrumentation.ThreadState newState)
    {
        lock (mutex_)
        {
            ++states;
        }
    }

    public int states;
}

public class CommunicatorObserverI : Ice.Instrumentation.CommunicatorObserver
{
    public void
    setObserverUpdater(Ice.Instrumentation.ObserverUpdater updater)
    {
        lock (_mutex)
        {
            this.updater = updater;
        }
    }

    public Ice.Instrumentation.Observer
    getConnectionEstablishmentObserver(Ice.Endpoint endpt, string connector)
    {
        lock (_mutex)
        {
            if (connectionEstablishmentObserver == null)
            {
                connectionEstablishmentObserver = new ObserverI();
                connectionEstablishmentObserver.reset();
            }
            return connectionEstablishmentObserver;
        }
    }

    public Ice.Instrumentation.Observer
    getEndpointLookupObserver(Ice.Endpoint endpt)
    {
        lock (_mutex)
        {
            if (endpointLookupObserver == null)
            {
                endpointLookupObserver = new ObserverI();
                endpointLookupObserver.reset();
            }
            return endpointLookupObserver;
        }
    }

    public Ice.Instrumentation.ConnectionObserver getConnectionObserver(
        Ice.ConnectionInfo c,
        Ice.Endpoint e,
        Ice.Instrumentation.ConnectionState s,
        Ice.Instrumentation.ConnectionObserver o)
    {
        lock (_mutex)
        {
            Debug.Assert(o == null || o is ConnectionObserverI);
            if (connectionObserver == null)
            {
                connectionObserver = new ConnectionObserverI();
                connectionObserver.reset();
            }
            return connectionObserver;
        }
    }

    public Ice.Instrumentation.ThreadObserver
    getThreadObserver(string parent, string id, Ice.Instrumentation.ThreadState s, Ice.Instrumentation.ThreadObserver o)
    {
        lock (_mutex)
        {
            Debug.Assert(o == null || o is ThreadObserverI);
            if (threadObserver == null)
            {
                threadObserver = new ThreadObserverI();
                threadObserver.reset();
            }
            return threadObserver;
        }
    }

    public Ice.Instrumentation.InvocationObserver
    getInvocationObserver(Ice.ObjectPrx prx, string operation, Dictionary<string, string> ctx)
    {
        lock (_mutex)
        {
            if (invocationObserver == null)
            {
                invocationObserver = new InvocationObserverI();
                invocationObserver.reset();
            }
            return invocationObserver;
        }
    }

    public Ice.Instrumentation.DispatchObserver getDispatchObserver(Ice.Current c, int size)
    {
        lock (_mutex)
        {
            if (dispatchObserver == null)
            {
                dispatchObserver = new DispatchObserverI();
                dispatchObserver.reset();
            }
            return dispatchObserver;
        }
    }

    protected Ice.Instrumentation.ObserverUpdater updater;

    public ObserverI connectionEstablishmentObserver;
    public ObserverI endpointLookupObserver;
    public ConnectionObserverI connectionObserver;
    public ThreadObserverI threadObserver;
    public InvocationObserverI invocationObserver;
    public DispatchObserverI dispatchObserver;

    private readonly object _mutex = new();
}
