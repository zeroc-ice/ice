// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Diagnostics;
using System.Threading;

class OnewaysAMI
{
    private static void test(bool b)
    {
        if(!b)
        {
            throw new System.SystemException();
        }
    }

    private class Callback
    {
        internal Callback()
        {
            _called = false;
        }

        public void check()
        {
            _m.Lock();
            try
            {
                while(!_called)
                {
                    _m.Wait();
                }

                _called = false;
            }
            finally
            {
                _m.Unlock();
            }
        }

        public void called()
        {
            _m.Lock();
            try
            {
                Debug.Assert(!_called);
                _called = true;
                _m.Notify();
            }
            finally
            {
                _m.Unlock();
            }
        }

        private bool _called;
        private readonly IceUtilInternal.Monitor _m = new IceUtilInternal.Monitor();
    }

    private class AMI_MyClass_opVoidI : Test.AMI_MyClass_opVoid
    {
        public override void ice_response()
        {
            test(false);
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }
    }

    private class AMI_MyClass_opIdempotentI : Test.AMI_MyClass_opIdempotent
    {
        public override void ice_response()
        {
            test(false);
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }
    }

    private class AMI_MyClass_opNonmutatingI : Test.AMI_MyClass_opNonmutating
    {
        public override void ice_response()
        {
            test(false);
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }
    }

    private class AMI_MyClass_opVoidExI : Test.AMI_MyClass_opVoid
    {
        public override void ice_response()
        {
            test(false);
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(ex is Ice.NoEndpointException);
            callback.called();
        }

        public void check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opByteExI : Test.AMI_MyClass_opByte
    {
        public override void ice_response(byte r, byte b)
        {
            test(false);
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(ex is Ice.TwowayOnlyException);
            callback.called();
        }

        public void check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    internal static void onewaysAMI(Ice.Communicator communicator, Test.MyClassPrx p)
    {
        p = Test.MyClassPrxHelper.uncheckedCast(p.ice_oneway());

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
            Test.MyClassPrx indirect = Test.MyClassPrxHelper.uncheckedCast(p.ice_adapterId("dummy"));
            AMI_MyClass_opVoidExI cb = new AMI_MyClass_opVoidExI();
            try
            {
                indirect.opVoid_async(cb);
            }
            catch(System.Exception)
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
