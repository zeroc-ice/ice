// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.operations;

import test.Ice.operations.Test.Callback_MyClass_opIdempotent;
import test.Ice.operations.Test.Callback_MyClass_opNonmutating;
import test.Ice.operations.Test.Callback_MyClass_opVoid;
import test.Ice.operations.Test.MyClassPrx;

class OnewaysAMI
{
    private static void
    test(boolean b)
    {
        if(!b)
        {
            throw new RuntimeException();
        }
    }

    private static class CallbackBase
    {
        CallbackBase()
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

        private boolean _called;
    }

    static class Callback extends CallbackBase
    {
        public Callback()
        {
        }

        public void
        sent(boolean sentSynchronously)
        {
            called();
        }

        void noException(Ice.LocalException ex)
        {
            test(false);
        }
    }

    static void
    onewaysAMI(test.Util.Application app, MyClassPrx proxy)
    {
        MyClassPrx p = (MyClassPrx)proxy.ice_oneway();

        {
            final Callback cb = new Callback();
            Ice.Callback_Object_ice_ping callback = new Ice.Callback_Object_ice_ping()
                {
                    @Override
                    public void
                    response()
                    {
                        test(false);
                    }

                    @Override
                    public void
                    exception(Ice.LocalException ex)
                    {
                        cb.noException(ex);
                    }

                    @Override
                    public void
                    sent(boolean sentSynchronously)
                    {
                        cb.sent(sentSynchronously);
                    }
                };
            p.begin_ice_ping(callback);
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
            Callback_MyClass_opVoid callback = new Callback_MyClass_opVoid()
                {
                    @Override
                    public void
                    response()
                    {
                        test(false);
                    }

                    @Override
                    public void
                    exception(Ice.LocalException ex)
                    {
                        cb.noException(ex);
                    }

                    @Override
                    public void
                    sent(boolean sentSynchronously)
                    {
                        cb.sent(sentSynchronously);
                    }
                };
            p.begin_opVoid(callback);
            cb.check();
        }

        {
            final Callback cb = new Callback();
            Callback_MyClass_opIdempotent callback = new Callback_MyClass_opIdempotent()
                {
                    @Override
                    public void
                    response()
                    {
                        test(false);
                    }

                    @Override
                    public void
                    exception(Ice.LocalException ex)
                    {
                        cb.noException(ex);
                    }

                    @Override
                    public void
                    sent(boolean sentSynchronously)
                    {
                        cb.sent(sentSynchronously);
                    }
                };
            p.begin_opIdempotent(callback);
            cb.check();
        }

        {
            final Callback cb = new Callback();
            Callback_MyClass_opNonmutating callback = new Callback_MyClass_opNonmutating()
                {
                    @Override
                    public void
                    response()
                    {
                        test(false);
                    }

                    @Override
                    public void
                    exception(Ice.LocalException ex)
                    {
                        cb.noException(ex);
                    }

                    @Override
                    public void
                    sent(boolean sentSynchronously)
                    {
                        cb.sent(sentSynchronously);
                    }
                };
            p.begin_opNonmutating(callback);
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
