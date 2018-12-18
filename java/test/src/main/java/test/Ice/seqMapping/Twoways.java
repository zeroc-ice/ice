// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.seqMapping;

import test.Ice.seqMapping.Test.*;
import test.Ice.seqMapping.Serialize.*;

class Twoways
{
    private static void test(boolean b)
    {
        if(!b)
        {
            throw new RuntimeException();
        }
    }

    static void twoways(MyClassPrx p)
    {
        {
            Small i = null;

            MyClass.OpSerialSmallJavaResult r = p.opSerialSmallJava(i);

            test(r.o == null);
            test(r.returnValue == null);
        }

        {
            Small i = new Small();
            i.i = 99;

            try
            {
                MyClass.OpSerialSmallJavaResult r = p.opSerialSmallJava(i);

                test(r.o.i == 99);
                test(r.returnValue.i == 99);
            }
            catch(com.zeroc.Ice.OperationNotExistException ex)
            {
                // OK, talking to non-Java server.
            }
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

            try
            {
                MyClass.OpSerialLargeJavaResult r = p.opSerialLargeJava(i);

                test(r.o.d1 == 1.0);
                test(r.o.d2 == 2.0);
                test(r.o.d3 == 3.0);
                test(r.o.d4 == 4.0);
                test(r.o.d5 == 5.0);
                test(r.o.d6 == 6.0);
                test(r.o.d7 == 7.0);
                test(r.o.d8 == 8.0);
                test(r.o.d9 == 9.0);
                test(r.o.d10 == 10.0);
                test(r.returnValue.d1 == 1.0);
                test(r.returnValue.d2 == 2.0);
                test(r.returnValue.d3 == 3.0);
                test(r.returnValue.d4 == 4.0);
                test(r.returnValue.d5 == 5.0);
                test(r.returnValue.d6 == 6.0);
                test(r.returnValue.d7 == 7.0);
                test(r.returnValue.d8 == 8.0);
                test(r.returnValue.d9 == 9.0);
                test(r.returnValue.d10 == 10.0);
            }
            catch(com.zeroc.Ice.OperationNotExistException ex)
            {
                // OK, talking to non-Java server.
            }
        }

        {
            Struct i = new Struct();
            i.o = null;
            i.o2 = i;
            i.s = null;
            i.s2 = "Hello";

            try
            {
                MyClass.OpSerialStructJavaResult r = p.opSerialStructJava(i);

                test(r.o.o == null);
                test(r.o.o2 != null);
                test(((Struct)(r.o.o2)).o == null);
                test(((Struct)(r.o.o2)).o2 == r.o.o2);
                test(r.o.s == null);
                test(r.o.s2.equals("Hello"));
                test(r.returnValue.o == null);
                test(r.returnValue.o2 != null);
                test(((Struct)(r.returnValue.o2)).o == null);
                test(((Struct)(r.returnValue.o2)).o2 == r.returnValue.o2);
                test(r.returnValue.s == null);
                test(r.returnValue.s2.equals("Hello"));
            }
            catch(com.zeroc.Ice.OperationNotExistException ex)
            {
                // OK, talking to non-Java server.
            }
        }
    }
}
