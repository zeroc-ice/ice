// **********************************************************************
//
// Copyright (c) 2003-2011 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using Test;

public class AllTests
{
    private static void test(bool b)
    {
        if(!b)
        {
            throw new Exception();
        }
    }

    private class Condition
    {
        public Condition(bool value)
        {
            _value = value;
        }
        
        public void
        set(bool value)
        {
            lock(this)
            {
                _value = value;
            }
        }

        public bool
        value()
        {
            lock(this)
            {
                return _value;
            }
        }
    
        private bool _value;
    }

    private class SetCB
    {
        public
        SetCB(Condition condition, int expected)
        {
            _condition = condition;
            _expected = expected;
        }

        public void
        response(int value)
        {
            if(value != _expected)
            {
                _condition.set(false);
            }
        }

        public void
        exception(Ice.Exception ex)
        {
        }

        public void
        sent(bool sync)
        {
            _m.Lock();
            try
            {
                _sent = true;
                _m.Notify();
            }
            finally
            {
                _m.Unlock();
            }
        }
    
        public void
        waitForSent()
        {
            _m.Lock();
            try
            {
                while(!_sent)
                {
                    _m.Wait();
                }
            }
            finally
            {
                _m.Unlock();
            }
        }

        private bool _sent = false;
        private Condition _condition;
        private int _expected;
        private readonly IceUtilInternal.Monitor _m = new IceUtilInternal.Monitor();
    }

    public static void allTests(Ice.Communicator communicator)
    {
        Console.Out.Write("testing stringToProxy... ");
        Console.Out.Flush();
        String @ref = "hold:default -p 12010";
        Ice.ObjectPrx @base = communicator.stringToProxy(@ref);
        test(@base != null);
        String refSerialized = "hold:default -p 12011";
        Ice.ObjectPrx baseSerialized = communicator.stringToProxy(refSerialized);
        test(baseSerialized != null);
        Console.Out.WriteLine("ok");
        
        Console.Out.Write("testing checked cast... ");
        Console.Out.Flush();
        HoldPrx hold = HoldPrxHelper.checkedCast(@base);
        HoldPrx holdOneway = HoldPrxHelper.uncheckedCast(@base.ice_oneway());
        test(hold != null);
        test(hold.Equals(@base));
        HoldPrx holdSerialized = HoldPrxHelper.checkedCast(baseSerialized);
        HoldPrx holdSerializedOneway = HoldPrxHelper.uncheckedCast(baseSerialized.ice_oneway());
        test(holdSerialized != null);
        test(holdSerialized.Equals(baseSerialized));
        Console.Out.WriteLine("ok");
        
        Console.Out.Write("changing state between active and hold rapidly... ");
        Console.Out.Flush();
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
        Console.Out.WriteLine("ok");
        
        Console.Out.Write("testing without serialize mode... ");
        Console.Out.Flush();
        System.Random rand = new System.Random();
        {
            Condition cond = new Condition(true);
            int value = 0;
            SetCB cb = null;
            while(cond.value())
            {
                cb = new SetCB(cond, value);
                hold.begin_set(++value, value < 500 ? rand.Next(5) : 0).
                        whenCompleted(cb.response, cb.exception).whenSent(cb.sent);
                if(value % 100 == 0)
                {
                    cb.waitForSent();
                    cb = null;
                }

                if(value > 100000)
                {
                    // Don't continue, it's possible that out-of-order dispatch doesn't occur
                    // after 100000 iterations and we don't want the test to last for too long
                    // when this occurs.
                    break;
                }
            }
            if(cb != null)
            {
                cb.waitForSent();
                cb = null;
            }
        }
        Console.Out.WriteLine("ok");

        Console.Out.Write("testing with serialize mode... ");
        Console.Out.Flush();
        {
            Condition cond = new Condition(true);
            int value = 0;
            SetCB cb = null;
            while(value < 3000 && cond.value())
            {
                cb = new SetCB(cond, value);
                holdSerialized.begin_set(++value, value < 500 ? rand.Next(5) : 0).
                        whenCompleted(cb.response, cb.exception).whenSent(cb.sent);
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
        Console.Out.WriteLine("ok");

        Console.Out.Write("testing waitForHold... ");
        Console.Out.Flush();
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
        Console.Out.WriteLine("ok");

        Console.Out.Write("changing state to hold and shutting down server... ");
        Console.Out.Flush();
        hold.shutdown();
        Console.Out.WriteLine("ok");
    }
}
        
