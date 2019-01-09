// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

namespace Ice
{
    namespace enums
    {
        public class AllTests : global::Test.AllTests
        {
            public static Test.TestIntfPrx allTests(global::Test.TestHelper helper)
            {
                Ice.Communicator communicator = helper.communicator();
                string sref = "test:" + helper.getTestEndpoint(0);
                Ice.ObjectPrx obj = communicator.stringToProxy(sref);
                test(obj != null);
                var proxy = Test.TestIntfPrxHelper.uncheckedCast(obj);
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

                bool encoding_1_0 = communicator.getProperties().getProperty("Ice.Default.EncodingVersion").Equals("1.0");

                ostr = new Ice.OutputStream(communicator);
                ostr.writeEnum((int)Test.ByteEnum.benum11,(int)Test.ByteEnum.benum11);
                bytes = ostr.finished();
                test(bytes.Length == 1); // ByteEnum should require one byte

                ostr = new Ice.OutputStream(communicator);
                ostr.writeEnum((int)Test.ShortEnum.senum11,(int)Test.ShortEnum.senum11);
                bytes = ostr.finished();
                test(bytes.Length ==(encoding_1_0 ? 2 : 5));

                ostr = new Ice.OutputStream(communicator);
                ostr.writeEnum((int)Test.IntEnum.ienum11,(int)Test.IntEnum.ienum12);
                bytes = ostr.finished();
                test(bytes.Length ==(encoding_1_0 ? 4 : 5));

                ostr = new Ice.OutputStream(communicator);
                ostr.writeEnum((int)Test.SimpleEnum.blue,(int)Test.SimpleEnum.blue);
                bytes = ostr.finished();
                test(bytes.Length == 1); // SimpleEnum should require one byte

                output.WriteLine("ok");

                output.Write("testing enum operations... ");
                output.Flush();

                Test.ByteEnum byteEnum;
                test(proxy.opByte(Test.ByteEnum.benum1, out byteEnum) == Test.ByteEnum.benum1);
                test(byteEnum == Test.ByteEnum.benum1);
                test(proxy.opByte(Test.ByteEnum.benum11, out byteEnum) == Test.ByteEnum.benum11);
                test(byteEnum == Test.ByteEnum.benum11);

                Test.ShortEnum shortEnum;
                test(proxy.opShort(Test.ShortEnum.senum1, out shortEnum) == Test.ShortEnum.senum1);
                test(shortEnum == Test.ShortEnum.senum1);
                test(proxy.opShort(Test.ShortEnum.senum11, out shortEnum) == Test.ShortEnum.senum11);
                test(shortEnum == Test.ShortEnum.senum11);

                Test.IntEnum intEnum;
                test(proxy.opInt(Test.IntEnum.ienum1, out intEnum) == Test.IntEnum.ienum1);
                test(intEnum == Test.IntEnum.ienum1);
                test(proxy.opInt(Test.IntEnum.ienum11, out intEnum) == Test.IntEnum.ienum11);
                test(intEnum == Test.IntEnum.ienum11);
                test(proxy.opInt(Test.IntEnum.ienum12, out intEnum) == Test.IntEnum.ienum12);
                test(intEnum == Test.IntEnum.ienum12);

                Test.SimpleEnum s;
                test(proxy.opSimple(Test.SimpleEnum.green, out s) == Test.SimpleEnum.green);
                test(s == Test.SimpleEnum.green);

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

                    Test.ByteEnum[] b2;
                    Test.ByteEnum[] b3 = proxy.opByteSeq(b1, out b2);

                    for(int i = 0; i < b1.Length; ++i)
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

                    Test.ShortEnum[] s2;
                    Test.ShortEnum[] s3 = proxy.opShortSeq(s1, out s2);

                    for(int i = 0; i < s1.Length; ++i)
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

                    Test.IntEnum[] i2;
                    Test.IntEnum[] i3 = proxy.opIntSeq(i1, out i2);

                    for(int i = 0; i < i1.Length; ++i)
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

                    Test.SimpleEnum[] s2;
                    Test.SimpleEnum[] s3 = proxy.opSimpleSeq(s1, out s2);

                    for(int i = 0; i < s1.Length; ++i)
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
}
