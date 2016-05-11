// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using Test;

public class AllTests : TestCommon.TestApp
{
    public static TestIntfPrx allTests(Ice.Communicator communicator)
    {
        string sref = "test:default -p 12010";
        Ice.ObjectPrx obj = communicator.stringToProxy(sref);
        test(obj != null);
        TestIntfPrx proxy = TestIntfPrxHelper.uncheckedCast(obj);
        test(proxy != null);

        Console.Out.Write("testing enum values... ");
        Console.Out.Flush();

        test((int)ByteEnum.benum1 == 0);
        test((int)ByteEnum.benum2 == 1);
        test((int)ByteEnum.benum3 == ByteConst1.value);
        test((int)ByteEnum.benum4 == ByteConst1.value + 1);
        test((int)ByteEnum.benum5 == ShortConst1.value);
        test((int)ByteEnum.benum6 == ShortConst1.value + 1);
        test((int)ByteEnum.benum7 == IntConst1.value);
        test((int)ByteEnum.benum8 == IntConst1.value + 1);
        test((int)ByteEnum.benum9 == LongConst1.value);
        test((int)ByteEnum.benum10 == LongConst1.value + 1);
        test((int)ByteEnum.benum11 == ByteConst2.value);

        test((int)ShortEnum.senum1 == 3);
        test((int)ShortEnum.senum2 == 4);
        test((int)ShortEnum.senum3 == ByteConst1.value);
        test((int)ShortEnum.senum4 == ByteConst1.value + 1);
        test((int)ShortEnum.senum5 == ShortConst1.value);
        test((int)ShortEnum.senum6 == ShortConst1.value + 1);
        test((int)ShortEnum.senum7 == IntConst1.value);
        test((int)ShortEnum.senum8 == IntConst1.value + 1);
        test((int)ShortEnum.senum9 == LongConst1.value);
        test((int)ShortEnum.senum10 == LongConst1.value + 1);
        test((int)ShortEnum.senum11 == ShortConst2.value);

        test((int)IntEnum.ienum1 == 0);
        test((int)IntEnum.ienum2 == 1);
        test((int)IntEnum.ienum3 == ByteConst1.value);
        test((int)IntEnum.ienum4 == ByteConst1.value + 1);
        test((int)IntEnum.ienum5 == ShortConst1.value);
        test((int)IntEnum.ienum6 == ShortConst1.value + 1);
        test((int)IntEnum.ienum7 == IntConst1.value);
        test((int)IntEnum.ienum8 == IntConst1.value + 1);
        test((int)IntEnum.ienum9 == LongConst1.value);
        test((int)IntEnum.ienum10 == LongConst1.value + 1);
        test((int)IntEnum.ienum11 == IntConst2.value);
        test((int)IntEnum.ienum12 == LongConst2.value);

        test((int)SimpleEnum.red == 0);
        test((int)SimpleEnum.green == 1);
        test((int)SimpleEnum.blue == 2);

        Console.Out.WriteLine("ok");

        Console.Out.Write("testing enum streaming... ");
        Console.Out.Flush();

        Ice.OutputStream ostr;
        byte[] bytes;

        bool encoding_1_0 = communicator.getProperties().getProperty("Ice.Default.EncodingVersion").Equals("1.0");

        ostr = new Ice.OutputStream(communicator);
        ostr.writeEnum((int)ByteEnum.benum11, (int)ByteEnum.benum11);
        bytes = ostr.finished();
        test(bytes.Length == 1); // ByteEnum should require one byte

        ostr = new Ice.OutputStream(communicator);
        ostr.writeEnum((int)ShortEnum.senum11, (int)ShortEnum.senum11);
        bytes = ostr.finished();
        test(bytes.Length == (encoding_1_0 ? 2 : 5));

        ostr = new Ice.OutputStream(communicator);
        ostr.writeEnum((int)IntEnum.ienum11, (int)IntEnum.ienum12);
        bytes = ostr.finished();
        test(bytes.Length == (encoding_1_0 ? 4 : 5));

        ostr = new Ice.OutputStream(communicator);
        ostr.writeEnum((int)SimpleEnum.blue, (int)SimpleEnum.blue);
        bytes = ostr.finished();
        test(bytes.Length == 1); // SimpleEnum should require one byte

        Console.Out.WriteLine("ok");

        Console.Out.Write("testing enum operations... ");
        Console.Out.Flush();

        ByteEnum byteEnum;
        test(proxy.opByte(ByteEnum.benum1, out byteEnum) == ByteEnum.benum1);
        test(byteEnum == ByteEnum.benum1);
        test(proxy.opByte(ByteEnum.benum11, out byteEnum) == ByteEnum.benum11);
        test(byteEnum == ByteEnum.benum11);

        ShortEnum shortEnum;
        test(proxy.opShort(ShortEnum.senum1, out shortEnum) == ShortEnum.senum1);
        test(shortEnum == ShortEnum.senum1);
        test(proxy.opShort(ShortEnum.senum11, out shortEnum) == ShortEnum.senum11);
        test(shortEnum == ShortEnum.senum11);

        IntEnum intEnum;
        test(proxy.opInt(IntEnum.ienum1, out intEnum) == IntEnum.ienum1);
        test(intEnum == IntEnum.ienum1);
        test(proxy.opInt(IntEnum.ienum11, out intEnum) == IntEnum.ienum11);
        test(intEnum == IntEnum.ienum11);
        test(proxy.opInt(IntEnum.ienum12, out intEnum) == IntEnum.ienum12);
        test(intEnum == IntEnum.ienum12);

        SimpleEnum s;
        test(proxy.opSimple(SimpleEnum.green, out s) == SimpleEnum.green);
        test(s == SimpleEnum.green);

        Console.Out.WriteLine("ok");

        Console.Out.Write("testing enum sequences operations... ");
        Console.Out.Flush();

        {
            ByteEnum[] b1 = new ByteEnum[11]
                    { ByteEnum.benum1, ByteEnum.benum2, ByteEnum.benum3, ByteEnum.benum4, ByteEnum.benum5,
                      ByteEnum.benum6, ByteEnum.benum7, ByteEnum.benum8, ByteEnum.benum9, ByteEnum.benum10,
                      ByteEnum.benum11};

            ByteEnum[] b2;
            ByteEnum[] b3 = proxy.opByteSeq(b1, out b2);

            for(int i = 0; i < b1.Length; ++i)
            {
                test(b1[i] == b2[i]);
                test(b1[i] == b3[i]);
            }
        }

        {
            ShortEnum[] s1 = new ShortEnum[11]
                    { ShortEnum.senum1, ShortEnum.senum2, ShortEnum.senum3, ShortEnum.senum4, ShortEnum.senum5,
                      ShortEnum.senum6, ShortEnum.senum7, ShortEnum.senum8, ShortEnum.senum9, ShortEnum.senum10,
                      ShortEnum.senum11};

            ShortEnum[] s2;
            ShortEnum[] s3 = proxy.opShortSeq(s1, out s2);

            for(int i = 0; i < s1.Length; ++i)
            {
                test(s1[i] == s2[i]);
                test(s1[i] == s3[i]);
            }
        }

        {
            IntEnum[] i1 = new IntEnum[11]
                    { IntEnum.ienum1, IntEnum.ienum2, IntEnum.ienum3, IntEnum.ienum4, IntEnum.ienum5,
                      IntEnum.ienum6, IntEnum.ienum7, IntEnum.ienum8, IntEnum.ienum9, IntEnum.ienum10,
                      IntEnum.ienum11};

            IntEnum[] i2;
            IntEnum[] i3 = proxy.opIntSeq(i1, out i2);

            for(int i = 0; i < i1.Length; ++i)
            {
                test(i1[i] == i2[i]);
                test(i1[i] == i3[i]);
            }
        }

        {
            SimpleEnum[] s1 = new SimpleEnum[3]
                    { SimpleEnum.red, SimpleEnum.green, SimpleEnum.blue };

            SimpleEnum[] s2;
            SimpleEnum[] s3 = proxy.opSimpleSeq(s1, out s2);

            for(int i = 0; i < s1.Length; ++i)
            {
                test(s1[i] == s2[i]);
                test(s1[i] == s3[i]);
            }
        }

        Console.Out.WriteLine("ok");
        return proxy;
    }
}
