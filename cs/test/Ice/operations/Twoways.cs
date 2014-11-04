// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Collections.Generic;
using System.Threading;

class Twoways
{
    private static void test(bool b)
    {
        if(!b)
        {
            throw new System.Exception();
        }
    }

    class PerThreadContextInvokeThread
    {
        public PerThreadContextInvokeThread(Test.MyClassPrx proxy)
        {
            _proxy = proxy;
        }

        public void Join()
        {
            _thread.Join();
        }

        public void Start()
        {
            _thread = new Thread(new ThreadStart(Run));
            _thread.Start();
        }

        public void Run()
        {
            Dictionary<string, string> ctx = _proxy.ice_getCommunicator().getImplicitContext().getContext();
            test(ctx.Count == 0);
            ctx["one"] =  "ONE";
            _proxy.ice_getCommunicator().getImplicitContext().setContext(ctx);
            test(Ice.CollectionComparer.Equals(_proxy.opContext(), ctx));
        }

        private Test.MyClassPrx _proxy;
        private Thread _thread;
    }

    internal static void twoways(Ice.Communicator communicator, Test.MyClassPrx p)
    {
        p.ice_ping();

	test(Test.MyClassPrxHelper.ice_staticId().Equals(Test.MyClass.ice_staticId()));
	test(Ice.ObjectPrxHelper.ice_staticId().Equals(Ice.ObjectImpl.ice_staticId()));


        test(p.ice_isA(Test.MyClass.ice_staticId()));

        test(p.ice_id().Equals(Test.MyDerivedClass.ice_staticId()));

        {
            string[] ids = p.ice_ids();
            test(ids.Length == 3);
            test(ids[0].Equals("::Ice::Object"));
            test(ids[1].Equals("::Test::MyClass"));
            test(ids[2].Equals("::Test::MyDerivedClass"));
        }

        {
            p.opVoid();
        }

        {
            byte b;
            byte r;

            r = p.opByte(0xff, 0x0f, out b);
            test(b == 0xf0);
            test(r == 0xff);
        }

        {
            bool b;
            bool r;

            r = p.opBool(true, false, out b);
            test(b);
            test(!r);
        }

        {
            short s;
            int i;
            long l;
            long r;

            r = p.opShortIntLong(10, 11, 12L, out s, out i, out l);
            test(s == 10);
            test(i == 11);
            test(l == 12);
            test(r == 12L);

            r = p.opShortIntLong(Int16.MinValue, Int32.MinValue, Int64.MinValue, out s, out i, out l);
            test(s == Int16.MinValue);
            test(i == Int32.MinValue);
            test(l == Int64.MinValue);
            test(r == Int64.MinValue);

            r = p.opShortIntLong(Int16.MaxValue, Int32.MaxValue, Int64.MaxValue, out s, out i, out l);
            test(s == Int16.MaxValue);
            test(i == Int32.MaxValue);
            test(l == Int64.MaxValue);
            test(r == Int64.MaxValue);
        }

        {
            float f;
            double d;
            double r;

            r = p.opFloatDouble(3.14f, 1.1e10, out f, out d);
            test(f == 3.14f);
            test(d == 1.1e10);
            test(r == 1.1e10);

            r = p.opFloatDouble(Single.Epsilon, Double.MinValue, out f, out d);
            test(f == Single.Epsilon);
            test(d == Double.MinValue);
            test(r == Double.MinValue);

            r = p.opFloatDouble(Single.MaxValue, Double.MaxValue, out f, out d);
            test(f == Single.MaxValue);
            test(d == Double.MaxValue);
            test(r == Double.MaxValue);
        }

        {
            string s;
            string r;

            r = p.opString("hello", "world", out s);
            test(s.Equals("world hello"));
            test(r.Equals("hello world"));
        }

        {
            Test.MyEnum e;
            Test.MyEnum r;

            r = p.opMyEnum(Test.MyEnum.enum2, out e);
            test(e == Test.MyEnum.enum2);
            test(r == Test.MyEnum.enum3);
        }

        {
            Test.MyClassPrx c1;
            Test.MyClassPrx c2;
            Test.MyClassPrx r;

            r = p.opMyClass(p, out c1, out c2);
            test(Ice.Util.proxyIdentityAndFacetCompare(c1, p) == 0);
            test(Ice.Util.proxyIdentityAndFacetCompare(c2, p) != 0);
            test(Ice.Util.proxyIdentityAndFacetCompare(r, p) == 0);
            test(c1.ice_getIdentity().Equals(communicator.stringToIdentity("test")));
            test(c2.ice_getIdentity().Equals(communicator.stringToIdentity("noSuchIdentity")));
            test(r.ice_getIdentity().Equals(communicator.stringToIdentity("test")));
            r.opVoid();
            c1.opVoid();
            try
            {
                c2.opVoid();
                test(false);
            }
            catch(Ice.ObjectNotExistException)
            {
            }

            r = p.opMyClass(null, out c1, out c2);
            test(c1 == null);
            test(c2 != null);
            test(Ice.Util.proxyIdentityAndFacetCompare(r, p) == 0);
            r.opVoid();
        }

        {
            Test.Structure si1 = new Test.Structure();
            si1.p = p;
            si1.e = Test.MyEnum.enum3;
            si1.s = new Test.AnotherStruct();
            si1.s.s = "abc";
            Test.Structure si2 = new Test.Structure();
            si2.p = null;
            si2.e = Test.MyEnum.enum2;
            si2.s = new Test.AnotherStruct();
            si2.s.s = "def";

            Test.Structure so;
            Test.Structure rso = p.opStruct(si1, si2, out so);
            test(rso.p == null);
            test(rso.e == Test.MyEnum.enum2);
            test(rso.s.s.Equals("def"));
            test(so.p.Equals(p));
            test(so.e == Test.MyEnum.enum3);
            test(so.s.s.Equals("a new string"));
            so.p.opVoid();
        }

        {
            byte[] bsi1 = new byte[] { 0x01, 0x11, 0x12, 0x22 };
            byte[] bsi2 = new byte[] { 0xf1, 0xf2, 0xf3, 0xf4 };

            byte[] bso;
            byte[] rso;

            rso = p.opByteS(bsi1, bsi2, out bso);
            test(bso.Length == 4);
            test(bso[0] == 0x22);
            test(bso[1] == 0x12);
            test(bso[2] == 0x11);
            test(bso[3] == 0x01);
            test(rso.Length == 8);
            test(rso[0] == 0x01);
            test(rso[1] == 0x11);
            test(rso[2] == 0x12);
            test(rso[3] == 0x22);
            test(rso[4] == 0xf1);
            test(rso[5] == 0xf2);
            test(rso[6] == 0xf3);
            test(rso[7] == 0xf4);
        }

        {
            bool[] bsi1 = new bool[] { true, true, false };
            bool[] bsi2 = new bool[] { false };

            bool[] bso;
            bool[] rso;

            rso = p.opBoolS(bsi1, bsi2, out bso);
            test(bso.Length == 4);
            test(bso[0]);
            test(bso[1]);
            test(!bso[2]);
            test(!bso[3]);
            test(rso.Length == 3);
            test(!rso[0]);
            test(rso[1]);
            test(rso[2]);
        }

        {
            short[] ssi = new short[] { 1, 2, 3 };
            int[] isi = new int[] { 5, 6, 7, 8 };
            long[] lsi = new long[] { 10, 30, 20 };

            short[] sso;
            int[] iso;
            long[] lso;
            long[] rso;

            rso = p.opShortIntLongS(ssi, isi, lsi, out sso, out iso, out lso);
            test(sso.Length == 3);
            test(sso[0] == 1);
            test(sso[1] == 2);
            test(sso[2] == 3);
            test(iso.Length == 4);
            test(iso[0] == 8);
            test(iso[1] == 7);
            test(iso[2] == 6);
            test(iso[3] == 5);
            test(lso.Length == 6);
            test(lso[0] == 10);
            test(lso[1] == 30);
            test(lso[2] == 20);
            test(lso[3] == 10);
            test(lso[4] == 30);
            test(lso[5] == 20);
            test(rso.Length == 3);
            test(rso[0] == 10);
            test(rso[1] == 30);
            test(rso[2] == 20);
        }

        {
            float[] fsi = new float[] { 3.14f, 1.11f };
            double[] dsi = new double[] { 1.1e10, 1.2e10, 1.3e10 };

            float[] fso;
            double[] dso;
            double[] rso;

            rso = p.opFloatDoubleS(fsi, dsi, out fso, out dso);
            test(fso.Length == 2);
            test(fso[0] == 3.14f);
            test(fso[1] == 1.11f);
            test(dso.Length == 3);
            test(dso[0] == 1.3e10);
            test(dso[1] == 1.2e10);
            test(dso[2] == 1.1e10);
            test(rso.Length == 5);
            test(rso[0] == 1.1e10);
            test(rso[1] == 1.2e10);
            test(rso[2] == 1.3e10);
            test((float)rso[3] == 3.14f);
            test((float)rso[4] == 1.11f);
        }

        {
            string[] ssi1 = new string[] { "abc", "de", "fghi" };
            string[] ssi2 = new string[] { "xyz" };

            string[] sso;
            string[] rso;

            rso = p.opStringS(ssi1, ssi2, out sso);
            test(sso.Length == 4);
            test(sso[0].Equals("abc"));
            test(sso[1].Equals("de"));
            test(sso[2].Equals("fghi"));
            test(sso[3].Equals("xyz"));
            test(rso.Length == 3);
            test(rso[0].Equals("fghi"));
            test(rso[1].Equals("de"));
            test(rso[2].Equals("abc"));
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

            rso = p.opByteSS(bsi1, bsi2, out bso);
            test(bso.Length == 2);
            test(bso[0].Length == 1);
            test(bso[0][0] == 0xff);
            test(bso[1].Length == 3);
            test(bso[1][0] == 0x01);
            test(bso[1][1] == 0x11);
            test(bso[1][2] == 0x12);
            test(rso.Length == 4);
            test(rso[0].Length == 3);
            test(rso[0][0] == 0x01);
            test(rso[0][1] == 0x11);
            test(rso[0][2] == 0x12);
            test(rso[1].Length == 1);
            test(rso[1][0] == 0xff);
            test(rso[2].Length == 1);
            test(rso[2][0] == 0x0e);
            test(rso[3].Length == 2);
            test(rso[3][0] == 0xf2);
            test(rso[3][1] == 0xf1);
        }

        {
            bool[] s11 = new bool[] {true};
            bool[] s12 = new bool[] {false};
            bool[] s13 = new bool[] {true, true};
            bool[][] bsi1 = new bool[][] {s11, s12, s13};

            bool[] s21 = new bool[] {false, false, true};
            bool[][] bsi2 = new bool[][] {s21};

            bool[][] rso;
            bool[][] bso;

            rso = p.opBoolSS(bsi1, bsi2, out bso);
            test(bso.Length == 4);
            test(bso[0].Length == 1);
            test(bso[0][0]);
            test(bso[1].Length == 1);
            test(!bso[1][0]);
            test(bso[2].Length == 2);
            test(bso[2][0]);
            test(bso[2][1]);
            test(bso[3].Length == 3);
            test(!bso[3][0]);
            test(!bso[3][1]);
            test(bso[3][2]);
            test(rso.Length == 3);
            test(rso[0].Length == 2);
            test(rso[0][0]);
            test(rso[0][1]);
            test(rso[1].Length == 1);
            test(!rso[1][0]);
            test(rso[2].Length == 1);
            test(rso[2][0]);
        }

        {
            short[] s11 = new short[] {1, 2, 5};
            short[] s12 = new short[] {13};
            short[] s13 = new short[] {};
            short[][] ssi = new short[][] {s11, s12, s13};

            int[] i11 = new int[] {24, 98};
            int[] i12 = new int[] {42};
            int[][] isi = new int[][] {i11, i12};

            long[] l11 = new long[] {496, 1729};
            long[][] lsi = new long[][] {l11};

            short[][] sso;
            int[][] iso;
            long[][] lso;
            long[][] rso;

            rso = p.opShortIntLongSS(ssi, isi, lsi, out sso, out iso, out lso);
            test(rso.Length == 1);
            test(rso[0].Length == 2);
            test(rso[0][0] == 496);
            test(rso[0][1] == 1729);
            test(sso.Length == 3);
            test(sso[0].Length == 3);
            test(sso[0][0] == 1);
            test(sso[0][1] == 2);
            test(sso[0][2] == 5);
            test(sso[1].Length == 1);
            test(sso[1][0] == 13);
            test(sso[2].Length == 0);
            test(iso.Length == 2);
            test(iso[0].Length == 1);
            test(iso[0][0] == 42);
            test(iso[1].Length == 2);
            test(iso[1][0] == 24);
            test(iso[1][1] == 98);
            test(lso.Length == 2);
            test(lso[0].Length == 2);
            test(lso[0][0] == 496);
            test(lso[0][1] == 1729);
            test(lso[1].Length == 2);
            test(lso[1][0] == 496);
            test(lso[1][1] == 1729);
        }

        {
            float[] f11 = new float[] { 3.14f };
            float[] f12 = new float[] { 1.11f };
            float[] f13 = new float[] { };
            float[][] fsi = new float[][] { f11, f12, f13 };

            double[] d11 = new double[] { 1.1e10, 1.2e10, 1.3e10 };
            double[][] dsi = new double[][] { d11 };

            float[][] fso;
            double[][] dso;
            double[][] rso;

            rso = p.opFloatDoubleSS(fsi, dsi, out fso, out dso);
            test(fso.Length == 3);
            test(fso[0].Length == 1);
            test(fso[0][0] == 3.14f);
            test(fso[1].Length == 1);
            test(fso[1][0] == 1.11f);
            test(fso[2].Length == 0);
            test(dso.Length == 1);
            test(dso[0].Length == 3);
            test(dso[0][0] == 1.1e10);
            test(dso[0][1] == 1.2e10);
            test(dso[0][2] == 1.3e10);
            test(rso.Length == 2);
            test(rso[0].Length == 3);
            test(rso[0][0] == 1.1e10);
            test(rso[0][1] == 1.2e10);
            test(rso[0][2] == 1.3e10);
            test(rso[1].Length == 3);
            test(rso[1][0] == 1.1e10);
            test(rso[1][1] == 1.2e10);
            test(rso[1][2] == 1.3e10);
        }

        {
            string[] s11 = new string[] { "abc" };
            string[] s12 = new string[] { "de", "fghi" };
            string[][] ssi1 = new string[][] { s11, s12 };

            string[] s21 = new string[] {};
            string[] s22 = new string[] {};
            string[] s23 = new string[] { "xyz" };
            string[][] ssi2 = new string[][] { s21, s22, s23 };

            string[][] sso;
            string[][] rso;

            rso = p.opStringSS(ssi1, ssi2, out sso);
            test(sso.Length == 5);
            test(sso[0].Length == 1);
            test(sso[0][0].Equals("abc"));
            test(sso[1].Length == 2);
            test(sso[1][0].Equals("de"));
            test(sso[1][1].Equals("fghi"));
            test(sso[2].Length == 0);
            test(sso[3].Length == 0);
            test(sso[4].Length == 1);
            test(sso[4][0].Equals("xyz"));
            test(rso.Length == 3);
            test(rso[0].Length == 1);
            test(rso[0][0].Equals("xyz"));
            test(rso[1].Length == 0);
            test(rso[2].Length == 0);
        }

        {
            string[] s111 = new string[] { "abc", "de"};
            string[] s112 = new string[] { "xyz" };
            string[][] ss11 = new string[][] { s111, s112 };
            string[] s121 = new string[] { "hello"};
            string[][] ss12 = new string[][] { s121 };
            string[][][] sssi1 = new string[][][] { ss11, ss12 };

            string[] s211 = new string[] { "", ""};
            string[] s212 = new string[] { "abcd" };
            string[][] ss21 = new string[][] { s211, s212 };
            string[] s221 = new string[] { ""};
            string[][] ss22 = new string[][] { s221 };
            string[][] ss23 = new string[][] {};
            string[][][] sssi2 = new string[][][] { ss21, ss22, ss23 };

            string[][][] ssso;
            string[][][] rsso;

            rsso = p.opStringSSS(sssi1, sssi2, out ssso);
            test(ssso.Length == 5);
            test(ssso[0].Length == 2);
            test(ssso[0][0].Length == 2);
            test(ssso[0][1].Length == 1);
            test(ssso[1].Length == 1);
            test(ssso[1][0].Length == 1);
            test(ssso[2].Length == 2);
            test(ssso[2][0].Length == 2);
            test(ssso[2][1].Length == 1);
            test(ssso[3].Length == 1);
            test(ssso[3][0].Length == 1);
            test(ssso[4].Length == 0);
            test(ssso[0][0][0].Equals("abc"));
            test(ssso[0][0][1].Equals("de"));
            test(ssso[0][1][0].Equals("xyz"));
            test(ssso[1][0][0].Equals("hello"));
            test(ssso[2][0][0].Equals(""));
            test(ssso[2][0][1].Equals(""));
            test(ssso[2][1][0].Equals("abcd"));
            test(ssso[3][0][0].Equals(""));

            test(rsso.Length == 3);
            test(rsso[0].Length == 0);
            test(rsso[1].Length == 1);
            test(rsso[1][0].Length == 1);
            test(rsso[2].Length == 2);
            test(rsso[2][0].Length == 2);
            test(rsso[2][1].Length == 1);
            test(rsso[1][0][0].Equals(""));
            test(rsso[2][0][0].Equals(""));
            test(rsso[2][0][1].Equals(""));
            test(rsso[2][1][0].Equals("abcd"));
        }

        {
            Dictionary<byte, bool> di1 = new Dictionary<byte, bool>();
            di1[10] = true;
            di1[100] = false;
            Dictionary<byte, bool> di2 = new Dictionary<byte, bool>();
            di2[10] = true;
            di2[11] = false;
            di2[101] = true;

            Dictionary<byte, bool> _do;
            Dictionary<byte, bool> ro = p.opByteBoolD(di1, di2, out _do);

            test(Ice.CollectionComparer.Equals(_do, di1));
            test(ro.Count == 4);
            test(ro[10] == true);
            test(ro[11] == false);
            test(ro[100] == false);
            test(ro[101] == true);
        }

        {
            Dictionary<short, int> di1 = new Dictionary<short, int>();
            di1[110] = -1;
            di1[1100] = 123123;
            Dictionary<short, int> di2 = new Dictionary<short, int>();
            di2[110] = -1;
            di2[111] = -100;
            di2[1101] = 0;

            Dictionary<short, int> _do;
            Dictionary<short, int> ro = p.opShortIntD(di1, di2, out _do);

            test(Ice.CollectionComparer.Equals(_do, di1));
            test(ro.Count == 4);
            test(ro[110] == -1);
            test(ro[111] == -100);
            test(ro[1100] == 123123);
            test(ro[1101] == 0);
        }

        {
            Dictionary<long, float> di1 = new Dictionary<long, float>();
            di1[999999110L] = -1.1f;
            di1[999999111L] = 123123.2f;
            Dictionary<long, float> di2 = new Dictionary<long, float>();
            di2[999999110L] = -1.1f;
            di2[999999120L] = -100.4f;
            di2[999999130L] = 0.5f;

            Dictionary<long, float> _do;
            Dictionary<long, float> ro = p.opLongFloatD(di1, di2, out _do);

            test(Ice.CollectionComparer.Equals(_do, di1));
            test(ro.Count == 4);
            test(ro[999999110L] == -1.1f);
            test(ro[999999120L] == -100.4f);
            test(ro[999999111L] == 123123.2f);
            test(ro[999999130L] == 0.5f);
        }

        {
            Dictionary<string, string> di1 = new Dictionary<string, string>();
            di1["foo"] = "abc -1.1";
            di1["bar"] = "abc 123123.2";
            Dictionary<string, string> di2 = new Dictionary<string, string>();
            di2["foo"] = "abc -1.1";
            di2["FOO"] = "abc -100.4";
            di2["BAR"] = "abc 0.5";

            Dictionary<string, string> _do;
            Dictionary<string, string> ro = p.opStringStringD(di1, di2, out _do);

            test(Ice.CollectionComparer.Equals(_do, di1));
            test(ro.Count == 4);
            test(ro["foo"].Equals("abc -1.1"));
            test(ro["FOO"].Equals("abc -100.4"));
            test(ro["bar"].Equals("abc 123123.2"));
            test(ro["BAR"].Equals("abc 0.5"));
        }

        {
            Dictionary<string, Test.MyEnum> di1 = new Dictionary<string, Test.MyEnum>();
            di1["abc"] = Test.MyEnum.enum1;
            di1[""] = Test.MyEnum.enum2;
            Dictionary<string, Test.MyEnum> di2 = new Dictionary<string, Test.MyEnum>();
            di2["abc"] = Test.MyEnum.enum1;
            di2["qwerty"] = Test.MyEnum.enum3;
            di2["Hello!!"] = Test.MyEnum.enum2;

            Dictionary<string, Test.MyEnum> _do;
            Dictionary<string, Test.MyEnum> ro = p.opStringMyEnumD(di1, di2, out _do);

            test(Ice.CollectionComparer.Equals(_do, di1));
            test(ro.Count == 4);
            test(ro["abc"] == Test.MyEnum.enum1);
            test(ro["qwerty"] == Test.MyEnum.enum3);
            test(ro[""] == Test.MyEnum.enum2);
            test(ro["Hello!!"] == Test.MyEnum.enum2);
        }

        {
            Dictionary<Test.MyEnum, string> di1 = new Dictionary<Test.MyEnum, string>();
            di1[Test.MyEnum.enum1] = "abc";
            Dictionary<Test.MyEnum, string> di2 = new Dictionary<Test.MyEnum, string>();
            di2[Test.MyEnum.enum2] = "Hello!!";
            di2[Test.MyEnum.enum3] = "qwerty";

            Dictionary<Test.MyEnum, string> _do;
            Dictionary<Test.MyEnum, string> ro = p.opMyEnumStringD(di1, di2, out _do);

            test(Ice.CollectionComparer.Equals(_do, di1));
            test(ro.Count == 3);
            test(ro[Test.MyEnum.enum1].Equals("abc"));
            test(ro[Test.MyEnum.enum2].Equals("Hello!!"));
            test(ro[Test.MyEnum.enum3].Equals("qwerty"));
        }

        {
            Test.MyStruct s11 = new Test.MyStruct(1, 1);
            Test.MyStruct s12 = new Test.MyStruct(1, 2);
            Dictionary<Test.MyStruct, Test.MyEnum> di1 = new Dictionary<Test.MyStruct, Test.MyEnum>();
            di1[s11] = Test.MyEnum.enum1;
            di1[s12] = Test.MyEnum.enum2;

            Test.MyStruct s22 = new Test.MyStruct(2, 2);
            Test.MyStruct s23 = new Test.MyStruct(2, 3);
            Dictionary<Test.MyStruct, Test.MyEnum> di2 = new Dictionary<Test.MyStruct, Test.MyEnum>();
            di2[s11] = Test.MyEnum.enum1;
            di2[s22] = Test.MyEnum.enum3;
            di2[s23] = Test.MyEnum.enum2;

            Dictionary<Test.MyStruct, Test.MyEnum> _do;
            Dictionary<Test.MyStruct, Test.MyEnum> ro = p.opMyStructMyEnumD(di1, di2, out _do);

            test(Ice.CollectionComparer.Equals(_do, di1));
            test(ro.Count == 4);
            test(ro[s11] == Test.MyEnum.enum1);
            test(ro[s12] == Test.MyEnum.enum2);
            test(ro[s22] == Test.MyEnum.enum3);
            test(ro[s23] == Test.MyEnum.enum2);
        }

        {
            int[] lengths = new int[] { 0, 1, 2, 126, 127, 128, 129, 253, 254, 255, 256, 257, 1000 };

            for(int l = 0; l < lengths.Length; ++l)
            {
                int[] s = new int[lengths[l]];
                for(int i = 0; i < lengths[l]; ++i)
                {
                    s[i] = i;
                }

                int[] r = p.opIntS(s);
                test(r.Length == lengths[l]);
                for(int j = 0; j < r.Length; ++j)
                {
                    test(r[j] == -j);
                }
            }
        }

        {
            Dictionary<string, string> ctx = new Dictionary<string, string>();
            ctx["one"] = "ONE";
            ctx["two"] = "TWO";
            ctx["three"] = "THREE";
            {
                test(p.ice_getContext().Count == 0);
                Dictionary<string, string> r = p.opContext();
                test(!r.Equals(ctx));
            }
            {
                Dictionary<string, string> r = p.opContext(ctx);
                test(p.ice_getContext().Count == 0);
                test(Ice.CollectionComparer.Equals(r, ctx));
            }
            {
                Test.MyClassPrx p2 = Test.MyClassPrxHelper.checkedCast(p.ice_context(ctx));
                test(Ice.CollectionComparer.Equals(p2.ice_getContext(), ctx));
                Dictionary<string, string> r = p2.opContext();
                test(Ice.CollectionComparer.Equals(r, ctx));
                r = p2.opContext(ctx);
                test(Ice.CollectionComparer.Equals(r, ctx));
            }
        }

        if(p.ice_getConnection() != null)
        {
            //
            // Test implicit context propagation
            //

            String[] impls = {"Shared", "PerThread"};
            for(int i = 0; i < 2; i++)
            {
                Ice.InitializationData initData = new Ice.InitializationData();
                initData.properties = communicator.getProperties().ice_clone_();
                initData.properties.setProperty("Ice.ImplicitContext", impls[i]);

                Ice.Communicator ic = Ice.Util.initialize(initData);

                Dictionary<string, string> ctx = new Dictionary<string, string>();
                ctx["one"] = "ONE";
                ctx["two"] = "TWO";
                ctx["three"] = "THREE";

                Test.MyClassPrx p3 = Test.MyClassPrxHelper.uncheckedCast(
                    ic.stringToProxy("test:default -p 12010"));

                ic.getImplicitContext().setContext(ctx);
                test(Ice.CollectionComparer.Equals(ic.getImplicitContext().getContext(), ctx));
                test(Ice.CollectionComparer.Equals(p3.opContext(), ctx));

                test(ic.getImplicitContext().containsKey("zero") == false);
                String r = ic.getImplicitContext().put("zero", "ZERO");
                test(r.Equals(""));
                test(ic.getImplicitContext().get("zero").Equals("ZERO"));

                ctx = ic.getImplicitContext().getContext();
                test(Ice.CollectionComparer.Equals(p3.opContext(), ctx));

                Dictionary<string, string> prxContext = new Dictionary<string, string>();
                prxContext["one"] = "UN";
                prxContext["four"] = "QUATRE";

                Dictionary<string, string> combined = new Dictionary<string, string>(prxContext);
                foreach(KeyValuePair<string, string> e in ctx)
                {
                    try
                    {
                        combined.Add(e.Key, e.Value);
                    }
                    catch(System.ArgumentException)
                    {
                        // Ignore.
                    }
                }
                test(combined["one"].Equals("UN"));

                p3 = Test.MyClassPrxHelper.uncheckedCast(p3.ice_context(prxContext));

                ic.getImplicitContext().setContext(null);
                test(Ice.CollectionComparer.Equals(p3.opContext(), prxContext));

                ic.getImplicitContext().setContext(ctx);
                test(Ice.CollectionComparer.Equals(p3.opContext(), combined));

                test(ic.getImplicitContext().remove("one").Equals("ONE"));

                if(impls[i].Equals("PerThread"))
                {
                    PerThreadContextInvokeThread thread = new PerThreadContextInvokeThread(
                        Test.MyClassPrxHelper.uncheckedCast(p3.ice_context(null)));
                    thread.Start();
                    thread.Join();
                }
                ic.destroy();
            }
        }

        {
            p.opIdempotent();
        }

        {
            p.opNonmutating();
        }
    }
}
