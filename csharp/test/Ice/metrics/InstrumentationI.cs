// Copyright (c) ZeroC, Inc. All rights reserved.

using System.Collections.Generic;
using ZeroC.Ice.Instrumentation;
using ZeroC.Test;

namespace ZeroC.Ice.Test.Metrics
{
    public class Observer : IObserver
    {
        public int Total;
        public int Current;
        public int FailedCount;

        protected readonly object Mutex = new();

        public virtual void Reset()
        {
            lock (Mutex)
            {
                Total = 0;
                Current = 0;
                FailedCount = 0;
            }
        }

        public void Attach()
        {
            lock (Mutex)
            {
                ++Total;
                ++Current;
            }
        }
        public void Detach()
        {
            lock (Mutex)
            {
                --Current;
            }
        }
        public void Failed(string s)
        {
            lock (Mutex)
            {
                ++FailedCount;
            }
        }

        public int GetTotal()
        {
            lock (Mutex)
            {
                return Total;
            }
        }

        public int GetCurrent()
        {
            lock (Mutex)
            {
                return Current;
            }
        }

        public int GetFailedCount()
        {
            lock (Mutex)
            {
                return FailedCount;
            }
        }
    }

    public class ChildInvocationObserver : Observer, IChildInvocationObserver
    {
        public int ReplySize;
        public override void Reset()
        {
            lock (Mutex)
            {
                base.Reset();
                ReplySize = 0;
            }
        }

        public void Reply(int s)
        {
            lock (Mutex)
            {
                ReplySize += s;
            }
        }
    }

    public class InvocationObserver : Observer, IInvocationObserver
    {
        public ChildInvocationObserver? ChildInvocationObserver;
        public int RetriedCount;
        public int UserExceptionCount;

        public override void Reset()
        {
            lock (Mutex)
            {
                base.Reset();
                RetriedCount = 0;
                UserExceptionCount = 0;
                if (ChildInvocationObserver != null)
                {
                    ChildInvocationObserver.Reset();
                }
            }
        }

        public void Retried()
        {
            lock (Mutex)
            {
                ++RetriedCount;
            }
        }

        public void RemoteException()
        {
            lock (Mutex)
            {
                ++UserExceptionCount;
            }
        }

        public IChildInvocationObserver GetChildInvocationObserver(Connection c, int a)
        {
            lock (Mutex)
            {
                if (ChildInvocationObserver == null)
                {
                    ChildInvocationObserver = new();
                    ChildInvocationObserver.Reset();
                }
                return ChildInvocationObserver;
            }
        }
    }

    public class DispatchObserver : Observer, IDispatchObserver
    {
        public override void Reset()
        {
            lock (Mutex)
            {
                base.Reset();
                UserExceptionCount = 0;
                ReplySize = 0;
            }
        }

        public void RemoteException()
        {
            lock (Mutex)
            {
                ++UserExceptionCount;
            }
        }

        public void Reply(int s)
        {
            lock (Mutex)
            {
                ReplySize += s;
            }
        }

        public int UserExceptionCount;
        public int ReplySize;
    }

    public class ConnectionObserver : Observer, IConnectionObserver
    {
        public int Sent;
        public int Received;
        public override void Reset()
        {
            lock (Mutex)
            {
                base.Reset();
                Received = 0;
                Sent = 0;
            }
        }

        public void SentBytes(int s)
        {
            lock (Mutex)
            {
                Sent += s;
            }
        }

        public void ReceivedBytes(int s)
        {
            lock (Mutex)
            {
                Received += s;
            }
        }
    }

    public class CommunicatorObserverI : ICommunicatorObserver
    {
        public Observer? ConnectionEstablishmentObserver;
        public ConnectionObserver? ConnectionObserver;
        public DispatchObserver? DispatchObserver;
        public Observer? EndpointLookupObserver;
        public InvocationObserver? InvocationObserver;
        protected IObserverUpdater? Updater;
        private readonly object _mutex = new();

        public void SetObserverUpdater(IObserverUpdater? u)
        {
            lock (_mutex)
            {
                Updater = u;
            }
        }

        public IObserver GetConnectionEstablishmentObserver(Endpoint e, string s)
        {
            lock (_mutex)
            {
                if (ConnectionEstablishmentObserver == null)
                {
                    ConnectionEstablishmentObserver = new Observer();
                    ConnectionEstablishmentObserver.Reset();
                }
                return ConnectionEstablishmentObserver;
            }
        }

        public IObserver GetEndpointLookupObserver(Endpoint e)
        {
            lock (_mutex)
            {
                if (EndpointLookupObserver == null)
                {
                    EndpointLookupObserver = new Observer();
                    EndpointLookupObserver.Reset();
                }
                return EndpointLookupObserver;
            }
        }

        public IConnectionObserver? GetConnectionObserver(Connection c, ConnectionState s, IConnectionObserver? old)
        {
            lock (_mutex)
            {
                TestHelper.Assert(old == null || old is ConnectionObserver);
                if (ConnectionObserver == null)
                {
                    ConnectionObserver = new();
                    ConnectionObserver.Reset();
                }
                return ConnectionObserver;
            }
        }

        public IInvocationObserver GetInvocationObserver(
            IObjectPrx? p,
            string op,
            IReadOnlyDictionary<string, string> ctx)
        {
            lock (_mutex)
            {
                if (InvocationObserver == null)
                {
                    InvocationObserver = new();
                    InvocationObserver.Reset();
                }
                return InvocationObserver;
            }
        }

        public IDispatchObserver GetDispatchObserver(Current current, long streamId, int s)
        {
            lock (_mutex)
            {
                if (DispatchObserver == null)
                {
                    DispatchObserver = new();
                    DispatchObserver.Reset();
                }
                return DispatchObserver;
            }
        }
    }
}
