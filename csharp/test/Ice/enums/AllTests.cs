//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;

namespace ZeroC.Ice.Test.Enums
{
    public class AllTests
    {
        public static ITestIntfPrx allTests(TestHelper helper)
        {
            Communicator? communicator = helper.Communicator();
            TestHelper.Assert(communicator != null);
            string sref = "test:" + helper.GetTestEndpoint(0);
            var obj = IObjectPrx.Parse(sref, communicator);
            TestHelper.Assert(obj != null);
            var proxy = ITestIntfPrx.UncheckedCast(obj);
            TestHelper.Assert(proxy != null);

            System.IO.TextWriter output = helper.GetWriter();

            output.Write("testing enum values... ");
            output.Flush();

            TestHelper.Assert((int)ByteEnum.benum1 == 0);
            TestHelper.Assert((int)ByteEnum.benum2 == 1);
            TestHelper.Assert((int)ByteEnum.benum3 == Constants.ByteConst1);
            TestHelper.Assert((int)ByteEnum.benum4 == Constants.ByteConst1 + 1);
            TestHelper.Assert((int)ByteEnum.benum5 == Constants.ShortConst1);
            TestHelper.Assert((int)ByteEnum.benum6 == Constants.ShortConst1 + 1);
            TestHelper.Assert((int)ByteEnum.benum7 == Constants.IntConst1);
            TestHelper.Assert((int)ByteEnum.benum8 == Constants.IntConst1 + 1);
            TestHelper.Assert((int)ByteEnum.benum9 == Constants.LongConst1);
            TestHelper.Assert((int)ByteEnum.benum10 == Constants.LongConst1 + 1);
            TestHelper.Assert((int)ByteEnum.benum11 == Constants.ByteConst2);

            TestHelper.Assert((int)ShortEnum.senum1 == 3);
            TestHelper.Assert((int)ShortEnum.senum2 == 4);
            TestHelper.Assert((int)ShortEnum.senum3 == Constants.ByteConst1);
            TestHelper.Assert((int)ShortEnum.senum4 == Constants.ByteConst1 + 1);
            TestHelper.Assert((int)ShortEnum.senum5 == Constants.ShortConst1);
            TestHelper.Assert((int)ShortEnum.senum6 == Constants.ShortConst1 + 1);
            TestHelper.Assert((int)ShortEnum.senum7 == Constants.IntConst1);
            TestHelper.Assert((int)ShortEnum.senum8 == Constants.IntConst1 + 1);
            TestHelper.Assert((int)ShortEnum.senum9 == Constants.LongConst1);
            TestHelper.Assert((int)ShortEnum.senum10 == Constants.LongConst1 + 1);
            TestHelper.Assert((int)ShortEnum.senum11 == Constants.ShortConst2);

            TestHelper.Assert((int)IntEnum.ienum1 == 0);
            TestHelper.Assert((int)IntEnum.ienum2 == 1);
            TestHelper.Assert((int)IntEnum.ienum3 == Constants.ByteConst1);
            TestHelper.Assert((int)IntEnum.ienum4 == Constants.ByteConst1 + 1);
            TestHelper.Assert((int)IntEnum.ienum5 == Constants.ShortConst1);
            TestHelper.Assert((int)IntEnum.ienum6 == Constants.ShortConst1 + 1);
            TestHelper.Assert((int)IntEnum.ienum7 == Constants.IntConst1);
            TestHelper.Assert((int)IntEnum.ienum8 == Constants.IntConst1 + 1);
            TestHelper.Assert((int)IntEnum.ienum9 == Constants.LongConst1);
            TestHelper.Assert((int)IntEnum.ienum10 == Constants.LongConst1 + 1);
            TestHelper.Assert((int)IntEnum.ienum11 == Constants.IntConst2);
            TestHelper.Assert((int)IntEnum.ienum12 == Constants.LongConst2);

            TestHelper.Assert((int)SimpleEnum.red == 0);
            TestHelper.Assert((int)SimpleEnum.green == 1);
            TestHelper.Assert((int)SimpleEnum.blue == 2);

            output.WriteLine("ok");

            output.Write("testing enum operations... ");
            output.Flush();
            {
                (ByteEnum r, ByteEnum o) = proxy.opByte(ByteEnum.benum1);
                TestHelper.Assert(r == ByteEnum.benum1 && o == ByteEnum.benum1);
                (r, o) = proxy.opByte(ByteEnum.benum11);
                TestHelper.Assert(r == ByteEnum.benum11 && o == ByteEnum.benum11);
            }

            {
                (ShortEnum r, ShortEnum o) = proxy.opShort(ShortEnum.senum1);
                TestHelper.Assert(r == ShortEnum.senum1 && o == ShortEnum.senum1);
                (r, o) = proxy.opShort(ShortEnum.senum11);
                TestHelper.Assert(r == ShortEnum.senum11 && o == ShortEnum.senum11);
            }

            {
                (IntEnum r, IntEnum o) = proxy.opInt(IntEnum.ienum1);
                TestHelper.Assert(r == IntEnum.ienum1 && o == IntEnum.ienum1);
                (r, o) = proxy.opInt(IntEnum.ienum11);
                TestHelper.Assert(r == IntEnum.ienum11 && o == IntEnum.ienum11);
                (r, o) = proxy.opInt(IntEnum.ienum12);
                TestHelper.Assert(r == IntEnum.ienum12 && o == IntEnum.ienum12);
            }

            {
                (SimpleEnum r, SimpleEnum o) = proxy.opSimple(SimpleEnum.green);
                TestHelper.Assert(r == SimpleEnum.green && o == SimpleEnum.green);
            }

            output.WriteLine("ok");

            output.Write("testing enum sequences operations... ");
            output.Flush();

            {
                var b1 = new ByteEnum[11]
                {
                    ByteEnum.benum1,
                    ByteEnum.benum2,
                    ByteEnum.benum3,
                    ByteEnum.benum4,
                    ByteEnum.benum5,
                    ByteEnum.benum6,
                    ByteEnum.benum7,
                    ByteEnum.benum8,
                    ByteEnum.benum9,
                    ByteEnum.benum10,
                    ByteEnum.benum11
                };

                (ByteEnum[] b3, ByteEnum[] b2) = proxy.opByteSeq(b1);

                for (int i = 0; i < b1.Length; ++i)
                {
                    TestHelper.Assert(b1[i] == b2[i]);
                    TestHelper.Assert(b1[i] == b3[i]);
                }
            }

            {
                var s1 = new ShortEnum[11]
                    {
                            ShortEnum.senum1,
                            ShortEnum.senum2,
                            ShortEnum.senum3,
                            ShortEnum.senum4,
                            ShortEnum.senum5,
                            ShortEnum.senum6,
                            ShortEnum.senum7,
                            ShortEnum.senum8,
                            ShortEnum.senum9,
                            ShortEnum.senum10,
                            ShortEnum.senum11
                    };

                (ShortEnum[] s3, ShortEnum[] s2) = proxy.opShortSeq(s1);

                for (int i = 0; i < s1.Length; ++i)
                {
                    TestHelper.Assert(s1[i] == s2[i]);
                    TestHelper.Assert(s1[i] == s3[i]);
                }
            }

            {
                var i1 = new IntEnum[11]
                {
                    IntEnum.ienum1,
                    IntEnum.ienum2,
                    IntEnum.ienum3,
                    IntEnum.ienum4,
                    IntEnum.ienum5,
                    IntEnum.ienum6,
                    IntEnum.ienum7,
                    IntEnum.ienum8,
                    IntEnum.ienum9,
                    IntEnum.ienum10,
                    IntEnum.ienum11
                };

                (IntEnum[] i3, IntEnum[] i2) = proxy.opIntSeq(i1);

                for (int i = 0; i < i1.Length; ++i)
                {
                    TestHelper.Assert(i1[i] == i2[i]);
                    TestHelper.Assert(i1[i] == i3[i]);
                }
            }

            {
                var s1 = new SimpleEnum[3]
                        {
                                SimpleEnum.red,
                                SimpleEnum.green,
                                SimpleEnum.blue
                        };

                (SimpleEnum[] s3, SimpleEnum[] s2) = proxy.opSimpleSeq(s1);

                for (int i = 0; i < s1.Length; ++i)
                {
                    TestHelper.Assert(s1[i] == s2[i]);
                    TestHelper.Assert(s1[i] == s3[i]);
                }
            }

            output.WriteLine("ok");
            return proxy;
        }
    }
}
