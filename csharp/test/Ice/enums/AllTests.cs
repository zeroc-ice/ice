// Copyright (c) ZeroC, Inc. All rights reserved.

using System.Linq;
using System.Threading.Tasks;
using ZeroC.Test;

namespace ZeroC.Ice.Test.Enums
{
    public static class AllTests
    {
        public static async Task RunAsync(TestHelper helper)
        {
            Communicator communicator = helper.Communicator;

            var proxy = ITestIntfPrx.Parse(helper.GetTestProxy("test", 0), communicator);
            System.IO.TextWriter output = helper.Output;

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
                (ByteEnum r, ByteEnum o) = proxy.OpByte(ByteEnum.benum1);
                TestHelper.Assert(r == ByteEnum.benum1 && o == ByteEnum.benum1);
                (r, o) = proxy.OpByte(ByteEnum.benum11);
                TestHelper.Assert(r == ByteEnum.benum11 && o == ByteEnum.benum11);
            }

            {
                // 42 does not correspond to a ByteEnum enumerator, so we expect failure since ByteEnum is checked.
                try
                {
                    _ = proxy.OpByte((ByteEnum)42);
                    TestHelper.Assert(false);
                }
                catch (UnhandledException) // unhandled dispatch exception
                {
                    // expected
                }
            }

            {
                (ByteEnum? r, ByteEnum? o) = proxy.OpTaggedByte(ByteEnum.benum1);
                TestHelper.Assert(r == ByteEnum.benum1 && o == ByteEnum.benum1);
                (r, o) = proxy.OpTaggedByte(ByteEnum.benum11);
                TestHelper.Assert(r == ByteEnum.benum11 && o == ByteEnum.benum11);
                (r, o) = proxy.OpTaggedByte(null);
                TestHelper.Assert(r == null && o == null);
            }

            {
                try
                {
                    _ = proxy.OpTaggedByte((ByteEnum)42);
                    TestHelper.Assert(false);
                }
                catch (UnhandledException) // unhandled dispatch exception
                {
                    // expected
                }
            }

            {
                (ShortEnum r, ShortEnum o) = proxy.OpShort(ShortEnum.senum1);
                TestHelper.Assert(r == ShortEnum.senum1 && o == ShortEnum.senum1);
                (r, o) = proxy.OpShort(ShortEnum.senum11);
                TestHelper.Assert(r == ShortEnum.senum11 && o == ShortEnum.senum11);
            }

            {
                (IntEnum r, IntEnum o) = proxy.OpInt(IntEnum.ienum1);
                TestHelper.Assert(r == IntEnum.ienum1 && o == IntEnum.ienum1);
                (r, o) = proxy.OpInt(IntEnum.ienum11);
                TestHelper.Assert(r == IntEnum.ienum11 && o == IntEnum.ienum11);
                (r, o) = proxy.OpInt(IntEnum.ienum12);
                TestHelper.Assert(r == IntEnum.ienum12 && o == IntEnum.ienum12);
            }

            {
                (SimpleEnum r, SimpleEnum o) = proxy.OpSimple(SimpleEnum.green);
                TestHelper.Assert(r == SimpleEnum.green && o == SimpleEnum.green);
            }

            {
                (FLByteEnum r, FLByteEnum o) = proxy.OpFLByte(FLByteEnum.benum1);
                TestHelper.Assert(r == FLByteEnum.benum1 && o == FLByteEnum.benum1);
                (r, o) = proxy.OpFLByte(FLByteEnum.benum11);
                TestHelper.Assert(r == FLByteEnum.benum11 && o == FLByteEnum.benum11);
            }

            {
                (FLByteEnum? r, FLByteEnum? o) = proxy.OpTaggedFLByte(FLByteEnum.benum1);
                TestHelper.Assert(r == FLByteEnum.benum1 && o == FLByteEnum.benum1);
                (r, o) = proxy.OpTaggedFLByte(FLByteEnum.benum11);
                TestHelper.Assert(r == FLByteEnum.benum11 && o == FLByteEnum.benum11);
                (r, o) = proxy.OpTaggedFLByte(null);
                TestHelper.Assert(r == null && o == null);
            }

            {
                (FLShortEnum r, FLShortEnum o) = proxy.OpFLShort(FLShortEnum.senum1);
                TestHelper.Assert(r == FLShortEnum.senum1 && o == FLShortEnum.senum1);
                (r, o) = proxy.OpFLShort(FLShortEnum.senum11);
                TestHelper.Assert(r == FLShortEnum.senum11 && o == FLShortEnum.senum11);
            }

            {
                (FLUShortEnum r, FLUShortEnum o) = proxy.OpFLUShort(FLUShortEnum.senum1);
                TestHelper.Assert(r == FLUShortEnum.senum1 && o == FLUShortEnum.senum1);
                (r, o) = proxy.OpFLUShort(FLUShortEnum.senum11);
                TestHelper.Assert(r == FLUShortEnum.senum11 && o == FLUShortEnum.senum11);
            }

            {
                (FLIntEnum r, FLIntEnum o) = proxy.OpFLInt(FLIntEnum.ienum1);
                TestHelper.Assert(r == FLIntEnum.ienum1 && o == FLIntEnum.ienum1);
                (r, o) = proxy.OpFLInt(FLIntEnum.ienum11);
                TestHelper.Assert(r == FLIntEnum.ienum11 && o == FLIntEnum.ienum11);
                (r, o) = proxy.OpFLInt(FLIntEnum.ienum12);
                TestHelper.Assert(r == FLIntEnum.ienum12 && o == FLIntEnum.ienum12);
            }

            {
                (FLUIntEnum r, FLUIntEnum o) = proxy.OpFLUInt(FLUIntEnum.ienum1);
                TestHelper.Assert(r == FLUIntEnum.ienum1 && o == FLUIntEnum.ienum1);
                (r, o) = proxy.OpFLUInt(FLUIntEnum.ienum11);
                TestHelper.Assert(r == FLUIntEnum.ienum11 && o == FLUIntEnum.ienum11);
                (r, o) = proxy.OpFLUInt(FLUIntEnum.ienum12);
                TestHelper.Assert(r == FLUIntEnum.ienum12 && o == FLUIntEnum.ienum12);
            }

            {
                (FLSimpleEnum r, FLSimpleEnum o) = proxy.OpFLSimple(FLSimpleEnum.green);
                TestHelper.Assert(r == FLSimpleEnum.green && o == FLSimpleEnum.green);
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

                (ByteEnum[] b3, ByteEnum[] b2) = proxy.OpByteSeq(b1);
                TestHelper.Assert(b1.SequenceEqual(b2));
                TestHelper.Assert(b1.SequenceEqual(b3));
            }

            {
                var b1 = new ByteEnum[12]
                {
                    ByteEnum.benum1,
                    ByteEnum.benum2,
                    (ByteEnum)42,
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

                try
                {
                    _ = proxy.OpByteSeq(b1);
                    TestHelper.Assert(false);
                }
                catch (UnhandledException)
                {
                    // expected
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

                (ShortEnum[] s3, ShortEnum[] s2) = proxy.OpShortSeq(s1);
                TestHelper.Assert(s1.SequenceEqual(s2));
                TestHelper.Assert(s1.SequenceEqual(s3));
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

                (IntEnum[] i3, IntEnum[] i2) = proxy.OpIntSeq(i1);
                TestHelper.Assert(i1.SequenceEqual(i2));
                TestHelper.Assert(i1.SequenceEqual(i3));
            }

            {
                var s1 = new SimpleEnum[3]
                {
                    SimpleEnum.red,
                    SimpleEnum.green,
                    SimpleEnum.blue
                };

                (SimpleEnum[] s3, SimpleEnum[] s2) = proxy.OpSimpleSeq(s1);
                TestHelper.Assert(s1.SequenceEqual(s2));
                TestHelper.Assert(s1.SequenceEqual(s3));
            }

            {
                var b1 = new FLByteEnum[11]
                {
                    FLByteEnum.benum1,
                    FLByteEnum.benum2,
                    FLByteEnum.benum3,
                    FLByteEnum.benum4,
                    FLByteEnum.benum5,
                    FLByteEnum.benum6,
                    FLByteEnum.benum7,
                    FLByteEnum.benum8,
                    FLByteEnum.benum9,
                    FLByteEnum.benum10,
                    FLByteEnum.benum11
                };

                (FLByteEnum[] b3, FLByteEnum[] b2) = proxy.OpFLByteSeq(b1);
                TestHelper.Assert(b1.SequenceEqual(b2));
                TestHelper.Assert(b1.SequenceEqual(b3));
            }

            {
                var s1 = new FLShortEnum[11]
                {
                    FLShortEnum.senum1,
                    FLShortEnum.senum2,
                    FLShortEnum.senum3,
                    FLShortEnum.senum4,
                    FLShortEnum.senum5,
                    FLShortEnum.senum6,
                    FLShortEnum.senum7,
                    FLShortEnum.senum8,
                    FLShortEnum.senum9,
                    FLShortEnum.senum10,
                    FLShortEnum.senum11
                };

                (FLShortEnum[] s3, FLShortEnum[] s2) = proxy.OpFLShortSeq(s1);
                TestHelper.Assert(s1.SequenceEqual(s2));
                TestHelper.Assert(s1.SequenceEqual(s3));
            }

            {
                var s1 = new FLUShortEnum[11]
                {
                    FLUShortEnum.senum1,
                    FLUShortEnum.senum2,
                    FLUShortEnum.senum3,
                    FLUShortEnum.senum4,
                    FLUShortEnum.senum5,
                    FLUShortEnum.senum6,
                    FLUShortEnum.senum7,
                    FLUShortEnum.senum8,
                    FLUShortEnum.senum9,
                    FLUShortEnum.senum10,
                    FLUShortEnum.senum11
                };

                (FLUShortEnum[] s3, FLUShortEnum[] s2) = proxy.OpFLUShortSeq(s1);
                TestHelper.Assert(s1.SequenceEqual(s2));
                TestHelper.Assert(s1.SequenceEqual(s3));
            }

            {
                var i1 = new FLIntEnum[11]
                {
                    FLIntEnum.ienum1,
                    FLIntEnum.ienum2,
                    FLIntEnum.ienum3,
                    FLIntEnum.ienum4,
                    FLIntEnum.ienum5,
                    FLIntEnum.ienum6,
                    FLIntEnum.ienum7,
                    FLIntEnum.ienum8,
                    FLIntEnum.ienum9,
                    FLIntEnum.ienum10,
                    FLIntEnum.ienum11
                };

                (FLIntEnum[] i3, FLIntEnum[] i2) = proxy.OpFLIntSeq(i1);
                TestHelper.Assert(i1.SequenceEqual(i2));
                TestHelper.Assert(i1.SequenceEqual(i3));
            }

            {
                var i1 = new FLUIntEnum[11]
                {
                    FLUIntEnum.ienum1,
                    FLUIntEnum.ienum2,
                    FLUIntEnum.ienum3,
                    FLUIntEnum.ienum4,
                    FLUIntEnum.ienum5,
                    FLUIntEnum.ienum6,
                    FLUIntEnum.ienum7,
                    FLUIntEnum.ienum8,
                    FLUIntEnum.ienum9,
                    FLUIntEnum.ienum10,
                    FLUIntEnum.ienum11
                };

                (FLUIntEnum[] i3, FLUIntEnum[] i2) = proxy.OpFLUIntSeq(i1);
                TestHelper.Assert(i1.SequenceEqual(i2));
                TestHelper.Assert(i1.SequenceEqual(i3));
            }

            {
                var s1 = new FLSimpleEnum[3]
                {
                    FLSimpleEnum.red,
                    FLSimpleEnum.green,
                    FLSimpleEnum.blue
                };

                (FLSimpleEnum[] s3, FLSimpleEnum[] s2) = proxy.OpFLSimpleSeq(s1);
                TestHelper.Assert(s1.SequenceEqual(s2));
                TestHelper.Assert(s1.SequenceEqual(s3));
            }

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

                (ByteEnum[]? b3, ByteEnum[]? b2) = proxy.OpTaggedByteSeq(b1);
                TestHelper.Assert(b2 != null && b3 != null);

                TestHelper.Assert(b1.SequenceEqual(b2));
                TestHelper.Assert(b1.SequenceEqual(b3));

                (b3, b2) = proxy.OpTaggedByteSeq(null);
                TestHelper.Assert(b2 == null && b3 == null);
            }

            {
                var b1 = new ByteEnum[12]
                {
                    ByteEnum.benum1,
                    ByteEnum.benum2,
                    ByteEnum.benum3,
                    ByteEnum.benum4,
                    ByteEnum.benum5,
                    ByteEnum.benum6,
                    ByteEnum.benum7,
                    ByteEnum.benum8,
                    (ByteEnum)42,
                    ByteEnum.benum9,
                    ByteEnum.benum10,
                    ByteEnum.benum11
                };

                try
                {
                    _ = proxy.OpTaggedByteSeq(b1);
                    TestHelper.Assert(false);
                }
                catch (UnhandledException)
                {
                    // expected
                }
            }

            {
                var b1 = new FLByteEnum[11]
                {
                    FLByteEnum.benum1,
                    FLByteEnum.benum2,
                    FLByteEnum.benum3,
                    FLByteEnum.benum4,
                    FLByteEnum.benum5,
                    FLByteEnum.benum6,
                    FLByteEnum.benum7,
                    FLByteEnum.benum8,
                    FLByteEnum.benum9,
                    FLByteEnum.benum10,
                    FLByteEnum.benum11
                };

                (FLByteEnum[]? b3, FLByteEnum[]? b2) = proxy.OpTaggedFLByteSeq(b1);
                TestHelper.Assert(b2 != null && b3 != null);

                TestHelper.Assert(b1.SequenceEqual(b2));
                TestHelper.Assert(b1.SequenceEqual(b3));

                (b3, b2) = proxy.OpTaggedFLByteSeq(null);
                TestHelper.Assert(b2 == null && b3 == null);
            }

            {
                var i1 = new FLIntEnum[11]
                {
                    FLIntEnum.ienum1,
                    FLIntEnum.ienum2,
                    FLIntEnum.ienum3,
                    FLIntEnum.ienum4,
                    FLIntEnum.ienum5,
                    FLIntEnum.ienum6,
                    FLIntEnum.ienum7,
                    FLIntEnum.ienum8,
                    FLIntEnum.ienum9,
                    FLIntEnum.ienum10,
                    FLIntEnum.ienum11
                };

                (FLIntEnum[]? i3, FLIntEnum[]? i2) = proxy.OpTaggedFLIntSeq(i1);
                TestHelper.Assert(i2 != null && i3 != null);

                TestHelper.Assert(i1.SequenceEqual(i2));
                TestHelper.Assert(i1.SequenceEqual(i3));

                (i3, i2) = proxy.OpTaggedFLIntSeq(null);
                TestHelper.Assert(i2 == null && i3 == null);
            }

            output.WriteLine("ok");

            output.Write("testing unchecked enums... ");
            output.Flush();
            {
                (MyFlags r, MyFlags f2) = proxy.OpMyFlags(MyFlags.E31);
                TestHelper.Assert(r == MyFlags.E31 && f2 == r);
                (r, f2) = proxy.OpMyFlags(MyFlags.E10 | MyFlags.E11);
                TestHelper.Assert(r == (MyFlags.E10 | MyFlags.E11) && f2 == r);
            }

            output.Flush();
            {
                (MyFlags? r, MyFlags? f2) = proxy.OpTaggedMyFlags(null);
                TestHelper.Assert(r == null && f2 == r);
                (r, f2) = proxy.OpTaggedMyFlags(MyFlags.E10 | MyFlags.E11);
                TestHelper.Assert(r == (MyFlags.E10 | MyFlags.E11) && f2 == r);
            }

            output.Flush();
            {
                var myFlagsSeq = new MyFlags[] { MyFlags.E10, MyFlags.E0, MyFlags.E5 | MyFlags.E6 };
                (MyFlags[] r, MyFlags[] f2) = proxy.OpMyFlagsSeq(myFlagsSeq);
                TestHelper.Assert(r.SequenceEqual(myFlagsSeq) && f2.SequenceEqual(myFlagsSeq));
            }

            output.Flush();
            {
                var myFlagsSeq = new MyFlags[] { MyFlags.E10, MyFlags.E0, MyFlags.E5 | MyFlags.E6 };
                (MyFlags[]? r, MyFlags[]? f2) = proxy.OpTaggedMyFlagsSeq(myFlagsSeq);
                TestHelper.Assert(r!.SequenceEqual(myFlagsSeq) && f2!.SequenceEqual(myFlagsSeq));
            }

            output.WriteLine("ok");
            await proxy.ShutdownAsync();
        }
    }
}
