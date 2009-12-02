// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Diagnostics;
using System.Threading;
using System.Collections.Generic;

public class OnewaysNewAMI
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
                    Monitor.Wait(this);
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
                Monitor.Pulse(this);
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
        
        public void twowayOnlyException(Ice.Exception ex)
        {
            try
            {
                throw ex;
            }
            catch(Ice.TwowayOnlyException)
            {
                called();
            }
            catch(Ice.Exception)
            {
                test(false);
            }
        }
    }

    internal static void onewaysNewAMI(Ice.Communicator communicator, Test.MyClassPrx proxy)
    {
        Test.MyClassPrx p = Test.MyClassPrxHelper.uncheckedCast(proxy.ice_oneway());

        {
            Callback cb = new Callback();
            p.begin_ice_ping().whenCompleted(cb.noException).whenSent((Ice.SentCallback)cb.sent);
            cb.check();
        }

        {
            Callback cb = new Callback();
            p.begin_ice_isA("::Test::MyClass").whenCompleted((Ice.ExceptionCallback)cb.twowayOnlyException);
            cb.check();
        }
    
        {
            Callback cb = new Callback();
            p.begin_ice_id().whenCompleted((Ice.ExceptionCallback)cb.twowayOnlyException);
            cb.check();
        }
    
        {
            Callback cb = new Callback();
            p.begin_ice_ids().whenCompleted((Ice.ExceptionCallback)cb.twowayOnlyException);
            cb.check();
        }

        {
            {
                Callback cb = new Callback();
                p.begin_opVoid().whenCompleted(cb.noException).whenSent((Ice.SentCallback)cb.sent);
                cb.check();
            }
        }

        //
        // Test that calling a twoway operation with a oneway proxy raises TwowayOnlyException.
        //
        {
            Callback cb = new Callback();
            p.begin_opByte((byte)0xff, (byte)0x0f).whenCompleted(cb.twowayOnlyException);
            cb.check();
        }
    }
}
