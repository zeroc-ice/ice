//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using Test;
using ZeroC.Ice.Instrumentation;

namespace ZeroC.Ice.Test.Metrics
{
    public class Observer : IObserver
    {
        public virtual void
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

    public class ChildInvocationObserver : Observer, IChildInvocationObserver
    {
        public override void
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

    public class RemoteObserver : ChildInvocationObserver, IRemoteObserver
    {
    };

    public class CollocatedObserver : ChildInvocationObserver, ICollocatedObserver
    {
    };

    public class InvocationObserver : Observer, IInvocationObserver
    {
        public override void
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

        public IRemoteObserver
        GetRemoteObserver(ConnectionInfo c, Endpoint e, int a, int b)
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

        public ICollocatedObserver
        GetCollocatedObserver(ObjectAdapter adapter, int a, int b)
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

        public RemoteObserver? remoteObserver = null;
        public CollocatedObserver? collocatedObserver = null;
    };

    public class DispatchObserver : Observer, IDispatchObserver
    {
        public override void
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

    public class ConnectionObserver : Observer, IConnectionObserver
    {
        public override void
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

    public class ThreadObserver : Observer, IThreadObserver
    {
        public override void
        reset()
        {
            lock (this)
            {
                base.reset();
                states = 0;
            }
        }

        public void
        StateChanged(ThreadState o, ThreadState n)
        {
            lock (this)
            {
                ++states;
            }
        }

        public int states;
    };

    public class CommunicatorObserver : ICommunicatorObserver
    {
        public void
        SetObserverUpdater(IObserverUpdater? u)
        {
            lock (this)
            {
                updater = u;
            }
        }

        public IObserver
        GetConnectionEstablishmentObserver(Endpoint e, string s)
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

        public IObserver
        GetEndpointLookupObserver(Endpoint e)
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

        public IConnectionObserver?
        GetConnectionObserver(ConnectionInfo c,
                            Endpoint e,
                            ConnectionState s,
                            IConnectionObserver? old)
        {
            lock (this)
            {
                TestHelper.Assert(old == null || old is ConnectionObserver);
                if (connectionObserver == null)
                {
                    connectionObserver = new ConnectionObserver();
                    connectionObserver.reset();
                }
                return connectionObserver;
            }
        }

        public IThreadObserver?
        GetThreadObserver(string p, string id, ThreadState s, IThreadObserver? old)
        {
            lock (this)
            {
                TestHelper.Assert(old == null || old is ThreadObserver);
                if (threadObserver == null)
                {
                    threadObserver = new ThreadObserver();
                    threadObserver.reset();
                }
                return threadObserver;
            }
        }

        public IInvocationObserver
        GetInvocationObserver(IObjectPrx? p, string op, IReadOnlyDictionary<string, string> ctx)
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

        public IDispatchObserver
        GetDispatchObserver(Current current, int s)
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

        protected IObserverUpdater? updater;

        public Observer? connectionEstablishmentObserver;
        public Observer? endpointLookupObserver;
        public ConnectionObserver? connectionObserver;
        public ThreadObserver? threadObserver;
        public InvocationObserver? invocationObserver;
        public DispatchObserver? dispatchObserver;
    };
}
