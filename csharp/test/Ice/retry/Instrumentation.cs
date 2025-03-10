// Copyright (c) ZeroC, Inc.

namespace Ice.retry
{
        public class Instrumentation
        {
            private static readonly object mutex = new();

            private class InvocationObserverI : Ice.Instrumentation.InvocationObserver
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

                public Ice.Instrumentation.RemoteObserver
                getRemoteObserver(Ice.ConnectionInfo ci, Ice.Endpoint ei, int i, int j) => null;

                public Ice.Instrumentation.CollocatedObserver
                getCollocatedObserver(Ice.ObjectAdapter adapter, int i, int j) => null;
            }
            private static readonly Ice.Instrumentation.InvocationObserver invocationObserver = new InvocationObserverI();

            private class CommunicatorObserverI : Ice.Instrumentation.CommunicatorObserver
            {
                public Ice.Instrumentation.Observer
                getConnectionEstablishmentObserver(Ice.Endpoint e, string s) => null;

                public Ice.Instrumentation.Observer
                getEndpointLookupObserver(Ice.Endpoint e) => null;

                public Ice.Instrumentation.ConnectionObserver
                getConnectionObserver(ConnectionInfo ci,
                                      Endpoint ei,
                                      Ice.Instrumentation.ConnectionState s,
                                      Ice.Instrumentation.ConnectionObserver o) => null;

                public Ice.Instrumentation.ThreadObserver
                getThreadObserver(string p,
                                  string n,
                                  Ice.Instrumentation.ThreadState s,
                                  Ice.Instrumentation.ThreadObserver o) => null;

                public Ice.Instrumentation.InvocationObserver
                getInvocationObserver(Ice.ObjectPrx p, string o, Dictionary<string, string> c) => invocationObserver;

                public Ice.Instrumentation.DispatchObserver
                getDispatchObserver(Ice.Current c, int i) => null;

                public void
                setObserverUpdater(Ice.Instrumentation.ObserverUpdater u)
                {
                }
            }

            private static readonly Ice.Instrumentation.CommunicatorObserver communicatorObserver = new CommunicatorObserverI();

            public static Ice.Instrumentation.CommunicatorObserver
            getObserver() => communicatorObserver;

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
                    System.Diagnostics.Debug.Assert(false);
                    throw new System.Exception();
                }
                value = 0;
            }

            public static void
            testRetryCount(int expected) => testEqual(ref nRetry, expected);

            public static void
            testFailureCount(int expected) => testEqual(ref nFailure, expected);

            public static void
            testInvocationCount(int expected) => testEqual(ref nInvocation, expected);

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

