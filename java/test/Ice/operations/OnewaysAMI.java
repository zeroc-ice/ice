// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.operations;

import test.Ice.operations.Test.AMI_MyClass_opByte;
import test.Ice.operations.Test.AMI_MyClass_opIdempotent;
import test.Ice.operations.Test.AMI_MyClass_opNonmutating;
import test.Ice.operations.Test.AMI_MyClass_opVoid;
import test.Ice.operations.Test.MyClassPrx;
import test.Ice.operations.Test.MyClassPrxHelper;

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

        public synchronized void
        check()
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
        
        public synchronized void
        called()
        {
            assert(!_called);
            _called = true;
            notify();
        }

        private boolean _called;
    }

    private static class AMI_MyClass_opVoidI extends AMI_MyClass_opVoid
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

    private static class AMI_MyClass_opIdempotentI extends AMI_MyClass_opIdempotent
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

    private static class AMI_MyClass_opNonmutatingI extends AMI_MyClass_opNonmutating
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

    private static class AMI_MyClass_opVoidExI extends AMI_MyClass_opVoid
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

        public void
        check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private static class AMI_MyClass_opByteExI extends AMI_MyClass_opByte
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

        public void
        check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    static void
    onewaysAMI(test.Util.Application app, MyClassPrx p)
    {
        Ice.Communicator communicator = app.communicator();
        p = MyClassPrxHelper.uncheckedCast(p.ice_oneway());

        {
            AMI_MyClass_opVoidI cb = new AMI_MyClass_opVoidI();
            p.opVoid_async(cb);
            // Let's check if we can reuse the same callback object for another call.
            p.opVoid_async(cb);
        }

        {
            AMI_MyClass_opIdempotentI cb = new AMI_MyClass_opIdempotentI();
            p.opIdempotent_async(cb);
        }

        {
            AMI_MyClass_opNonmutatingI cb = new AMI_MyClass_opNonmutatingI();
            p.opNonmutating_async(cb);
        }

        {
            // Check that a call to a void operation raises NoEndpointException
            // in the ice_exception() callback instead of at the point of call.
            MyClassPrx indirect = MyClassPrxHelper.uncheckedCast(p.ice_adapterId("dummy"));
            AMI_MyClass_opVoidExI cb = new AMI_MyClass_opVoidExI();
            try
            {
                indirect.opVoid_async(cb);
            }
            catch(java.lang.Exception ex)
            {
                test(false);
            }
            cb.check();
        }

        {
            AMI_MyClass_opByteExI cb = new AMI_MyClass_opByteExI();
            p.opByte_async(cb, (byte)0xff, (byte)0x0f);
            cb.check();
        }

    }
}
