// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

package test.Ice.seqMapping;

import test.Ice.seqMapping.Test.*;
import test.Ice.seqMapping.Serialize.*;

class TwowaysAMI
{
    private static void test(boolean b)
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

        public synchronized boolean check()
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

        public synchronized void called()
        {
            assert(!_called);
            _called = true;
            notify();
        }

        private boolean _called;
    }

    static void twowaysAMI(MyClassPrx p)
    {
        {
            Small i = null;

            Callback cb = new Callback();
            p.opSerialSmallJavaAsync(i).whenComplete((result, ex) ->
                {
                    if(ex != null)
                    {
                        test(ex instanceof com.zeroc.Ice.OperationNotExistException); // OK, talking to non-Java server.
                    }
                    else
                    {
                        test(result.o == null);
                        test(result.returnValue == null);
                        cb.called();
                    }
                });
            test(cb.check());
        }

        {
            Small i = new Small();
            i.i = 99;

            Callback cb = new Callback();
            p.opSerialSmallJavaAsync(i).whenComplete((result, ex) ->
                {
                    if(ex != null)
                    {
                        test(ex instanceof com.zeroc.Ice.OperationNotExistException); // OK, talking to non-Java server.
                    }
                    else
                    {
                        test(result.o.i == 99);
                        test(result.returnValue.i == 99);
                        cb.called();
                    }
                });
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

            Callback cb = new Callback();
            p.opSerialLargeJavaAsync(i).whenComplete((result, ex) ->
                {
                    if(ex != null)
                    {
                        test(ex instanceof com.zeroc.Ice.OperationNotExistException); // OK, talking to non-Java server.
                    }
                    else
                    {
                        test(result.o.d1 == 1.0);
                        test(result.o.d2 == 2.0);
                        test(result.o.d3 == 3.0);
                        test(result.o.d4 == 4.0);
                        test(result.o.d5 == 5.0);
                        test(result.o.d6 == 6.0);
                        test(result.o.d7 == 7.0);
                        test(result.o.d8 == 8.0);
                        test(result.o.d9 == 9.0);
                        test(result.o.d10 == 10.0);
                        test(result.returnValue.d1 == 1.0);
                        test(result.returnValue.d2 == 2.0);
                        test(result.returnValue.d3 == 3.0);
                        test(result.returnValue.d4 == 4.0);
                        test(result.returnValue.d5 == 5.0);
                        test(result.returnValue.d6 == 6.0);
                        test(result.returnValue.d7 == 7.0);
                        test(result.returnValue.d8 == 8.0);
                        test(result.returnValue.d9 == 9.0);
                        test(result.returnValue.d10 == 10.0);
                        cb.called();
                    }
                });
            test(cb.check());
        }

        {
            Struct i = new Struct();
            i.o = null;
            i.o2 = i;
            i.s = null;
            i.s2 = "Hello";

            Callback cb = new Callback();
            p.opSerialStructJavaAsync(i).whenComplete((result, ex) ->
                {
                    test(result.o.o == null);
                    test(result.o.o2 != null);
                    test(((Struct)(result.o.o2)).o == null);
                    test(((Struct)(result.o.o2)).o2 == result.o.o2);
                    test(result.o.s == null);
                    test(result.o.s2.equals("Hello"));
                    test(result.returnValue.o == null);
                    test(result.returnValue.o2 != null);
                    test(((Struct)(result.returnValue.o2)).o == null);
                    test(((Struct)(result.returnValue.o2)).o2 == result.returnValue.o2);
                    test(result.returnValue.s == null);
                    test(result.returnValue.s2.equals("Hello"));
                    cb.called();
                });
            test(cb.check());
        }
    }
}
