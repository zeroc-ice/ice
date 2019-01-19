//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package test.Ice.enums;

import test.Ice.enums.Test.*;
import java.io.PrintWriter;

public class AllTests
{
    private static void test(boolean b)
    {
        if(!b)
        {
            throw new RuntimeException();
        }
    }

    public static TestIntfPrx allTests(test.TestHelper helper)
    {
        com.zeroc.Ice.Communicator communicator = helper.communicator();
        PrintWriter out = helper.getWriter();
        String ref = "test:" + helper.getTestEndpoint(0);
        com.zeroc.Ice.ObjectPrx obj = communicator.stringToProxy(ref);
        test(obj != null);
        TestIntfPrx proxy = TestIntfPrx.checkedCast(obj);
        test(proxy != null);

        out.print("testing enum values... ");
        out.flush();

        test(ByteEnum.benum1.value() == 0);
        test(ByteEnum.benum2.value() == 1);
        test(ByteEnum.benum3.value() == ByteConst1.value);
        test(ByteEnum.benum4.value() == ByteConst1.value + 1);
        test(ByteEnum.benum5.value() == ShortConst1.value);
        test(ByteEnum.benum6.value() == ShortConst1.value + 1);
        test(ByteEnum.benum7.value() == IntConst1.value);
        test(ByteEnum.benum8.value() == IntConst1.value + 1);
        test(ByteEnum.benum9.value() == LongConst1.value);
        test(ByteEnum.benum10.value() == LongConst1.value + 1);
        test(ByteEnum.benum11.value() == ByteConst2.value);

        test(ByteEnum.valueOf(0) == ByteEnum.benum1);
        test(ByteEnum.valueOf(1) == ByteEnum.benum2);
        test(ByteEnum.valueOf(ByteConst1.value) == ByteEnum.benum3);
        test(ByteEnum.valueOf(ByteConst1.value + 1) == ByteEnum.benum4);
        test(ByteEnum.valueOf(ShortConst1.value) == ByteEnum.benum5);
        test(ByteEnum.valueOf(ShortConst1.value + 1) == ByteEnum.benum6);
        test(ByteEnum.valueOf(IntConst1.value) == ByteEnum.benum7);
        test(ByteEnum.valueOf(IntConst1.value + 1) == ByteEnum.benum8);
        test(ByteEnum.valueOf((int)LongConst1.value) == ByteEnum.benum9);
        test(ByteEnum.valueOf((int)LongConst1.value + 1) == ByteEnum.benum10);
        test(ByteEnum.valueOf(ByteConst2.value) == ByteEnum.benum11);

        test(ShortEnum.senum1.value() == 3);
        test(ShortEnum.senum2.value() == 4);
        test(ShortEnum.senum3.value() == ByteConst1.value);
        test(ShortEnum.senum4.value() == ByteConst1.value + 1);
        test(ShortEnum.senum5.value() == ShortConst1.value);
        test(ShortEnum.senum6.value() == ShortConst1.value + 1);
        test(ShortEnum.senum7.value() == IntConst1.value);
        test(ShortEnum.senum8.value() == IntConst1.value + 1);
        test(ShortEnum.senum9.value() == LongConst1.value);
        test(ShortEnum.senum10.value() == LongConst1.value + 1);
        test(ShortEnum.senum11.value() == ShortConst2.value);

        test(ShortEnum.valueOf(3) == ShortEnum.senum1);
        test(ShortEnum.valueOf(4) == ShortEnum.senum2);
        test(ShortEnum.valueOf(ByteConst1.value) == ShortEnum.senum3);
        test(ShortEnum.valueOf(ByteConst1.value + 1) == ShortEnum.senum4);
        test(ShortEnum.valueOf(ShortConst1.value) == ShortEnum.senum5);
        test(ShortEnum.valueOf(ShortConst1.value + 1) == ShortEnum.senum6);
        test(ShortEnum.valueOf(IntConst1.value) == ShortEnum.senum7);
        test(ShortEnum.valueOf(IntConst1.value + 1) == ShortEnum.senum8);
        test(ShortEnum.valueOf((int)LongConst1.value) == ShortEnum.senum9);
        test(ShortEnum.valueOf((int)LongConst1.value + 1) == ShortEnum.senum10);
        test(ShortEnum.valueOf(ShortConst2.value) == ShortEnum.senum11);

        test(IntEnum.ienum1.value() == 0);
        test(IntEnum.ienum2.value() == 1);
        test(IntEnum.ienum3.value() == ByteConst1.value);
        test(IntEnum.ienum4.value() == ByteConst1.value + 1);
        test(IntEnum.ienum5.value() == ShortConst1.value);
        test(IntEnum.ienum6.value() == ShortConst1.value + 1);
        test(IntEnum.ienum7.value() == IntConst1.value);
        test(IntEnum.ienum8.value() == IntConst1.value + 1);
        test(IntEnum.ienum9.value() == LongConst1.value);
        test(IntEnum.ienum10.value() == LongConst1.value + 1);
        test(IntEnum.ienum11.value() == IntConst2.value);
        test(IntEnum.ienum12.value() == LongConst2.value);

        test(IntEnum.valueOf(0) == IntEnum.ienum1);
        test(IntEnum.valueOf(1) == IntEnum.ienum2);
        test(IntEnum.valueOf(ByteConst1.value) == IntEnum.ienum3);
        test(IntEnum.valueOf(ByteConst1.value + 1) == IntEnum.ienum4);
        test(IntEnum.valueOf(ShortConst1.value) == IntEnum.ienum5);
        test(IntEnum.valueOf(ShortConst1.value + 1) == IntEnum.ienum6);
        test(IntEnum.valueOf(IntConst1.value) == IntEnum.ienum7);
        test(IntEnum.valueOf(IntConst1.value + 1) == IntEnum.ienum8);
        test(IntEnum.valueOf((int)LongConst1.value) == IntEnum.ienum9);
        test(IntEnum.valueOf((int)LongConst1.value + 1) == IntEnum.ienum10);
        test(IntEnum.valueOf(IntConst2.value) == IntEnum.ienum11);
        test(IntEnum.valueOf((int)LongConst2.value) == IntEnum.ienum12);

        test(SimpleEnum.red.value() == 0);
        test(SimpleEnum.green.value() == 1);
        test(SimpleEnum.blue.value() == 2);

        test(SimpleEnum.valueOf(0) == SimpleEnum.red);
        test(SimpleEnum.valueOf(1) == SimpleEnum.green);
        test(SimpleEnum.valueOf(2) == SimpleEnum.blue);

        out.println("ok");

        out.print("testing enum streaming... ");
        out.flush();

        com.zeroc.Ice.OutputStream os;
        byte[] bytes;

        final boolean encoding_1_0 =
            communicator.getProperties().getProperty("Ice.Default.EncodingVersion").equals("1.0");

        os = new com.zeroc.Ice.OutputStream(communicator);
        ByteEnum.ice_write(os, ByteEnum.benum11);
        bytes = os.finished();
        test(bytes.length == 1); // ByteEnum should require one byte

        os = new com.zeroc.Ice.OutputStream(communicator);
        ShortEnum.ice_write(os, ShortEnum.senum11);
        bytes = os.finished();
        test(bytes.length == (encoding_1_0 ? 2 : 5));

        os = new com.zeroc.Ice.OutputStream(communicator);
        IntEnum.ice_write(os, IntEnum.ienum11);
        bytes = os.finished();
        test(bytes.length == (encoding_1_0 ? 4 : 5));

        os = new com.zeroc.Ice.OutputStream(communicator);
        SimpleEnum.ice_write(os, SimpleEnum.blue);
        bytes = os.finished();
        test(bytes.length == 1); // SimpleEnum should require one byte

        out.println("ok");

        out.print("testing enum operations... ");
        out.flush();

        {
            TestIntf.OpByteResult r;
            r = proxy.opByte(ByteEnum.benum1);
            test(r.returnValue == ByteEnum.benum1 && r.b2 == ByteEnum.benum1);
            r = proxy.opByte(ByteEnum.benum11);
            test(r.returnValue == ByteEnum.benum11 && r.b2 == ByteEnum.benum11);
        }

        {
            TestIntf.OpShortResult r;
            r = proxy.opShort(ShortEnum.senum1);
            test(r.returnValue == ShortEnum.senum1 && r.s2 == ShortEnum.senum1);
            r = proxy.opShort(ShortEnum.senum11);
            test(r.returnValue == ShortEnum.senum11 && r.s2 == ShortEnum.senum11);
        }

        {
            TestIntf.OpIntResult r;
            r = proxy.opInt(IntEnum.ienum1);
            test(r.returnValue == IntEnum.ienum1 && r.i2 == IntEnum.ienum1);
            r = proxy.opInt(IntEnum.ienum11);
            test(r.returnValue == IntEnum.ienum11 && r.i2 == IntEnum.ienum11);
            r = proxy.opInt(IntEnum.ienum12);
            test(r.returnValue == IntEnum.ienum12 && r.i2 == IntEnum.ienum12);
        }

        {
            TestIntf.OpSimpleResult r;
            r = proxy.opSimple(SimpleEnum.green);
            test(r.returnValue == SimpleEnum.green && r.s2 == SimpleEnum.green);
        }

        out.println("ok");

        out.print("testing enum sequences operations... ");
        out.flush();

        {
            ByteEnum b1[] = { ByteEnum.benum1, ByteEnum.benum2, ByteEnum.benum3, ByteEnum.benum4, ByteEnum.benum5,
                              ByteEnum.benum6, ByteEnum.benum7, ByteEnum.benum8, ByteEnum.benum9, ByteEnum.benum10,
                              ByteEnum.benum11};

            TestIntf.OpByteSeqResult r = proxy.opByteSeq(b1);

            for(int i = 0; i < b1.length; ++i)
            {
                test(b1[i] == r.b2[i]);
                test(b1[i] == r.returnValue[i]);
            }
        }

        {
            ShortEnum s1[] = { ShortEnum.senum1, ShortEnum.senum2, ShortEnum.senum3, ShortEnum.senum4, ShortEnum.senum5,
                               ShortEnum.senum6, ShortEnum.senum7, ShortEnum.senum8, ShortEnum.senum9, ShortEnum.senum10,
                               ShortEnum.senum11};

            TestIntf.OpShortSeqResult r = proxy.opShortSeq(s1);

            for(int i = 0; i < s1.length; ++i)
            {
                test(s1[i] == r.s2[i]);
                test(s1[i] == r.returnValue[i]);
            }
        }

        {
            IntEnum i1[] = { IntEnum.ienum1, IntEnum.ienum2, IntEnum.ienum3, IntEnum.ienum4, IntEnum.ienum5,
                             IntEnum.ienum6, IntEnum.ienum7, IntEnum.ienum8, IntEnum.ienum9, IntEnum.ienum10,
                             IntEnum.ienum11};

            TestIntf.OpIntSeqResult r = proxy.opIntSeq(i1);

            for(int i = 0; i < i1.length; ++i)
            {
                test(i1[i] == r.i2[i]);
                test(i1[i] == r.returnValue[i]);
            }
        }

        {
            SimpleEnum s1[] = { SimpleEnum.red, SimpleEnum.green, SimpleEnum.blue };

            TestIntf.OpSimpleSeqResult r = proxy.opSimpleSeq(s1);

            for(int i = 0; i < s1.length; ++i)
            {
                test(s1[i] == r.s2[i]);
                test(s1[i] == r.returnValue[i]);
            }
        }

        out.println("ok");

        out.print("testing enum exceptions... ");
        out.flush();

        try
        {
            os = new com.zeroc.Ice.OutputStream(communicator);
            os.writeByte((byte)2); // Invalid enumerator
            com.zeroc.Ice.InputStream in = new com.zeroc.Ice.InputStream(communicator, os.finished());
            ByteEnum.ice_read(in);
            test(false);
        }
        catch(com.zeroc.Ice.MarshalException ex)
        {
        }

        try
        {
            os = new com.zeroc.Ice.OutputStream(communicator);
            os.writeByte((byte)128); // Invalid enumerator
            com.zeroc.Ice.InputStream in = new com.zeroc.Ice.InputStream(communicator, os.finished());
            ByteEnum.ice_read(in);
            test(false);
        }
        catch(com.zeroc.Ice.MarshalException ex)
        {
        }

        try
        {
            os = new com.zeroc.Ice.OutputStream(communicator);
            os.writeShort((short)-1); // Negative enumerators are not supported
            com.zeroc.Ice.InputStream in = new com.zeroc.Ice.InputStream(communicator, os.finished());
            ShortEnum.ice_read(in);
            test(false);
        }
        catch(com.zeroc.Ice.MarshalException ex)
        {
        }

        try
        {
            os = new com.zeroc.Ice.OutputStream(communicator);
            os.writeShort((short)0); // Invalid enumerator
            com.zeroc.Ice.InputStream in = new com.zeroc.Ice.InputStream(communicator, os.finished());
            ShortEnum.ice_read(in);
            test(false);
        }
        catch(com.zeroc.Ice.MarshalException ex)
        {
        }

        try
        {
            os = new com.zeroc.Ice.OutputStream(communicator);
            os.writeShort((short)32767); // Invalid enumerator
            com.zeroc.Ice.InputStream in = new com.zeroc.Ice.InputStream(communicator, os.finished());
            ShortEnum.ice_read(in);
            test(false);
        }
        catch(com.zeroc.Ice.MarshalException ex)
        {
        }

        try
        {
            os = new com.zeroc.Ice.OutputStream(communicator);
            os.writeInt(-1); // Negative enumerators are not supported
            com.zeroc.Ice.InputStream in = new com.zeroc.Ice.InputStream(communicator, os.finished());
            IntEnum.ice_read(in);
            test(false);
        }
        catch(com.zeroc.Ice.MarshalException ex)
        {
        }

        try
        {
            os = new com.zeroc.Ice.OutputStream(communicator);
            os.writeInt(2); // Invalid enumerator
            com.zeroc.Ice.InputStream in = new com.zeroc.Ice.InputStream(communicator, os.finished());
            IntEnum.ice_read(in);
            test(false);
        }
        catch(com.zeroc.Ice.MarshalException ex)
        {
        }

        out.println("ok");

        return proxy;
    }
}
