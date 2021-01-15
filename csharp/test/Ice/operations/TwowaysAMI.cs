// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Threading.Tasks;
using ZeroC.Test;

namespace ZeroC.Ice.Test.Operations
{
    public static class TwowaysAMI
    {
        internal static void Run(TestHelper helper, IMyClassPrx p)
        {
            Communicator communicator = helper.Communicator;

            {
                p.IcePingAsync().Wait();
            }

            {
                TestHelper.Assert(p.IceIsAAsync("::ZeroC::Ice::Test::Operations::MyClass").Result);
            }

            {
                TestHelper.Assert(p.IceIdsAsync().Result.Length == 3);
            }

            {
                TestHelper.Assert(p.IceIdAsync().Result.Equals("::ZeroC::Ice::Test::Operations::MyDerivedClass"));
            }

            {
                p.OpVoidAsync().Wait();
            }

            {
                (byte ReturnValue, byte p3) = p.OpByteAsync(0xff, 0x0f).Result;
                TestHelper.Assert(p3 == 0xf0);
                TestHelper.Assert(ReturnValue == 0xff);
            }

            {
                (bool returnValue, bool p3) = p.OpBoolAsync(true, false).Result;
                TestHelper.Assert(p3);
                TestHelper.Assert(!returnValue);
            }

            {
                (long returnValue, short p4, int p5, long p6) = p.OpShortIntLongAsync(10, 11, 12).Result;

                TestHelper.Assert(p4 == 10);
                TestHelper.Assert(p5 == 11);
                TestHelper.Assert(p6 == 12);
                TestHelper.Assert(returnValue == 12);
            }

            {
                (ulong returnValue, ushort p4, uint p5, ulong p6) = p.OpUShortUIntULongAsync(10, 11, 12).Result;
                TestHelper.Assert(p4 == 10);
                TestHelper.Assert(p5 == 11);
                TestHelper.Assert(p6 == 12);
                TestHelper.Assert(returnValue == 12);
            }

            {
                (double returnValue, float p3, double p4) = p.OpFloatDoubleAsync(3.14f, 1.1E10).Result;
                TestHelper.Assert(p3 == 3.14f);
                TestHelper.Assert(p4 == 1.1e10);
                TestHelper.Assert(returnValue == 1.1e10);
            }

            {
                (string returnValue, string p3) = p.OpStringAsync("hello", "world").Result;
                TestHelper.Assert(p3.Equals("world hello"));
                TestHelper.Assert(returnValue.Equals("hello world"));
            }

            {
                (MyEnum returnValue, MyEnum p2) = p.OpMyEnumAsync(MyEnum.enum2).Result;
                TestHelper.Assert(p2 == MyEnum.enum2);
                TestHelper.Assert(returnValue == MyEnum.enum3);
            }

            {
                (IMyClassPrx? returnValue, IMyClassPrx? p2, IMyClassPrx? p3) = p.OpMyClassAsync(p).Result;
                TestHelper.Assert(p2!.Identity.Equals(Identity.Parse("test")));
                TestHelper.Assert(p3!.Identity.Equals(Identity.Parse("noSuchIdentity")));
                TestHelper.Assert(returnValue!.Identity.Equals(Identity.Parse("test")));
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

                (Structure returnValue, Structure p3) = p.OpStructAsync(si1, si2).Result;

                TestHelper.Assert(returnValue.P == null);
                TestHelper.Assert(returnValue.E == MyEnum.enum2);
                TestHelper.Assert(returnValue.S.S.Equals("def"));
                TestHelper.Assert(p3.E == MyEnum.enum3);
                TestHelper.Assert(p3.S.S.Equals("a new string"));
            }

            {
                byte[] bsi1 = new byte[] { 0x01, 0x11, 0x12, 0x22 };
                byte[] bsi2 = new byte[] { 0xf1, 0xf2, 0xf3, 0xf4 };

                (byte[] returnValue, byte[] bso) = p.OpByteSAsync(bsi1, bsi2).Result;

                TestHelper.Assert(bso.Length == 4);
                TestHelper.Assert(bso[0] == 0x22);
                TestHelper.Assert(bso[1] == 0x12);
                TestHelper.Assert(bso[2] == 0x11);
                TestHelper.Assert(bso[3] == 0x01);
                TestHelper.Assert(returnValue.Length == 8);
                TestHelper.Assert(returnValue[0] == 0x01);
                TestHelper.Assert(returnValue[1] == 0x11);
                TestHelper.Assert(returnValue[2] == 0x12);
                TestHelper.Assert(returnValue[3] == 0x22);
                TestHelper.Assert(returnValue[4] == 0xf1);
                TestHelper.Assert(returnValue[5] == 0xf2);
                TestHelper.Assert(returnValue[6] == 0xf3);
                TestHelper.Assert(returnValue[7] == 0xf4);
            }

            {
                bool[] bsi1 = new bool[] { true, true, false };
                bool[] bsi2 = new bool[] { false };

                (bool[] returnValue, bool[] p3) = p.OpBoolSAsync(bsi1, bsi2).Result;

                TestHelper.Assert(p3.Length == 4);
                TestHelper.Assert(p3[0]);
                TestHelper.Assert(p3[1]);
                TestHelper.Assert(!p3[2]);
                TestHelper.Assert(!p3[3]);
                TestHelper.Assert(returnValue.Length == 3);
                TestHelper.Assert(!returnValue[0]);
                TestHelper.Assert(returnValue[1]);
                TestHelper.Assert(returnValue[2]);
            }

            {
                short[] ssi = new short[] { 1, 2, 3 };
                int[] isi = new int[] { 5, 6, 7, 8 };
                long[] lsi = new long[] { 10, 30, 20 };

                (long[] returnValue, short[] p4, int[] p5, long[] p6) = p.OpShortIntLongSAsync(ssi, isi, lsi).Result;

                TestHelper.Assert(p4.Length == 3);
                TestHelper.Assert(p4[0] == 1);
                TestHelper.Assert(p4[1] == 2);
                TestHelper.Assert(p4[2] == 3);
                TestHelper.Assert(p5.Length == 4);
                TestHelper.Assert(p5[0] == 8);
                TestHelper.Assert(p5[1] == 7);
                TestHelper.Assert(p5[2] == 6);
                TestHelper.Assert(p5[3] == 5);
                TestHelper.Assert(p6.Length == 6);
                TestHelper.Assert(p6[0] == 10);
                TestHelper.Assert(p6[1] == 30);
                TestHelper.Assert(p6[2] == 20);
                TestHelper.Assert(p6[3] == 10);
                TestHelper.Assert(p6[4] == 30);
                TestHelper.Assert(p6[5] == 20);
                TestHelper.Assert(returnValue.Length == 3);
                TestHelper.Assert(returnValue[0] == 10);
                TestHelper.Assert(returnValue[1] == 30);
                TestHelper.Assert(returnValue[2] == 20);
            }

            {
                float[] fsi = new float[] { 3.14f, 1.11f };
                double[] dsi = new double[] { 1.1e10, 1.2e10, 1.3e10 };

                (double[] returnValue, float[] p3, double[] p4) = p.OpFloatDoubleSAsync(fsi, dsi).Result;
                TestHelper.Assert(p3.Length == 2);
                TestHelper.Assert(p3[0] == 3.14f);
                TestHelper.Assert(p3[1] == 1.11f);
                TestHelper.Assert(p4.Length == 3);
                TestHelper.Assert(p4[0] == 1.3e10);
                TestHelper.Assert(p4[1] == 1.2e10);
                TestHelper.Assert(p4[2] == 1.1e10);
                TestHelper.Assert(returnValue.Length == 5);
                TestHelper.Assert(returnValue[0] == 1.1e10);
                TestHelper.Assert(returnValue[1] == 1.2e10);
                TestHelper.Assert(returnValue[2] == 1.3e10);
                TestHelper.Assert((float)returnValue[3] == 3.14f);
                TestHelper.Assert((float)returnValue[4] == 1.11f);
            }

            {
                string[] ssi1 = new string[] { "abc", "de", "fghi" };
                string[] ssi2 = new string[] { "xyz" };

                (string[] returnValue, string[] p3) = p.OpStringSAsync(ssi1, ssi2).Result;
                TestHelper.Assert(p3.Length == 4);
                TestHelper.Assert(p3[0].Equals("abc"));
                TestHelper.Assert(p3[1].Equals("de"));
                TestHelper.Assert(p3[2].Equals("fghi"));
                TestHelper.Assert(p3[3].Equals("xyz"));
                TestHelper.Assert(returnValue.Length == 3);
                TestHelper.Assert(returnValue[0].Equals("fghi"));
                TestHelper.Assert(returnValue[1].Equals("de"));
                TestHelper.Assert(returnValue[2].Equals("abc"));
            }

            {
                byte[] s11 = new byte[] { 0x01, 0x11, 0x12 };
                byte[] s12 = new byte[] { 0xff };
                byte[][] bsi1 = new byte[][] { s11, s12 };

                byte[] s21 = new byte[] { 0x0e };
                byte[] s22 = new byte[] { 0xf2, 0xf1 };
                byte[][] bsi2 = new byte[][] { s21, s22 };

                (byte[][] returnValue, byte[][] p3) = p.OpByteSSAsync(bsi1, bsi2).Result;
                TestHelper.Assert(p3.Length == 2);
                TestHelper.Assert(p3[0].Length == 1);
                TestHelper.Assert(p3[0][0] == 0xff);
                TestHelper.Assert(p3[1].Length == 3);
                TestHelper.Assert(p3[1][0] == 0x01);
                TestHelper.Assert(p3[1][1] == 0x11);
                TestHelper.Assert(p3[1][2] == 0x12);
                TestHelper.Assert(returnValue.Length == 4);
                TestHelper.Assert(returnValue[0].Length == 3);
                TestHelper.Assert(returnValue[0][0] == 0x01);
                TestHelper.Assert(returnValue[0][1] == 0x11);
                TestHelper.Assert(returnValue[0][2] == 0x12);
                TestHelper.Assert(returnValue[1].Length == 1);
                TestHelper.Assert(returnValue[1][0] == 0xff);
                TestHelper.Assert(returnValue[2].Length == 1);
                TestHelper.Assert(returnValue[2][0] == 0x0e);
                TestHelper.Assert(returnValue[3].Length == 2);
                TestHelper.Assert(returnValue[3][0] == 0xf2);
                TestHelper.Assert(returnValue[3][1] == 0xf1);
            }

            {
                bool[] s11 = new bool[] { true };
                bool[] s12 = new bool[] { false };
                bool[] s13 = new bool[] { true, true };
                bool[][] bsi1 = new bool[][] { s11, s12, s13 };

                bool[] s21 = new bool[] { false, false, true };
                bool[][] bsi2 = new bool[][] { s21 };

                (bool[][] returnValue, bool[][] p3) = p.OpBoolSSAsync(bsi1, bsi2).Result;

                TestHelper.Assert(p3.Length == 4);
                TestHelper.Assert(p3[0].Length == 1);
                TestHelper.Assert(p3[0][0]);
                TestHelper.Assert(p3[1].Length == 1);
                TestHelper.Assert(!p3[1][0]);
                TestHelper.Assert(p3[2].Length == 2);
                TestHelper.Assert(p3[2][0]);
                TestHelper.Assert(p3[2][1]);
                TestHelper.Assert(p3[3].Length == 3);
                TestHelper.Assert(!p3[3][0]);
                TestHelper.Assert(!p3[3][1]);
                TestHelper.Assert(p3[3][2]);
                TestHelper.Assert(returnValue.Length == 3);
                TestHelper.Assert(returnValue[0].Length == 2);
                TestHelper.Assert(returnValue[0][0]);
                TestHelper.Assert(returnValue[0][1]);
                TestHelper.Assert(returnValue[1].Length == 1);
                TestHelper.Assert(!returnValue[1][0]);
                TestHelper.Assert(returnValue[2].Length == 1);
                TestHelper.Assert(returnValue[2][0]);
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

                (long[][] returnValue, short[][] p4, int[][] p5, long[][] p6) =
                    p.OpShortIntLongSSAsync(ssi, isi, lsi).Result;

                TestHelper.Assert(returnValue.Length == 1);
                TestHelper.Assert(returnValue[0].Length == 2);
                TestHelper.Assert(returnValue[0][0] == 496);
                TestHelper.Assert(returnValue[0][1] == 1729);
                TestHelper.Assert(p4.Length == 3);
                TestHelper.Assert(p4[0].Length == 3);
                TestHelper.Assert(p4[0][0] == 1);
                TestHelper.Assert(p4[0][1] == 2);
                TestHelper.Assert(p4[0][2] == 5);
                TestHelper.Assert(p4[1].Length == 1);
                TestHelper.Assert(p4[1][0] == 13);
                TestHelper.Assert(p4[2].Length == 0);
                TestHelper.Assert(p5.Length == 2);
                TestHelper.Assert(p5[0].Length == 1);
                TestHelper.Assert(p5[0][0] == 42);
                TestHelper.Assert(p5[1].Length == 2);
                TestHelper.Assert(p5[1][0] == 24);
                TestHelper.Assert(p5[1][1] == 98);
                TestHelper.Assert(p6.Length == 2);
                TestHelper.Assert(p6[0].Length == 2);
                TestHelper.Assert(p6[0][0] == 496);
                TestHelper.Assert(p6[0][1] == 1729);
                TestHelper.Assert(p6[1].Length == 2);
                TestHelper.Assert(p6[1][0] == 496);
                TestHelper.Assert(p6[1][1] == 1729);
            }

            {
                float[] f11 = new float[] { 3.14f };
                float[] f12 = new float[] { 1.11f };
                float[] f13 = Array.Empty<float>();
                float[][] fsi = new float[][] { f11, f12, f13 };

                double[] d11 = new double[] { 1.1e10, 1.2e10, 1.3e10 };
                double[][] dsi = new double[][] { d11 };

                (double[][] returnValue, float[][] p3, double[][] p4) = p.OpFloatDoubleSSAsync(fsi, dsi).Result;

                TestHelper.Assert(p3.Length == 3);
                TestHelper.Assert(p3[0].Length == 1);
                TestHelper.Assert(p3[0][0] == 3.14f);
                TestHelper.Assert(p3[1].Length == 1);
                TestHelper.Assert(p3[1][0] == 1.11f);
                TestHelper.Assert(p3[2].Length == 0);
                TestHelper.Assert(p4.Length == 1);
                TestHelper.Assert(p4[0].Length == 3);
                TestHelper.Assert(p4[0][0] == 1.1e10);
                TestHelper.Assert(p4[0][1] == 1.2e10);
                TestHelper.Assert(p4[0][2] == 1.3e10);
                TestHelper.Assert(returnValue.Length == 2);
                TestHelper.Assert(returnValue[0].Length == 3);
                TestHelper.Assert(returnValue[0][0] == 1.1e10);
                TestHelper.Assert(returnValue[0][1] == 1.2e10);
                TestHelper.Assert(returnValue[0][2] == 1.3e10);
                TestHelper.Assert(returnValue[1].Length == 3);
                TestHelper.Assert(returnValue[1][0] == 1.1e10);
                TestHelper.Assert(returnValue[1][1] == 1.2e10);
                TestHelper.Assert(returnValue[1][2] == 1.3e10);
            }

            {
                string[] s11 = new string[] { "abc" };
                string[] s12 = new string[] { "de", "fghi" };
                string[][] ssi1 = new string[][] { s11, s12 };

                string[] s21 = Array.Empty<string>();
                string[] s22 = Array.Empty<string>();
                string[] s23 = new string[] { "xyz" };
                string[][] ssi2 = new string[][] { s21, s22, s23 };

                (string[][] returnValue, string[][] p3) = p.OpStringSSAsync(ssi1, ssi2).Result;

                TestHelper.Assert(p3.Length == 5);
                TestHelper.Assert(p3[0].Length == 1);
                TestHelper.Assert(p3[0][0].Equals("abc"));
                TestHelper.Assert(p3[1].Length == 2);
                TestHelper.Assert(p3[1][0].Equals("de"));
                TestHelper.Assert(p3[1][1].Equals("fghi"));
                TestHelper.Assert(p3[2].Length == 0);
                TestHelper.Assert(p3[3].Length == 0);
                TestHelper.Assert(p3[4].Length == 1);
                TestHelper.Assert(p3[4][0].Equals("xyz"));
                TestHelper.Assert(returnValue.Length == 3);
                TestHelper.Assert(returnValue[0].Length == 1);
                TestHelper.Assert(returnValue[0][0].Equals("xyz"));
                TestHelper.Assert(returnValue[1].Length == 0);
                TestHelper.Assert(returnValue[2].Length == 0);
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

                (string[][][] returnValue, string[][][] p3) = p.OpStringSSSAsync(sssi1, sssi2).Result;

                TestHelper.Assert(p3.Length == 5);
                TestHelper.Assert(p3[0].Length == 2);
                TestHelper.Assert(p3[0][0].Length == 2);
                TestHelper.Assert(p3[0][1].Length == 1);
                TestHelper.Assert(p3[1].Length == 1);
                TestHelper.Assert(p3[1][0].Length == 1);
                TestHelper.Assert(p3[2].Length == 2);
                TestHelper.Assert(p3[2][0].Length == 2);
                TestHelper.Assert(p3[2][1].Length == 1);
                TestHelper.Assert(p3[3].Length == 1);
                TestHelper.Assert(p3[3][0].Length == 1);
                TestHelper.Assert(p3[4].Length == 0);
                TestHelper.Assert(p3[0][0][0].Equals("abc"));
                TestHelper.Assert(p3[0][0][1].Equals("de"));
                TestHelper.Assert(p3[0][1][0].Equals("xyz"));
                TestHelper.Assert(p3[1][0][0].Equals("hello"));
                TestHelper.Assert(p3[2][0][0].Length == 0);
                TestHelper.Assert(p3[2][0][1].Length == 0);
                TestHelper.Assert(p3[2][1][0].Equals("abcd"));
                TestHelper.Assert(p3[3][0][0].Length == 0);

                TestHelper.Assert(returnValue.Length == 3);
                TestHelper.Assert(returnValue[0].Length == 0);
                TestHelper.Assert(returnValue[1].Length == 1);
                TestHelper.Assert(returnValue[1][0].Length == 1);
                TestHelper.Assert(returnValue[2].Length == 2);
                TestHelper.Assert(returnValue[2][0].Length == 2);
                TestHelper.Assert(returnValue[2][1].Length == 1);
                TestHelper.Assert(returnValue[1][0][0].Length == 0);
                TestHelper.Assert(returnValue[2][0][0].Length == 0);
                TestHelper.Assert(returnValue[2][0][1].Length == 0);
                TestHelper.Assert(returnValue[2][1][0].Equals("abcd"));
            }

            {
                var di1 = new Dictionary<byte, bool>
                {
                    [10] = true,
                    [100] = false
                };

                var di2 = new Dictionary<byte, bool>
                {
                    [10] = true,
                    [11] = false,
                    [101] = true
                };

                (Dictionary<byte, bool> returnValue, Dictionary<byte, bool> p3) = p.OpByteBoolDAsync(di1, di2).Result;

                TestHelper.Assert(p3.DictionaryEqual(di1));
                TestHelper.Assert(returnValue.Count == 4);
                TestHelper.Assert(returnValue[10] == true);
                TestHelper.Assert(returnValue[11] == false);
                TestHelper.Assert(returnValue[100] == false);
                TestHelper.Assert(returnValue[101] == true);
            }

            {
                var di1 = new Dictionary<short, int>
                {
                    [110] = -1,
                    [1100] = 123123
                };

                var di2 = new Dictionary<short, int>
                {
                    [110] = -1,
                    [111] = -100,
                    [1101] = 0
                };

                (Dictionary<short, int> returnValue, Dictionary<short, int> p3) = p.OpShortIntDAsync(di1, di2).Result;

                TestHelper.Assert(p3.DictionaryEqual(di1));
                TestHelper.Assert(returnValue.Count == 4);
                TestHelper.Assert(returnValue[110] == -1);
                TestHelper.Assert(returnValue[111] == -100);
                TestHelper.Assert(returnValue[1100] == 123123);
                TestHelper.Assert(returnValue[1101] == 0);
            }

            {
                var di1 = new Dictionary<long, float>
                {
                    [999999110L] = -1.1f,
                    [999999111L] = 123123.2f
                };

                var di2 = new Dictionary<long, float>
                {
                    [999999110L] = -1.1f,
                    [999999120L] = -100.4f,
                    [999999130L] = 0.5f
                };

                (Dictionary<long, float> returnValue, Dictionary<long, float> p3) =
                    p.OpLongFloatDAsync(di1, di2).Result;

                TestHelper.Assert(p3.DictionaryEqual(di1));
                TestHelper.Assert(returnValue.Count == 4);
                TestHelper.Assert(returnValue[999999110L] == -1.1f);
                TestHelper.Assert(returnValue[999999120L] == -100.4f);
                TestHelper.Assert(returnValue[999999111L] == 123123.2f);
                TestHelper.Assert(returnValue[999999130L] == 0.5f);
            }

            {
                var di1 = new Dictionary<string, string>
                {
                    ["foo"] = "abc -1.1",
                    ["bar"] = "abc 123123.2"
                };

                var di2 = new Dictionary<string, string>
                {
                    ["foo"] = "abc -1.1",
                    ["FOO"] = "abc -100.4",
                    ["BAR"] = "abc 0.5"
                };

                (Dictionary<string, string> returnValue, Dictionary<string, string> p3) =
                    p.OpStringStringDAsync(di1, di2).Result;

                TestHelper.Assert(p3.DictionaryEqual(di1));
                TestHelper.Assert(returnValue.Count == 4);
                TestHelper.Assert(returnValue["foo"].Equals("abc -1.1"));
                TestHelper.Assert(returnValue["FOO"].Equals("abc -100.4"));
                TestHelper.Assert(returnValue["bar"].Equals("abc 123123.2"));
                TestHelper.Assert(returnValue["BAR"].Equals("abc 0.5"));
            }

            {
                var di1 = new Dictionary<string, MyEnum>
                {
                    ["abc"] = MyEnum.enum1,
                    [""] = MyEnum.enum2
                };

                var di2 = new Dictionary<string, MyEnum>
                {
                    ["abc"] = MyEnum.enum1,
                    ["qwerty"] = MyEnum.enum3,
                    ["Hello!!"] = MyEnum.enum2
                };

                (Dictionary<string, MyEnum> returnValue, Dictionary<string, MyEnum> p3) =
                    p.OpStringMyEnumDAsync(di1, di2).Result;

                TestHelper.Assert(p3.DictionaryEqual(di1));
                TestHelper.Assert(returnValue.Count == 4);
                TestHelper.Assert(returnValue["abc"] == MyEnum.enum1);
                TestHelper.Assert(returnValue["qwerty"] == MyEnum.enum3);
                TestHelper.Assert(returnValue[""] == MyEnum.enum2);
                TestHelper.Assert(returnValue["Hello!!"] == MyEnum.enum2);
            }

            {
                var di1 = new Dictionary<MyEnum, string>
                {
                    [MyEnum.enum1] = "abc"
                };

                var di2 = new Dictionary<MyEnum, string>
                {
                    [MyEnum.enum2] = "Hello!!",
                    [MyEnum.enum3] = "qwerty"
                };

                (Dictionary<MyEnum, string> returnValue, Dictionary<MyEnum, string> p3) =
                    p.OpMyEnumStringDAsync(di1, di2).Result;

                di1[MyEnum.enum1] = "abc";
                TestHelper.Assert(p3.DictionaryEqual(di1));
                TestHelper.Assert(returnValue.Count == 3);
                TestHelper.Assert(returnValue[MyEnum.enum1].Equals("abc"));
                TestHelper.Assert(returnValue[MyEnum.enum2].Equals("Hello!!"));
                TestHelper.Assert(returnValue[MyEnum.enum3].Equals("qwerty"));
            }

            {
                var s11 = new MyStruct(1, 1);
                var s12 = new MyStruct(1, 2);
                var di1 = new Dictionary<MyStruct, MyEnum>
                {
                    [s11] = MyEnum.enum1,
                    [s12] = MyEnum.enum2
                };

                var s22 = new MyStruct(2, 2);
                var s23 = new MyStruct(2, 3);
                var di2 = new Dictionary<MyStruct, MyEnum>
                {
                    [s11] = MyEnum.enum1,
                    [s22] = MyEnum.enum3,
                    [s23] = MyEnum.enum2
                };

                (Dictionary<MyStruct, MyEnum> returnValue, Dictionary<MyStruct, MyEnum> p3) =
                    p.OpMyStructMyEnumDAsync(di1, di2).Result;

                TestHelper.Assert(p3.DictionaryEqual(di1));
                TestHelper.Assert(returnValue.Count == 4);
                TestHelper.Assert(returnValue[s11] == MyEnum.enum1);
                TestHelper.Assert(returnValue[s12] == MyEnum.enum2);
                TestHelper.Assert(returnValue[s22] == MyEnum.enum3);
                TestHelper.Assert(returnValue[s23] == MyEnum.enum2);
            }

            {
                var dsi1 = new Dictionary<byte, bool>[2];
                var dsi2 = new Dictionary<byte, bool>[1];

                var di1 = new Dictionary<byte, bool>
                {
                    [10] = true,
                    [100] = false
                };

                var di2 = new Dictionary<byte, bool>
                {
                    [10] = true,
                    [11] = false,
                    [101] = true
                };

                var di3 = new Dictionary<byte, bool>
                {
                    [100] = false,
                    [101] = false
                };

                dsi1[0] = di1;
                dsi1[1] = di2;
                dsi2[0] = di3;

                (Dictionary<byte, bool>[] returnValue, Dictionary<byte, bool>[] p3) =
                    p.OpByteBoolDSAsync(dsi1, dsi2).Result;

                TestHelper.Assert(returnValue.Length == 2);
                TestHelper.Assert(returnValue[0].Count == 3);
                TestHelper.Assert(returnValue[0][10]);
                TestHelper.Assert(!returnValue[0][11]);
                TestHelper.Assert(returnValue[0][101]);
                TestHelper.Assert(returnValue[1].Count == 2);
                TestHelper.Assert(returnValue[1][10]);
                TestHelper.Assert(!returnValue[1][100]);

                TestHelper.Assert(p3.Length == 3);
                TestHelper.Assert(p3[0].Count == 2);
                TestHelper.Assert(!p3[0][100]);
                TestHelper.Assert(!p3[0][101]);
                TestHelper.Assert(p3[1].Count == 2);
                TestHelper.Assert(p3[1][10]);
                TestHelper.Assert(!p3[1][100]);
                TestHelper.Assert(p3[2].Count == 3);
                TestHelper.Assert(p3[2][10]);
                TestHelper.Assert(!p3[2][11]);
                TestHelper.Assert(p3[2][101]);
            }

            {
                var dsi1 = new Dictionary<long, float>[2];
                var dsi2 = new Dictionary<long, float>[1];

                var di1 = new Dictionary<long, float>
                {
                    [999999110L] = -1.1f,
                    [999999111L] = 123123.2f
                };

                var di2 = new Dictionary<long, float>
                {
                    [999999110L] = -1.1f,
                    [999999120L] = -100.4f,
                    [999999130L] = 0.5f
                };

                var di3 = new Dictionary<long, float>
                {
                    [999999140L] = 3.14f
                };

                dsi1[0] = di1;
                dsi1[1] = di2;
                dsi2[0] = di3;

                (Dictionary<long, float>[] returnValue, Dictionary<long, float>[] p3) =
                    p.OpLongFloatDSAsync(dsi1, dsi2).Result;
                TestHelper.Assert(returnValue.Length == 2);
                TestHelper.Assert(returnValue[0].Count == 3);
                TestHelper.Assert(returnValue[0][999999110L] == -1.1f);
                TestHelper.Assert(returnValue[0][999999120L] == -100.4f);
                TestHelper.Assert(returnValue[0][999999130L] == 0.5f);
                TestHelper.Assert(returnValue[1].Count == 2);
                TestHelper.Assert(returnValue[1][999999110L] == -1.1f);
                TestHelper.Assert(returnValue[1][999999111L] == 123123.2f);

                TestHelper.Assert(p3.Length == 3);
                TestHelper.Assert(p3[0].Count == 1);
                TestHelper.Assert(p3[0][999999140L] == 3.14f);
                TestHelper.Assert(p3[1].Count == 2);
                TestHelper.Assert(p3[1][999999110L] == -1.1f);
                TestHelper.Assert(p3[1][999999111L] == 123123.2f);
                TestHelper.Assert(p3[2].Count == 3);
                TestHelper.Assert(p3[2][999999110L] == -1.1f);
                TestHelper.Assert(p3[2][999999120L] == -100.4f);
                TestHelper.Assert(p3[2][999999130L] == 0.5f);
            }

            {
                var dsi1 = new Dictionary<string, string>[2];
                var dsi2 = new Dictionary<string, string>[1];

                var di1 = new Dictionary<string, string>
                {
                    ["foo"] = "abc -1.1",
                    ["bar"] = "abc 123123.2"
                };
                var di2 = new Dictionary<string, string>
                {
                    ["foo"] = "abc -1.1",
                    ["FOO"] = "abc -100.4",
                    ["BAR"] = "abc 0.5"
                };
                var di3 = new Dictionary<string, string>
                {
                    ["f00"] = "ABC -3.14"
                };

                dsi1[0] = di1;
                dsi1[1] = di2;
                dsi2[0] = di3;

                (Dictionary<string, string>[] returnValue, Dictionary<string, string>[] p3) =
                    p.OpStringStringDSAsync(dsi1, dsi2).Result;
                TestHelper.Assert(returnValue.Length == 2);
                TestHelper.Assert(returnValue[0].Count == 3);
                TestHelper.Assert(returnValue[0]["foo"].Equals("abc -1.1"));
                TestHelper.Assert(returnValue[0]["FOO"].Equals("abc -100.4"));
                TestHelper.Assert(returnValue[0]["BAR"].Equals("abc 0.5"));
                TestHelper.Assert(returnValue[1].Count == 2);
                TestHelper.Assert(returnValue[1]["foo"] == "abc -1.1");
                TestHelper.Assert(returnValue[1]["bar"] == "abc 123123.2");

                TestHelper.Assert(p3.Length == 3);
                TestHelper.Assert(p3[0].Count == 1);
                TestHelper.Assert(p3[0]["f00"].Equals("ABC -3.14"));
                TestHelper.Assert(p3[1].Count == 2);
                TestHelper.Assert(p3[1]["foo"].Equals("abc -1.1"));
                TestHelper.Assert(p3[1]["bar"].Equals("abc 123123.2"));
                TestHelper.Assert(p3[2].Count == 3);
                TestHelper.Assert(p3[2]["foo"].Equals("abc -1.1"));
                TestHelper.Assert(p3[2]["FOO"].Equals("abc -100.4"));
                TestHelper.Assert(p3[2]["BAR"].Equals("abc 0.5"));
            }

            {
                var dsi1 = new Dictionary<string, MyEnum>[2];
                var dsi2 = new Dictionary<string, MyEnum>[1];

                var di1 = new Dictionary<string, MyEnum>
                {
                    ["abc"] = MyEnum.enum1,
                    [""] = MyEnum.enum2
                };

                var di2 = new Dictionary<string, MyEnum>
                {
                    ["abc"] = MyEnum.enum1,
                    ["qwerty"] = MyEnum.enum3,
                    ["Hello!!"] = MyEnum.enum2
                };

                var di3 = new Dictionary<string, MyEnum>
                {
                    ["Goodbye"] = MyEnum.enum1
                };

                dsi1[0] = di1;
                dsi1[1] = di2;
                dsi2[0] = di3;

                (Dictionary<string, MyEnum>[] returnValue, Dictionary<string, MyEnum>[] p3) =
                    p.OpStringMyEnumDSAsync(dsi1, dsi2).Result;
                TestHelper.Assert(returnValue.Length == 2);
                TestHelper.Assert(returnValue[0].Count == 3);
                TestHelper.Assert(returnValue[0]["abc"] == MyEnum.enum1);
                TestHelper.Assert(returnValue[0]["qwerty"] == MyEnum.enum3);
                TestHelper.Assert(returnValue[0]["Hello!!"] == MyEnum.enum2);
                TestHelper.Assert(returnValue[1].Count == 2);
                TestHelper.Assert(returnValue[1]["abc"] == MyEnum.enum1);
                TestHelper.Assert(returnValue[1][""] == MyEnum.enum2);

                TestHelper.Assert(p3.Length == 3);
                TestHelper.Assert(p3[0].Count == 1);
                TestHelper.Assert(p3[0]["Goodbye"] == MyEnum.enum1);
                TestHelper.Assert(p3[1].Count == 2);
                TestHelper.Assert(p3[1]["abc"] == MyEnum.enum1);
                TestHelper.Assert(p3[1][""] == MyEnum.enum2);
                TestHelper.Assert(p3[2].Count == 3);
                TestHelper.Assert(p3[2]["abc"] == MyEnum.enum1);
                TestHelper.Assert(p3[2]["qwerty"] == MyEnum.enum3);
                TestHelper.Assert(p3[2]["Hello!!"] == MyEnum.enum2);
            }

            {
                var dsi1 = new Dictionary<MyEnum, string>[2];
                var dsi2 = new Dictionary<MyEnum, string>[1];

                var di1 = new Dictionary<MyEnum, string>
                {
                    [MyEnum.enum1] = "abc"
                };
                var di2 = new Dictionary<MyEnum, string>
                {
                    [MyEnum.enum2] = "Hello!!",
                    [MyEnum.enum3] = "qwerty"
                };
                var di3 = new Dictionary<MyEnum, string>
                {
                    [MyEnum.enum1] = "Goodbye"
                };

                dsi1[0] = di1;
                dsi1[1] = di2;
                dsi2[0] = di3;

                (Dictionary<MyEnum, string>[] returnValue, Dictionary<MyEnum, string>[] p3) =
                    p.OpMyEnumStringDSAsync(dsi1, dsi2).Result;

                TestHelper.Assert(returnValue.Length == 2);
                TestHelper.Assert(returnValue[0].Count == 2);
                TestHelper.Assert(returnValue[0][MyEnum.enum2].Equals("Hello!!"));
                TestHelper.Assert(returnValue[0][MyEnum.enum3].Equals("qwerty"));
                TestHelper.Assert(returnValue[1].Count == 1);
                TestHelper.Assert(returnValue[1][MyEnum.enum1].Equals("abc"));

                TestHelper.Assert(p3.Length == 3);
                TestHelper.Assert(p3[0].Count == 1);
                TestHelper.Assert(p3[0][MyEnum.enum1].Equals("Goodbye"));
                TestHelper.Assert(p3[1].Count == 1);
                TestHelper.Assert(p3[1][MyEnum.enum1].Equals("abc"));
                TestHelper.Assert(p3[2].Count == 2);
                TestHelper.Assert(p3[2][MyEnum.enum2].Equals("Hello!!"));
                TestHelper.Assert(p3[2][MyEnum.enum3].Equals("qwerty"));
            }

            {
                var dsi1 = new Dictionary<MyStruct, MyEnum>[2];
                var dsi2 = new Dictionary<MyStruct, MyEnum>[1];

                var s11 = new MyStruct(1, 1);
                var s12 = new MyStruct(1, 2);
                var di1 = new Dictionary<MyStruct, MyEnum>
                {
                    [s11] = MyEnum.enum1,
                    [s12] = MyEnum.enum2
                };

                var s22 = new MyStruct(2, 2);
                var s23 = new MyStruct(2, 3);
                var di2 = new Dictionary<MyStruct, MyEnum>
                {
                    [s11] = MyEnum.enum1,
                    [s22] = MyEnum.enum3,
                    [s23] = MyEnum.enum2
                };

                var di3 = new Dictionary<MyStruct, MyEnum>
                {
                    [s23] = MyEnum.enum3
                };

                dsi1[0] = di1;
                dsi1[1] = di2;
                dsi2[0] = di3;

                (Dictionary<MyStruct, MyEnum>[] returnValue, Dictionary<MyStruct, MyEnum>[] p3) =
                    p.OpMyStructMyEnumDSAsync(dsi1, dsi2).Result;

                TestHelper.Assert(returnValue.Length == 2);
                TestHelper.Assert(returnValue[0].Count == 3);
                TestHelper.Assert(returnValue[0][s11] == MyEnum.enum1);
                TestHelper.Assert(returnValue[0][s22] == MyEnum.enum3);
                TestHelper.Assert(returnValue[0][s23] == MyEnum.enum2);
                TestHelper.Assert(returnValue[1].Count == 2);
                TestHelper.Assert(returnValue[1][s11] == MyEnum.enum1);
                TestHelper.Assert(returnValue[1][s12] == MyEnum.enum2);

                TestHelper.Assert(p3.Length == 3);
                TestHelper.Assert(p3[0].Count == 1);
                TestHelper.Assert(p3[0][s23] == MyEnum.enum3);
                TestHelper.Assert(p3[1].Count == 2);
                TestHelper.Assert(p3[1][s11] == MyEnum.enum1);
                TestHelper.Assert(p3[1][s12] == MyEnum.enum2);
                TestHelper.Assert(p3[2].Count == 3);
                TestHelper.Assert(p3[2][s11] == MyEnum.enum1);
                TestHelper.Assert(p3[2][s22] == MyEnum.enum3);
                TestHelper.Assert(p3[2][s23] == MyEnum.enum2);
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

                (Dictionary<byte, byte[]> returnValue, Dictionary<byte, byte[]> p3) =
                    p.OpByteByteSDAsync(sdi1, sdi2).Result;
                TestHelper.Assert(p3.Count == 1);
                TestHelper.Assert(p3[0xf1].Length == 2);
                TestHelper.Assert(p3[0xf1][0] == 0xf2);
                TestHelper.Assert(p3[0xf1][1] == 0xf3);

                TestHelper.Assert(returnValue.Count == 3);
                TestHelper.Assert(returnValue[0x01].Length == 2);
                TestHelper.Assert(returnValue[0x01][0] == 0x01);
                TestHelper.Assert(returnValue[0x01][1] == 0x11);
                TestHelper.Assert(returnValue[0x22].Length == 1);
                TestHelper.Assert(returnValue[0x22][0] == 0x12);
                TestHelper.Assert(returnValue[0xf1].Length == 2);
                TestHelper.Assert(returnValue[0xf1][0] == 0xf2);
                TestHelper.Assert(returnValue[0xf1][1] == 0xf3);
            }

            {
                var sdi1 = new Dictionary<bool, bool[]>();
                var sdi2 = new Dictionary<bool, bool[]>();

                bool[] si1 = new bool[] { true, false };
                bool[] si2 = new bool[] { false, true, true };

                sdi1[false] = si1;
                sdi1[true] = si2;
                sdi2[false] = si1;

                (Dictionary<bool, bool[]> returnValue, Dictionary<bool, bool[]> p3) =
                    p.OpBoolBoolSDAsync(sdi1, sdi2).Result;
                TestHelper.Assert(p3.Count == 1);
                TestHelper.Assert(p3[false].Length == 2);
                TestHelper.Assert(p3[false][0]);
                TestHelper.Assert(!p3[false][1]);
                TestHelper.Assert(returnValue.Count == 2);
                TestHelper.Assert(returnValue[false].Length == 2);
                TestHelper.Assert(returnValue[false][0]);
                TestHelper.Assert(!returnValue[false][1]);
                TestHelper.Assert(returnValue[true].Length == 3);
                TestHelper.Assert(!returnValue[true][0]);
                TestHelper.Assert(returnValue[true][1]);
                TestHelper.Assert(returnValue[true][2]);
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

                (Dictionary<short, short[]> returnValue, Dictionary<short, short[]> p3) =
                    p.OpShortShortSDAsync(sdi1, sdi2).Result;

                TestHelper.Assert(p3.Count == 1);
                TestHelper.Assert(p3[4].Length == 2);
                TestHelper.Assert(p3[4][0] == 6);
                TestHelper.Assert(p3[4][1] == 7);

                TestHelper.Assert(returnValue.Count == 3);
                TestHelper.Assert(returnValue[1].Length == 3);
                TestHelper.Assert(returnValue[1][0] == 1);
                TestHelper.Assert(returnValue[1][1] == 2);
                TestHelper.Assert(returnValue[1][2] == 3);
                TestHelper.Assert(returnValue[2].Length == 2);
                TestHelper.Assert(returnValue[2][0] == 4);
                TestHelper.Assert(returnValue[2][1] == 5);
                TestHelper.Assert(returnValue[4].Length == 2);
                TestHelper.Assert(returnValue[4][0] == 6);
                TestHelper.Assert(returnValue[4][1] == 7);
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

                (Dictionary<int, int[]> ro, Dictionary<int, int[]> p3) = p.OpIntIntSDAsync(sdi1, sdi2).Result;

                TestHelper.Assert(p3.Count == 1);
                TestHelper.Assert(p3[400].Length == 2);
                TestHelper.Assert(p3[400][0] == 600);
                TestHelper.Assert(p3[400][1] == 700);

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

                long[] si1 = new long[] { 999999110L, 999999111L, 999999110L };
                long[] si2 = new long[] { 999999120L, 999999130L };
                long[] si3 = new long[] { 999999110L, 999999120L };

                sdi1[999999990L] = si1;
                sdi1[999999991L] = si2;
                sdi2[999999992L] = si3;

                (Dictionary<long, long[]> returnValue, Dictionary<long, long[]> p3) =
                    p.OpLongLongSDAsync(sdi1, sdi2).Result;

                TestHelper.Assert(p3.Count == 1);
                TestHelper.Assert(p3[999999992L].Length == 2);
                TestHelper.Assert(p3[999999992L][0] == 999999110L);
                TestHelper.Assert(p3[999999992L][1] == 999999120L);
                TestHelper.Assert(returnValue.Count == 3);
                TestHelper.Assert(returnValue[999999990L].Length == 3);
                TestHelper.Assert(returnValue[999999990L][0] == 999999110L);
                TestHelper.Assert(returnValue[999999990L][1] == 999999111L);
                TestHelper.Assert(returnValue[999999990L][2] == 999999110L);
                TestHelper.Assert(returnValue[999999991L].Length == 2);
                TestHelper.Assert(returnValue[999999991L][0] == 999999120L);
                TestHelper.Assert(returnValue[999999991L][1] == 999999130L);
                TestHelper.Assert(returnValue[999999992L].Length == 2);
                TestHelper.Assert(returnValue[999999992L][0] == 999999110L);
                TestHelper.Assert(returnValue[999999992L][1] == 999999120L);
            }

            {
                var sdi1 = new Dictionary<string, float[]>();
                var sdi2 = new Dictionary<string, float[]>();

                float[] si1 = new float[] { -1.1f, 123123.2f, 100.0f };
                float[] si2 = new float[] { 42.24f, -1.61f };
                float[] si3 = new float[] { -3.14f, 3.14f };

                sdi1["abc"] = si1;
                sdi1["ABC"] = si2;
                sdi2["aBc"] = si3;

                (Dictionary<string, float[]> returnValue, Dictionary<string, float[]> p3) =
                    p.OpStringFloatSDAsync(sdi1, sdi2).Result;
                TestHelper.Assert(p3.Count == 1);
                TestHelper.Assert(p3["aBc"].Length == 2);
                TestHelper.Assert(p3["aBc"][0] == -3.14f);
                TestHelper.Assert(p3["aBc"][1] == 3.14f);

                TestHelper.Assert(returnValue.Count == 3);
                TestHelper.Assert(returnValue["abc"].Length == 3);
                TestHelper.Assert(returnValue["abc"][0] == -1.1f);
                TestHelper.Assert(returnValue["abc"][1] == 123123.2f);
                TestHelper.Assert(returnValue["abc"][2] == 100.0f);
                TestHelper.Assert(returnValue["ABC"].Length == 2);
                TestHelper.Assert(returnValue["ABC"][0] == 42.24f);
                TestHelper.Assert(returnValue["ABC"][1] == -1.61f);
                TestHelper.Assert(returnValue["aBc"].Length == 2);
                TestHelper.Assert(returnValue["aBc"][0] == -3.14f);
                TestHelper.Assert(returnValue["aBc"][1] == 3.14f);
            }

            {
                var sdi1 = new Dictionary<string, double[]>();
                var sdi2 = new Dictionary<string, double[]>();

                double[] si1 = new double[] { 1.1E10, 1.2E10, 1.3E10 };
                double[] si2 = new double[] { 1.4E10, 1.5E10 };
                double[] si3 = new double[] { 1.6E10, 1.7E10 };

                sdi1["Hello!!"] = si1;
                sdi1["Goodbye"] = si2;
                sdi2[""] = si3;

                (Dictionary<string, double[]> returnValue, Dictionary<string, double[]> p3) =
                    p.OpStringDoubleSDAsync(sdi1, sdi2).Result;
                TestHelper.Assert(p3.Count == 1);
                TestHelper.Assert(p3[""].Length == 2);
                TestHelper.Assert(p3[""][0] == 1.6E10);
                TestHelper.Assert(p3[""][1] == 1.7E10);
                TestHelper.Assert(returnValue.Count == 3);
                TestHelper.Assert(returnValue["Hello!!"].Length == 3);
                TestHelper.Assert(returnValue["Hello!!"][0] == 1.1E10);
                TestHelper.Assert(returnValue["Hello!!"][1] == 1.2E10);
                TestHelper.Assert(returnValue["Hello!!"][2] == 1.3E10);
                TestHelper.Assert(returnValue["Goodbye"].Length == 2);
                TestHelper.Assert(returnValue["Goodbye"][0] == 1.4E10);
                TestHelper.Assert(returnValue["Goodbye"][1] == 1.5E10);
                TestHelper.Assert(returnValue[""].Length == 2);
                TestHelper.Assert(returnValue[""][0] == 1.6E10);
                TestHelper.Assert(returnValue[""][1] == 1.7E10);
            }

            {
                var sdi1 = new Dictionary<string, string[]>();
                var sdi2 = new Dictionary<string, string[]>();

                string[] si1 = new string[] { "abc", "de", "fghi" };
                string[] si2 = new string[] { "xyz", "or" };
                string[] si3 = new string[] { "and", "xor" };

                sdi1["abc"] = si1;
                sdi1["def"] = si2;
                sdi2["ghi"] = si3;

                (Dictionary<string, string[]> returnValue, Dictionary<string, string[]> p3) =
                    p.OpStringStringSDAsync(sdi1, sdi2).Result;
                TestHelper.Assert(p3.Count == 1);
                TestHelper.Assert(p3["ghi"].Length == 2);
                TestHelper.Assert(p3["ghi"][0].Equals("and"));
                TestHelper.Assert(p3["ghi"][1].Equals("xor"));

                TestHelper.Assert(returnValue.Count == 3);
                TestHelper.Assert(returnValue["abc"].Length == 3);
                TestHelper.Assert(returnValue["abc"][0].Equals("abc"));
                TestHelper.Assert(returnValue["abc"][1].Equals("de"));
                TestHelper.Assert(returnValue["abc"][2].Equals("fghi"));
                TestHelper.Assert(returnValue["def"].Length == 2);
                TestHelper.Assert(returnValue["def"][0].Equals("xyz"));
                TestHelper.Assert(returnValue["def"][1].Equals("or"));
                TestHelper.Assert(returnValue["ghi"].Length == 2);
                TestHelper.Assert(returnValue["ghi"][0].Equals("and"));
                TestHelper.Assert(returnValue["ghi"][1].Equals("xor"));
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

                (Dictionary<MyEnum, MyEnum[]> returnValue, Dictionary<MyEnum, MyEnum[]> p3) =
                    p.OpMyEnumMyEnumSDAsync(sdi1, sdi2).Result;
                TestHelper.Assert(p3.Count == 1);
                TestHelper.Assert(p3[MyEnum.enum1].Length == 2);
                TestHelper.Assert(p3[MyEnum.enum1][0] == MyEnum.enum3);
                TestHelper.Assert(p3[MyEnum.enum1][1] == MyEnum.enum3);
                TestHelper.Assert(returnValue.Count == 3);
                TestHelper.Assert(returnValue[MyEnum.enum3].Length == 3);
                TestHelper.Assert(returnValue[MyEnum.enum3][0] == MyEnum.enum1);
                TestHelper.Assert(returnValue[MyEnum.enum3][1] == MyEnum.enum1);
                TestHelper.Assert(returnValue[MyEnum.enum3][2] == MyEnum.enum2);
                TestHelper.Assert(returnValue[MyEnum.enum2].Length == 2);
                TestHelper.Assert(returnValue[MyEnum.enum2][0] == MyEnum.enum1);
                TestHelper.Assert(returnValue[MyEnum.enum2][1] == MyEnum.enum2);
                TestHelper.Assert(returnValue[MyEnum.enum1].Length == 2);
                TestHelper.Assert(returnValue[MyEnum.enum1][0] == MyEnum.enum3);
                TestHelper.Assert(returnValue[MyEnum.enum1][1] == MyEnum.enum3);
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

                    int[] r = p.OpIntSAsync(s).Result;
                    TestHelper.Assert(r.Length == lengths[l]);
                    for (int j = 0; j < r.Length; ++j)
                    {
                        TestHelper.Assert(r[j] == -j);
                    }
                }
            }

            {
                var ctx = new SortedDictionary<string, string>
                {
                    ["one"] = "ONE",
                    ["two"] = "TWO",
                    ["three"] = "THREE"
                };
                {
                    TestHelper.Assert(p.Context.Count == 0);
                    TestHelper.Assert(!ctx.DictionaryEqual(p.OpContextAsync().Result));
                }
                {
                    TestHelper.Assert(p.Context.Count == 0);
                    TestHelper.Assert(ctx.DictionaryEqual(p.OpContextAsync(ctx).Result));
                }
                {
                    IMyClassPrx p2 = p.Clone(context: ctx);
                    TestHelper.Assert(p2.Context.DictionaryEqual(ctx));
                    TestHelper.Assert(ctx.DictionaryEqual(p2.OpContextAsync().Result));
                }
                {
                    IMyClassPrx? p2 = p.Clone(context: ctx);
                    TestHelper.Assert(ctx.DictionaryEqual(p2.OpContextAsync(ctx).Result));
                }
            }

            // Test implicit context propagation with async task
            {
                communicator.CurrentContext["one"] = "ONE";
                communicator.CurrentContext["two"] = "TWO";
                communicator.CurrentContext["three"] = "THREE";

                var p3 = IMyClassPrx.Parse(helper.GetTestProxy("test", 0), communicator);
                TestHelper.Assert(p3.OpContextAsync().Result.DictionaryEqual(communicator.CurrentContext));

                var prxContext = new Dictionary<string, string>
                {
                    ["one"] = "UN",
                    ["four"] = "QUATRE"
                };

                var combined = new Dictionary<string, string>(prxContext);
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
                TestHelper.Assert(p3.OpContextAsync().Result.DictionaryEqual(prxContext));

                communicator.CurrentContext = ctx;
                TestHelper.Assert(p3.OpContextAsync().Result.DictionaryEqual(combined));

                // Cleanup
                communicator.CurrentContext.Clear();
                communicator.DefaultContext = new SortedDictionary<string, string>();
            }

            p.OpIdempotentAsync().Wait();

            try
            {
                p.OpOnewayAsync().Wait();
                TestHelper.Assert(false);
            }
            catch (AggregateException ex)
            {
                TestHelper.Assert(ex.InnerException is SomeException);
            }

            // This is invoked as a oneway, despite using a twoway proxy.
            p.OpOnewayMetadataAsync().Wait();

            {
                var derived = p.Clone(IMyDerivedClassPrx.Factory);
                TestHelper.Assert(derived != null);
                derived.OpDerivedAsync().Wait();
            }

            {
                TestHelper.Assert(p.OpByte1Async(0xFF).Result == 0xFF);
                TestHelper.Assert(p.OpInt1Async(0x7FFFFFFF).Result == 0x7FFFFFFF);
                TestHelper.Assert(p.OpLong1Async(0x7FFFFFFFFFFFFFFF).Result == 0x7FFFFFFFFFFFFFFF);
                TestHelper.Assert(p.OpFloat1Async(1.0f).Result == 1.0f);
                TestHelper.Assert(p.OpDouble1Async(1.0d).Result == 1.0d);
                TestHelper.Assert(p.OpString1Async("opString1").Result.Equals("opString1"));
                TestHelper.Assert(p.OpStringS1Async(Array.Empty<string>()).Result.Length == 0);
                TestHelper.Assert(p.OpByteBoolD1Async(new Dictionary<byte, bool>()).Result.Count == 0);
                TestHelper.Assert(p.OpStringS2Async(Array.Empty<string>()).Result.Length == 0);
                TestHelper.Assert(p.OpByteBoolD2Async(new Dictionary<byte, bool>()).Result.Count == 0);
            }

            Task.Run(async () =>
                {
                    {
                        Structure p1 = await p.OpMStruct1Async();

                        p1.E = MyEnum.enum3;
                        (Structure ReturnValue, Structure p2) = await p.OpMStruct2Async(p1);
                        TestHelper.Assert(p2.Equals(p1) && ReturnValue.Equals(p1));
                    }

                    {
                        await p.OpMSeq1Async();

                        string[]? p1 = new string[1];
                        p1[0] = "test";
                        (string[] ReturnValue, string[] p2) = await p.OpMSeq2Async(p1);
                        TestHelper.Assert(p2.SequenceEqual(p1) && ReturnValue.SequenceEqual(p1));
                    }

                    {
                        await p.OpMDict1Async();

                        var p1 = new Dictionary<string, string>
                        {
                            ["test"] = "test"
                        };
                        (Dictionary<string, string> ReturnValue, Dictionary<string, string> p2) =
                            await p.OpMDict2Async(p1);
                        TestHelper.Assert(p2.DictionaryEqual(p1) && ReturnValue.DictionaryEqual(p1));
                    }
                }).Wait();

            {
                void AssertStreamEquals(System.IO.Stream s1, System.IO.Stream s2)
                {
                    int v1, v2;
                    do
                    {
                        v1 = s1.ReadByte();
                        v2 = s2.ReadByte();
                        TestHelper.Assert(v1 == v2);
                    }
                    while (v1 != -1 && v2 != -1);
                }

                if (p.Protocol != Protocol.Ice1)
                {
                    Task.Run(async () =>
                    {
                        await p.OpSendStream1Async(File.OpenRead("AllTests.cs")).ConfigureAwait(false);

                        await p.OpSendStream2Async("AllTests.cs", File.OpenRead("AllTests.cs")).ConfigureAwait(false);

                        {
                            using Stream stream = await p.OpGetStream1Async().ConfigureAwait(false);
                            using Stream fileStream = File.OpenRead("AllTests.cs");
                            AssertStreamEquals(stream, fileStream);
                        }

                        {
                            (string fileName, Stream stream) = await p.OpGetStream2Async().ConfigureAwait(false);
                            using Stream fileStream = File.OpenRead(fileName);
                            AssertStreamEquals(stream, File.OpenRead(fileName));
                            stream.Dispose();
                        }

                        {
                            using Stream stream =
                                await p.OpSendAndGetStream1Async(File.OpenRead("AllTests.cs")).ConfigureAwait(false);
                            using Stream fileStream = File.OpenRead("AllTests.cs");
                            AssertStreamEquals(stream, fileStream);
                        }

                        {
                            (string fileName, Stream stream) = await p.OpSendAndGetStream2Async(
                                "AllTests.cs",
                                File.OpenRead("AllTests.cs")).ConfigureAwait(false);
                            using Stream fileStream = File.OpenRead(fileName);
                            AssertStreamEquals(stream, fileStream);
                            stream.Dispose();
                        }
                    }).Wait();
                }
            }
        }
    }
}
