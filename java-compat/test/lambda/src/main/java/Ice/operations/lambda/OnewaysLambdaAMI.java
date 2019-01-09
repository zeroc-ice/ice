// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

package test.Ice.operations.lambda;

import test.Ice.operations.Test.MyClassPrx;

public class OnewaysLambdaAMI
{
    private static void
    test(boolean b)
    {
        if(!b)
        {
            throw new RuntimeException();
        }
    }

    private static class Callback
    {
        Callback()
        {
            _called = false;
        }

        public synchronized void check()
        {
            while(!_called)
            {
                try
                {
                    wait();
                }
                catch(InterruptedException ex)
                {
                }
            }

            _called = false;
        }

        public synchronized void called()
        {
            assert(!_called);
            _called = true;
            notify();
        }

        public void
        sent(boolean sentSynchronously)
        {
            called();
        }

        void noException(Ice.Exception ex)
        {
            test(false);
        }

        private boolean _called;
    }

    public static void
    onewaysLambdaAMI(test.TestHelper helper, MyClassPrx proxy)
    {
        MyClassPrx p = (MyClassPrx)proxy.ice_oneway();

        {
            final Callback cb = new Callback();
            p.begin_ice_ping(
                () -> test(false),
                (Ice.Exception ex) -> cb.noException(ex),
                (boolean sent) -> cb.sent(sent)
            );
            cb.check();
        }

        {
            try
            {
                p.begin_ice_isA("::Test::MyClass");
                test(false);
            }
            catch(java.lang.IllegalArgumentException ex)
            {
            }
        }

        {
            try
            {
                p.begin_ice_id();
                test(false);
            }
            catch(java.lang.IllegalArgumentException ex)
            {
            }
        }

        {
            try
            {
                p.begin_ice_ids();
                test(false);
            }
            catch(java.lang.IllegalArgumentException ex)
            {
            }
        }

        {
            final Callback cb = new Callback();
            p.begin_opVoid(
                () -> test(false),
                (Ice.Exception ex) -> cb.noException(ex),
                (boolean sent) -> cb.sent(sent)
            );
            cb.check();
        }

        {
            final Callback cb = new Callback();
            p.begin_opIdempotent(
                () -> test(false),
                (Ice.Exception ex) -> cb.noException(ex),
                (boolean sent) -> cb.sent(sent)
            );
            cb.check();
        }

        {
            final Callback cb = new Callback();
            p.begin_opNonmutating(
                () -> test(false),
                (Ice.Exception ex) -> cb.noException(ex),
                (boolean sent) -> cb.sent(sent)
            );
            cb.check();
        }

        {
            try
            {
                p.begin_opByte((byte)0xff, (byte)0x0f);
                test(false);
            }
            catch(java.lang.IllegalArgumentException ex)
            {
            }
        }
    }
}
