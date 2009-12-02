// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.operations;

import test.Ice.operations.Test.Callback_MyClass_opVoid;
import test.Ice.operations.Test.MyClass;
import test.Ice.operations.Test.MyClassPrx;
import test.Ice.operations.Test.MyClassPrxHelper;
import test.Ice.operations.Test.Callback_MyClass_opVoid;
import test.Ice.operations.Test.Callback_MyClass_opByte;

class OnewaysNewAMI
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
        
        void twowayOnlyException(Ice.LocalException ex)
        {
            try
            {
                throw ex;
            }
            catch(Ice.TwowayOnlyException e)
            {
                called();
            }
            catch(Ice.LocalException e)
            {
                test(false);
            }
        }
    };

    static void
    onewaysNewAMI(test.Util.Application app, MyClassPrx proxy)
    {
        Ice.Communicator communicator = app.communicator();
        MyClassPrx p = (MyClassPrx)proxy.ice_oneway();

        {
            final Callback cb = new Callback();
            Ice.Callback_Object_ice_ping callback = new Ice.Callback_Object_ice_ping()
                {
                    public void
                    response()
                    {
                        test(false);
                    }

                    public void
                    exception(Ice.LocalException ex)
                    {
                        cb.noException(ex);
                    }
                    
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
            final Callback cb = new Callback();
            Ice.Callback_Object_ice_isA callback = new Ice.Callback_Object_ice_isA()
                {
                    public void
                    response(boolean isA)
                    {
                        test(false);
                    }

                    public void 
                    exception(Ice.LocalException ex)
                    {
                        cb.twowayOnlyException(ex);
                    }
                };
            p.begin_ice_isA("::Test::MyClass", callback);
            cb.check();
        }
    
        {
            final Callback cb = new Callback();
            Ice.Callback_Object_ice_id callback = new Ice.Callback_Object_ice_id()
                {
                    public void
                    response(String id)
                    {
                        test(false);
                    }

                    public void 
                    exception(Ice.LocalException ex)
                    {
                        cb.twowayOnlyException(ex);
                    }
                };
            p.begin_ice_id(callback);
            cb.check();
        }
    
        {
            final Callback cb = new Callback();
            Ice.Callback_Object_ice_ids callback = new Ice.Callback_Object_ice_ids()
                {
                    public void
                    response(String[] ids)
                    {
                        test(false);
                    }

                    public void 
                    exception(Ice.LocalException ex)
                    {
                        cb.twowayOnlyException(ex);
                    }
                };
            p.begin_ice_ids(callback);
            cb.check();
        }

        {
            final Callback cb = new Callback();
            Callback_MyClass_opVoid callback = new Callback_MyClass_opVoid()
                {
                    public void
                    response()
                    {
                        test(false);
                    }

                    public void
                    exception(Ice.LocalException ex)
                    {
                        cb.noException(ex);
                    }
                    
                    public void
                    sent(boolean sentSynchronously)
                    {
                        cb.sent(sentSynchronously);
                    }
                };
            p.begin_opVoid(callback);
            cb.check();
        }

        //
        // Test that calling a twoway operation with a oneway proxy raises TwowayOnlyException.
        //
        {
            final Callback cb = new Callback();
            Callback_MyClass_opByte callback = new Callback_MyClass_opByte()
                {
                    public void
                    response(byte r, byte o)
                    {
                        test(false);
                    }

                    public void 
                    exception(Ice.LocalException ex)
                    {
                        cb.twowayOnlyException(ex);
                    }
                };
            p.begin_opByte((byte)0xff, (byte)0x0f, callback);
            cb.check();
        }
    }
}
