// Copyright (c) ZeroC, Inc.

package test.Ice.retry;

import com.zeroc.Ice.ConnectionInfo;
import com.zeroc.Ice.Current;
import com.zeroc.Ice.Endpoint;
import com.zeroc.Ice.Instrumentation.*;
import com.zeroc.Ice.ObjectAdapter;
import com.zeroc.Ice.ObjectPrx;

import java.util.Map;

public class Instrumentation {
    private static void test(boolean b) {
        if (!b) {
            throw new RuntimeException();
        }
    }

    class InvocationObserverI implements InvocationObserver {
        @Override
        public void attach() {
        }

        @Override
        public void detach() {
            synchronized (Instrumentation.class) {
                ++nInvocation.value;
            }
        }

        @Override
        public void failed(String msg) {
            synchronized (Instrumentation.class) {
                ++nFailure.value;
            }
        }

        @Override
        public void retried() {
            synchronized (Instrumentation.class) {
                ++nRetry.value;
            }
        }

        @Override
        public void userException() {
        }

        @Override
        public RemoteObserver getRemoteObserver(
                ConnectionInfo ci, Endpoint ei, int i, int j) {
            return null;
        }

        @Override
        public CollocatedObserver getCollocatedObserver(
                ObjectAdapter adapter, int i, int j) {
            return null;
        }
    }

    private final InvocationObserver invocationObserver =
        new InvocationObserverI();

    class CommunicatorObserverI implements CommunicatorObserver {
        @Override
        public Observer getConnectionEstablishmentObserver(
                Endpoint e, String s) {
            return null;
        }

        @Override
        public Observer getEndpointLookupObserver(
                Endpoint e) {
            return null;
        }

        @Override
        public ConnectionObserver getConnectionObserver(
                ConnectionInfo ci,
                Endpoint ei,
                ConnectionState s,
                ConnectionObserver o) {
            return null;
        }

        @Override
        public ThreadObserver getThreadObserver(
                String p,
                String n,
                ThreadState s,
                ThreadObserver o) {
            return null;
        }

        @Override
        public InvocationObserver getInvocationObserver(
                ObjectPrx p, String o, Map<String, String> c) {
            return invocationObserver;
        }

        @Override
        public DispatchObserver getDispatchObserver(
                Current c, int i) {
            return null;
        }

        @Override
        public void setObserverUpdater(ObserverUpdater u) {
        }
    }

    private final CommunicatorObserver communicatorObserver =
        new CommunicatorObserverI();

    public CommunicatorObserver getObserver() {
        return communicatorObserver;
    }

    private static void testEqual(IntValue value, int expected) {
        if (expected < 0) {
            value.value = 0;
            return;
        }

        int retry = 0;
        while (++retry < 100) {
            synchronized (Instrumentation.class) {
                if (value.value == expected) {
                    break;
                }
            }
            try {
                Thread.sleep(10);
            } catch (InterruptedException ex) {
            }
        }
        if (value.value != expected) {
            System.err.println("value = " + value.value + ", expected = " + expected);
            test(false);
        }
        value.value = 0;
    }

    public void testRetryCount(int expected) {
        testEqual(nRetry, expected);
    }

    public void testFailureCount(int expected) {
        testEqual(nFailure, expected);
    }

    public void testInvocationCount(int expected) {
        testEqual(nInvocation, expected);
    }

    static class IntValue {
        IntValue(int v) {
            value = v;
        }

        int value;
    }

    private final IntValue nRetry = new IntValue(0);
    private final IntValue nFailure = new IntValue(0);
    private final IntValue nInvocation = new IntValue(0);
}
