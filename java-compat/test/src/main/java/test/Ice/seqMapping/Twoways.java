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
    private static void
    test(boolean b)
    {
        if(!b)
        {
            throw new RuntimeException();
        }
    }

    static void
    twoways(MyClassPrx p)
    {
        {
            Small i = null;
            Ice.Holder<Small> o = new Ice.Holder<Small>();
            Small r;

            r = p.opSerialSmallJava(i, o);

            test(o.value == null);
            test(r == null);
        }

        {
            Small i = new Small();
            i.i = 99;
            Ice.Holder<Small> o = new Ice.Holder<Small>();
            Small r;

            try
            {
                r = p.opSerialSmallJava(i, o);

                test(o.value.i == 99);
                test(r.i == 99);
            }
            catch(Ice.OperationNotExistException ex)
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
            Ice.Holder<Large> o = new Ice.Holder<Large>();
            Large r;

            try
            {
                r = p.opSerialLargeJava(i, o);

                test(o.value.d1 == 1.0);
                test(o.value.d2 == 2.0);
                test(o.value.d3 == 3.0);
                test(o.value.d4 == 4.0);
                test(o.value.d5 == 5.0);
                test(o.value.d6 == 6.0);
                test(o.value.d7 == 7.0);
                test(o.value.d8 == 8.0);
                test(o.value.d9 == 9.0);
                test(o.value.d10 == 10.0);
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
            }
            catch(Ice.OperationNotExistException ex)
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
            Ice.Holder<Struct> o = new Ice.Holder<Struct>();
            Struct r;

            try
            {
                r = p.opSerialStructJava(i, o);

                test(o.value.o == null);
                test(o.value.o2 != null);
                test(((Struct)(o.value.o2)).o == null);
                test(((Struct)(o.value.o2)).o2 == o.value.o2);
                test(o.value.s == null);
                test(o.value.s2.equals("Hello"));
                test(r.o == null);
                test(r.o2 != null);
                test(((Struct)(r.o2)).o == null);
                test(((Struct)(r.o2)).o2 == r.o2);
                test(r.s == null);
                test(r.s2.equals("Hello"));
            }
            catch(Ice.OperationNotExistException ex)
            {
                // OK, talking to non-Java server.
            }
        }
    }
}
