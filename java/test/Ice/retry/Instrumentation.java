// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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

    static class InvocationObserverI implements Ice.Instrumentation.InvocationObserver
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
    static private Ice.Instrumentation.InvocationObserver invocationObserver = new InvocationObserverI();

    static class CommunicatorObserverI implements Ice.Instrumentation.CommunicatorObserver
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
    
    static private Ice.Instrumentation.CommunicatorObserver communicatorObserver = new CommunicatorObserverI();

    static public Ice.Instrumentation.CommunicatorObserver
    getObserver()
    {
        return communicatorObserver;
    }

    static private void
    testEqual(Ice.IntHolder value, int expected)
    {
        int retry = 0;
        while(retry < 100)
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
    }
    
    static public void
    testRetryCount(int expected)
    {
        testEqual(nRetry, expected);
    }

    static public void
    testFailureCount(int expected)
    {
        testEqual(nFailure, expected);
    }

    static public void
    testInvocationCount(int expected)
    {
        testEqual(nInvocation, expected);
    }

    static private Ice.IntHolder nRetry = new Ice.IntHolder(0);
    static private Ice.IntHolder nFailure = new Ice.IntHolder(0);
    static private Ice.IntHolder nInvocation = new Ice.IntHolder(0);
};
