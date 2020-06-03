//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Collections.Generic;
using ZeroC.Ice.Instrumentation;
using Test;

namespace ZeroC.Ice.Test.Retry
{
    public class Instrumentation
    {
        private static object mutex = new object();

        private class InvocationObserver : IInvocationObserver
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

            public IRemoteObserver?
            GetRemoteObserver(ConnectionInfo ci, Endpoint ei, int i, int j) => null;

            public ICollocatedObserver?
            GetCollocatedObserver(ObjectAdapter adapter, int i, int j) => null;

        };

        private static IInvocationObserver invocationObserver = new InvocationObserver();

        private class CommunicatorObserverI : ICommunicatorObserver
        {
            public IObserver? GetConnectionEstablishmentObserver(Endpoint e, string s) => null;

            public IObserver? GetEndpointLookupObserver(Endpoint e) => null;

            public IConnectionObserver? GetConnectionObserver(
                ConnectionInfo ci,
                Endpoint ei,
                ConnectionState s,
                IConnectionObserver? o) => null;

            public IThreadObserver? GetThreadObserver(string p, string n, ThreadState s, IThreadObserver? o) => null;

            public IInvocationObserver? GetInvocationObserver(IObjectPrx? p, string o,
                IReadOnlyDictionary<string, string> c) => invocationObserver;

            public IDispatchObserver? GetDispatchObserver(Current c, int i) => null;

            public void SetObserverUpdater(IObserverUpdater? u)
            {
            }
        };

        private static ICommunicatorObserver communicatorObserver = new CommunicatorObserverI();

        public static ICommunicatorObserver GetObserver() => communicatorObserver;

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
