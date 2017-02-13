// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.enums;

import test.Ice.enums.Test.*;
import java.io.PrintWriter;

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

    public static TestIntfPrx
    allTests(Ice.Communicator communicator, PrintWriter out)
    {
        String ref = "test:default -p 12010";
        Ice.ObjectPrx obj = communicator.stringToProxy(ref);
        test(obj != null);
        TestIntfPrx proxy = TestIntfPrxHelper.checkedCast(obj);
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

        Ice.OutputStream os;
        byte[] bytes;

        final boolean encoding_1_0 =
            communicator.getProperties().getProperty("Ice.Default.EncodingVersion").equals("1.0");

        os = Ice.Util.createOutputStream(communicator);
        ByteEnum.benum11.ice_write(os);
        bytes = os.finished();
        test(bytes.length == 1); // ByteEnum should require one byte

        os = Ice.Util.createOutputStream(communicator);
        ShortEnum.senum11.ice_write(os);
        bytes = os.finished();
        test(bytes.length == (encoding_1_0 ? 2 : 5));

        os = Ice.Util.createOutputStream(communicator);
        IntEnum.ienum11.ice_write(os);
        bytes = os.finished();
        test(bytes.length == (encoding_1_0 ? 4 : 5));

        os = Ice.Util.createOutputStream(communicator);
        SimpleEnum.blue.ice_write(os);
        bytes = os.finished();
        test(bytes.length == 1); // SimpleEnum should require one byte

        out.println("ok");

        out.print("testing enum operations... ");
        out.flush();

        ByteEnumHolder byteEnum = new ByteEnumHolder();
        test(proxy.opByte(ByteEnum.benum1, byteEnum) == ByteEnum.benum1);
        test(byteEnum.value == ByteEnum.benum1);
        test(proxy.opByte(ByteEnum.benum11, byteEnum) == ByteEnum.benum11);
        test(byteEnum.value == ByteEnum.benum11);

        ShortEnumHolder shortEnum = new ShortEnumHolder();
        test(proxy.opShort(ShortEnum.senum1, shortEnum) == ShortEnum.senum1);
        test(shortEnum.value == ShortEnum.senum1);
        test(proxy.opShort(ShortEnum.senum11, shortEnum) == ShortEnum.senum11);
        test(shortEnum.value == ShortEnum.senum11);

        IntEnumHolder intEnum = new IntEnumHolder();
        test(proxy.opInt(IntEnum.ienum1, intEnum) == IntEnum.ienum1);
        test(intEnum.value == IntEnum.ienum1);
        test(proxy.opInt(IntEnum.ienum11, intEnum) == IntEnum.ienum11);
        test(intEnum.value == IntEnum.ienum11);
        test(proxy.opInt(IntEnum.ienum12, intEnum) == IntEnum.ienum12);
        test(intEnum.value == IntEnum.ienum12);

        SimpleEnumHolder s = new SimpleEnumHolder();
        test(proxy.opSimple(SimpleEnum.green, s) == SimpleEnum.green);
        test(s.value == SimpleEnum.green);

        out.println("ok");

        out.print("testing enum sequences operations... ");
        out.flush();

        {
            ByteEnum b1[] = { ByteEnum.benum1, ByteEnum.benum2, ByteEnum.benum3, ByteEnum.benum4, ByteEnum.benum5,
                              ByteEnum.benum6, ByteEnum.benum7, ByteEnum.benum8, ByteEnum.benum9, ByteEnum.benum10,
                              ByteEnum.benum11};

            ByteEnumSeqHolder b2 = new ByteEnumSeqHolder();
            ByteEnum b3[] = proxy.opByteSeq(b1, b2);

            for(int i = 0; i < b1.length; ++i)
            {
                test(b1[i] == b2.value[i]);
                test(b1[i] == b3[i]);
            }
        }

        {
            ShortEnum s1[] = { ShortEnum.senum1, ShortEnum.senum2, ShortEnum.senum3, ShortEnum.senum4, ShortEnum.senum5,
                               ShortEnum.senum6, ShortEnum.senum7, ShortEnum.senum8, ShortEnum.senum9, ShortEnum.senum10,
                               ShortEnum.senum11};

            ShortEnumSeqHolder s2 = new ShortEnumSeqHolder();
            ShortEnum s3[] = proxy.opShortSeq(s1, s2);

            for(int i = 0; i < s1.length; ++i)
            {
                test(s1[i] == s2.value[i]);
                test(s1[i] == s3[i]);
            }
        }

        {
            IntEnum i1[] = { IntEnum.ienum1, IntEnum.ienum2, IntEnum.ienum3, IntEnum.ienum4, IntEnum.ienum5,
                             IntEnum.ienum6, IntEnum.ienum7, IntEnum.ienum8, IntEnum.ienum9, IntEnum.ienum10,
                             IntEnum.ienum11};

            IntEnumSeqHolder i2 = new IntEnumSeqHolder();
            IntEnum i3[] = proxy.opIntSeq(i1, i2);

            for(int i = 0; i < i1.length; ++i)
            {
                test(i1[i] == i2.value[i]);
                test(i1[i] == i3[i]);
            }
        }

        {
            SimpleEnum s1[] = { SimpleEnum.red, SimpleEnum.green, SimpleEnum.blue };

            SimpleEnumSeqHolder s2 = new SimpleEnumSeqHolder();
            SimpleEnum s3[] = proxy.opSimpleSeq(s1, s2);

            for(int i = 0; i < s1.length; ++i)
            {
                test(s1[i] == s2.value[i]);
                test(s1[i] == s3[i]);
            }
        }

        out.println("ok");

        out.print("testing enum exceptions... ");
        out.flush();

        try
        {
            os = Ice.Util.createOutputStream(communicator);
            os.writeByte((byte)2); // Invalid enumerator
            Ice.InputStream in = Ice.Util.createInputStream(communicator, os.finished());
            ByteEnum.ice_read(in);
            test(false);
        }
        catch(Ice.MarshalException ex)
        {
        }

        try
        {
            os = Ice.Util.createOutputStream(communicator);
            os.writeByte((byte)128); // Invalid enumerator
            Ice.InputStream in = Ice.Util.createInputStream(communicator, os.finished());
            ByteEnum.ice_read(in);
            test(false);
        }
        catch(Ice.MarshalException ex)
        {
        }

        try
        {
            os = Ice.Util.createOutputStream(communicator);
            os.writeShort((short)-1); // Negative enumerators are not supported
            Ice.InputStream in = Ice.Util.createInputStream(communicator, os.finished());
            ShortEnum.ice_read(in);
            test(false);
        }
        catch(Ice.MarshalException ex)
        {
        }

        try
        {
            os = Ice.Util.createOutputStream(communicator);
            os.writeShort((short)0); // Invalid enumerator
            Ice.InputStream in = Ice.Util.createInputStream(communicator, os.finished());
            ShortEnum.ice_read(in);
            test(false);
        }
        catch(Ice.MarshalException ex)
        {
        }

        try
        {
            os = Ice.Util.createOutputStream(communicator);
            os.writeShort((short)32767); // Invalid enumerator
            Ice.InputStream in = Ice.Util.createInputStream(communicator, os.finished());
            ShortEnum.ice_read(in);
            test(false);
        }
        catch(Ice.MarshalException ex)
        {
        }

        try
        {
            os = Ice.Util.createOutputStream(communicator);
            os.writeInt(-1); // Negative enumerators are not supported
            Ice.InputStream in = Ice.Util.createInputStream(communicator, os.finished());
            IntEnum.ice_read(in);
            test(false);
        }
        catch(Ice.MarshalException ex)
        {
        }

        try
        {
            os = Ice.Util.createOutputStream(communicator);
            os.writeInt(2); // Invalid enumerator
            Ice.InputStream in = Ice.Util.createInputStream(communicator, os.finished());
            IntEnum.ice_read(in);
            test(false);
        }
        catch(Ice.MarshalException ex)
        {
        }

        out.println("ok");

        return proxy;
    }
}
