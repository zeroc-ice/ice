// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.serialize;
import test.Ice.serialize.Test.*;
import java.io.*;

public class AllTests
{
    private static void
    test(boolean b)
    {
        if(!b)
        {
            throw new RuntimeException();
        }
    }

    public static InitialPrx
    allTests(test.Util.Application app, boolean collocated)
    {
        Ice.Communicator communicator = app.communicator();
        PrintWriter out = app.getWriter();

        String ref = "initial:" + app.getTestEndpoint(0);
        Ice.ObjectPrx base = communicator.stringToProxy(ref);
        InitialPrx initial = InitialPrxHelper.checkedCast(base);

        out.print("testing serialization... ");
        out.flush();

        //
        // Call getStruct1 and force an error.
        //
        try
        {
            //
            // We expect this test to raise an exception: we are attempting to deserialize
            // an instance of Struct1 using java.io.ObjectInputStream. However, we must
            // use Ice.ObjectInputStream instead because Struct1 contains a proxy.
            //
            byte[] bytes = initial.getStruct1();
            ByteArrayInputStream bais = new ByteArrayInputStream(bytes);
            ObjectInputStream ois = new ObjectInputStream(bais);
            ois.readObject();
            test(false);
        }
        catch(IOException ex)
        {
            // Expected.
        }
        catch(Throwable ex)
        {
            test(false);
        }

        //
        // Call getStruct1.
        //
        try
        {
            byte[] bytes = initial.getStruct1();
            ByteArrayInputStream bais = new ByteArrayInputStream(bytes);
            Ice.ObjectInputStream ois = new Ice.ObjectInputStream(communicator, bais);
            try
            {
                Struct1 s = (Struct1)ois.readObject();
                checkStruct1(s);
            }
            finally
            {
                ois.close();
            }
        }
        catch(Throwable ex)
        {
            test(false);
        }

        //
        // Call getBase.
        //
        try
        {
            byte[] bytes = initial.getBase();
            ByteArrayInputStream bais = new ByteArrayInputStream(bytes);
            Ice.ObjectInputStream ois = new Ice.ObjectInputStream(communicator, bais);
            try
            {
                Base b = (Base) ois.readObject();
                checkBase(b);
            }
            finally
            {
                ois.close();
            }
        }
        catch(Throwable ex)
        {
            test(false);
        }

        //
        // Call getEx.
        //
        try
        {
            byte[] bytes = initial.getEx();
            ByteArrayInputStream bais = new ByteArrayInputStream(bytes);
            Ice.ObjectInputStream ois = new Ice.ObjectInputStream(communicator, bais);
            try
            {
                Ex ex = (Ex)ois.readObject();
                checkStruct1(ex.s);
                checkBase(ex.b);
            }
            finally
            {
                ois.close();
            }
        }
        catch(Throwable ex)
        {
            test(false);
        }

        out.println("ok");

        return initial;
    }

    private static void
    checkStruct1(Struct1 s)
    {
        test(s.bo);
        test(s.by == (byte)1);
        test(s.sh == (short)2);
        test(s.i == 3);
        test(s.l == 4);
        test(s.f == (float)5.0);
        test(s.d == 6.0);
        test(s.str.equals("7"));
        test(s.e == MyEnum.enum2);
        test(s.p != null);
        s.p.ice_ping(); // Make sure the deserialized proxy is usable.
    }

    private static void
    checkBase(Base b)
    {
        test(b.b == b);
        test(b.o == b);
        checkStruct1(b.s);
        test(java.util.Arrays.equals(b.seq1, new byte[] { 0, 1, 2, 3, 4 }));
        test(java.util.Arrays.equals(b.seq2, new int[] { 5, 6, 7, 8, 9 }));
        test(java.util.Arrays.equals(b.seq3, new MyEnum[] { MyEnum.enum3, MyEnum.enum2, MyEnum.enum1 }));
        test(java.util.Arrays.equals(b.seq4, new Base[] { b }));
        test(b.d1.get(Byte.valueOf((byte)1)).equals(Boolean.TRUE));
        test(b.d2.get(Short.valueOf((short)2)).equals(Integer.valueOf(3)));
        test(b.d3.get("enum3") == MyEnum.enum3);
        test(b.d4.get("b") == b);
        test(b instanceof Derived);
        Derived d = (Derived)b;
        test(d.p != null);
        d.p.ice_ping();
    }
}
