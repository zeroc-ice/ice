//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Collections.Generic;

namespace Ice.retry
{
    public class Instrumentation
    {
        static object mutex = new object();

        class InvocationObserver : Ice.Instrumentation.IInvocationObserver
        {
            public void
            attach()
            {
            }

            public void
            detach()
            {
                lock (mutex)
                {
                    ++nInvocation;
                }
            }

            public void
            failed(string msg)
            {
                lock (mutex)
                {
                    ++nFailure;
                }
            }

            public void
            retried()
            {
                lock (mutex)
                {
                    ++nRetry;
                }
            }

            public void
            userException()
            {
            }

            public Ice.Instrumentation.IRemoteObserver
            getRemoteObserver(ConnectionInfo ci, IEndpoint ei, int i, int j)
            {
                return null;
            }

            public Ice.Instrumentation.ICollocatedObserver
            getCollocatedObserver(Ice.ObjectAdapter adapter, int i, int j)
            {
                return null;
            }

        };

        static private Ice.Instrumentation.IInvocationObserver invocationObserver = new InvocationObserver();

        class CommunicatorObserverI : Ice.Instrumentation.ICommunicatorObserver
        {
            public Ice.Instrumentation.IObserver
            getConnectionEstablishmentObserver(Ice.IEndpoint e, string s)
            {
                return null;
            }

            public Ice.Instrumentation.IObserver
            getEndpointLookupObserver(Ice.IEndpoint e)
            {
                return null;
            }

            public Ice.Instrumentation.IConnectionObserver
            getConnectionObserver(Ice.ConnectionInfo ci,
                                    Ice.IEndpoint ei,
                                    Ice.Instrumentation.ConnectionState s,
                                    Ice.Instrumentation.IConnectionObserver o)
            {
                return null;
            }

            public Ice.Instrumentation.IThreadObserver
            getThreadObserver(string p,
                                string n,
                                Ice.Instrumentation.ThreadState s,
                                Ice.Instrumentation.IThreadObserver o)
            {
                return null;
            }

            public Ice.Instrumentation.IInvocationObserver
            getInvocationObserver(Ice.IObjectPrx p, string o, Dictionary<string, string> c)
            {
                return invocationObserver;
            }

            public Ice.Instrumentation.IDispatchObserver
            getDispatchObserver(Ice.Current c, int i)
            {
                return null;
            }

            public void
            setObserverUpdater(Ice.Instrumentation.IObserverUpdater u)
            {
            }
        };

        static private Ice.Instrumentation.ICommunicatorObserver communicatorObserver = new CommunicatorObserverI();

        static public Ice.Instrumentation.ICommunicatorObserver
        getObserver()
        {
            return communicatorObserver;
        }

        static private void
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
                System.Diagnostics.Debug.Assert(false);
                throw new System.Exception();
            }
            value = 0;
        }

        static public void
        testRetryCount(int expected)
        {
            testEqual(ref nRetry, expected);
        }

        static public void
        testFailureCount(int expected)
        {
            testEqual(ref nFailure, expected);
        }

        static public void
        testInvocationCount(int expected)
        {
            testEqual(ref nInvocation, expected);
        }

        static public void
        testInvocationReset()
        {
            nRetry = 0;
            nFailure = 0;
            nInvocation = 0;
        }

        static private int nRetry = 0;
        static private int nFailure = 0;
        static private int nInvocation = 0;
    }
}
