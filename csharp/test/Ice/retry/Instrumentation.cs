//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Collections.Generic;
using Test;

namespace Ice.retry
{
    public class Instrumentation
    {
        private static object mutex = new object();

        private class InvocationObserver : Ice.Instrumentation.IInvocationObserver
        {
            public void
            Attach()
            {
            }

            public void
            Detach()
            {
                lock (mutex)
                {
                    ++nInvocation;
                }
            }

            public void
            Failed(string msg)
            {
                lock (mutex)
                {
                    ++nFailure;
                }
            }

            public void
            Retried()
            {
                lock (mutex)
                {
                    ++nRetry;
                }
            }

            public void
            RemoteException()
            {
            }

            public Ice.Instrumentation.IRemoteObserver?
            GetRemoteObserver(ConnectionInfo ci, Endpoint ei, int i, int j) => null;

            public Ice.Instrumentation.ICollocatedObserver?
            GetCollocatedObserver(ObjectAdapter adapter, int i, int j) => null;

        };

        private static Ice.Instrumentation.IInvocationObserver invocationObserver = new InvocationObserver();

        private class CommunicatorObserverI : Ice.Instrumentation.ICommunicatorObserver
        {
            public Ice.Instrumentation.IObserver? GetConnectionEstablishmentObserver(Endpoint e, string s) => null;

            public Ice.Instrumentation.IObserver? GetEndpointLookupObserver(Endpoint e) => null;

            public Ice.Instrumentation.IConnectionObserver? GetConnectionObserver(
                ConnectionInfo ci,
                Endpoint ei,
                Ice.Instrumentation.ConnectionState s,
                Ice.Instrumentation.IConnectionObserver? o) => null;

            public Ice.Instrumentation.IThreadObserver? GetThreadObserver(
                string p, string n, Ice.Instrumentation.ThreadState s, Ice.Instrumentation.IThreadObserver? o) => null;

            public Ice.Instrumentation.IInvocationObserver? GetInvocationObserver(
                IObjectPrx? p, string o, IReadOnlyDictionary<string, string> c) => invocationObserver;

            public Ice.Instrumentation.IDispatchObserver? GetDispatchObserver(Current c, int i) => null;

            public void SetObserverUpdater(Ice.Instrumentation.IObserverUpdater? u)
            {
            }
        };

        private static Ice.Instrumentation.ICommunicatorObserver communicatorObserver = new CommunicatorObserverI();

        public static Ice.Instrumentation.ICommunicatorObserver GetObserver() => communicatorObserver;

        private static void
        testEqual(ref int value, int expected)
        {
            if (expected < 0)
            {
                value = 0;
                return;
            }

            int retry = 0;
            while (++retry < 100)
            {
                lock (mutex)
                {
                    if (value == expected)
                    {
                        break;
                    }
                }
                System.Threading.Thread.Sleep(10);
            }
            if (value != expected)
            {
                System.Console.Error.WriteLine("value = " + value + ", expected = " + expected);
                TestHelper.Assert(false);
                throw new System.Exception();
            }
            value = 0;
        }

        public static void
        testRetryCount(int expected) => testEqual(ref nRetry, expected);

        public static void
        testFailureCount(int expected)
        {
            testEqual(ref nFailure, expected);
        }

        public static void
        testInvocationCount(int expected)
        {
            testEqual(ref nInvocation, expected);
        }

        public static void
        testInvocationReset()
        {
            nRetry = 0;
            nFailure = 0;
            nInvocation = 0;
        }

        private static int nRetry = 0;
        private static int nFailure = 0;
        private static int nInvocation = 0;
    }
}
