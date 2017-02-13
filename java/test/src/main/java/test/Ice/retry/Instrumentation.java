// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.retry;

public class Instrumentation
{
    private static void test(boolean b)
    {
        if(!b)
        {
            throw new RuntimeException();
        }
    }

    class InvocationObserverI implements com.zeroc.Ice.Instrumentation.InvocationObserver
    {
        @Override
        public void attach()
        {
        }

        @Override
        public void detach()
        {
            synchronized(Instrumentation.class)
            {
                ++nInvocation.value;
            }
        }

        @Override
        public void failed(String msg)
        {
            synchronized(Instrumentation.class)
            {
                ++nFailure.value;
            }
        }

        @Override
        public void retried()
        {
            synchronized(Instrumentation.class)
            {
                ++nRetry.value;
            }
        }

        @Override
        public void userException()
        {
        }

        @Override
        public com.zeroc.Ice.Instrumentation.RemoteObserver getRemoteObserver(com.zeroc.Ice.ConnectionInfo ci,
                                                                              com.zeroc.Ice.Endpoint ei, int i, int j)
        {
            return null;
        }

        @Override
        public com.zeroc.Ice.Instrumentation.CollocatedObserver getCollocatedObserver(
            com.zeroc.Ice.ObjectAdapter adapter,
            int i,
            int j)
        {
            return null;
        }

    }
    private com.zeroc.Ice.Instrumentation.InvocationObserver invocationObserver = new InvocationObserverI();

    class CommunicatorObserverI implements com.zeroc.Ice.Instrumentation.CommunicatorObserver
    {
        @Override
        public com.zeroc.Ice.Instrumentation.Observer getConnectionEstablishmentObserver(com.zeroc.Ice.Endpoint e,
                                                                                         String s)
        {
            return null;
        }

        @Override
        public com.zeroc.Ice.Instrumentation.Observer getEndpointLookupObserver(com.zeroc.Ice.Endpoint e)
        {
            return null;
        }

        @Override
        public com.zeroc.Ice.Instrumentation.ConnectionObserver getConnectionObserver(
            com.zeroc.Ice.ConnectionInfo ci,
            com.zeroc.Ice.Endpoint ei,
            com.zeroc.Ice.Instrumentation.ConnectionState s,
            com.zeroc.Ice.Instrumentation.ConnectionObserver o)
        {
            return null;
        }

        @Override
        public com.zeroc.Ice.Instrumentation.ThreadObserver getThreadObserver(
            String p,
            String n,
            com.zeroc.Ice.Instrumentation.ThreadState s,
            com.zeroc.Ice.Instrumentation.ThreadObserver o)
        {
            return null;
        }

        @Override
        public com.zeroc.Ice.Instrumentation.InvocationObserver getInvocationObserver(com.zeroc.Ice.ObjectPrx p,
                                                                                      String o,
                                                                                      java.util.Map<String, String> c)
        {
            return invocationObserver;
        }

        @Override
        public com.zeroc.Ice.Instrumentation.DispatchObserver getDispatchObserver(com.zeroc.Ice.Current c, int i)
        {
            return null;
        }

        @Override
        public void setObserverUpdater(com.zeroc.Ice.Instrumentation.ObserverUpdater u)
        {
        }
    }
    private com.zeroc.Ice.Instrumentation.CommunicatorObserver communicatorObserver = new CommunicatorObserverI();

    public com.zeroc.Ice.Instrumentation.CommunicatorObserver getObserver()
    {
        return communicatorObserver;
    }

    static private void testEqual(IntValue value, int expected)
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

    public void testRetryCount(int expected)
    {
        testEqual(nRetry, expected);
    }

    public void testFailureCount(int expected)
    {
        testEqual(nFailure, expected);
    }

    public void testInvocationCount(int expected)
    {
        testEqual(nInvocation, expected);
    }

    static class IntValue
    {
        IntValue(int v)
        {
            value = v;
        }

        int value;
    }

    private IntValue nRetry = new IntValue(0);
    private IntValue nFailure = new IntValue(0);
    private IntValue nInvocation = new IntValue(0);
}
