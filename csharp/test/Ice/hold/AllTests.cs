// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using Test;

public class AllTests : TestCommon.AllTests
{
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

        private Condition _condition;
        private int _expected;
    }

    public static void allTests(TestCommon.Application app)
    {
        Ice.Communicator communicator = app.communicator();
        Write("testing stringToProxy... ");
        Flush();
        String @ref = "hold:" + app.getTestEndpoint(0);
        Ice.ObjectPrx @base = communicator.stringToProxy(@ref);
        test(@base != null);
        String refSerialized = "hold:" + app.getTestEndpoint(1);
        Ice.ObjectPrx baseSerialized = communicator.stringToProxy(refSerialized);
        test(baseSerialized != null);
        WriteLine("ok");

        Write("testing checked cast... ");
        Flush();
        HoldPrx hold = HoldPrxHelper.checkedCast(@base);
        HoldPrx holdOneway = HoldPrxHelper.uncheckedCast(@base.ice_oneway());
        test(hold != null);
        test(hold.Equals(@base));
        HoldPrx holdSerialized = HoldPrxHelper.checkedCast(baseSerialized);
        HoldPrx holdSerializedOneway = HoldPrxHelper.uncheckedCast(baseSerialized.ice_oneway());
        test(holdSerialized != null);
        test(holdSerialized.Equals(baseSerialized));
        WriteLine("ok");

        Write("changing state between active and hold rapidly... ");
        Flush();
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
        WriteLine("ok");

        Write("testing without serialize mode... ");
        Flush();
        System.Random rand = new System.Random();
        {
            Condition cond = new Condition(true);
            int value = 0;
            Ice.AsyncResult result = null;
            while(cond.value())
            {
                SetCB cb = new SetCB(cond, value);
                result = hold.begin_set(++value, value < 500 ? rand.Next(5) : 0).whenCompleted(cb.response,
                                                                                               cb.exception);
                if(value % 100 == 0)
                {
                    result.waitForSent();
                }

                if(value > 100000)
                {
                    // Don't continue, it's possible that out-of-order dispatch doesn't occur
                    // after 100000 iterations and we don't want the test to last for too long
                    // when this occurs.
                    break;
                }
            }
            test(value > 100000 || !cond.value());
            result.waitForSent();
        }
        WriteLine("ok");

        Write("testing with serialize mode... ");
        Flush();
        {
            Condition cond = new Condition(true);
            int value = 0;
            Ice.AsyncResult result = null;
            while(value < 3000 && cond.value())
            {
                SetCB cb = new SetCB(cond, value);
                result = holdSerialized.begin_set(++value, 0).whenCompleted(cb.response, cb.exception);
                if(value % 100 == 0)
                {
                    result.waitForSent();
                }
            }
            result.waitForCompleted();
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
        WriteLine("ok");

        Write("testing serialization... ");
        Flush();
        {
            int value = 0;
            holdSerialized.set(value, 0);
            Ice.AsyncResult result = null;
            for(int i = 0; i < 10000; ++i)
            {
                // Create a new proxy for each request
                result = ((HoldPrx)holdSerialized.ice_oneway()).begin_setOneway(value + 1, value);
                ++value;
                if((i % 100) == 0)
                {
                    result.waitForSent();
                    holdSerialized.ice_ping(); // Ensure everything's dispatched.
                    holdSerialized.ice_getConnection().close(Ice.ConnectionClose.GracefullyWithWait);
                }
            }
            result.waitForCompleted();
        }
        WriteLine("ok");

        Write("testing waitForHold... ");
        Flush();
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
        WriteLine("ok");

        Write("changing state to hold and shutting down server... ");
        Flush();
        hold.shutdown();
        WriteLine("ok");
    }
}
