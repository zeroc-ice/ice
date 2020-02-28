//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Ice.enums.Test;

namespace Ice.enums
{
    public class AllTests : global::Test.AllTests
    {
        public static Test.ITestIntfPrx allTests(global::Test.TestHelper helper)
        {
            Communicator communicator = helper.communicator();
            string sref = "test:" + helper.getTestEndpoint(0);
            var obj = IObjectPrx.Parse(sref, communicator);
            test(obj != null);
            var proxy = Test.ITestIntfPrx.UncheckedCast(obj);
            test(proxy != null);

            var output = helper.getWriter();

            output.Write("testing enum values... ");
            output.Flush();

            test((int)Test.ByteEnum.benum1 == 0);
            test((int)Test.ByteEnum.benum2 == 1);
            test((int)Test.ByteEnum.benum3 == Test.ByteConst1.value);
            test((int)Test.ByteEnum.benum4 == Test.ByteConst1.value + 1);
            test((int)Test.ByteEnum.benum5 == Test.ShortConst1.value);
            test((int)Test.ByteEnum.benum6 == Test.ShortConst1.value + 1);
            test((int)Test.ByteEnum.benum7 == Test.IntConst1.value);
            test((int)Test.ByteEnum.benum8 == Test.IntConst1.value + 1);
            test((int)Test.ByteEnum.benum9 == Test.LongConst1.value);
            test((int)Test.ByteEnum.benum10 == Test.LongConst1.value + 1);
            test((int)Test.ByteEnum.benum11 == Test.ByteConst2.value);

            test((int)Test.ShortEnum.senum1 == 3);
            test((int)Test.ShortEnum.senum2 == 4);
            test((int)Test.ShortEnum.senum3 == Test.ByteConst1.value);
            test((int)Test.ShortEnum.senum4 == Test.ByteConst1.value + 1);
            test((int)Test.ShortEnum.senum5 == Test.ShortConst1.value);
            test((int)Test.ShortEnum.senum6 == Test.ShortConst1.value + 1);
            test((int)Test.ShortEnum.senum7 == Test.IntConst1.value);
            test((int)Test.ShortEnum.senum8 == Test.IntConst1.value + 1);
            test((int)Test.ShortEnum.senum9 == Test.LongConst1.value);
            test((int)Test.ShortEnum.senum10 == Test.LongConst1.value + 1);
            test((int)Test.ShortEnum.senum11 == Test.ShortConst2.value);

            test((int)Test.IntEnum.ienum1 == 0);
            test((int)Test.IntEnum.ienum2 == 1);
            test((int)Test.IntEnum.ienum3 == Test.ByteConst1.value);
            test((int)Test.IntEnum.ienum4 == Test.ByteConst1.value + 1);
            test((int)Test.IntEnum.ienum5 == Test.ShortConst1.value);
            test((int)Test.IntEnum.ienum6 == Test.ShortConst1.value + 1);
            test((int)Test.IntEnum.ienum7 == Test.IntConst1.value);
            test((int)Test.IntEnum.ienum8 == Test.IntConst1.value + 1);
            test((int)Test.IntEnum.ienum9 == Test.LongConst1.value);
            test((int)Test.IntEnum.ienum10 == Test.LongConst1.value + 1);
            test((int)Test.IntEnum.ienum11 == Test.IntConst2.value);
            test((int)Test.IntEnum.ienum12 == Test.LongConst2.value);

            test((int)Test.SimpleEnum.red == 0);
            test((int)Test.SimpleEnum.green == 1);
            test((int)Test.SimpleEnum.blue == 2);

            output.WriteLine("ok");

            output.Write("testing enum streaming... ");
            output.Flush();

            Ice.OutputStream ostr;
            byte[] bytes;

            ostr = new OutputStream(communicator);
            ostr.Write(ByteEnum.benum11);
            bytes = ostr.ToArray();
            test(bytes.Length == 1); // ByteEnum should require one byte

            ostr = new OutputStream(communicator);
            ostr.Write(ShortEnum.senum11);
            bytes = ostr.ToArray();
            test(bytes.Length == 5);

            ostr = new OutputStream(communicator);
            ostr.Write(IntEnum.ienum11);
            bytes = ostr.ToArray();
            test(bytes.Length == 5);

            ostr = new OutputStream(communicator);
            ostr.Write(SimpleEnum.blue);
            bytes = ostr.ToArray();
            test(bytes.Length == 1); // SimpleEnum should require one byte

            output.WriteLine("ok");

            output.Write("testing enum operations... ");
            output.Flush();
            {
                var (r, o) = proxy.opByte(Test.ByteEnum.benum1);
                test(r == Test.ByteEnum.benum1 && o == Test.ByteEnum.benum1);
                (r, o) = proxy.opByte(Test.ByteEnum.benum11);
                test(r == Test.ByteEnum.benum11 && o == Test.ByteEnum.benum11);
            }

            {
                var (r, o) = proxy.opShort(Test.ShortEnum.senum1);
                test(r == Test.ShortEnum.senum1 && o == Test.ShortEnum.senum1);
                (r, o) = proxy.opShort(Test.ShortEnum.senum11);
                test(r == Test.ShortEnum.senum11 && o == Test.ShortEnum.senum11);
            }

            {
                var (r, o) = proxy.opInt(Test.IntEnum.ienum1);
                test(r == Test.IntEnum.ienum1 && o == Test.IntEnum.ienum1);
                (r, o) = proxy.opInt(Test.IntEnum.ienum11);
                test(r == Test.IntEnum.ienum11 && o == Test.IntEnum.ienum11);
                (r, o) = proxy.opInt(Test.IntEnum.ienum12);
                test(r == Test.IntEnum.ienum12 && o == Test.IntEnum.ienum12);
            }

            {
                var (r, o) = proxy.opSimple(Test.SimpleEnum.green);
                test(r == Test.SimpleEnum.green && o == Test.SimpleEnum.green);
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

                var (b3, b2) = proxy.opByteSeq(b1);

                for (int i = 0; i < b1.Length; ++i)
                {
                    test(b1[i] == b2[i]);
                    test(b1[i] == b3[i]);
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

                var (s3, s2) = proxy.opShortSeq(s1);

                for (int i = 0; i < s1.Length; ++i)
                {
                    test(s1[i] == s2[i]);
                    test(s1[i] == s3[i]);
                }
            }

            {
                Test.IntEnum[] i1 = new Test.IntEnum[11]
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

                var (i3, i2) = proxy.opIntSeq(i1);

                for (int i = 0; i < i1.Length; ++i)
                {
                    test(i1[i] == i2[i]);
                    test(i1[i] == i3[i]);
                }
            }

            {
                var s1 = new Test.SimpleEnum[3]
                        {
                                Test.SimpleEnum.red,
                                Test.SimpleEnum.green,
                                Test.SimpleEnum.blue
                        };

                var (s3, s2) = proxy.opSimpleSeq(s1);

                for (int i = 0; i < s1.Length; ++i)
                {
                    test(s1[i] == s2[i]);
                    test(s1[i] == s3[i]);
                }
            }

            output.WriteLine("ok");
            return proxy;
        }
    }
}
