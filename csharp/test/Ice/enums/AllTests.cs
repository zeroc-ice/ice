//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Diagnostics;
using Test;

namespace Ice.enums
{
    public class AllTests
    {
        public static Test.ITestIntfPrx allTests(TestHelper helper)
        {
            Communicator? communicator = helper.Communicator();
            TestHelper.Assert(communicator != null);
            string sref = "test:" + helper.GetTestEndpoint(0);
            var obj = IObjectPrx.Parse(sref, communicator);
            TestHelper.Assert(obj != null);
            var proxy = Test.ITestIntfPrx.UncheckedCast(obj);
            TestHelper.Assert(proxy != null);

            System.IO.TextWriter output = helper.GetWriter();

            output.Write("testing enum values... ");
            output.Flush();

            TestHelper.Assert((int)Test.ByteEnum.benum1 == 0);
            TestHelper.Assert((int)Test.ByteEnum.benum2 == 1);
            TestHelper.Assert((int)Test.ByteEnum.benum3 == Test.Constants.ByteConst1);
            TestHelper.Assert((int)Test.ByteEnum.benum4 == Test.Constants.ByteConst1 + 1);
            TestHelper.Assert((int)Test.ByteEnum.benum5 == Test.Constants.ShortConst1);
            TestHelper.Assert((int)Test.ByteEnum.benum6 == Test.Constants.ShortConst1 + 1);
            TestHelper.Assert((int)Test.ByteEnum.benum7 == Test.Constants.IntConst1);
            TestHelper.Assert((int)Test.ByteEnum.benum8 == Test.Constants.IntConst1 + 1);
            TestHelper.Assert((int)Test.ByteEnum.benum9 == Test.Constants.LongConst1);
            TestHelper.Assert((int)Test.ByteEnum.benum10 == Test.Constants.LongConst1 + 1);
            TestHelper.Assert((int)Test.ByteEnum.benum11 == Test.Constants.ByteConst2);

            TestHelper.Assert((int)Test.ShortEnum.senum1 == 3);
            TestHelper.Assert((int)Test.ShortEnum.senum2 == 4);
            TestHelper.Assert((int)Test.ShortEnum.senum3 == Test.Constants.ByteConst1);
            TestHelper.Assert((int)Test.ShortEnum.senum4 == Test.Constants.ByteConst1 + 1);
            TestHelper.Assert((int)Test.ShortEnum.senum5 == Test.Constants.ShortConst1);
            TestHelper.Assert((int)Test.ShortEnum.senum6 == Test.Constants.ShortConst1 + 1);
            TestHelper.Assert((int)Test.ShortEnum.senum7 == Test.Constants.IntConst1);
            TestHelper.Assert((int)Test.ShortEnum.senum8 == Test.Constants.IntConst1 + 1);
            TestHelper.Assert((int)Test.ShortEnum.senum9 == Test.Constants.LongConst1);
            TestHelper.Assert((int)Test.ShortEnum.senum10 == Test.Constants.LongConst1 + 1);
            TestHelper.Assert((int)Test.ShortEnum.senum11 == Test.Constants.ShortConst2);

            TestHelper.Assert((int)Test.IntEnum.ienum1 == 0);
            TestHelper.Assert((int)Test.IntEnum.ienum2 == 1);
            TestHelper.Assert((int)Test.IntEnum.ienum3 == Test.Constants.ByteConst1);
            TestHelper.Assert((int)Test.IntEnum.ienum4 == Test.Constants.ByteConst1 + 1);
            TestHelper.Assert((int)Test.IntEnum.ienum5 == Test.Constants.ShortConst1);
            TestHelper.Assert((int)Test.IntEnum.ienum6 == Test.Constants.ShortConst1 + 1);
            TestHelper.Assert((int)Test.IntEnum.ienum7 == Test.Constants.IntConst1);
            TestHelper.Assert((int)Test.IntEnum.ienum8 == Test.Constants.IntConst1 + 1);
            TestHelper.Assert((int)Test.IntEnum.ienum9 == Test.Constants.LongConst1);
            TestHelper.Assert((int)Test.IntEnum.ienum10 == Test.Constants.LongConst1 + 1);
            TestHelper.Assert((int)Test.IntEnum.ienum11 == Test.Constants.IntConst2);
            TestHelper.Assert((int)Test.IntEnum.ienum12 == Test.Constants.LongConst2);

            TestHelper.Assert((int)Test.SimpleEnum.red == 0);
            TestHelper.Assert((int)Test.SimpleEnum.green == 1);
            TestHelper.Assert((int)Test.SimpleEnum.blue == 2);

            output.WriteLine("ok");

            output.Write("testing enum operations... ");
            output.Flush();
            {
                (Test.ByteEnum r, Test.ByteEnum o) = proxy.opByte(Test.ByteEnum.benum1);
                TestHelper.Assert(r == Test.ByteEnum.benum1 && o == Test.ByteEnum.benum1);
                (r, o) = proxy.opByte(Test.ByteEnum.benum11);
                TestHelper.Assert(r == Test.ByteEnum.benum11 && o == Test.ByteEnum.benum11);
            }

            {
                (Test.ShortEnum r, Test.ShortEnum o) = proxy.opShort(Test.ShortEnum.senum1);
                TestHelper.Assert(r == Test.ShortEnum.senum1 && o == Test.ShortEnum.senum1);
                (r, o) = proxy.opShort(Test.ShortEnum.senum11);
                TestHelper.Assert(r == Test.ShortEnum.senum11 && o == Test.ShortEnum.senum11);
            }

            {
                (Test.IntEnum r, Test.IntEnum o) = proxy.opInt(Test.IntEnum.ienum1);
                TestHelper.Assert(r == Test.IntEnum.ienum1 && o == Test.IntEnum.ienum1);
                (r, o) = proxy.opInt(Test.IntEnum.ienum11);
                TestHelper.Assert(r == Test.IntEnum.ienum11 && o == Test.IntEnum.ienum11);
                (r, o) = proxy.opInt(Test.IntEnum.ienum12);
                TestHelper.Assert(r == Test.IntEnum.ienum12 && o == Test.IntEnum.ienum12);
            }

            {
                (Test.SimpleEnum r, Test.SimpleEnum o) = proxy.opSimple(Test.SimpleEnum.green);
                TestHelper.Assert(r == Test.SimpleEnum.green && o == Test.SimpleEnum.green);
            }

            output.WriteLine("ok");

            output.Write("testing enum sequences operations... ");
            output.Flush();

            {
                var b1 = new Test.ByteEnum[11]
                {
                    Test.ByteEnum.benum1,
                    Test.ByteEnum.benum2,
                    Test.ByteEnum.benum3,
                    Test.ByteEnum.benum4,
                    Test.ByteEnum.benum5,
                    Test.ByteEnum.benum6,
                    Test.ByteEnum.benum7,
                    Test.ByteEnum.benum8,
                    Test.ByteEnum.benum9,
                    Test.ByteEnum.benum10,
                    Test.ByteEnum.benum11
                };

                (Test.ByteEnum[] b3, Test.ByteEnum[] b2) = proxy.opByteSeq(b1);

                for (int i = 0; i < b1.Length; ++i)
                {
                    TestHelper.Assert(b1[i] == b2[i]);
                    TestHelper.Assert(b1[i] == b3[i]);
                }
            }

            {
                var s1 = new Test.ShortEnum[11]
                    {
                            Test.ShortEnum.senum1,
                            Test.ShortEnum.senum2,
                            Test.ShortEnum.senum3,
                            Test.ShortEnum.senum4,
                            Test.ShortEnum.senum5,
                            Test.ShortEnum.senum6,
                            Test.ShortEnum.senum7,
                            Test.ShortEnum.senum8,
                            Test.ShortEnum.senum9,
                            Test.ShortEnum.senum10,
                            Test.ShortEnum.senum11
                    };

                (Test.ShortEnum[] s3, Test.ShortEnum[] s2) = proxy.opShortSeq(s1);

                for (int i = 0; i < s1.Length; ++i)
                {
                    TestHelper.Assert(s1[i] == s2[i]);
                    TestHelper.Assert(s1[i] == s3[i]);
                }
            }

            {
                var i1 = new Test.IntEnum[11]
                {
                    Test.IntEnum.ienum1,
                    Test.IntEnum.ienum2,
                    Test.IntEnum.ienum3,
                    Test.IntEnum.ienum4,
                    Test.IntEnum.ienum5,
                    Test.IntEnum.ienum6,
                    Test.IntEnum.ienum7,
                    Test.IntEnum.ienum8,
                    Test.IntEnum.ienum9,
                    Test.IntEnum.ienum10,
                    Test.IntEnum.ienum11
                };

                (Test.IntEnum[] i3, Test.IntEnum[] i2) = proxy.opIntSeq(i1);

                for (int i = 0; i < i1.Length; ++i)
                {
                    TestHelper.Assert(i1[i] == i2[i]);
                    TestHelper.Assert(i1[i] == i3[i]);
                }
            }

            {
                var s1 = new Test.SimpleEnum[3]
                        {
                                Test.SimpleEnum.red,
                                Test.SimpleEnum.green,
                                Test.SimpleEnum.blue
                        };

                (Test.SimpleEnum[] s3, Test.SimpleEnum[] s2) = proxy.opSimpleSeq(s1);

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
