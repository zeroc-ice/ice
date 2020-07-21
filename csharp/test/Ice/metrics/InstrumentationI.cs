//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Collections.Generic;
using Test;
using ZeroC.Ice.Instrumentation;

namespace ZeroC.Ice.Test.Metrics
{
    public class Observer : IObserver
    {
        public int total;
        public int current;
        public int failedCount;

        protected readonly object _mutex = new object();

        public virtual void Reset()
        {
            lock (_mutex)
            {
                total = 0;
                current = 0;
                failedCount = 0;
            }
        }

        public void Attach()
        {
            lock (_mutex)
            {
                ++total;
                ++current;
            }
        }
        public void Detach()
        {
            lock (_mutex)
            {
                --current;
            }
        }
        public void Failed(string s)
        {
            lock (_mutex)
            {
                ++failedCount;
            }
        }

        public int GetTotal()
        {
            lock (_mutex)
            {
                return total;
            }
        }

        public int GetCurrent()
        {
            lock (_mutex)
            {
                return current;
            }
        }

        public int GetFailedCount()
        {
            lock (_mutex)
            {
                return failedCount;
            }
        }
    }

    public class ChildInvocationObserver : Observer, IChildInvocationObserver
    {
        public override void Reset()
        {
            lock (_mutex)
            {
                base.Reset();
                replySize = 0;
            }
        }

        public void Reply(int s)
        {
            lock (_mutex)
            {
                replySize += s;
            }
        }

        public int replySize;
    }

    public class RemoteObserver : ChildInvocationObserver, IRemoteObserver
    {
    }

    public class CollocatedObserver : ChildInvocationObserver, ICollocatedObserver
    {
    }

    public class InvocationObserver : Observer, IInvocationObserver
    {
        public override void Reset()
        {
            lock (_mutex)
            {
                base.Reset();
                retriedCount = 0;
                userExceptionCount = 0;
                if (remoteObserver != null)
                {
                    remoteObserver.Reset();
                }
                if (collocatedObserver != null)
                {
                    collocatedObserver.Reset();
                }
            }
        }

        public void Retried()
        {
            lock (_mutex)
            {
                ++retriedCount;
            }
        }

        public void RemoteException()
        {
            lock (_mutex)
            {
                ++userExceptionCount;
            }
        }

        public IRemoteObserver GetRemoteObserver(Connection c, int a, int b)
        {
            lock (_mutex)
            {
                if (remoteObserver == null)
                {
                    remoteObserver = new RemoteObserver();
                    remoteObserver.Reset();
                }
                return remoteObserver;
            }
        }

        public ICollocatedObserver
        GetCollocatedObserver(ObjectAdapter adapter, int a, int b)
        {
            lock (_mutex)
            {
                if (collocatedObserver == null)
                {
                    collocatedObserver = new CollocatedObserver();
                    collocatedObserver.Reset();
                }
                return collocatedObserver;
            }
        }

        public int userExceptionCount;
        public int retriedCount;

        public RemoteObserver? remoteObserver = null;
        public CollocatedObserver? collocatedObserver = null;
    }

    public class DispatchObserver : Observer, IDispatchObserver
    {
        public override void
        Reset()
        {
            lock (_mutex)
            {
                base.Reset();
                userExceptionCount = 0;
                replySize = 0;
            }
        }

        public void
        RemoteException()
        {
            lock (_mutex)
            {
                ++userExceptionCount;
            }
        }

        public void
        Reply(int s)
        {
            lock (_mutex)
            {
                replySize += s;
            }
        }

        public int userExceptionCount;
        public int replySize;
    }

    public class ConnectionObserver : Observer, IConnectionObserver
    {
        public override void
        Reset()
        {
            lock (_mutex)
            {
                base.Reset();
                received = 0;
                sent = 0;
            }
        }

        public void SentBytes(int s)
        {
            lock (_mutex)
            {
                sent += s;
            }
        }

        public void ReceivedBytes(int s)
        {
            lock (_mutex)
            {
                received += s;
            }
        }

        public int sent;
        public int received;
    }

    public class CommunicatorObserver : ICommunicatorObserver
    {
        private readonly object _mutex = new object();

        public void SetObserverUpdater(IObserverUpdater? u)
        {
            lock (_mutex)
            {
                updater = u;
            }
        }

        public IObserver GetConnectionEstablishmentObserver(Endpoint e, string s)
        {
            lock (_mutex)
            {
                if (connectionEstablishmentObserver == null)
                {
                    connectionEstablishmentObserver = new Observer();
                    connectionEstablishmentObserver.Reset();
                }
                return connectionEstablishmentObserver;
            }
        }

        public IObserver GetEndpointLookupObserver(Endpoint e)
        {
            lock (_mutex)
            {
                if (endpointLookupObserver == null)
                {
                    endpointLookupObserver = new Observer();
                    endpointLookupObserver.Reset();
                }
                return endpointLookupObserver;
            }
        }

        public IConnectionObserver? GetConnectionObserver(Connection c, ConnectionState s, IConnectionObserver? old)
        {
            lock (_mutex)
            {
                TestHelper.Assert(old == null || old is ConnectionObserver);
                if (connectionObserver == null)
                {
                    connectionObserver = new ConnectionObserver();
                    connectionObserver.Reset();
                }
                return connectionObserver;
            }
        }

        public IInvocationObserver GetInvocationObserver(
            IObjectPrx? p,
            string op,
            IReadOnlyDictionary<string, string> ctx)
        {
            lock (_mutex)
            {
                if (invocationObserver == null)
                {
                    invocationObserver = new InvocationObserver();
                    invocationObserver.Reset();
                }
                return invocationObserver;
            }
        }

        public IDispatchObserver GetDispatchObserver(Current current, int requestId, int s)
        {
            lock (_mutex)
            {
                if (dispatchObserver == null)
                {
                    dispatchObserver = new DispatchObserver();
                    dispatchObserver.Reset();
                }
                return dispatchObserver;
            }
        }

        protected IObserverUpdater? updater;

        public Observer? connectionEstablishmentObserver;
        public Observer? endpointLookupObserver;
        public ConnectionObserver? connectionObserver;
        public InvocationObserver? invocationObserver;
        public DispatchObserver? dispatchObserver;
    };
}
