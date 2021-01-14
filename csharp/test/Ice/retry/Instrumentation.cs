// Copyright (c) ZeroC, Inc. All rights reserved.

using System.Collections.Generic;
using ZeroC.Ice.Instrumentation;
using ZeroC.Test;

namespace ZeroC.Ice.Test.Retry
{
    public static class Instrumentation
    {
        private static readonly object _mutex = new();
        private static int _nFailure;
        private static int _nInvocation;
        private static int _nRetry;

        private class InvocationObserver : IInvocationObserver
        {
            public void Attach()
            {
            }

            public void Detach()
            {
                lock (_mutex)
                {
                    ++_nInvocation;
                }
            }

            public void Failed(string msg)
            {
                lock (_mutex)
                {
                    ++_nFailure;
                }
            }

            public void Retried()
            {
                lock (_mutex)
                {
                    ++_nRetry;
                }
            }

            public void RemoteException()
            {
            }

            public IChildInvocationObserver? GetChildInvocationObserver(Connection connection, int size) => null;
        }

        private static readonly IInvocationObserver _invocationObserver = new InvocationObserver();

        private class CommunicatorObserverI : ICommunicatorObserver
        {
            public IObserver? GetConnectionEstablishmentObserver(Endpoint e, string s) => null;

            public IObserver? GetEndpointLookupObserver(Endpoint e) => null;

            public IConnectionObserver? GetConnectionObserver(
                Connection connection,
                ConnectionState connectionState,
                IConnectionObserver? observer) => null;

            public IInvocationObserver? GetInvocationObserver(IObjectPrx? p, string o,
                IReadOnlyDictionary<string, string> c) => _invocationObserver;

            public IDispatchObserver? GetDispatchObserver(Current c, long requestId, int i) => null;

            public void SetObserverUpdater(IObserverUpdater? u)
            {
            }
        }

        public static ICommunicatorObserver GetObserver() => _communicatorObserver;

        private static readonly ICommunicatorObserver _communicatorObserver = new CommunicatorObserverI();

        private static void TestEqual(ref int value, int expected)
        {
            if (expected < 0)
            {
                value = 0;
                return;
            }

            int retry = 0;
            while (++retry < 100)
            {
                lock (_mutex)
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

        public static void TestRetryCount(int expected) => TestEqual(ref _nRetry, expected);

        public static void TestFailureCount(int expected) => TestEqual(ref _nFailure, expected);

        public static void TestInvocationCount(int expected) => TestEqual(ref _nInvocation, expected);

        public static void TestInvocationReset()
        {
            _nRetry = 0;
            _nFailure = 0;
            _nInvocation = 0;
        }
    }
}
