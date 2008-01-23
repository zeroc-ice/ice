// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

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

    private static class Callback
    {
        Callback()
        {
            _called = false;
        }

        public synchronized boolean
        check()
        {
            while(!_called)
            {
                try
                {
                    wait(5000);
                }
                catch(InterruptedException ex)
                {
                    continue;
                }

                if(!_called)
                {
                    return false; // Must be timeout.
                }
            }

            _called = false;
            return true;
        }
        
        public synchronized void
        called()
        {
            assert(!_called);
            _called = true;
            notify();
        }

        private boolean _called;
    }

    private static class AMI_MyClass_opVoidI extends Test.AMI_MyClass_opVoid
    {
        public void
        ice_response()
        {
            test(false);
        }

        public void
        ice_exception(Ice.LocalException ex)
        {
            test(false);
        }
    }

    private static class AMI_MyClass_opVoidExI extends Test.AMI_MyClass_opVoid
    {
        public void
        ice_response()
        {
            test(false);
        }

        public void
        ice_exception(Ice.LocalException ex)
        {
            test(ex instanceof Ice.NoEndpointException);
            callback.called();
        }

        public boolean
        check()
        {
            return callback.check();
        }

        private Callback callback = new Callback();
    }

    private static class AMI_MyClass_opByteExI extends Test.AMI_MyClass_opByte
    {
        public void
        ice_response(byte r, byte b)
        {
            test(false);
        }

        public void
        ice_exception(Ice.LocalException ex)
        {
            test(ex instanceof Ice.TwowayOnlyException);
            callback.called();
        }

        public boolean
        check()
        {
            return callback.check();
        }

        private Callback callback = new Callback();
    }

    static void
    onewaysAMI(Ice.Communicator communicator, Test.MyClassPrx p)
    {
        p = Test.MyClassPrxHelper.uncheckedCast(p.ice_oneway());

        {
            AMI_MyClass_opVoidI cb = new AMI_MyClass_opVoidI();
            p.opVoid_async(cb);
            // Let's check if we can reuse the same callback object for another call.
            p.opVoid_async(cb);
        }

        {
            // Check that a call to a void operation raises NoEndpointException
            // in the ice_exception() callback instead of at the point of call.
            Test.MyClassPrx indirect = Test.MyClassPrxHelper.uncheckedCast(p.ice_adapterId("dummy"));
            AMI_MyClass_opVoidExI cb = new AMI_MyClass_opVoidExI();
            try
            {
                indirect.opVoid_async(cb);
            }
            catch(java.lang.Exception ex)
            {
                test(false);
            }
            test(cb.check());
        }

        {
            AMI_MyClass_opByteExI cb = new AMI_MyClass_opByteExI();
            p.opByte_async(cb, (byte)0xff, (byte)0x0f);
            test(cb.check());
        }

    }
}