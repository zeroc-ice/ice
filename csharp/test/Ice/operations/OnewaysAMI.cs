// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Diagnostics;
using System.Threading;
using System.Collections.Generic;

public class OnewaysAMI
{
    private static void test(bool b)
    {
        if(!b)
        {
            throw new System.SystemException();
        }
    }

    private class CallbackBase
    {
        internal CallbackBase()
        {
            _called = false;
        }

        public virtual void check()
        {
            lock(this)
            {
                while(!_called)
                {
                    System.Threading.Monitor.Wait(this);
                }
                _called = false;
            }
        }

        public virtual void called()
        {
            lock(this)
            {
                Debug.Assert(!_called);
                _called = true;
                System.Threading.Monitor.Pulse(this);
            }
        }

        private bool _called;
    }

    private class Callback : CallbackBase
    {
        public Callback()
        {
        }

        public void
        sent(bool sentSynchronously)
        {
            called();
        }

        public void noException(Ice.Exception ex)
        {
            test(false);
        }
    }

    internal static void onewaysAMI(Ice.Communicator communicator, Test.MyClassPrx proxy)
    {
        Test.MyClassPrx p = Test.MyClassPrxHelper.uncheckedCast(proxy.ice_oneway());

        {
            Callback cb = new Callback();
            p.begin_ice_ping().whenCompleted(cb.noException).whenSent(cb.sent);
            cb.check();
        }

        {
            Callback cb = new Callback();
            p.begin_ice_ping().whenCompleted(
                (Ice.Exception ex) =>
                {
                    cb.noException(ex);
                }
            ).whenSent(
                (bool sentSynchronously) =>
                {
                    cb.sent(sentSynchronously);
                });
            cb.check();
        }

        {
            try
            {
                p.begin_ice_isA("::Test::MyClass");
                test(false);
            }
            catch(System.ArgumentException)
            {
            }
        }

        {
            try
            {
                p.begin_ice_id();
                test(false);
            }
            catch(System.ArgumentException)
            {
            }
        }

        {
            try
            {
                p.begin_ice_ids();
                test(false);
            }
            catch(System.ArgumentException)
            {
            }
        }

        {
            Callback cb = new Callback();
            p.begin_opVoid().whenCompleted(cb.noException).whenSent(cb.sent);
            cb.check();
        }

        {
            Callback cb = new Callback();
            p.begin_opVoid().whenCompleted(
                (Ice.Exception ex) =>
                {
                    cb.noException(ex);
                }
            ).whenSent(
                (bool sentSynchronously) =>
                {
                    cb.sent(sentSynchronously);
                });
            cb.check();
        }

        {
            Callback cb = new Callback();
            p.begin_opIdempotent().whenCompleted(cb.noException).whenSent(cb.sent);
            cb.check();
        }

        {
            Callback cb = new Callback();
            p.begin_opIdempotent().whenCompleted(
                (Ice.Exception ex) =>
                {
                    cb.noException(ex);
                }
            ).whenSent(
                (bool sentSynchronously) =>
                {
                    cb.sent(sentSynchronously);
                });
            cb.check();
        }

        {
            Callback cb = new Callback();
            p.begin_opNonmutating().whenCompleted(cb.noException).whenSent(cb.sent);
            cb.check();
        }

        {
            Callback cb = new Callback();
            p.begin_opNonmutating().whenCompleted(
                (Ice.Exception ex) =>
                {
                    cb.noException(ex);
                }
            ).whenSent(
                (bool sentSynchronously) =>
                {
                    cb.sent(sentSynchronously);
                });
            cb.check();
        }

        {
            try
            {
                p.begin_opByte((byte)0xff, (byte)0x0f);
                test(false);
            }
            catch(System.ArgumentException)
            {
            }
        }
    }
}
