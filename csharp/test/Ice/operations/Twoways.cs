// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Collections.Generic;
using System.Linq;
using ZeroC.Test;

namespace ZeroC.Ice.Test.Operations
{
    public static class Twoways
    {
        internal static void Run(TestHelper helper, IMyClassPrx p)
        {
            Communicator communicator = helper.Communicator;

            string[] literals = p.OpStringLiterals();

            TestHelper.Assert(Constants.s0.Equals("\\") &&
                    Constants.s0.Equals(Constants.sw0) &&
                    Constants.s0.Equals(literals[0]) &&
                    Constants.s0.Equals(literals[11]));

            TestHelper.Assert(Constants.s1.Equals("A") &&
                    Constants.s1.Equals(Constants.sw1) &&
                    Constants.s1.Equals(literals[1]) &&
                    Constants.s1.Equals(literals[12]));

            TestHelper.Assert(Constants.s2.Equals("Ice") &&
                    Constants.s2.Equals(Constants.sw2) &&
                    Constants.s2.Equals(literals[2]) &&
                    Constants.s2.Equals(literals[13]));

            TestHelper.Assert(Constants.s3.Equals("A21") &&
                    Constants.s3.Equals(Constants.sw3) &&
                    Constants.s3.Equals(literals[3]) &&
                    Constants.s3.Equals(literals[14]));

            TestHelper.Assert(Constants.s4.Equals("\\u0041 \\U00000041") &&
                    Constants.s4.Equals(Constants.sw4) &&
                    Constants.s4.Equals(literals[4]) &&
                    Constants.s4.Equals(literals[15]));

            TestHelper.Assert(Constants.s5.Equals("\u00FF") &&
                    Constants.s5.Equals(Constants.sw5) &&
                    Constants.s5.Equals(literals[5]) &&
                    Constants.s5.Equals(literals[16]));

            TestHelper.Assert(Constants.s6.Equals("\u03FF") &&
                    Constants.s6.Equals(Constants.sw6) &&
                    Constants.s6.Equals(literals[6]) &&
                    Constants.s6.Equals(literals[17]));

            TestHelper.Assert(Constants.s7.Equals("\u05F0") &&
                    Constants.s7.Equals(Constants.sw7) &&
                    Constants.s7.Equals(literals[7]) &&
                    Constants.s7.Equals(literals[18]));

            TestHelper.Assert(Constants.s8.Equals("\U00010000") &&
                    Constants.s8.Equals(Constants.sw8) &&
                    Constants.s8.Equals(literals[8]) &&
                    Constants.s8.Equals(literals[19]));

            TestHelper.Assert(Constants.s9.Equals("\U0001F34C") &&
                    Constants.s9.Equals(Constants.sw9) &&
                    Constants.s9.Equals(literals[9]) &&
                    Constants.s9.Equals(literals[20]));

            TestHelper.Assert(Constants.s10.Equals("\u0DA7") &&
                    Constants.s10.Equals(Constants.sw10) &&
                    Constants.s10.Equals(literals[10]) &&
                    Constants.s10.Equals(literals[21]));

            TestHelper.Assert(Constants.ss0.Equals("\'\"\u003f\\\a\b\f\n\r\t\v\u0006") &&
                    Constants.ss0.Equals(Constants.ss1) &&
                    Constants.ss0.Equals(Constants.ss2) &&
                    Constants.ss0.Equals(literals[22]) &&
                    Constants.ss0.Equals(literals[23]) &&
                    Constants.ss0.Equals(literals[24]));

            TestHelper.Assert(Constants.ss3.Equals("\\\\U\\u\\") &&
                    Constants.ss3.Equals(literals[25]));

            TestHelper.Assert(Constants.ss4.Equals("\\A\\") &&
                Constants.ss4.Equals(literals[26]));

            TestHelper.Assert(Constants.ss5.Equals("\\u0041\\") &&
                    Constants.ss5.Equals(literals[27]));

            TestHelper.Assert(Constants.su0.Equals(Constants.su1) &&
                    Constants.su0.Equals(Constants.su2) &&
                    Constants.su0.Equals(literals[28]) &&
                    Constants.su0.Equals(literals[29]) &&
                    Constants.su0.Equals(literals[30]));

            p.IcePing();

            TestHelper.Assert(p.IceIsA("::ZeroC::Ice::Test::Operations::MyClass"));
            TestHelper.Assert(p.IceId().Equals("::ZeroC::Ice::Test::Operations::MyDerivedClass"));

            {
                string[] ids = p.IceIds();
                TestHelper.Assert(ids.Length == 3);
                TestHelper.Assert(ids[0].Equals("::Ice::Object"));
                TestHelper.Assert(ids[1].Equals("::ZeroC::Ice::Test::Operations::MyClass"));
                TestHelper.Assert(ids[2].Equals("::ZeroC::Ice::Test::Operations::MyDerivedClass"));
            }

            {
                p.OpVoid();
            }

            {
                byte b;
                byte r;

                (r, b) = p.OpByte(0xff, 0x0f);
                TestHelper.Assert(b == 0xf0);
                TestHelper.Assert(r == 0xff);
            }

            {
                bool b;
                bool r;

                (r, b) = p.OpBool(true, false);
                TestHelper.Assert(b);
                TestHelper.Assert(!r);
            }

            {
                short s;
                int i;
                long l;
                long r;

                (r, s, i, l) = p.OpShortIntLong(10, 11, 12L);
                TestHelper.Assert(s == 10);
                TestHelper.Assert(i == 11);
                TestHelper.Assert(l == 12);
                TestHelper.Assert(r == 12L);

                (r, s, i, l) = p.OpShortIntLong(short.MinValue, int.MinValue, long.MinValue);
                TestHelper.Assert(s == short.MinValue);
                TestHelper.Assert(i == int.MinValue);
                TestHelper.Assert(l == long.MinValue);
                TestHelper.Assert(r == long.MinValue);

                (r, s, i, l) = p.OpShortIntLong(short.MaxValue, int.MaxValue, long.MaxValue);
                TestHelper.Assert(s == short.MaxValue);
                TestHelper.Assert(i == int.MaxValue);
                TestHelper.Assert(l == long.MaxValue);
                TestHelper.Assert(r == long.MaxValue);
            }

            {
                ushort s;
                uint i;
                ulong l;
                ulong r;

                (r, s, i, l) = p.OpUShortUIntULong(10, 11, 12UL);
                TestHelper.Assert(s == 10);
                TestHelper.Assert(i == 11);
                TestHelper.Assert(l == 12);
                TestHelper.Assert(r == 12UL);

                (r, s, i, l) = p.OpUShortUIntULong(ushort.MinValue, uint.MinValue, ulong.MinValue);
                TestHelper.Assert(s == ushort.MinValue);
                TestHelper.Assert(i == uint.MinValue);
                TestHelper.Assert(l == ulong.MinValue);
                TestHelper.Assert(r == ulong.MinValue);

                (r, s, i, l) = p.OpUShortUIntULong(ushort.MaxValue, uint.MaxValue, ulong.MaxValue);
                TestHelper.Assert(s == ushort.MaxValue);
                TestHelper.Assert(i == uint.MaxValue);
                TestHelper.Assert(l == ulong.MaxValue);
                TestHelper.Assert(r == ulong.MaxValue);
            }

            {
                TestHelper.Assert(p.OpVarInt(0) == 0);
                TestHelper.Assert(p.OpVarInt(1) == 1);
                TestHelper.Assert(p.OpVarInt(-1) == -1);
                TestHelper.Assert(p.OpVarInt(5) == 5);
                TestHelper.Assert(p.OpVarInt(-5) == -5);
                TestHelper.Assert(p.OpVarInt(50) == 50);
                TestHelper.Assert(p.OpVarInt(-50) == -50);
                TestHelper.Assert(p.OpVarInt(500_000) == 500_000);
                TestHelper.Assert(p.OpVarInt(-500_000) == -500_000);
                TestHelper.Assert(p.OpVarInt(int.MaxValue) == int.MaxValue);
                TestHelper.Assert(p.OpVarInt(int.MinValue) == int.MinValue);

                TestHelper.Assert(p.OpVarLong(500_000) == 500_000);
                TestHelper.Assert(p.OpVarLong(-500_000) == -500_000);
                TestHelper.Assert(p.OpVarLong(2_305_843_009_213_693_951) == 2_305_843_009_213_693_951);
                TestHelper.Assert(p.OpVarLong(-2_305_843_009_213_693_952) == -2_305_843_009_213_693_952);

                try
                {
                    p.OpVarLong(long.MinValue);
                    TestHelper.Assert(false);
                }
                catch (ArgumentOutOfRangeException)
                {
                    // expected
                }

                try
                {
                    p.OpVarLong(long.MaxValue);
                    TestHelper.Assert(false);
                }
                catch (ArgumentOutOfRangeException)
                {
                    // expected
                }
            }

            {
                TestHelper.Assert(p.OpVarUInt(0) == 0);
                TestHelper.Assert(p.OpVarUInt(1) == 1);
                TestHelper.Assert(p.OpVarUInt(5) == 5);
                TestHelper.Assert(p.OpVarUInt(50) == 50);
                TestHelper.Assert(p.OpVarUInt(500_000) == 500_000);
                TestHelper.Assert(p.OpVarUInt(uint.MaxValue) == uint.MaxValue);

                TestHelper.Assert(p.OpVarULong(500_000) == 500_000);
                TestHelper.Assert(p.OpVarULong(4_611_686_018_427_387_903) == 4_611_686_018_427_387_903);

                try
                {
                    p.OpVarULong(ulong.MaxValue);
                    TestHelper.Assert(false);
                }
                catch (ArgumentOutOfRangeException)
                {
                    // expected
                }
            }

            {
                float f;
                double d;
                double r;

                (r, f, d) = p.OpFloatDouble(3.14f, 1.1e10);
                TestHelper.Assert(f == 3.14f);
                TestHelper.Assert(d == 1.1e10);
                TestHelper.Assert(r == 1.1e10);

                (r, f, d) = p.OpFloatDouble(float.Epsilon, double.MinValue);
                TestHelper.Assert(f == float.Epsilon);
                TestHelper.Assert(d == double.MinValue);
                TestHelper.Assert(r == double.MinValue);

                (r, f, d) = p.OpFloatDouble(float.MaxValue, double.MaxValue);
                TestHelper.Assert(f == float.MaxValue);
                TestHelper.Assert(d == double.MaxValue);
                TestHelper.Assert(r == double.MaxValue);
            }

            {
                string s;
                string r;

                (r, s) = p.OpString("hello", "world");
                TestHelper.Assert(s.Equals("world hello"));
                TestHelper.Assert(r.Equals("hello world"));
            }

            {
                MyEnum e;
                MyEnum r;

                (r, e) = p.OpMyEnum(MyEnum.enum2);
                TestHelper.Assert(e == MyEnum.enum2);
                TestHelper.Assert(r == MyEnum.enum3);
            }

            {
                IMyClassPrx? c1;
                IMyClassPrx? c2;
                IMyClassPrx? r;

                (r, c1, c2) = p.OpMyClass(p);
                TestHelper.Assert(ProxyComparer.IdentityAndFacet.Equals(c1!, p));
                TestHelper.Assert(!ProxyComparer.IdentityAndFacet.Equals(c2!, p));
                TestHelper.Assert(ProxyComparer.IdentityAndFacet.Equals(r!, p));
                TestHelper.Assert(c1!.Identity.Equals(Identity.Parse("test")));
                TestHelper.Assert(c2!.Identity.Equals(Identity.Parse("noSuchIdentity")));
                TestHelper.Assert(r!.Identity.Equals(Identity.Parse("test")));
                r.OpVoid();
                c1.OpVoid();
                try
                {
                    c2.OpVoid();
                    TestHelper.Assert(false);
                }
                catch (ObjectNotExistException)
                {
                }

                (r, c1, c2) = p.OpMyClass(null);
                TestHelper.Assert(c1 == null);
                TestHelper.Assert(c2 != null);
                TestHelper.Assert(ProxyComparer.IdentityAndFacet.Equals(r!, p));
                r!.OpVoid();
            }

            {
                var si1 = new Structure();
                si1.P = p;
                si1.E = MyEnum.enum3;
                si1.S = new AnotherStruct();
                si1.S.S = "abc";
                var si2 = new Structure();
                si2.P = null;
                si2.E = MyEnum.enum2;
                si2.S = new AnotherStruct();
                si2.S.S = "def";

                var (rso, so) = p.OpStruct(si1, si2);
                TestHelper.Assert(rso.P == null);
                TestHelper.Assert(rso.E == MyEnum.enum2);
                TestHelper.Assert(rso.S.S.Equals("def"));
                TestHelper.Assert(so.P!.Equals(p));
                TestHelper.Assert(so.E == MyEnum.enum3);
                TestHelper.Assert(so.S.S.Equals("a new string"));
                so.P.OpVoid();

                // Test marshalling of null structs and structs with null members.
                si1 = new Structure();
                si1.S = new AnotherStruct("");
                si2 = new Structure();
                si2.S = new AnotherStruct("");

                (rso, so) = p.OpStruct(si1, si2);
                TestHelper.Assert(rso.P == null);
                TestHelper.Assert(rso.E == MyEnum.enum1);
                TestHelper.Assert(rso.S.S.Length == 0);
                TestHelper.Assert(so.P == null);
                TestHelper.Assert(so.E == MyEnum.enum1);
                TestHelper.Assert(so.S.S.Equals("a new string"));
            }

            {
                byte[] bsi1 = new byte[] { 0x01, 0x11, 0x12, 0x22 };
                byte[] bsi2 = new byte[] { 0xf1, 0xf2, 0xf3, 0xf4 };

                byte[] bso;
                byte[] rso;

                (rso, bso) = p.OpByteS(bsi1, bsi2);
                TestHelper.Assert(bso.Length == 4);
                TestHelper.Assert(bso[0] == 0x22);
                TestHelper.Assert(bso[1] == 0x12);
                TestHelper.Assert(bso[2] == 0x11);
                TestHelper.Assert(bso[3] == 0x01);
                TestHelper.Assert(rso.Length == 8);
                TestHelper.Assert(rso[0] == 0x01);
                TestHelper.Assert(rso[1] == 0x11);
                TestHelper.Assert(rso[2] == 0x12);
                TestHelper.Assert(rso[3] == 0x22);
                TestHelper.Assert(rso[4] == 0xf1);
                TestHelper.Assert(rso[5] == 0xf2);
                TestHelper.Assert(rso[6] == 0xf3);
                TestHelper.Assert(rso[7] == 0xf4);
            }

            {
                bool[] bsi1 = new bool[] { true, true, false };
                bool[] bsi2 = new bool[] { false };

                bool[] bso;
                bool[] rso;

                (rso, bso) = p.OpBoolS(bsi1, bsi2);
                TestHelper.Assert(bso.Length == 4);
                TestHelper.Assert(bso[0]);
                TestHelper.Assert(bso[1]);
                TestHelper.Assert(!bso[2]);
                TestHelper.Assert(!bso[3]);
                TestHelper.Assert(rso.Length == 3);
                TestHelper.Assert(!rso[0]);
                TestHelper.Assert(rso[1]);
                TestHelper.Assert(rso[2]);
            }

            {
                short[] ssi = new short[] { 1, 2, 3 };
                int[] isi = new int[] { 5, 6, 7, 8 };
                long[] lsi = new long[] { 10, 30, 20 };

                short[] sso;
                int[] iso;
                long[] lso;
                long[] rso;

                (rso, sso, iso, lso) = p.OpShortIntLongS(ssi, isi, lsi);
                TestHelper.Assert(sso.Length == 3);
                TestHelper.Assert(sso[0] == 1);
                TestHelper.Assert(sso[1] == 2);
                TestHelper.Assert(sso[2] == 3);
                TestHelper.Assert(iso.Length == 4);
                TestHelper.Assert(iso[0] == 8);
                TestHelper.Assert(iso[1] == 7);
                TestHelper.Assert(iso[2] == 6);
                TestHelper.Assert(iso[3] == 5);
                TestHelper.Assert(lso.Length == 6);
                TestHelper.Assert(lso[0] == 10);
                TestHelper.Assert(lso[1] == 30);
                TestHelper.Assert(lso[2] == 20);
                TestHelper.Assert(lso[3] == 10);
                TestHelper.Assert(lso[4] == 30);
                TestHelper.Assert(lso[5] == 20);
                TestHelper.Assert(rso.Length == 3);
                TestHelper.Assert(rso[0] == 10);
                TestHelper.Assert(rso[1] == 30);
                TestHelper.Assert(rso[2] == 20);
            }

            {
                ushort[] ssi = new ushort[] { 1, 2, 3 };
                uint[] isi = new uint[] { 5, 6, 7, 8 };
                ulong[] lsi = new ulong[] { 10, 30, 20 };

                ushort[] sso;
                uint[] iso;
                ulong[] lso;
                ulong[] rso;

                (rso, sso, iso, lso) = p.OpUShortUIntULongS(ssi, isi, lsi);
                TestHelper.Assert(sso.Length == 3);
                TestHelper.Assert(sso[0] == 1);
                TestHelper.Assert(sso[1] == 2);
                TestHelper.Assert(sso[2] == 3);
                TestHelper.Assert(iso.Length == 4);
                TestHelper.Assert(iso[0] == 8);
                TestHelper.Assert(iso[1] == 7);
                TestHelper.Assert(iso[2] == 6);
                TestHelper.Assert(iso[3] == 5);
                TestHelper.Assert(lso.Length == 6);
                TestHelper.Assert(lso[0] == 10);
                TestHelper.Assert(lso[1] == 30);
                TestHelper.Assert(lso[2] == 20);
                TestHelper.Assert(lso[3] == 10);
                TestHelper.Assert(lso[4] == 30);
                TestHelper.Assert(lso[5] == 20);
                TestHelper.Assert(rso.Length == 3);
                TestHelper.Assert(rso[0] == 10);
                TestHelper.Assert(rso[1] == 30);
                TestHelper.Assert(rso[2] == 20);
            }

            {
                int[] isi = new int[] { 5, 6, 7, 8 };
                long[] lsi = new long[] { 1_000, 3_000_000_000_000, -200_000 };

                int[] iso;
                long[] lso;
                long[] rso;

                (rso, iso, lso) = p.OpVarIntVarLongS(isi, lsi);
                TestHelper.Assert(iso.Length == 4);
                TestHelper.Assert(iso[0] == 8);
                TestHelper.Assert(iso[1] == 7);
                TestHelper.Assert(iso[2] == 6);
                TestHelper.Assert(iso[3] == 5);
                TestHelper.Assert(lso.Length == 6);
                TestHelper.Assert(lso[0] == 1_000);
                TestHelper.Assert(lso[1] == 3_000_000_000_000);
                TestHelper.Assert(lso[2] == -200_000);
                TestHelper.Assert(lso[3] == 1_000);
                TestHelper.Assert(lso[4] == 3_000_000_000_000);
                TestHelper.Assert(lso[5] == -200_000);
                TestHelper.Assert(rso.Length == 3);
                TestHelper.Assert(rso[0] == 1_000);
                TestHelper.Assert(rso[1] == 3_000_000_000_000);
                TestHelper.Assert(rso[2] == -200_000);
            }

            {
                uint[] isi = new uint[] { 5, 6, 7, 8 };
                ulong[] lsi = new ulong[] { 1_000, 3_000_000_000_000, 200_000 };

                uint[] iso;
                ulong[] lso;
                ulong[] rso;

                (rso, iso, lso) = p.OpVarUIntVarULongS(isi, lsi);
                TestHelper.Assert(iso.Length == 4);
                TestHelper.Assert(iso[0] == 8);
                TestHelper.Assert(iso[1] == 7);
                TestHelper.Assert(iso[2] == 6);
                TestHelper.Assert(iso[3] == 5);
                TestHelper.Assert(lso.Length == 6);
                TestHelper.Assert(lso[0] == 1_000);
                TestHelper.Assert(lso[1] == 3_000_000_000_000);
                TestHelper.Assert(lso[2] == 200_000);
                TestHelper.Assert(lso[3] == 1_000);
                TestHelper.Assert(lso[4] == 3_000_000_000_000);
                TestHelper.Assert(lso[5] == 200_000);
                TestHelper.Assert(rso.Length == 3);
                TestHelper.Assert(rso[0] == 1_000);
                TestHelper.Assert(rso[1] == 3_000_000_000_000);
                TestHelper.Assert(rso[2] == 200_000);
            }

            {
                float[] fsi = new float[] { 3.14f, 1.11f };
                double[] dsi = new double[] { 1.1e10, 1.2e10, 1.3e10 };

                float[] fso;
                double[] dso;
                double[] rso;

                (rso, fso, dso) = p.OpFloatDoubleS(fsi, dsi);
                TestHelper.Assert(fso.Length == 2);
                TestHelper.Assert(fso[0] == 3.14f);
                TestHelper.Assert(fso[1] == 1.11f);
                TestHelper.Assert(dso.Length == 3);
                TestHelper.Assert(dso[0] == 1.3e10);
                TestHelper.Assert(dso[1] == 1.2e10);
                TestHelper.Assert(dso[2] == 1.1e10);
                TestHelper.Assert(rso.Length == 5);
                TestHelper.Assert(rso[0] == 1.1e10);
                TestHelper.Assert(rso[1] == 1.2e10);
                TestHelper.Assert(rso[2] == 1.3e10);
                TestHelper.Assert((float)rso[3] == 3.14f);
                TestHelper.Assert((float)rso[4] == 1.11f);
            }

            {
                string[] ssi1 = new string[] { "abc", "de", "fghi" };
                string[] ssi2 = new string[] { "xyz" };

                string[] sso;
                string[] rso;

                (rso, sso) = p.OpStringS(ssi1, ssi2);
                TestHelper.Assert(sso.Length == 4);
                TestHelper.Assert(sso[0].Equals("abc"));
                TestHelper.Assert(sso[1].Equals("de"));
                TestHelper.Assert(sso[2].Equals("fghi"));
                TestHelper.Assert(sso[3].Equals("xyz"));
                TestHelper.Assert(rso.Length == 3);
                TestHelper.Assert(rso[0].Equals("fghi"));
                TestHelper.Assert(rso[1].Equals("de"));
                TestHelper.Assert(rso[2].Equals("abc"));
            }

            {
                byte[] s11 = new byte[] { 0x01, 0x11, 0x12 };
                byte[] s12 = new byte[] { 0xff };
                byte[][] bsi1 = new byte[][] { s11, s12 };

                byte[] s21 = new byte[] { 0x0e };
                byte[] s22 = new byte[] { 0xf2, 0xf1 };
                byte[][] bsi2 = new byte[][] { s21, s22 };

                byte[][] bso;
                byte[][] rso;

                (rso, bso) = p.OpByteSS(bsi1, bsi2);
                TestHelper.Assert(bso.Length == 2);
                TestHelper.Assert(bso[0].Length == 1);
                TestHelper.Assert(bso[0][0] == 0xff);
                TestHelper.Assert(bso[1].Length == 3);
                TestHelper.Assert(bso[1][0] == 0x01);
                TestHelper.Assert(bso[1][1] == 0x11);
                TestHelper.Assert(bso[1][2] == 0x12);
                TestHelper.Assert(rso.Length == 4);
                TestHelper.Assert(rso[0].Length == 3);
                TestHelper.Assert(rso[0][0] == 0x01);
                TestHelper.Assert(rso[0][1] == 0x11);
                TestHelper.Assert(rso[0][2] == 0x12);
                TestHelper.Assert(rso[1].Length == 1);
                TestHelper.Assert(rso[1][0] == 0xff);
                TestHelper.Assert(rso[2].Length == 1);
                TestHelper.Assert(rso[2][0] == 0x0e);
                TestHelper.Assert(rso[3].Length == 2);
                TestHelper.Assert(rso[3][0] == 0xf2);
                TestHelper.Assert(rso[3][1] == 0xf1);
            }

            {
                bool[] s11 = new bool[] { true };
                bool[] s12 = new bool[] { false };
                bool[] s13 = new bool[] { true, true };
                bool[][] bsi1 = new bool[][] { s11, s12, s13 };

                bool[] s21 = new bool[] { false, false, true };
                bool[][] bsi2 = new bool[][] { s21 };

                bool[][] rso;
                bool[][] bso;

                (rso, bso) = p.OpBoolSS(bsi1, bsi2);
                TestHelper.Assert(bso.Length == 4);
                TestHelper.Assert(bso[0].Length == 1);
                TestHelper.Assert(bso[0][0]);
                TestHelper.Assert(bso[1].Length == 1);
                TestHelper.Assert(!bso[1][0]);
                TestHelper.Assert(bso[2].Length == 2);
                TestHelper.Assert(bso[2][0]);
                TestHelper.Assert(bso[2][1]);
                TestHelper.Assert(bso[3].Length == 3);
                TestHelper.Assert(!bso[3][0]);
                TestHelper.Assert(!bso[3][1]);
                TestHelper.Assert(bso[3][2]);
                TestHelper.Assert(rso.Length == 3);
                TestHelper.Assert(rso[0].Length == 2);
                TestHelper.Assert(rso[0][0]);
                TestHelper.Assert(rso[0][1]);
                TestHelper.Assert(rso[1].Length == 1);
                TestHelper.Assert(!rso[1][0]);
                TestHelper.Assert(rso[2].Length == 1);
                TestHelper.Assert(rso[2][0]);
            }

            {
                short[] s11 = new short[] { 1, 2, 5 };
                short[] s12 = new short[] { 13 };
                short[] s13 = Array.Empty<short>();
                short[][] ssi = new short[][] { s11, s12, s13 };

                int[] i11 = new int[] { 24, 98 };
                int[] i12 = new int[] { 42 };
                int[][] isi = new int[][] { i11, i12 };

                long[] l11 = new long[] { 496, 1729 };
                long[][] lsi = new long[][] { l11 };

                short[][] sso;
                int[][] iso;
                long[][] lso;
                long[][] rso;

                (rso, sso, iso, lso) = p.OpShortIntLongSS(ssi, isi, lsi);
                TestHelper.Assert(rso.Length == 1);
                TestHelper.Assert(rso[0].Length == 2);
                TestHelper.Assert(rso[0][0] == 496);
                TestHelper.Assert(rso[0][1] == 1729);
                TestHelper.Assert(sso.Length == 3);
                TestHelper.Assert(sso[0].Length == 3);
                TestHelper.Assert(sso[0][0] == 1);
                TestHelper.Assert(sso[0][1] == 2);
                TestHelper.Assert(sso[0][2] == 5);
                TestHelper.Assert(sso[1].Length == 1);
                TestHelper.Assert(sso[1][0] == 13);
                TestHelper.Assert(sso[2].Length == 0);
                TestHelper.Assert(iso.Length == 2);
                TestHelper.Assert(iso[0].Length == 1);
                TestHelper.Assert(iso[0][0] == 42);
                TestHelper.Assert(iso[1].Length == 2);
                TestHelper.Assert(iso[1][0] == 24);
                TestHelper.Assert(iso[1][1] == 98);
                TestHelper.Assert(lso.Length == 2);
                TestHelper.Assert(lso[0].Length == 2);
                TestHelper.Assert(lso[0][0] == 496);
                TestHelper.Assert(lso[0][1] == 1729);
                TestHelper.Assert(lso[1].Length == 2);
                TestHelper.Assert(lso[1][0] == 496);
                TestHelper.Assert(lso[1][1] == 1729);
            }

            {
                ushort[] s11 = new ushort[] { 1, 2, 5 };
                ushort[] s12 = new ushort[] { 13 };
                ushort[] s13 = Array.Empty<ushort>();
                ushort[][] ssi = new ushort[][] { s11, s12, s13 };

                uint[] i11 = new uint[] { 24, 98 };
                uint[] i12 = new uint[] { 42 };
                uint[][] isi = new uint[][] { i11, i12 };

                ulong[] l11 = new ulong[] { 496, 1729 };
                ulong[][] lsi = new ulong[][] { l11 };

                ushort[][] sso;
                uint[][] iso;
                ulong[][] lso;
                ulong[][] rso;

                (rso, sso, iso, lso) = p.OpUShortUIntULongSS(ssi, isi, lsi);
                TestHelper.Assert(rso.Length == 1);
                TestHelper.Assert(rso[0].Length == 2);
                TestHelper.Assert(rso[0][0] == 496);
                TestHelper.Assert(rso[0][1] == 1729);
                TestHelper.Assert(sso.Length == 3);
                TestHelper.Assert(sso[0].Length == 3);
                TestHelper.Assert(sso[0][0] == 1);
                TestHelper.Assert(sso[0][1] == 2);
                TestHelper.Assert(sso[0][2] == 5);
                TestHelper.Assert(sso[1].Length == 1);
                TestHelper.Assert(sso[1][0] == 13);
                TestHelper.Assert(sso[2].Length == 0);
                TestHelper.Assert(iso.Length == 2);
                TestHelper.Assert(iso[0].Length == 1);
                TestHelper.Assert(iso[0][0] == 42);
                TestHelper.Assert(iso[1].Length == 2);
                TestHelper.Assert(iso[1][0] == 24);
                TestHelper.Assert(iso[1][1] == 98);
                TestHelper.Assert(lso.Length == 2);
                TestHelper.Assert(lso[0].Length == 2);
                TestHelper.Assert(lso[0][0] == 496);
                TestHelper.Assert(lso[0][1] == 1729);
                TestHelper.Assert(lso[1].Length == 2);
                TestHelper.Assert(lso[1][0] == 496);
                TestHelper.Assert(lso[1][1] == 1729);
            }

            {
                float[] f11 = new float[] { 3.14f };
                float[] f12 = new float[] { 1.11f };
                float[] f13 = Array.Empty<float>();
                float[][] fsi = new float[][] { f11, f12, f13 };

                double[] d11 = new double[] { 1.1e10, 1.2e10, 1.3e10 };
                double[][] dsi = new double[][] { d11 };

                float[][] fso;
                double[][] dso;
                double[][] rso;

                (rso, fso, dso) = p.OpFloatDoubleSS(fsi, dsi);
                TestHelper.Assert(fso.Length == 3);
                TestHelper.Assert(fso[0].Length == 1);
                TestHelper.Assert(fso[0][0] == 3.14f);
                TestHelper.Assert(fso[1].Length == 1);
                TestHelper.Assert(fso[1][0] == 1.11f);
                TestHelper.Assert(fso[2].Length == 0);
                TestHelper.Assert(dso.Length == 1);
                TestHelper.Assert(dso[0].Length == 3);
                TestHelper.Assert(dso[0][0] == 1.1e10);
                TestHelper.Assert(dso[0][1] == 1.2e10);
                TestHelper.Assert(dso[0][2] == 1.3e10);
                TestHelper.Assert(rso.Length == 2);
                TestHelper.Assert(rso[0].Length == 3);
                TestHelper.Assert(rso[0][0] == 1.1e10);
                TestHelper.Assert(rso[0][1] == 1.2e10);
                TestHelper.Assert(rso[0][2] == 1.3e10);
                TestHelper.Assert(rso[1].Length == 3);
                TestHelper.Assert(rso[1][0] == 1.1e10);
                TestHelper.Assert(rso[1][1] == 1.2e10);
                TestHelper.Assert(rso[1][2] == 1.3e10);
            }

            {
                string[] s11 = new string[] { "abc" };
                string[] s12 = new string[] { "de", "fghi" };
                string[][] ssi1 = new string[][] { s11, s12 };

                string[] s21 = Array.Empty<string>();
                string[] s22 = Array.Empty<string>();
                string[] s23 = new string[] { "xyz" };
                string[][] ssi2 = new string[][] { s21, s22, s23 };

                string[][] sso;
                string[][] rso;

                (rso, sso) = p.OpStringSS(ssi1, ssi2);
                TestHelper.Assert(sso.Length == 5);
                TestHelper.Assert(sso[0].Length == 1);
                TestHelper.Assert(sso[0][0].Equals("abc"));
                TestHelper.Assert(sso[1].Length == 2);
                TestHelper.Assert(sso[1][0].Equals("de"));
                TestHelper.Assert(sso[1][1].Equals("fghi"));
                TestHelper.Assert(sso[2].Length == 0);
                TestHelper.Assert(sso[3].Length == 0);
                TestHelper.Assert(sso[4].Length == 1);
                TestHelper.Assert(sso[4][0].Equals("xyz"));
                TestHelper.Assert(rso.Length == 3);
                TestHelper.Assert(rso[0].Length == 1);
                TestHelper.Assert(rso[0][0].Equals("xyz"));
                TestHelper.Assert(rso[1].Length == 0);
                TestHelper.Assert(rso[2].Length == 0);
            }

            {
                string[] s111 = new string[] { "abc", "de" };
                string[] s112 = new string[] { "xyz" };
                string[][] ss11 = new string[][] { s111, s112 };
                string[] s121 = new string[] { "hello" };
                string[][] ss12 = new string[][] { s121 };
                string[][][] sssi1 = new string[][][] { ss11, ss12 };

                string[] s211 = new string[] { "", "" };
                string[] s212 = new string[] { "abcd" };
                string[][] ss21 = new string[][] { s211, s212 };
                string[] s221 = new string[] { "" };
                string[][] ss22 = new string[][] { s221 };
                string[][] ss23 = Array.Empty<string[]>();
                string[][][] sssi2 = new string[][][] { ss21, ss22, ss23 };

                string[][][] ssso;
                string[][][] rsso;

                (rsso, ssso) = p.OpStringSSS(sssi1, sssi2);
                TestHelper.Assert(ssso.Length == 5);
                TestHelper.Assert(ssso[0].Length == 2);
                TestHelper.Assert(ssso[0][0].Length == 2);
                TestHelper.Assert(ssso[0][1].Length == 1);
                TestHelper.Assert(ssso[1].Length == 1);
                TestHelper.Assert(ssso[1][0].Length == 1);
                TestHelper.Assert(ssso[2].Length == 2);
                TestHelper.Assert(ssso[2][0].Length == 2);
                TestHelper.Assert(ssso[2][1].Length == 1);
                TestHelper.Assert(ssso[3].Length == 1);
                TestHelper.Assert(ssso[3][0].Length == 1);
                TestHelper.Assert(ssso[4].Length == 0);
                TestHelper.Assert(ssso[0][0][0].Equals("abc"));
                TestHelper.Assert(ssso[0][0][1].Equals("de"));
                TestHelper.Assert(ssso[0][1][0].Equals("xyz"));
                TestHelper.Assert(ssso[1][0][0].Equals("hello"));
                TestHelper.Assert(ssso[2][0][0].Length == 0);
                TestHelper.Assert(ssso[2][0][1].Length == 0);
                TestHelper.Assert(ssso[2][1][0].Equals("abcd"));
                TestHelper.Assert(ssso[3][0][0].Length == 0);

                TestHelper.Assert(rsso.Length == 3);
                TestHelper.Assert(rsso[0].Length == 0);
                TestHelper.Assert(rsso[1].Length == 1);
                TestHelper.Assert(rsso[1][0].Length == 1);
                TestHelper.Assert(rsso[2].Length == 2);
                TestHelper.Assert(rsso[2][0].Length == 2);
                TestHelper.Assert(rsso[2][1].Length == 1);
                TestHelper.Assert(rsso[1][0][0].Length == 0);
                TestHelper.Assert(rsso[2][0][0].Length == 0);
                TestHelper.Assert(rsso[2][0][1].Length == 0);
                TestHelper.Assert(rsso[2][1][0].Equals("abcd"));
            }

            {
                var di1 = new Dictionary<byte, bool>
                {
                    [10] = true,
                    [100] = false
                };
                var di2 = new Dictionary<byte, bool>();
                di2[10] = true;
                di2[11] = false;
                di2[101] = true;

                var (ro, _do) = p.OpByteBoolD(di1, di2);

                TestHelper.Assert(_do.DictionaryEqual(di1));
                TestHelper.Assert(ro.Count == 4);
                TestHelper.Assert(ro[10] == true);
                TestHelper.Assert(ro[11] == false);
                TestHelper.Assert(ro[100] == false);
                TestHelper.Assert(ro[101] == true);
            }

            {
                var di1 = new Dictionary<short, int>();
                di1[110] = -1;
                di1[1100] = 123123;
                var di2 = new Dictionary<short, int>();
                di2[110] = -1;
                di2[111] = -100;
                di2[1101] = 0;

                var (ro, _do) = p.OpShortIntD(di1, di2);

                TestHelper.Assert(_do.DictionaryEqual(di1));
                TestHelper.Assert(ro.Count == 4);
                TestHelper.Assert(ro[110] == -1);
                TestHelper.Assert(ro[111] == -100);
                TestHelper.Assert(ro[1100] == 123123);
                TestHelper.Assert(ro[1101] == 0);
            }

            {
                var di1 = new Dictionary<ushort, uint>();
                di1[110] = 1;
                di1[1100] = 123123;
                var di2 = new Dictionary<ushort, uint>();
                di2[110] = 1;
                di2[111] = 100;
                di2[1101] = 0;

                var (ro, _do) = p.OpUShortUIntD(di1, di2);

                TestHelper.Assert(_do.DictionaryEqual(di1));
                TestHelper.Assert(ro.Count == 4);
                TestHelper.Assert(ro[110] == 1);
                TestHelper.Assert(ro[111] == 100);
                TestHelper.Assert(ro[1100] == 123123);
                TestHelper.Assert(ro[1101] == 0);
            }

            {
                var di1 = new Dictionary<long, float>();
                di1[999999110L] = -1.1f;
                di1[999999111L] = 123123.2f;
                var di2 = new Dictionary<long, float>();
                di2[999999110L] = -1.1f;
                di2[999999120L] = -100.4f;
                di2[999999130L] = 0.5f;

                var (ro, _do) = p.OpLongFloatD(di1, di2);

                TestHelper.Assert(_do.DictionaryEqual(di1));
                TestHelper.Assert(ro.Count == 4);
                TestHelper.Assert(ro[999999110L] == -1.1f);
                TestHelper.Assert(ro[999999120L] == -100.4f);
                TestHelper.Assert(ro[999999111L] == 123123.2f);
                TestHelper.Assert(ro[999999130L] == 0.5f);
            }

            {
                var di1 = new Dictionary<string, string>();
                di1["foo"] = "abc -1.1";
                di1["bar"] = "abc 123123.2";
                var di2 = new Dictionary<string, string>();
                di2["foo"] = "abc -1.1";
                di2["FOO"] = "abc -100.4";
                di2["BAR"] = "abc 0.5";

                var (ro, _do) = p.OpStringStringD(di1, di2);

                TestHelper.Assert(_do.DictionaryEqual(di1));
                TestHelper.Assert(ro.Count == 4);
                TestHelper.Assert(ro["foo"].Equals("abc -1.1"));
                TestHelper.Assert(ro["FOO"].Equals("abc -100.4"));
                TestHelper.Assert(ro["bar"].Equals("abc 123123.2"));
                TestHelper.Assert(ro["BAR"].Equals("abc 0.5"));
            }

            {
                var di1 = new Dictionary<string, MyEnum>();
                di1["abc"] = MyEnum.enum1;
                di1[""] = MyEnum.enum2;
                var di2 = new Dictionary<string, MyEnum>();
                di2["abc"] = MyEnum.enum1;
                di2["qwerty"] = MyEnum.enum3;
                di2["Hello!!"] = MyEnum.enum2;

                var (ro, _do) = p.OpStringMyEnumD(di1, di2);

                TestHelper.Assert(_do.DictionaryEqual(di1));
                TestHelper.Assert(ro.Count == 4);
                TestHelper.Assert(ro["abc"] == MyEnum.enum1);
                TestHelper.Assert(ro["qwerty"] == MyEnum.enum3);
                TestHelper.Assert(ro[""] == MyEnum.enum2);
                TestHelper.Assert(ro["Hello!!"] == MyEnum.enum2);
            }

            {
                var di1 = new Dictionary<MyEnum, string>();
                di1[MyEnum.enum1] = "abc";
                var di2 = new Dictionary<MyEnum, string>();
                di2[MyEnum.enum2] = "Hello!!";
                di2[MyEnum.enum3] = "qwerty";

                var (ro, _do) = p.OpMyEnumStringD(di1, di2);

                TestHelper.Assert(_do.DictionaryEqual(di1));
                TestHelper.Assert(ro.Count == 3);
                TestHelper.Assert(ro[MyEnum.enum1].Equals("abc"));
                TestHelper.Assert(ro[MyEnum.enum2].Equals("Hello!!"));
                TestHelper.Assert(ro[MyEnum.enum3].Equals("qwerty"));
            }

            {
                var s11 = new MyStruct(1, 1);
                var s12 = new MyStruct(1, 2);
                var di1 = new Dictionary<MyStruct, MyEnum>();
                di1[s11] = MyEnum.enum1;
                di1[s12] = MyEnum.enum2;

                var s22 = new MyStruct(2, 2);
                var s23 = new MyStruct(2, 3);
                var di2 = new Dictionary<MyStruct, MyEnum>();
                di2[s11] = MyEnum.enum1;
                di2[s22] = MyEnum.enum3;
                di2[s23] = MyEnum.enum2;

                var (ro, _do) = p.OpMyStructMyEnumD(di1, di2);

                TestHelper.Assert(_do.DictionaryEqual(di1));
                TestHelper.Assert(ro.Count == 4);
                TestHelper.Assert(ro[s11] == MyEnum.enum1);
                TestHelper.Assert(ro[s12] == MyEnum.enum2);
                TestHelper.Assert(ro[s22] == MyEnum.enum3);
                TestHelper.Assert(ro[s23] == MyEnum.enum2);
            }

            {
                Dictionary<byte, bool>[] dsi1 = new Dictionary<byte, bool>[2];
                Dictionary<byte, bool>[] dsi2 = new Dictionary<byte, bool>[1];

                var di1 = new Dictionary<byte, bool>();
                di1[10] = true;
                di1[100] = false;
                var di2 = new Dictionary<byte, bool>();
                di2[10] = true;
                di2[11] = false;
                di2[101] = true;
                var di3 = new Dictionary<byte, bool>();
                di3[100] = false;
                di3[101] = false;

                dsi1[0] = di1;
                dsi1[1] = di2;
                dsi2[0] = di3;

                var (ro, _do) = p.OpByteBoolDS(dsi1, dsi2);

                TestHelper.Assert(ro.Length == 2);
                TestHelper.Assert(ro[0].Count == 3);
                TestHelper.Assert(ro[0][10]);
                TestHelper.Assert(!ro[0][11]);
                TestHelper.Assert(ro[0][101]);
                TestHelper.Assert(ro[1].Count == 2);
                TestHelper.Assert(ro[1][10]);
                TestHelper.Assert(!ro[1][100]);

                TestHelper.Assert(_do.Length == 3);
                TestHelper.Assert(_do[0].Count == 2);
                TestHelper.Assert(!_do[0][100]);
                TestHelper.Assert(!_do[0][101]);
                TestHelper.Assert(_do[1].Count == 2);
                TestHelper.Assert(_do[1][10]);
                TestHelper.Assert(!_do[1][100]);
                TestHelper.Assert(_do[2].Count == 3);
                TestHelper.Assert(_do[2][10]);
                TestHelper.Assert(!_do[2][11]);
                TestHelper.Assert(_do[2][101]);
            }

            {
                Dictionary<short, int>[] dsi1 = new Dictionary<short, int>[2];
                Dictionary<short, int>[] dsi2 = new Dictionary<short, int>[1];

                var di1 = new Dictionary<short, int>();
                di1[110] = -1;
                di1[1100] = 123123;
                var di2 = new Dictionary<short, int>();
                di2[110] = -1;
                di2[111] = -100;
                di2[1101] = 0;
                var di3 = new Dictionary<short, int>();
                di3[100] = -1001;

                dsi1[0] = di1;
                dsi1[1] = di2;
                dsi2[0] = di3;

                var (ro, _do) = p.OpShortIntDS(dsi1, dsi2);

                TestHelper.Assert(ro.Length == 2);
                TestHelper.Assert(ro[0].Count == 3);
                TestHelper.Assert(ro[0][110] == -1);
                TestHelper.Assert(ro[0][111] == -100);
                TestHelper.Assert(ro[0][1101] == 0);
                TestHelper.Assert(ro[1].Count == 2);
                TestHelper.Assert(ro[1][110] == -1);
                TestHelper.Assert(ro[1][1100] == 123123);

                TestHelper.Assert(_do.Length == 3);
                TestHelper.Assert(_do[0].Count == 1);
                TestHelper.Assert(_do[0][100] == -1001);
                TestHelper.Assert(_do[1].Count == 2);
                TestHelper.Assert(_do[1][110] == -1);
                TestHelper.Assert(_do[1][1100] == 123123);
                TestHelper.Assert(_do[2].Count == 3);
                TestHelper.Assert(_do[2][110] == -1);
                TestHelper.Assert(_do[2][111] == -100);
                TestHelper.Assert(_do[2][1101] == 0);
            }

            {
                Dictionary<ushort, uint>[] dsi1 = new Dictionary<ushort, uint>[2];
                Dictionary<ushort, uint>[] dsi2 = new Dictionary<ushort, uint>[1];

                var di1 = new Dictionary<ushort, uint>();
                di1[110] = 1;
                di1[1100] = 123123;
                var di2 = new Dictionary<ushort, uint>();
                di2[110] = 1;
                di2[111] = 100;
                di2[1101] = 0;
                var di3 = new Dictionary<ushort, uint>();
                di3[100] = 1001;

                dsi1[0] = di1;
                dsi1[1] = di2;
                dsi2[0] = di3;

                var (ro, _do) = p.OpUShortUIntDS(dsi1, dsi2);

                TestHelper.Assert(ro.Length == 2);
                TestHelper.Assert(ro[0].Count == 3);
                TestHelper.Assert(ro[0][110] == 1);
                TestHelper.Assert(ro[0][111] == 100);
                TestHelper.Assert(ro[0][1101] == 0);
                TestHelper.Assert(ro[1].Count == 2);
                TestHelper.Assert(ro[1][110] == 1);
                TestHelper.Assert(ro[1][1100] == 123123);

                TestHelper.Assert(_do.Length == 3);
                TestHelper.Assert(_do[0].Count == 1);
                TestHelper.Assert(_do[0][100] == 1001);
                TestHelper.Assert(_do[1].Count == 2);
                TestHelper.Assert(_do[1][110] == 1);
                TestHelper.Assert(_do[1][1100] == 123123);
                TestHelper.Assert(_do[2].Count == 3);
                TestHelper.Assert(_do[2][110] == 1);
                TestHelper.Assert(_do[2][111] == 100);
                TestHelper.Assert(_do[2][1101] == 0);
            }

            {
                Dictionary<long, float>[] dsi1 = new Dictionary<long, float>[2];
                Dictionary<long, float>[] dsi2 = new Dictionary<long, float>[1];

                var di1 = new Dictionary<long, float>();
                di1[999999110L] = -1.1f;
                di1[999999111L] = 123123.2f;
                var di2 = new Dictionary<long, float>();
                di2[999999110L] = -1.1f;
                di2[999999120L] = -100.4f;
                di2[999999130L] = 0.5f;
                var di3 = new Dictionary<long, float>();
                di3[999999140L] = 3.14f;

                dsi1[0] = di1;
                dsi1[1] = di2;
                dsi2[0] = di3;

                var (ro, _do) = p.OpLongFloatDS(dsi1, dsi2);

                TestHelper.Assert(ro.Length == 2);
                TestHelper.Assert(ro[0].Count == 3);
                TestHelper.Assert(ro[0][999999110L] == -1.1f);
                TestHelper.Assert(ro[0][999999120L] == -100.4f);
                TestHelper.Assert(ro[0][999999130L] == 0.5f);
                TestHelper.Assert(ro[1].Count == 2);
                TestHelper.Assert(ro[1][999999110L] == -1.1f);
                TestHelper.Assert(ro[1][999999111L] == 123123.2f);

                TestHelper.Assert(_do.Length == 3);
                TestHelper.Assert(_do[0].Count == 1);
                TestHelper.Assert(_do[0][999999140L] == 3.14f);
                TestHelper.Assert(_do[1].Count == 2);
                TestHelper.Assert(_do[1][999999110L] == -1.1f);
                TestHelper.Assert(_do[1][999999111L] == 123123.2f);
                TestHelper.Assert(_do[2].Count == 3);
                TestHelper.Assert(_do[2][999999110L] == -1.1f);
                TestHelper.Assert(_do[2][999999120L] == -100.4f);
                TestHelper.Assert(_do[2][999999130L] == 0.5f);
            }

            {
                Dictionary<ulong, float>[] dsi1 = new Dictionary<ulong, float>[2];
                Dictionary<ulong, float>[] dsi2 = new Dictionary<ulong, float>[1];

                var di1 = new Dictionary<ulong, float>();
                di1[999999110L] = -1.1f;
                di1[999999111L] = 123123.2f;
                var di2 = new Dictionary<ulong, float>();
                di2[999999110L] = -1.1f;
                di2[999999120L] = -100.4f;
                di2[999999130L] = 0.5f;
                var di3 = new Dictionary<ulong, float>();
                di3[999999140L] = 3.14f;

                dsi1[0] = di1;
                dsi1[1] = di2;
                dsi2[0] = di3;

                var (ro, _do) = p.OpULongFloatDS(dsi1, dsi2);

                TestHelper.Assert(ro.Length == 2);
                TestHelper.Assert(ro[0].Count == 3);
                TestHelper.Assert(ro[0][999999110L] == -1.1f);
                TestHelper.Assert(ro[0][999999120L] == -100.4f);
                TestHelper.Assert(ro[0][999999130L] == 0.5f);
                TestHelper.Assert(ro[1].Count == 2);
                TestHelper.Assert(ro[1][999999110L] == -1.1f);
                TestHelper.Assert(ro[1][999999111L] == 123123.2f);

                TestHelper.Assert(_do.Length == 3);
                TestHelper.Assert(_do[0].Count == 1);
                TestHelper.Assert(_do[0][999999140L] == 3.14f);
                TestHelper.Assert(_do[1].Count == 2);
                TestHelper.Assert(_do[1][999999110L] == -1.1f);
                TestHelper.Assert(_do[1][999999111L] == 123123.2f);
                TestHelper.Assert(_do[2].Count == 3);
                TestHelper.Assert(_do[2][999999110L] == -1.1f);
                TestHelper.Assert(_do[2][999999120L] == -100.4f);
                TestHelper.Assert(_do[2][999999130L] == 0.5f);
            }

            {
                Dictionary<string, string>[] dsi1 = new Dictionary<string, string>[2];
                Dictionary<string, string>[] dsi2 = new Dictionary<string, string>[1];

                var di1 = new Dictionary<string, string>();
                di1["foo"] = "abc -1.1";
                di1["bar"] = "abc 123123.2";
                var di2 = new Dictionary<string, string>();
                di2["foo"] = "abc -1.1";
                di2["FOO"] = "abc -100.4";
                di2["BAR"] = "abc 0.5";
                var di3 = new Dictionary<string, string>();
                di3["f00"] = "ABC -3.14";

                dsi1[0] = di1;
                dsi1[1] = di2;
                dsi2[0] = di3;

                var (ro, _do) = p.OpStringStringDS(dsi1, dsi2);

                TestHelper.Assert(ro.Length == 2);
                TestHelper.Assert(ro[0].Count == 3);
                TestHelper.Assert(ro[0]["foo"].Equals("abc -1.1"));
                TestHelper.Assert(ro[0]["FOO"].Equals("abc -100.4"));
                TestHelper.Assert(ro[0]["BAR"].Equals("abc 0.5"));
                TestHelper.Assert(ro[1].Count == 2);
                TestHelper.Assert(ro[1]["foo"] == "abc -1.1");
                TestHelper.Assert(ro[1]["bar"] == "abc 123123.2");

                TestHelper.Assert(_do.Length == 3);
                TestHelper.Assert(_do[0].Count == 1);
                TestHelper.Assert(_do[0]["f00"].Equals("ABC -3.14"));
                TestHelper.Assert(_do[1].Count == 2);
                TestHelper.Assert(_do[1]["foo"].Equals("abc -1.1"));
                TestHelper.Assert(_do[1]["bar"].Equals("abc 123123.2"));
                TestHelper.Assert(_do[2].Count == 3);
                TestHelper.Assert(_do[2]["foo"].Equals("abc -1.1"));
                TestHelper.Assert(_do[2]["FOO"].Equals("abc -100.4"));
                TestHelper.Assert(_do[2]["BAR"].Equals("abc 0.5"));
            }

            {
                var dsi1 = new Dictionary<string, MyEnum>[2];
                var dsi2 = new Dictionary<string, MyEnum>[1];

                var di1 = new Dictionary<string, MyEnum>();
                di1["abc"] = MyEnum.enum1;
                di1[""] = MyEnum.enum2;
                var di2 = new Dictionary<string, MyEnum>();
                di2["abc"] = MyEnum.enum1;
                di2["qwerty"] = MyEnum.enum3;
                di2["Hello!!"] = MyEnum.enum2;
                var di3 = new Dictionary<string, MyEnum>();
                di3["Goodbye"] = MyEnum.enum1;

                dsi1[0] = di1;
                dsi1[1] = di2;
                dsi2[0] = di3;

                var (ro, _do) = p.OpStringMyEnumDS(dsi1, dsi2);

                TestHelper.Assert(ro.Length == 2);
                TestHelper.Assert(ro[0].Count == 3);
                TestHelper.Assert(ro[0]["abc"] == MyEnum.enum1);
                TestHelper.Assert(ro[0]["qwerty"] == MyEnum.enum3);
                TestHelper.Assert(ro[0]["Hello!!"] == MyEnum.enum2);
                TestHelper.Assert(ro[1].Count == 2);
                TestHelper.Assert(ro[1]["abc"] == MyEnum.enum1);
                TestHelper.Assert(ro[1][""] == MyEnum.enum2);

                TestHelper.Assert(_do.Length == 3);
                TestHelper.Assert(_do[0].Count == 1);
                TestHelper.Assert(_do[0]["Goodbye"] == MyEnum.enum1);
                TestHelper.Assert(_do[1].Count == 2);
                TestHelper.Assert(_do[1]["abc"] == MyEnum.enum1);
                TestHelper.Assert(_do[1][""] == MyEnum.enum2);
                TestHelper.Assert(_do[2].Count == 3);
                TestHelper.Assert(_do[2]["abc"] == MyEnum.enum1);
                TestHelper.Assert(_do[2]["qwerty"] == MyEnum.enum3);
                TestHelper.Assert(_do[2]["Hello!!"] == MyEnum.enum2);
            }

            {
                var dsi1 = new Dictionary<MyEnum, string>[2];
                var dsi2 = new Dictionary<MyEnum, string>[1];

                var di1 = new Dictionary<MyEnum, string>();
                di1[MyEnum.enum1] = "abc";
                var di2 = new Dictionary<MyEnum, string>();
                di2[MyEnum.enum2] = "Hello!!";
                di2[MyEnum.enum3] = "qwerty";
                var di3 = new Dictionary<MyEnum, string>();
                di3[MyEnum.enum1] = "Goodbye";

                dsi1[0] = di1;
                dsi1[1] = di2;
                dsi2[0] = di3;

                var (ro, _do) = p.OpMyEnumStringDS(dsi1, dsi2);

                TestHelper.Assert(ro.Length == 2);
                TestHelper.Assert(ro[0].Count == 2);
                TestHelper.Assert(ro[0][MyEnum.enum2].Equals("Hello!!"));
                TestHelper.Assert(ro[0][MyEnum.enum3].Equals("qwerty"));
                TestHelper.Assert(ro[1].Count == 1);
                TestHelper.Assert(ro[1][MyEnum.enum1].Equals("abc"));

                TestHelper.Assert(_do.Length == 3);
                TestHelper.Assert(_do[0].Count == 1);
                TestHelper.Assert(_do[0][MyEnum.enum1].Equals("Goodbye"));
                TestHelper.Assert(_do[1].Count == 1);
                TestHelper.Assert(_do[1][MyEnum.enum1].Equals("abc"));
                TestHelper.Assert(_do[2].Count == 2);
                TestHelper.Assert(_do[2][MyEnum.enum2].Equals("Hello!!"));
                TestHelper.Assert(_do[2][MyEnum.enum3].Equals("qwerty"));
            }

            {
                var dsi1 = new Dictionary<MyStruct, MyEnum>[2];
                var dsi2 = new Dictionary<MyStruct, MyEnum>[1];

                var s11 = new MyStruct(1, 1);
                var s12 = new MyStruct(1, 2);
                var di1 = new Dictionary<MyStruct, MyEnum>();
                di1[s11] = MyEnum.enum1;
                di1[s12] = MyEnum.enum2;

                var s22 = new MyStruct(2, 2);
                var s23 = new MyStruct(2, 3);
                var di2 = new Dictionary<MyStruct, MyEnum>();
                di2[s11] = MyEnum.enum1;
                di2[s22] = MyEnum.enum3;
                di2[s23] = MyEnum.enum2;

                var di3 = new Dictionary<MyStruct, MyEnum>();
                di3[s23] = MyEnum.enum3;

                dsi1[0] = di1;
                dsi1[1] = di2;
                dsi2[0] = di3;

                var (ro, _do) = p.OpMyStructMyEnumDS(dsi1, dsi2);

                TestHelper.Assert(ro.Length == 2);
                TestHelper.Assert(ro[0].Count == 3);
                TestHelper.Assert(ro[0][s11] == MyEnum.enum1);
                TestHelper.Assert(ro[0][s22] == MyEnum.enum3);
                TestHelper.Assert(ro[0][s23] == MyEnum.enum2);
                TestHelper.Assert(ro[1].Count == 2);
                TestHelper.Assert(ro[1][s11] == MyEnum.enum1);
                TestHelper.Assert(ro[1][s12] == MyEnum.enum2);

                TestHelper.Assert(_do.Length == 3);
                TestHelper.Assert(_do[0].Count == 1);
                TestHelper.Assert(_do[0][s23] == MyEnum.enum3);
                TestHelper.Assert(_do[1].Count == 2);
                TestHelper.Assert(_do[1][s11] == MyEnum.enum1);
                TestHelper.Assert(_do[1][s12] == MyEnum.enum2);
                TestHelper.Assert(_do[2].Count == 3);
                TestHelper.Assert(_do[2][s11] == MyEnum.enum1);
                TestHelper.Assert(_do[2][s22] == MyEnum.enum3);
                TestHelper.Assert(_do[2][s23] == MyEnum.enum2);
            }

            {
                var sdi1 = new Dictionary<byte, byte[]>();
                var sdi2 = new Dictionary<byte, byte[]>();

                byte[] si1 = new byte[] { 0x01, 0x11 };
                byte[] si2 = new byte[] { 0x12 };
                byte[] si3 = new byte[] { 0xf2, 0xf3 };

                sdi1[0x01] = si1;
                sdi1[0x22] = si2;
                sdi2[0xf1] = si3;

                var (ro, _do) = p.OpByteByteSD(sdi1, sdi2);

                TestHelper.Assert(_do.Count == 1);
                TestHelper.Assert(_do[0xf1].Length == 2);
                TestHelper.Assert(_do[0xf1][0] == 0xf2);
                TestHelper.Assert(_do[0xf1][1] == 0xf3);

                TestHelper.Assert(ro.Count == 3);
                TestHelper.Assert(ro[0x01].Length == 2);
                TestHelper.Assert(ro[0x01][0] == 0x01);
                TestHelper.Assert(ro[0x01][1] == 0x11);
                TestHelper.Assert(ro[0x22].Length == 1);
                TestHelper.Assert(ro[0x22][0] == 0x12);
                TestHelper.Assert(ro[0xf1].Length == 2);
                TestHelper.Assert(ro[0xf1][0] == 0xf2);
                TestHelper.Assert(ro[0xf1][1] == 0xf3);
            }

            {
                var sdi1 = new Dictionary<bool, bool[]>();
                var sdi2 = new Dictionary<bool, bool[]>();

                bool[] si1 = new bool[] { true, false };
                bool[] si2 = new bool[] { false, true, true };

                sdi1[false] = si1;
                sdi1[true] = si2;
                sdi2[false] = si1;

                var (ro, _do) = p.OpBoolBoolSD(sdi1, sdi2);

                TestHelper.Assert(_do.Count == 1);
                TestHelper.Assert(_do[false].Length == 2);
                TestHelper.Assert(_do[false][0]);
                TestHelper.Assert(!_do[false][1]);
                TestHelper.Assert(ro.Count == 2);
                TestHelper.Assert(ro[false].Length == 2);
                TestHelper.Assert(ro[false][0]);
                TestHelper.Assert(!ro[false][1]);
                TestHelper.Assert(ro[true].Length == 3);
                TestHelper.Assert(!ro[true][0]);
                TestHelper.Assert(ro[true][1]);
                TestHelper.Assert(ro[true][2]);
            }

            {
                var sdi1 = new Dictionary<short, short[]>();
                var sdi2 = new Dictionary<short, short[]>();

                short[] si1 = new short[] { 1, 2, 3 };
                short[] si2 = new short[] { 4, 5 };
                short[] si3 = new short[] { 6, 7 };

                sdi1[1] = si1;
                sdi1[2] = si2;
                sdi2[4] = si3;

                var (ro, _do) = p.OpShortShortSD(sdi1, sdi2);

                TestHelper.Assert(_do.Count == 1);
                TestHelper.Assert(_do[4].Length == 2);
                TestHelper.Assert(_do[4][0] == 6);
                TestHelper.Assert(_do[4][1] == 7);

                TestHelper.Assert(ro.Count == 3);
                TestHelper.Assert(ro[1].Length == 3);
                TestHelper.Assert(ro[1][0] == 1);
                TestHelper.Assert(ro[1][1] == 2);
                TestHelper.Assert(ro[1][2] == 3);
                TestHelper.Assert(ro[2].Length == 2);
                TestHelper.Assert(ro[2][0] == 4);
                TestHelper.Assert(ro[2][1] == 5);
                TestHelper.Assert(ro[4].Length == 2);
                TestHelper.Assert(ro[4][0] == 6);
                TestHelper.Assert(ro[4][1] == 7);
            }

            {
                var sdi1 = new Dictionary<ushort, ushort[]>();
                var sdi2 = new Dictionary<ushort, ushort[]>();

                ushort[] si1 = new ushort[] { 1, 2, 3 };
                ushort[] si2 = new ushort[] { 4, 5 };
                ushort[] si3 = new ushort[] { 6, 7 };

                sdi1[1] = si1;
                sdi1[2] = si2;
                sdi2[4] = si3;

                var (ro, _do) = p.OpUShortUShortSD(sdi1, sdi2);

                TestHelper.Assert(_do.Count == 1);
                TestHelper.Assert(_do[4].Length == 2);
                TestHelper.Assert(_do[4][0] == 6);
                TestHelper.Assert(_do[4][1] == 7);

                TestHelper.Assert(ro.Count == 3);
                TestHelper.Assert(ro[1].Length == 3);
                TestHelper.Assert(ro[1][0] == 1);
                TestHelper.Assert(ro[1][1] == 2);
                TestHelper.Assert(ro[1][2] == 3);
                TestHelper.Assert(ro[2].Length == 2);
                TestHelper.Assert(ro[2][0] == 4);
                TestHelper.Assert(ro[2][1] == 5);
                TestHelper.Assert(ro[4].Length == 2);
                TestHelper.Assert(ro[4][0] == 6);
                TestHelper.Assert(ro[4][1] == 7);
            }

            {
                var sdi1 = new Dictionary<int, int[]>();
                var sdi2 = new Dictionary<int, int[]>();

                int[] si1 = new int[] { 100, 200, 300 };
                int[] si2 = new int[] { 400, 500 };
                int[] si3 = new int[] { 600, 700 };

                sdi1[100] = si1;
                sdi1[200] = si2;
                sdi2[400] = si3;

                var (ro, _do) = p.OpIntIntSD(sdi1, sdi2);

                TestHelper.Assert(_do.Count == 1);
                TestHelper.Assert(_do[400].Length == 2);
                TestHelper.Assert(_do[400][0] == 600);
                TestHelper.Assert(_do[400][1] == 700);

                TestHelper.Assert(ro.Count == 3);
                TestHelper.Assert(ro[100].Length == 3);
                TestHelper.Assert(ro[100][0] == 100);
                TestHelper.Assert(ro[100][1] == 200);
                TestHelper.Assert(ro[100][2] == 300);
                TestHelper.Assert(ro[200].Length == 2);
                TestHelper.Assert(ro[200][0] == 400);
                TestHelper.Assert(ro[200][1] == 500);
                TestHelper.Assert(ro[400].Length == 2);
                TestHelper.Assert(ro[400][0] == 600);
                TestHelper.Assert(ro[400][1] == 700);
            }

            {
                var sdi1 = new Dictionary<uint, uint[]>();
                var sdi2 = new Dictionary<uint, uint[]>();

                uint[] si1 = new uint[] { 100, 200, 300 };
                uint[] si2 = new uint[] { 400, 500 };
                uint[] si3 = new uint[] { 600, 700 };

                sdi1[100] = si1;
                sdi1[200] = si2;
                sdi2[400] = si3;

                var (ro, _do) = p.OpUIntUIntSD(sdi1, sdi2);

                TestHelper.Assert(_do.Count == 1);
                TestHelper.Assert(_do[400].Length == 2);
                TestHelper.Assert(_do[400][0] == 600);
                TestHelper.Assert(_do[400][1] == 700);

                TestHelper.Assert(ro.Count == 3);
                TestHelper.Assert(ro[100].Length == 3);
                TestHelper.Assert(ro[100][0] == 100);
                TestHelper.Assert(ro[100][1] == 200);
                TestHelper.Assert(ro[100][2] == 300);
                TestHelper.Assert(ro[200].Length == 2);
                TestHelper.Assert(ro[200][0] == 400);
                TestHelper.Assert(ro[200][1] == 500);
                TestHelper.Assert(ro[400].Length == 2);
                TestHelper.Assert(ro[400][0] == 600);
                TestHelper.Assert(ro[400][1] == 700);
            }

            {
                var sdi1 = new Dictionary<long, long[]>();
                var sdi2 = new Dictionary<long, long[]>();

                var si1 = new long[] { 999999110L, 999999111L, 999999110L };
                var si2 = new long[] { 999999120L, 999999130L };
                long[] si3 = new long[] { 999999110L, 999999120L };

                sdi1[999999990L] = si1;
                sdi1[999999991L] = si2;
                sdi2[999999992L] = si3;

                var (ro, _do) = p.OpLongLongSD(sdi1, sdi2);

                TestHelper.Assert(_do.Count == 1);
                TestHelper.Assert(_do[999999992L].Length == 2);
                TestHelper.Assert(_do[999999992L][0] == 999999110L);
                TestHelper.Assert(_do[999999992L][1] == 999999120L);
                TestHelper.Assert(ro.Count == 3);
                TestHelper.Assert(ro[999999990L].Length == 3);
                TestHelper.Assert(ro[999999990L][0] == 999999110L);
                TestHelper.Assert(ro[999999990L][1] == 999999111L);
                TestHelper.Assert(ro[999999990L][2] == 999999110L);
                TestHelper.Assert(ro[999999991L].Length == 2);
                TestHelper.Assert(ro[999999991L][0] == 999999120L);
                TestHelper.Assert(ro[999999991L][1] == 999999130L);
                TestHelper.Assert(ro[999999992L].Length == 2);
                TestHelper.Assert(ro[999999992L][0] == 999999110L);
                TestHelper.Assert(ro[999999992L][1] == 999999120L);
            }

            {
                var sdi1 = new Dictionary<ulong, ulong[]>();
                var sdi2 = new Dictionary<ulong, ulong[]>();

                var si1 = new ulong[] { 999999110L, 999999111L, 999999110L };
                var si2 = new ulong[] { 999999120L, 999999130L };
                ulong[] si3 = new ulong[] { 999999110L, 999999120L };

                sdi1[999999990L] = si1;
                sdi1[999999991L] = si2;
                sdi2[999999992L] = si3;

                var (ro, _do) = p.OpULongULongSD(sdi1, sdi2);

                TestHelper.Assert(_do.Count == 1);
                TestHelper.Assert(_do[999999992L].Length == 2);
                TestHelper.Assert(_do[999999992L][0] == 999999110L);
                TestHelper.Assert(_do[999999992L][1] == 999999120L);
                TestHelper.Assert(ro.Count == 3);
                TestHelper.Assert(ro[999999990L].Length == 3);
                TestHelper.Assert(ro[999999990L][0] == 999999110L);
                TestHelper.Assert(ro[999999990L][1] == 999999111L);
                TestHelper.Assert(ro[999999990L][2] == 999999110L);
                TestHelper.Assert(ro[999999991L].Length == 2);
                TestHelper.Assert(ro[999999991L][0] == 999999120L);
                TestHelper.Assert(ro[999999991L][1] == 999999130L);
                TestHelper.Assert(ro[999999992L].Length == 2);
                TestHelper.Assert(ro[999999992L][0] == 999999110L);
                TestHelper.Assert(ro[999999992L][1] == 999999120L);
            }

            {
                var sdi1 = new Dictionary<string, float[]>();
                var sdi2 = new Dictionary<string, float[]>();

                var si1 = new float[] { -1.1f, 123123.2f, 100.0f };
                var si2 = new float[] { 42.24f, -1.61f };
                var si3 = new float[] { -3.14f, 3.14f };

                sdi1["abc"] = si1;
                sdi1["ABC"] = si2;
                sdi2["aBc"] = si3;

                var (ro, _do) = p.OpStringFloatSD(sdi1, sdi2);

                TestHelper.Assert(_do.Count == 1);
                TestHelper.Assert(_do["aBc"].Length == 2);
                TestHelper.Assert(_do["aBc"][0] == -3.14f);
                TestHelper.Assert(_do["aBc"][1] == 3.14f);

                TestHelper.Assert(ro.Count == 3);
                TestHelper.Assert(ro["abc"].Length == 3);
                TestHelper.Assert(ro["abc"][0] == -1.1f);
                TestHelper.Assert(ro["abc"][1] == 123123.2f);
                TestHelper.Assert(ro["abc"][2] == 100.0f);
                TestHelper.Assert(ro["ABC"].Length == 2);
                TestHelper.Assert(ro["ABC"][0] == 42.24f);
                TestHelper.Assert(ro["ABC"][1] == -1.61f);
                TestHelper.Assert(ro["aBc"].Length == 2);
                TestHelper.Assert(ro["aBc"][0] == -3.14f);
                TestHelper.Assert(ro["aBc"][1] == 3.14f);
            }

            {
                var sdi1 = new Dictionary<string, double[]>();
                var sdi2 = new Dictionary<string, double[]>();

                var si1 = new double[] { 1.1E10, 1.2E10, 1.3E10 };
                var si2 = new double[] { 1.4E10, 1.5E10 };
                var si3 = new double[] { 1.6E10, 1.7E10 };

                sdi1["Hello!!"] = si1;
                sdi1["Goodbye"] = si2;
                sdi2[""] = si3;

                var (ro, _do) = p.OpStringDoubleSD(sdi1, sdi2);

                TestHelper.Assert(_do.Count == 1);
                TestHelper.Assert(_do[""].Length == 2);
                TestHelper.Assert(_do[""][0] == 1.6E10);
                TestHelper.Assert(_do[""][1] == 1.7E10);
                TestHelper.Assert(ro.Count == 3);
                TestHelper.Assert(ro["Hello!!"].Length == 3);
                TestHelper.Assert(ro["Hello!!"][0] == 1.1E10);
                TestHelper.Assert(ro["Hello!!"][1] == 1.2E10);
                TestHelper.Assert(ro["Hello!!"][2] == 1.3E10);
                TestHelper.Assert(ro["Goodbye"].Length == 2);
                TestHelper.Assert(ro["Goodbye"][0] == 1.4E10);
                TestHelper.Assert(ro["Goodbye"][1] == 1.5E10);
                TestHelper.Assert(ro[""].Length == 2);
                TestHelper.Assert(ro[""][0] == 1.6E10);
                TestHelper.Assert(ro[""][1] == 1.7E10);
            }

            {
                var sdi1 = new Dictionary<string, string[]>();
                var sdi2 = new Dictionary<string, string[]>();

                var si1 = new string[] { "abc", "de", "fghi" };
                var si2 = new string[] { "xyz", "or" };
                var si3 = new string[] { "and", "xor" };

                sdi1["abc"] = si1;
                sdi1["def"] = si2;
                sdi2["ghi"] = si3;

                var (ro, _do) = p.OpStringStringSD(sdi1, sdi2);

                TestHelper.Assert(_do.Count == 1);
                TestHelper.Assert(_do["ghi"].Length == 2);
                TestHelper.Assert(_do["ghi"][0].Equals("and"));
                TestHelper.Assert(_do["ghi"][1].Equals("xor"));

                TestHelper.Assert(ro.Count == 3);
                TestHelper.Assert(ro["abc"].Length == 3);
                TestHelper.Assert(ro["abc"][0].Equals("abc"));
                TestHelper.Assert(ro["abc"][1].Equals("de"));
                TestHelper.Assert(ro["abc"][2].Equals("fghi"));
                TestHelper.Assert(ro["def"].Length == 2);
                TestHelper.Assert(ro["def"][0].Equals("xyz"));
                TestHelper.Assert(ro["def"][1].Equals("or"));
                TestHelper.Assert(ro["ghi"].Length == 2);
                TestHelper.Assert(ro["ghi"][0].Equals("and"));
                TestHelper.Assert(ro["ghi"][1].Equals("xor"));
            }

            {
                var sdi1 = new Dictionary<MyEnum, MyEnum[]>();
                var sdi2 = new Dictionary<MyEnum, MyEnum[]>();

                var si1 = new MyEnum[] { MyEnum.enum1, MyEnum.enum1, MyEnum.enum2 };
                var si2 = new MyEnum[] { MyEnum.enum1, MyEnum.enum2 };
                var si3 = new MyEnum[] { MyEnum.enum3, MyEnum.enum3 };

                sdi1[MyEnum.enum3] = si1;
                sdi1[MyEnum.enum2] = si2;
                sdi2[MyEnum.enum1] = si3;

                var (ro, _do) = p.OpMyEnumMyEnumSD(sdi1, sdi2);

                TestHelper.Assert(_do.Count == 1);
                TestHelper.Assert(_do[MyEnum.enum1].Length == 2);
                TestHelper.Assert(_do[MyEnum.enum1][0] == MyEnum.enum3);
                TestHelper.Assert(_do[MyEnum.enum1][1] == MyEnum.enum3);
                TestHelper.Assert(ro.Count == 3);
                TestHelper.Assert(ro[MyEnum.enum3].Length == 3);
                TestHelper.Assert(ro[MyEnum.enum3][0] == MyEnum.enum1);
                TestHelper.Assert(ro[MyEnum.enum3][1] == MyEnum.enum1);
                TestHelper.Assert(ro[MyEnum.enum3][2] == MyEnum.enum2);
                TestHelper.Assert(ro[MyEnum.enum2].Length == 2);
                TestHelper.Assert(ro[MyEnum.enum2][0] == MyEnum.enum1);
                TestHelper.Assert(ro[MyEnum.enum2][1] == MyEnum.enum2);
                TestHelper.Assert(ro[MyEnum.enum1].Length == 2);
                TestHelper.Assert(ro[MyEnum.enum1][0] == MyEnum.enum3);
                TestHelper.Assert(ro[MyEnum.enum1][1] == MyEnum.enum3);
            }

            {
                int[] lengths = new int[] { 0, 1, 2, 126, 127, 128, 129, 253, 254, 255, 256, 257, 1000 };

                for (int l = 0; l < lengths.Length; ++l)
                {
                    int[] s = new int[lengths[l]];
                    for (int i = 0; i < lengths[l]; ++i)
                    {
                        s[i] = i;
                    }

                    int[] r = p.OpIntS(s);
                    TestHelper.Assert(r.Length == lengths[l]);
                    for (int j = 0; j < r.Length; ++j)
                    {
                        TestHelper.Assert(r[j] == -j);
                    }
                }
            }

            {
                var ctx = new SortedDictionary<string, string>();
                ctx["one"] = "ONE";
                ctx["two"] = "TWO";
                ctx["three"] = "THREE";
                {
                    TestHelper.Assert(p.Context.Count == 0);
                    SortedDictionary<string, string> r = p.OpContext();
                    TestHelper.Assert(!r.Equals(ctx));
                }
                {
                    SortedDictionary<string, string> r = p.OpContext(ctx);
                    TestHelper.Assert(r.DictionaryEqual(ctx));
                }
                {
                    var p2 = p.Clone(context: ctx);
                    TestHelper.Assert(p2.Context.DictionaryEqual(ctx));
                    SortedDictionary<string, string> r = p2.OpContext();
                    TestHelper.Assert(r.DictionaryEqual(ctx));
                    r = p2.OpContext(ctx);
                    TestHelper.Assert(r.DictionaryEqual(ctx));
                }
            }

            {
                communicator.CurrentContext["one"] = "ONE";
                communicator.CurrentContext["two"] = "TWO";
                communicator.CurrentContext["three"] = "THREE";

                var p3 = IMyClassPrx.Parse(helper.GetTestProxy("test", 0), communicator);
                TestHelper.Assert(p3.OpContext().DictionaryEqual(communicator.CurrentContext));

                var prxContext = new SortedDictionary<string, string>();
                prxContext["one"] = "UN";
                prxContext["four"] = "QUATRE";

                var combined = new SortedDictionary<string, string>(prxContext);
                foreach (KeyValuePair<string, string> e in communicator.CurrentContext)
                {
                    try
                    {
                        combined.Add(e.Key, e.Value);
                    }
                    catch (ArgumentException)
                    {
                        // Ignore.
                    }
                }
                TestHelper.Assert(combined["one"].Equals("UN"));

                TestHelper.Assert(communicator.DefaultContext.Count == 0);
                communicator.DefaultContext = prxContext;
                TestHelper.Assert(communicator.DefaultContext != prxContext); // it's a copy
                TestHelper.Assert(communicator.DefaultContext.DictionaryEqual(prxContext));

                p3 = IMyClassPrx.Parse(helper.GetTestProxy("test", 0), communicator);
                var ctx = new SortedDictionary<string, string>(communicator.CurrentContext);

                communicator.CurrentContext.Clear();
                TestHelper.Assert(p3.OpContext().DictionaryEqual(prxContext));

                communicator.CurrentContext = ctx;
                TestHelper.Assert(p3.OpContext().DictionaryEqual(combined));

                // Cleanup
                communicator.CurrentContext.Clear();
                communicator.DefaultContext = new SortedDictionary<string, string>();
            }

            p.OpIdempotent();

            try
            {
                p.OpOneway();
                TestHelper.Assert(false);
            }
            catch (SomeException)
            {
                // expected
            }

            // This is invoked as a oneway, despite using a twoway proxy.
            p.OpOnewayMetadata();

            {
                TestHelper.Assert(p.OpByte1(0xFF) == 0xFF);
                TestHelper.Assert(p.OpShort1(0x7FFF) == 0x7FFF);
                TestHelper.Assert(p.OpUShort1(ushort.MaxValue) == ushort.MaxValue);
                TestHelper.Assert(p.OpInt1(0x7FFFFFFF) == 0x7FFFFFFF);
                TestHelper.Assert(p.OpUInt1(uint.MaxValue) == uint.MaxValue);
                TestHelper.Assert(p.OpLong1(0x7FFFFFFFFFFFFFFF) == 0x7FFFFFFFFFFFFFFF);
                TestHelper.Assert(p.OpULong1(ulong.MaxValue) == ulong.MaxValue);
                TestHelper.Assert(p.OpFloat1(1.0f) == 1.0f);
                TestHelper.Assert(p.OpDouble1(1.0d) == 1.0d);
                TestHelper.Assert(p.OpString1("opString1").Equals("opString1"));
                TestHelper.Assert(p.OpStringS1(Array.Empty<string>()).Length == 0);
                TestHelper.Assert(p.OpByteBoolD1(new Dictionary<byte, bool>()).Count == 0);
                TestHelper.Assert(p.OpStringS2(Array.Empty<string>()).Length == 0);
                TestHelper.Assert(p.OpByteBoolD2(new Dictionary<byte, bool>()).Count == 0);

                var d = p.Clone(IMyDerivedClassPrx.Factory);
                var s = new MyStruct1();
                s.TesT = "MyStruct1.s";
                s.MyClass = null;
                s = d.OpMyStruct1(s);
                TestHelper.Assert(s.TesT.Equals("MyStruct1.s"));
                TestHelper.Assert(s.MyClass == null);
                MyClass1? c = new MyClass1("MyClass1.testT", null);
                c = d.OpMyClass1(c);
                TestHelper.Assert(c != null);
                TestHelper.Assert(c.TesT.Equals("MyClass1.testT"));
                TestHelper.Assert(c.MyClass == null);
            }

            {
                var p1 = p.OpMStruct1();
                p1.E = MyEnum.enum3;
                Structure p2, p3;
                (p3, p2) = p.OpMStruct2(p1);
                TestHelper.Assert(p2.Equals(p1) && p3.Equals(p1));
            }

            {
                p.OpMSeq1();

                string[] p1 = new string[1];
                p1[0] = "test";
                string[] p2, p3;
                (p3, p2) = p.OpMSeq2(p1);
                TestHelper.Assert(p2.SequenceEqual(p1) && p3.SequenceEqual(p1));
            }

            {
                p.OpMDict1();

                var p1 = new Dictionary<string, string>();
                p1["test"] = "test";
                Dictionary<string, string> p2, p3;
                (p3, p2) = p.OpMDict2(p1);
                TestHelper.Assert(p2.DictionaryEqual(p1) && p3.DictionaryEqual(p1));
            }
        }
    }
}
