// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

package test.Ice.retry;

public class Instrumentation
{
    private static void
    test(boolean b)
    {
        if(!b)
        {
            throw new RuntimeException();
        }
    }

    class InvocationObserverI implements Ice.Instrumentation.InvocationObserver
    {
        @Override
        public void
        attach()
        {
        }

        @Override
        public void
        detach()
        {
            synchronized(Instrumentation.class)
            {
                ++nInvocation.value;
            }
        }

        @Override
        public void
        failed(String msg)
        {
            synchronized(Instrumentation.class)
            {
                ++nFailure.value;
            }
        }

        @Override
        public void
        retried()
        {
            synchronized(Instrumentation.class)
            {
                ++nRetry.value;
            }
        }

        @Override
        public void
        userException()
        {
        }

        @Override
        public Ice.Instrumentation.RemoteObserver
        getRemoteObserver(Ice.ConnectionInfo ci, Ice.Endpoint ei, int i, int j)
        {
            return null;
        }

        @Override
        public Ice.Instrumentation.CollocatedObserver
        getCollocatedObserver(Ice.ObjectAdapter adapter, int i , int j)
        {
            return null;
        }

    };
    private Ice.Instrumentation.InvocationObserver invocationObserver = new InvocationObserverI();

    class CommunicatorObserverI implements Ice.Instrumentation.CommunicatorObserver
    {
        @Override
        public Ice.Instrumentation.Observer
        getConnectionEstablishmentObserver(Ice.Endpoint e, String s)
        {
            return null;
        }

        @Override
        public Ice.Instrumentation.Observer
        getEndpointLookupObserver(Ice.Endpoint e)
        {
            return null;
        }

        @Override
        public Ice.Instrumentation.ConnectionObserver
        getConnectionObserver(Ice.ConnectionInfo ci,
                              Ice.Endpoint ei,
                              Ice.Instrumentation.ConnectionState s,
                              Ice.Instrumentation.ConnectionObserver o)
        {
            return null;
        }

        @Override
        public Ice.Instrumentation.ThreadObserver
        getThreadObserver(String p,
                          String n,
                          Ice.Instrumentation.ThreadState s,
                          Ice.Instrumentation.ThreadObserver o)
        {
            return null;
        }

        @Override
        public Ice.Instrumentation.InvocationObserver
        getInvocationObserver(Ice.ObjectPrx p, String o, java.util.Map<String, String> c)
        {
            return invocationObserver;
        }

        @Override
        public Ice.Instrumentation.DispatchObserver
        getDispatchObserver(Ice.Current c, int i)
        {
            return null;
        }

        @Override
        public void
        setObserverUpdater(Ice.Instrumentation.ObserverUpdater u)
        {
        }
    };

    private Ice.Instrumentation.CommunicatorObserver communicatorObserver = new CommunicatorObserverI();

    public Ice.Instrumentation.CommunicatorObserver
    getObserver()
    {
        return communicatorObserver;
    }

    static private void
    testEqual(Ice.IntHolder value, int expected)
    {
        if(expected < 0)
        {
            value.value = 0;
            return;
        }

        int retry = 0;
        while(++retry < 100)
        {
            synchronized(Instrumentation.class)
            {
                if(value.value == expected)
                {
                    break;
                }
            }
            try
            {
                Thread.sleep(10);
            }
            catch(java.lang.InterruptedException ex)
            {
            }
        }
        if(value.value != expected)
        {
            System.err.println("value = " + value.value + ", expected = " + expected);
            test(false);
        }
        value.value = 0;
    }

    public void
    testRetryCount(int expected)
    {
        testEqual(nRetry, expected);
    }

    public void
    testFailureCount(int expected)
    {
        testEqual(nFailure, expected);
    }

    public void
    testInvocationCount(int expected)
    {
        testEqual(nInvocation, expected);
    }

    private Ice.IntHolder nRetry = new Ice.IntHolder(0);
    private Ice.IntHolder nFailure = new Ice.IntHolder(0);
    private Ice.IntHolder nInvocation = new Ice.IntHolder(0);
};
