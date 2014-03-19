// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.hold;

import java.io.PrintStream;
import java.io.PrintWriter;

import test.Ice.hold.Test.HoldPrx;
import test.Ice.hold.Test.HoldPrxHelper;
import test.Ice.hold.Test.Callback_Hold_set;

public class AllTests
{
    private static void
    test(boolean b)
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
        
        synchronized public void
        set(boolean value)
        {
            _value = value;
        }

        synchronized boolean
        value()
        {
            return _value;
        }
    
        private boolean _value;
    };

    static class AMICheckSetValue extends Callback_Hold_set
    {
        public
        AMICheckSetValue(Condition condition, int expected)
        {
            _condition = condition;
            _expected = expected;
        }

        @Override
        public void
        response(int value)
        {
            if(value != _expected)
            {
                _condition.set(false);
            }
        }

        @Override
        public void
        exception(Ice.LocalException ex)
        {
        }

        synchronized public void
        sent(boolean sync)
        {
            _sent = true;
            notify();
        }
    
        synchronized void
        waitForSent()
        {
            while(!_sent)
            {
                try
                {
                    wait();
                }
                catch(java.lang.InterruptedException ex)
                {
                }
            }
        }

        private boolean _sent = false;
        private Condition _condition;
        private int _expected;
    };

    public static void
    allTests(Ice.Communicator communicator, PrintWriter out)
    {
        out.print("testing stringToProxy... ");
        out.flush();
        String ref = "hold:default -p 12010";
        Ice.ObjectPrx base = communicator.stringToProxy(ref);
        test(base != null);
        String refSerialized = "hold:default -p 12011";
        Ice.ObjectPrx baseSerialized = communicator.stringToProxy(refSerialized);
        test(baseSerialized != null);
        out.println("ok");
        
        out.print("testing checked cast... ");
        out.flush();
        HoldPrx hold = HoldPrxHelper.checkedCast(base);
        HoldPrx holdOneway = HoldPrxHelper.uncheckedCast(base.ice_oneway());
        test(hold != null);
        test(hold.equals(base));
        HoldPrx holdSerialized = HoldPrxHelper.checkedCast(baseSerialized);
        HoldPrx holdSerializedOneway = HoldPrxHelper.uncheckedCast(baseSerialized.ice_oneway());
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
            AMICheckSetValue cb = null;
            while(cond.value())
            {
                cb = new AMICheckSetValue(cond, value);
                hold.begin_set(++value, random.nextInt(5), cb);
                if(value % 100 == 0)
                {
                    cb.waitForSent();
                    cb = null;
                }
            }
            if(cb != null)
            {
                cb.waitForSent();
                cb = null;
            }
        }
        out.println("ok");

        out.print("testing with serialize mode... ");
        out.flush();
        {
            Condition cond = new Condition(true);
            int value = 0;
            AMICheckSetValue cb = null;
            while(value < 3000 && cond.value())
            {
                cb = new AMICheckSetValue(cond, value);
                holdSerialized.begin_set(++value, 0, cb);
                if(value % 100 == 0)
                {
                    cb.waitForSent();
                    cb = null;
                }
            }
            if(cb != null)
            {
                cb.waitForSent();
                cb = null;
            }
            test(cond.value());

            for(int i = 0; i < 20000; ++i)
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
