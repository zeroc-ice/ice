// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.seqMapping;

import test.Ice.seqMapping.Test.*;
import test.Ice.seqMapping.Serialize.*;

class TwowaysAMI
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
                    wait();
                }
                catch(InterruptedException ex)
                {
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

    private static class AMI_MyClass_opSerialSmallJavaNull extends AMI_MyClass_opSerialSmallJava
    {
        public void
        ice_response(Small r, Small o)
        {
            test(o == null);
            test(r == null);
            callback.called();
        }

        public void
        ice_exception(Ice.LocalException ex)
        {
            test(ex instanceof Ice.OperationNotExistException); // OK, talking to non-Java server.
        }

        public boolean
        check()
        {
            return callback.check();
        }

        private Callback callback = new Callback();
    }

    private static class AMI_MyClass_opSerialSmallJavaI extends AMI_MyClass_opSerialSmallJava
    {
        public void
        ice_response(Small r, Small o)
        {
            test(o.i == 99);
            test(r.i == 99);
            callback.called();
        }

        public void
        ice_exception(Ice.LocalException ex)
        {
            test(ex instanceof Ice.OperationNotExistException); // OK, talking to non-Java server.
        }

        public boolean
        check()
        {
            return callback.check();
        }

        private Callback callback = new Callback();
    }

    private static class AMI_MyClass_opSerialLargeJavaI extends AMI_MyClass_opSerialLargeJava
    {
        public void
        ice_response(Large r, Large o)
        {
            test(o.d1 == 1.0);
            test(o.d2 == 2.0);
            test(o.d3 == 3.0);
            test(o.d4 == 4.0);
            test(o.d5 == 5.0);
            test(o.d6 == 6.0);
            test(o.d7 == 7.0);
            test(o.d8 == 8.0);
            test(o.d9 == 9.0);
            test(o.d10 == 10.0);
            test(r.d1 == 1.0);
            test(r.d2 == 2.0);
            test(r.d3 == 3.0);
            test(r.d4 == 4.0);
            test(r.d5 == 5.0);
            test(r.d6 == 6.0);
            test(r.d7 == 7.0);
            test(r.d8 == 8.0);
            test(r.d9 == 9.0);
            test(r.d10 == 10.0);
            callback.called();
        }

        public void
        ice_exception(Ice.LocalException ex)
        {
            test(ex instanceof Ice.OperationNotExistException); // OK, talking to non-Java server.
        }

        public boolean
        check()
        {
            return callback.check();
        }

        private Callback callback = new Callback();
    }

    private static class AMI_MyClass_opSerialStructJavaI extends AMI_MyClass_opSerialStructJava
    {
        public void
        ice_response(Struct r, Struct o)
        {
            test(o.o == null);
            test(o.o2 != null);
            test(((Struct)(o.o2)).o == null);
            test(((Struct)(o.o2)).o2 == o.o2);
            test(o.s == null);
            test(o.s2.equals("Hello"));
            test(r.o == null);
            test(r.o2 != null);
            test(((Struct)(r.o2)).o == null);
            test(((Struct)(r.o2)).o2 == r.o2);
            test(r.s == null);
            test(r.s2.equals("Hello"));
            callback.called();
        }

        public void
        ice_exception(Ice.LocalException ex)
        {
            test(ex instanceof Ice.OperationNotExistException); // OK, talking to non-Java server.
        }

        public boolean
        check()
        {
            return callback.check();
        }

        private Callback callback = new Callback();
    }

    static void
    twowaysAMI(MyClassPrx p)
    {
        {
            Small i = null;

            AMI_MyClass_opSerialSmallJavaNull cb = new AMI_MyClass_opSerialSmallJavaNull();
            p.opSerialSmallJava_async(cb, i);
            test(cb.check());
        }

        {
            Small i = new Small();
            i.i = 99;

            AMI_MyClass_opSerialSmallJavaI cb = new AMI_MyClass_opSerialSmallJavaI();
            p.opSerialSmallJava_async(cb, i);
            test(cb.check());
        }

        {
            Large i = new Large();
            i.d1 = 1.0;
            i.d2 = 2.0;
            i.d3 = 3.0;
            i.d4 = 4.0;
            i.d5 = 5.0;
            i.d6 = 6.0;
            i.d7 = 7.0;
            i.d8 = 8.0;
            i.d9 = 9.0;
            i.d10 = 10.0;

            AMI_MyClass_opSerialLargeJavaI cb = new AMI_MyClass_opSerialLargeJavaI();
            p.opSerialLargeJava_async(cb, i);
            test(cb.check());
        }

        {
            Struct i = new Struct();
            i.o = null;
            i.o2 = i;
            i.s = null;
            i.s2 = "Hello";

            AMI_MyClass_opSerialStructJavaI cb = new AMI_MyClass_opSerialStructJavaI();
            p.opSerialStructJava_async(cb, i);
            test(cb.check());
        }
    }
}
