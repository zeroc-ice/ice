// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import Test.*;

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

    static class AMICheckSetValue extends AMI_Hold_set implements Ice.AMISentCallback
    {
        public
        AMICheckSetValue(Condition condition, int expected)
        {
            _condition = condition;
            _expected = expected;
        }

        public void
        ice_response(int value)
        {
            if(value != _expected)
            {
                _condition.set(false);
            }
        }

        public void
        ice_exception(Ice.LocalException ex)
        {
        }

        synchronized public void
        ice_sent()
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
    allTests(Ice.Communicator communicator)
    {
        System.out.print("testing stringToProxy... ");
        System.out.flush();
        String ref = "hold:default -p 12010 -t 30000";
        Ice.ObjectPrx base = communicator.stringToProxy(ref);
        test(base != null);
        String refSerialized = "hold:default -p 12011 -t 30000";
        Ice.ObjectPrx baseSerialized = communicator.stringToProxy(refSerialized);
        test(baseSerialized != null);
        System.out.println("ok");
        
        System.out.print("testing checked cast... ");
        System.out.flush();
        HoldPrx hold = HoldPrxHelper.checkedCast(base);
        HoldPrx holdOneway = HoldPrxHelper.uncheckedCast(base.ice_oneway());
        test(hold != null);
        test(hold.equals(base));
        HoldPrx holdSerialized = HoldPrxHelper.checkedCast(baseSerialized);
        HoldPrx holdSerializedOneway = HoldPrxHelper.uncheckedCast(baseSerialized.ice_oneway());
        test(holdSerialized != null);
        test(holdSerialized.equals(baseSerialized));
        System.out.println("ok");
        
        System.out.print("changing state between active and hold rapidly... ");
        System.out.flush();
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
        System.out.println("ok");
        
        System.out.print("testing without serialize mode... ");
        System.out.flush();
        java.util.Random random = new java.util.Random();
        {
            Condition cond = new Condition(true);
            int value = 0;
            AMICheckSetValue cb = null;
            while(cond.value())
            {
                cb = new AMICheckSetValue(cond, value);
                if(hold.set_async(cb, ++value, random.nextInt(5)))
                {
                    cb = null;
                }
                if(value % 100 == 0)
                {
                    if(cb != null)
                    {
                        cb.waitForSent();
                        cb = null;
                    }
                }
            }
            if(cb != null)
            {
                cb.waitForSent();
                cb = null;
            }
        }
        System.out.println("ok");

        System.out.print("testing with serialize mode... ");
        System.out.flush();
        {
            Condition cond = new Condition(true);
            int value = 0;
            AMICheckSetValue cb = null;
            while(value < 3000 && cond.value())
            {
                cb = new AMICheckSetValue(cond, value);
                if(holdSerialized.set_async(cb, ++value, 0))
                {
                    cb = null;
                }
                if(value % 100 == 0)
                {
                    if(cb != null)
                    {
                        cb.waitForSent();
                        cb = null;
                    }
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
        System.out.println("ok");

        System.out.print("changing state to hold and shutting down server... ");
        System.out.flush();
        hold.shutdown();
        System.out.println("ok");
    }
}
        
