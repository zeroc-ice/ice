// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.hold;

import java.io.PrintWriter;
import java.util.concurrent.CompletableFuture;

import com.zeroc.Ice.InvocationFuture;

import test.Ice.hold.Test.HoldPrx;

public class AllTests
{
    private static void test(boolean b)
    {
        if(!b)
        {
            throw new RuntimeException();
        }
    }

    static class Condition
    {
        public Condition(boolean value)
        {
            _value = value;
        }

        synchronized public void set(boolean value)
        {
            _value = value;
        }

        synchronized boolean value()
        {
            return _value;
        }

        private boolean _value;
    }

    static class AMICheckSetValue
    {
        public AMICheckSetValue(Condition condition, int expected)
        {
            _condition = condition;
            _expected = expected;
        }

        public void response(int value)
        {
            if(value != _expected)
            {
                _condition.set(false);
            }
        }

        private Condition _condition;
        private int _expected;
    }

    public static void allTests(test.TestHelper helper)
    {
        PrintWriter out = helper.getWriter();
        com.zeroc.Ice.Communicator communicator = helper.communicator();
        out.print("testing stringToProxy... ");
        out.flush();
        String ref = "hold:" + helper.getTestEndpoint(0);
        com.zeroc.Ice.ObjectPrx base = communicator.stringToProxy(ref);
        test(base != null);
        String refSerialized = "hold:" + helper.getTestEndpoint(1);
        com.zeroc.Ice.ObjectPrx baseSerialized = communicator.stringToProxy(refSerialized);
        test(baseSerialized != null);
        out.println("ok");

        out.print("testing checked cast... ");
        out.flush();
        HoldPrx hold = HoldPrx.checkedCast(base);
        HoldPrx holdOneway = hold.ice_oneway();
        test(hold != null);
        test(hold.equals(base));
        HoldPrx holdSerialized = HoldPrx.checkedCast(baseSerialized);
        HoldPrx holdSerializedOneway = holdSerialized.ice_oneway();
        test(holdSerialized != null);
        test(holdSerialized.equals(baseSerialized));
        out.println("ok");

        out.print("changing state between active and hold rapidly... ");
        out.flush();
        for(int i = 0; i < 100; ++i)
        {
            hold.putOnHold(0);
        }
        for(int i = 0; i < 100; ++i)
        {
            holdOneway.putOnHold(0);
        }
        for(int i = 0; i < 100; ++i)
        {
            holdSerialized.putOnHold(0);
        }
        for(int i = 0; i < 100; ++i)
        {
            holdSerializedOneway.putOnHold(0);
        }
        out.println("ok");

        out.print("testing without serialize mode... ");
        out.flush();
        java.util.Random random = new java.util.Random();
        {
            Condition cond = new Condition(true);
            int value = 0;
            CompletableFuture<Integer> r = null;
            InvocationFuture<Integer> f = null;
            while(cond.value())
            {
                AMICheckSetValue cb = new AMICheckSetValue(cond, value);
                r = hold.setAsync(value + 1, random.nextInt(5));
                f = com.zeroc.Ice.Util.getInvocationFuture(r);
                r.whenComplete((result, ex) ->
                    {
                        test(ex == null);
                        cb.response(result);
                    });
                ++value;
                if(value % 100 == 0)
                {
                    f.waitForSent();
                }

                if(value > 1000000)
                {
                    // Don't continue, it's possible that out-of-order dispatch doesn't occur
                    // after 100000 iterations and we don't want the test to last for too long
                    // when this occurs.
                    break;
                }
            }
            test(value > 100000 || !cond.value());
            r.join();
        }
        out.println("ok");

        out.print("testing with serialize mode... ");
        out.flush();
        {
            Condition cond = new Condition(true);
            int value = 0;
            CompletableFuture<Integer> r = null;
            InvocationFuture<Integer> f = null;
            while(value < 3000 && cond.value())
            {
                AMICheckSetValue cb = new AMICheckSetValue(cond, value);
                r = holdSerialized.setAsync(value + 1, random.nextInt(1));
                f = com.zeroc.Ice.Util.getInvocationFuture(r);
                r.whenComplete((result, ex) ->
                    {
                        test(ex == null);
                        cb.response(result);
                    });
                ++value;
                if(value % 100 == 0)
                {
                    f.waitForSent();
                }
            }
            r.join();
            test(cond.value());

            for(int i = 0; i < 10000; ++i)
            {
                holdSerializedOneway.setOneway(value + 1, value);
                ++value;
                if((i % 100) == 0)
                {
                    holdSerializedOneway.putOnHold(1);
                }
            }
        }
        out.println("ok");

        out.print("testing serialization... ");
        out.flush();
        {
            int value = 0;
            holdSerialized.set(value, 0);
            CompletableFuture<Void> r = null;
            InvocationFuture<Void> f = null;
            int max = helper.isAndroid() ? 5000 : 10000;
            for(int i = 0; i < max; ++i)
            {
                // Create a new proxy for each request
                r = holdSerialized.ice_oneway().setOnewayAsync(value + 1, value);
                f = com.zeroc.Ice.Util.getInvocationFuture(r);
                ++value;
                if((i % 100) == 0)
                {
                    f.waitForSent();
                    holdSerialized.ice_ping(); // Ensure everything's dispatched
                    holdSerialized.ice_getConnection().close(com.zeroc.Ice.ConnectionClose.GracefullyWithWait);
                }
            }
            r.join();
        }
        out.println("ok");

        out.print("testing waitForHold... ");
        out.flush();
        {
            hold.waitForHold();
            hold.waitForHold();
            for(int i = 0; i < 1000; ++i)
            {
                holdOneway.ice_ping();
                if((i % 20) == 0)
                {
                    hold.putOnHold(0);
                }
            }
            hold.putOnHold(-1);
            hold.ice_ping();
            hold.putOnHold(-1);
            hold.ice_ping();
        }
        out.println("ok");

        out.print("changing state to hold and shutting down server... ");
        out.flush();
        hold.shutdown();
        out.println("ok");
    }
}
