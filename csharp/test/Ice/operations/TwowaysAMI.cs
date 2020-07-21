//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading;
using System.Threading.Tasks;
using Test;

namespace ZeroC.Ice.Test.Operations
{
    public class TwowaysAMI
    {
        private class CallbackBase
        {
            private bool _called = false;
            private readonly object _mutex = new object();

            public virtual void Check()
            {
                lock (_mutex)
                {
                    while (!_called)
                    {
                        Monitor.Wait(_mutex);
                    }
                    _called = false;
                }
            }

            public virtual void Called()
            {
                lock (_mutex)
                {
                    TestHelper.Assert(!_called);
                    _called = true;
                    Monitor.Pulse(_mutex);
                }
            }
        }

        private class GenericCallback<T> : CallbackBase
        {
            public T Value { get; private set; }

            private bool _succeeded = false;

            public GenericCallback(T value) => Value = value;

            public void Response(T value)
            {
                Value = value;
                _succeeded = true;
                Called();
            }

            public void Exception()
            {
                _succeeded = false;
                Called();
            }

            public bool Succeeded()
            {
                Check();
                return _succeeded;
            }
        }

        private class Callback : CallbackBase
        {
            public Callback()
            {
            }

            public Callback(Communicator c) => _communicator = c;

            public Callback(int l) => _l = l;

            public Callback(Dictionary<string, string> d) => _d = d;

            public void opVoid() => Called();

            public void opContext() => Called();

            public void opByte(byte r, byte b)
            {
                TestHelper.Assert(b == 0xf0);
                TestHelper.Assert(r == 0xff);
                Called();
            }

            public void opBool(bool r, bool b)
            {
                TestHelper.Assert(b);
                TestHelper.Assert(!r);
                Called();
            }

            public void opShortIntLong(long r, short s, int i, long l)
            {
                TestHelper.Assert(s == 10);
                TestHelper.Assert(i == 11);
                TestHelper.Assert(l == 12);
                TestHelper.Assert(r == 12);
                Called();
            }

            public void opUShortUIntULong(ulong r, ushort s, uint i, ulong l)
            {
                TestHelper.Assert(s == 10);
                TestHelper.Assert(i == 11);
                TestHelper.Assert(l == 12);
                TestHelper.Assert(r == 12);
                Called();
            }

            public void opFloatDouble(double r, float f, double d)
            {
                TestHelper.Assert(f == 3.14f);
                TestHelper.Assert(d == 1.1e10);
                TestHelper.Assert(r == 1.1e10);
                Called();
            }

            public void opString(string r, string s)
            {
                TestHelper.Assert(s.Equals("world hello"));
                TestHelper.Assert(r.Equals("hello world"));
                Called();
            }

            public void opMyEnum(MyEnum r, MyEnum e)
            {
                TestHelper.Assert(e == MyEnum.enum2);
                TestHelper.Assert(r == MyEnum.enum3);
                Called();
            }

            public void opMyClass(IMyClassPrx? r, IMyClassPrx? c1, IMyClassPrx? c2)
            {
                TestHelper.Assert(c1!.Identity.Equals(Identity.Parse("test")));
                TestHelper.Assert(c2!.Identity.Equals(Identity.Parse("noSuchIdentity")));
                TestHelper.Assert(r!.Identity.Equals(Identity.Parse("test")));
                Called();
            }

            public void opStruct(Structure rso, Structure so)
            {
                TestHelper.Assert(rso.P == null);
                TestHelper.Assert(rso.E == MyEnum.enum2);
                TestHelper.Assert(rso.S.S.Equals("def"));
                TestHelper.Assert(so.E == MyEnum.enum3);
                TestHelper.Assert(so.S.S.Equals("a new string"));
                Called();
            }

            public void opByteS(byte[] rso, byte[] bso)
            {
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
                Called();
            }

            public void opBoolS(bool[] rso, bool[] bso)
            {
                TestHelper.Assert(bso.Length == 4);
                TestHelper.Assert(bso[0]);
                TestHelper.Assert(bso[1]);
                TestHelper.Assert(!bso[2]);
                TestHelper.Assert(!bso[3]);
                TestHelper.Assert(rso.Length == 3);
                TestHelper.Assert(!rso[0]);
                TestHelper.Assert(rso[1]);
                TestHelper.Assert(rso[2]);
                Called();
            }

            public void opShortIntLongS(long[] rso, short[] sso, int[] iso, long[] lso)
            {
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
                Called();
            }

            public void opFloatDoubleS(double[] rso, float[] fso, double[] dso)
            {
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
                Called();
            }

            public void opStringS(string[] rso, string[] sso)
            {
                TestHelper.Assert(sso.Length == 4);
                TestHelper.Assert(sso[0].Equals("abc"));
                TestHelper.Assert(sso[1].Equals("de"));
                TestHelper.Assert(sso[2].Equals("fghi"));
                TestHelper.Assert(sso[3].Equals("xyz"));
                TestHelper.Assert(rso.Length == 3);
                TestHelper.Assert(rso[0].Equals("fghi"));
                TestHelper.Assert(rso[1].Equals("de"));
                TestHelper.Assert(rso[2].Equals("abc"));
                Called();
            }

            public void opByteSS(byte[][] rso, byte[][] bso)
            {
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
                Called();
            }

            public void opBoolSS(bool[][] rso, bool[][] bso)
            {
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
                Called();
            }

            public void opShortIntLongSS(long[][] rso, short[][] sso, int[][] iso, long[][] lso)
            {
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
                Called();
            }

            public void opFloatDoubleSS(double[][] rso, float[][] fso, double[][] dso)
            {
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
                Called();
            }

            public void opStringSS(string[][] rso, string[][] sso)
            {
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
                Called();
            }

            public void opStringSSS(string[][][] rsso, string[][][] ssso)
            {
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
                Called();
            }

            public void opByteBoolD(Dictionary<byte, bool> ro, Dictionary<byte, bool> _do)
            {
                var di1 = new Dictionary<byte, bool>();
                di1[10] = true;
                di1[100] = false;
                TestHelper.Assert(_do.DictionaryEqual(di1));
                TestHelper.Assert(ro.Count == 4);
                TestHelper.Assert(ro[10] == true);
                TestHelper.Assert(ro[11] == false);
                TestHelper.Assert(ro[100] == false);
                TestHelper.Assert(ro[101] == true);
                Called();
            }

            public void opShortIntD(Dictionary<short, int> ro, Dictionary<short, int> _do)
            {
                var di1 = new Dictionary<short, int>();
                di1[110] = -1;
                di1[1100] = 123123;
                TestHelper.Assert(_do.DictionaryEqual(di1));
                TestHelper.Assert(ro.Count == 4);
                TestHelper.Assert(ro[110] == -1);
                TestHelper.Assert(ro[111] == -100);
                TestHelper.Assert(ro[1100] == 123123);
                TestHelper.Assert(ro[1101] == 0);
                Called();
            }

            public void opLongFloatD(Dictionary<long, float> ro, Dictionary<long, float> _do)
            {
                Dictionary<long, float> di1 = new Dictionary<long, float>();
                di1[999999110L] = -1.1f;
                di1[999999111L] = 123123.2f;
                TestHelper.Assert(_do.DictionaryEqual(di1));
                TestHelper.Assert(ro.Count == 4);
                TestHelper.Assert(ro[999999110L] == -1.1f);
                TestHelper.Assert(ro[999999120L] == -100.4f);
                TestHelper.Assert(ro[999999111L] == 123123.2f);
                TestHelper.Assert(ro[999999130L] == 0.5f);
                Called();
            }

            public void opStringStringD(Dictionary<string, string> ro, Dictionary<string, string> _do)
            {
                Dictionary<string, string> di1 = new Dictionary<string, string>();
                di1["foo"] = "abc -1.1";
                di1["bar"] = "abc 123123.2";
                TestHelper.Assert(_do.DictionaryEqual(di1));
                TestHelper.Assert(ro.Count == 4);
                TestHelper.Assert(ro["foo"].Equals("abc -1.1"));
                TestHelper.Assert(ro["FOO"].Equals("abc -100.4"));
                TestHelper.Assert(ro["bar"].Equals("abc 123123.2"));
                TestHelper.Assert(ro["BAR"].Equals("abc 0.5"));
                Called();
            }

            public void opStringMyEnumD(Dictionary<string, MyEnum> ro, Dictionary<string, MyEnum> _do)
            {
                var di1 = new Dictionary<string, MyEnum>();
                di1["abc"] = MyEnum.enum1;
                di1[""] = MyEnum.enum2;
                TestHelper.Assert(_do.DictionaryEqual(di1));
                TestHelper.Assert(ro.Count == 4);
                TestHelper.Assert(ro["abc"] == MyEnum.enum1);
                TestHelper.Assert(ro["qwerty"] == MyEnum.enum3);
                TestHelper.Assert(ro[""] == MyEnum.enum2);
                TestHelper.Assert(ro["Hello!!"] == MyEnum.enum2);
                Called();
            }

            public void opMyEnumStringD(Dictionary<MyEnum, string> ro, Dictionary<MyEnum, string> _do)
            {
                var di1 = new Dictionary<MyEnum, string>();
                di1[MyEnum.enum1] = "abc";
                TestHelper.Assert(_do.DictionaryEqual(di1));
                TestHelper.Assert(ro.Count == 3);
                TestHelper.Assert(ro[MyEnum.enum1].Equals("abc"));
                TestHelper.Assert(ro[MyEnum.enum2].Equals("Hello!!"));
                TestHelper.Assert(ro[MyEnum.enum3].Equals("qwerty"));
                Called();
            }

            public void opMyStructMyEnumD(Dictionary<MyStruct, MyEnum> ro,
                                            Dictionary<MyStruct, MyEnum> _do)
            {
                var s11 = new MyStruct(1, 1);
                var s12 = new MyStruct(1, 2);
                var di1 = new Dictionary<MyStruct, MyEnum>();
                di1[s11] = MyEnum.enum1;
                di1[s12] = MyEnum.enum2;
                TestHelper.Assert(_do.DictionaryEqual(di1));
                var s22 = new MyStruct(2, 2);
                var s23 = new MyStruct(2, 3);
                TestHelper.Assert(ro.Count == 4);
                TestHelper.Assert(ro[s11] == MyEnum.enum1);
                TestHelper.Assert(ro[s12] == MyEnum.enum2);
                TestHelper.Assert(ro[s22] == MyEnum.enum3);
                TestHelper.Assert(ro[s23] == MyEnum.enum2);
                Called();
            }

            public void opByteBoolDS(Dictionary<byte, bool>[] ro,
                                        Dictionary<byte, bool>[] _do)
            {
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
                Called();
            }

            public void opShortIntDS(Dictionary<short, int>[] ro,
                                        Dictionary<short, int>[] _do)
            {
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
                Called();
            }

            public void opLongFloatDS(Dictionary<long, float>[] ro,
                                        Dictionary<long, float>[] _do)
            {
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
                Called();
            }

            public void opStringStringDS(Dictionary<string, string>[] ro,
                                            Dictionary<string, string>[] _do)
            {
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
                Called();
            }

            public void opStringMyEnumDS(Dictionary<string, MyEnum>[] ro,
                                            Dictionary<string, MyEnum>[] _do)
            {
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
                Called();
            }

            public void opMyEnumStringDS(Dictionary<MyEnum, string>[] ro,
                                            Dictionary<MyEnum, string>[] _do)
            {
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
                Called();
            }

            public void opMyStructMyEnumDS(Dictionary<MyStruct, MyEnum>[] ro,
                                            Dictionary<MyStruct, MyEnum>[] _do)
            {
                var s11 = new MyStruct(1, 1);
                var s12 = new MyStruct(1, 2);
                var s22 = new MyStruct(2, 2);
                var s23 = new MyStruct(2, 3);

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
                Called();
            }

            public void opByteByteSD(Dictionary<byte, byte[]> ro,
                                        Dictionary<byte, byte[]> _do)
            {
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
                Called();
            }

            public void opBoolBoolSD(Dictionary<bool, bool[]> ro,
                                        Dictionary<bool, bool[]> _do)
            {
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
                Called();
            }

            public void opShortShortSD(Dictionary<short, short[]> ro,
                                        Dictionary<short, short[]> _do)
            {
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
                Called();
            }

            public void opIntIntSD(Dictionary<int, int[]> ro,
                                    Dictionary<int, int[]> _do)
            {
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
                Called();
            }

            public void opLongLongSD(Dictionary<long, long[]> ro,
                                        Dictionary<long, long[]> _do)
            {
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
                Called();
            }

            public void opStringFloatSD(Dictionary<string, float[]> ro,
                                        Dictionary<string, float[]> _do)
            {
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
                Called();
            }

            public void opStringDoubleSD(Dictionary<string, double[]> ro,
                                            Dictionary<string, double[]> _do)
            {
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
                Called();
            }

            public void opStringStringSD(Dictionary<string, string[]> ro,
                                            Dictionary<string, string[]> _do)
            {
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
                Called();
            }

            public void opMyEnumMyEnumSD(Dictionary<MyEnum, MyEnum[]> ro,
                                            Dictionary<MyEnum, MyEnum[]> _do)
            {
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
                Called();
            }

            public void opIntS(int[] r)
            {
                TestHelper.Assert(r.Length == _l);
                for (int j = 0; j < r.Length; ++j)
                {
                    TestHelper.Assert(r[j] == -j);
                }
                Called();
            }

            public void opContextNotEqual(Dictionary<string, string> r)
            {
                TestHelper.Assert(!r.DictionaryEqual(_d));
                Called();
            }

            public void opContextEqual(Dictionary<string, string> r)
            {
                TestHelper.Assert(r.DictionaryEqual(_d));
                Called();
            }

            private Communicator? _communicator;
            private readonly int _l;
            private readonly Dictionary<string, string>? _d;
        }

        internal static void twowaysAMI(TestHelper helper, IMyClassPrx p)
        {
            Communicator? communicator = helper.Communicator();
            TestHelper.Assert(communicator != null);

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
                var ret = p.OpByteAsync(0xff, 0x0f).Result;
                TestHelper.Assert(ret.p3 == 0xf0);
                TestHelper.Assert(ret.ReturnValue == 0xff);
            }

            {
                var cb = new Callback();
                var ret = p.OpBoolAsync(true, false).Result;
                cb.opBool(ret.ReturnValue, ret.p3);
            }

            {
                var cb = new Callback();
                var ret = p.OpShortIntLongAsync(10, 11, 12).Result;
                cb.opShortIntLong(ret.ReturnValue, ret.p4, ret.p5, ret.p6);
            }

            {
                var cb = new Callback();
                var ret = p.OpUShortUIntULongAsync(10, 11, 12).Result;
                cb.opUShortUIntULong(ret.ReturnValue, ret.p4, ret.p5, ret.p6);
            }

            {
                var cb = new Callback();
                var ret = p.OpFloatDoubleAsync(3.14f, 1.1E10).Result;
                cb.opFloatDouble(ret.ReturnValue, ret.p3, ret.p4);
            }

            {
                var cb = new Callback();
                var ret = p.OpStringAsync("hello", "world").Result;
                cb.opString(ret.ReturnValue, ret.p3);
            }

            {
                var cb = new Callback();
                var ret = p.OpMyEnumAsync(MyEnum.enum2).Result;
                cb.opMyEnum(ret.ReturnValue, ret.p2);
            }

            {
                var cb = new Callback(communicator);
                var ret = p.OpMyClassAsync(p).Result;
                cb.opMyClass(ret.ReturnValue, ret.p2, ret.p3);
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

                var cb = new Callback(communicator);
                var ret = p.OpStructAsync(si1, si2).Result;
                cb.opStruct(ret.ReturnValue, ret.p3);
            }

            {
                byte[] bsi1 = new byte[] { 0x01, 0x11, 0x12, 0x22 };
                byte[] bsi2 = new byte[] { 0xf1, 0xf2, 0xf3, 0xf4 };

                var cb = new Callback();
                var ret = p.OpByteSAsync(bsi1, bsi2).Result;
                cb.opByteS(ret.ReturnValue, ret.p3);
            }

            {
                bool[] bsi1 = new bool[] { true, true, false };
                bool[] bsi2 = new bool[] { false };

                var cb = new Callback();
                var ret = p.OpBoolSAsync(bsi1, bsi2).Result;
                cb.opBoolS(ret.ReturnValue, ret.p3);
            }

            {
                short[] ssi = new short[] { 1, 2, 3 };
                int[] isi = new int[] { 5, 6, 7, 8 };
                long[] lsi = new long[] { 10, 30, 20 };

                var cb = new Callback();
                var ret = p.OpShortIntLongSAsync(ssi, isi, lsi).Result;
                cb.opShortIntLongS(ret.ReturnValue, ret.p4, ret.p5, ret.p6);
            }

            {
                float[] fsi = new float[] { 3.14f, 1.11f };
                double[] dsi = new double[] { 1.1e10, 1.2e10, 1.3e10 };

                var cb = new Callback();
                var ret = p.OpFloatDoubleSAsync(fsi, dsi).Result;
                cb.opFloatDoubleS(ret.ReturnValue, ret.p3, ret.p4);
            }

            {
                string[] ssi1 = new string[] { "abc", "de", "fghi" };
                string[] ssi2 = new string[] { "xyz" };

                var cb = new Callback();
                var ret = p.OpStringSAsync(ssi1, ssi2).Result;
                cb.opStringS(ret.ReturnValue, ret.p3);
            }

            {
                byte[] s11 = new byte[] { 0x01, 0x11, 0x12 };
                byte[] s12 = new byte[] { 0xff };
                byte[][] bsi1 = new byte[][] { s11, s12 };

                byte[] s21 = new byte[] { 0x0e };
                byte[] s22 = new byte[] { 0xf2, 0xf1 };
                byte[][] bsi2 = new byte[][] { s21, s22 };

                var cb = new Callback();
                var ret = p.OpByteSSAsync(bsi1, bsi2).Result;
                cb.opByteSS(ret.ReturnValue, ret.p3);
            }

            {
                bool[] s11 = new bool[] { true };
                bool[] s12 = new bool[] { false };
                bool[] s13 = new bool[] { true, true };
                bool[][] bsi1 = new bool[][] { s11, s12, s13 };

                bool[] s21 = new bool[] { false, false, true };
                bool[][] bsi2 = new bool[][] { s21 };

                var cb = new Callback();
                var ret = p.OpBoolSSAsync(bsi1, bsi2).Result;
                cb.opBoolSS(ret.ReturnValue, ret.p3);
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

                var cb = new Callback();
                var ret = p.OpShortIntLongSSAsync(ssi, isi, lsi).Result;
                cb.opShortIntLongSS(ret.ReturnValue, ret.p4, ret.p5, ret.p6);
            }

            {
                float[] f11 = new float[] { 3.14f };
                float[] f12 = new float[] { 1.11f };
                float[] f13 = Array.Empty<float>();
                float[][] fsi = new float[][] { f11, f12, f13 };

                double[] d11 = new double[] { 1.1e10, 1.2e10, 1.3e10 };
                double[][] dsi = new double[][] { d11 };

                var cb = new Callback();
                var ret = p.OpFloatDoubleSSAsync(fsi, dsi).Result;
                cb.opFloatDoubleSS(ret.ReturnValue, ret.p3, ret.p4);
            }

            {
                string[] s11 = new string[] { "abc" };
                string[] s12 = new string[] { "de", "fghi" };
                string[][] ssi1 = new string[][] { s11, s12 };

                string[] s21 = Array.Empty<string>();
                string[] s22 = Array.Empty<string>();
                string[] s23 = new string[] { "xyz" };
                string[][] ssi2 = new string[][] { s21, s22, s23 };

                var cb = new Callback();
                var ret = p.OpStringSSAsync(ssi1, ssi2).Result;
                cb.opStringSS(ret.ReturnValue, ret.p3);
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

                var cb = new Callback();
                var ret = p.OpStringSSSAsync(sssi1, sssi2).Result;
                cb.opStringSSS(ret.ReturnValue, ret.p3);
            }

            {
                Dictionary<byte, bool> di1 = new Dictionary<byte, bool>();
                di1[10] = true;
                di1[100] = false;
                Dictionary<byte, bool> di2 = new Dictionary<byte, bool>();
                di2[10] = true;
                di2[11] = false;
                di2[101] = true;

                var cb = new Callback();
                var ret = p.OpByteBoolDAsync(di1, di2).Result;
                cb.opByteBoolD(ret.ReturnValue, ret.p3);
            }

            {
                Dictionary<short, int> di1 = new Dictionary<short, int>();
                di1[110] = -1;
                di1[1100] = 123123;
                Dictionary<short, int> di2 = new Dictionary<short, int>();
                di2[110] = -1;
                di2[111] = -100;
                di2[1101] = 0;

                var cb = new Callback();
                var ret = p.OpShortIntDAsync(di1, di2).Result;
                cb.opShortIntD(ret.ReturnValue, ret.p3);
            }

            {
                Dictionary<long, float> di1 = new Dictionary<long, float>();
                di1[999999110L] = -1.1f;
                di1[999999111L] = 123123.2f;
                Dictionary<long, float> di2 = new Dictionary<long, float>();
                di2[999999110L] = -1.1f;
                di2[999999120L] = -100.4f;
                di2[999999130L] = 0.5f;

                var cb = new Callback();
                var ret = p.OpLongFloatDAsync(di1, di2).Result;
                cb.opLongFloatD(ret.ReturnValue, ret.p3);
            }

            {
                Dictionary<string, string> di1 = new Dictionary<string, string>();
                di1["foo"] = "abc -1.1";
                di1["bar"] = "abc 123123.2";
                Dictionary<string, string> di2 = new Dictionary<string, string>();
                di2["foo"] = "abc -1.1";
                di2["FOO"] = "abc -100.4";
                di2["BAR"] = "abc 0.5";

                var cb = new Callback();
                var ret = p.OpStringStringDAsync(di1, di2).Result;
                cb.opStringStringD(ret.ReturnValue, ret.p3);
            }

            {
                var di1 = new Dictionary<string, MyEnum>();
                di1["abc"] = MyEnum.enum1;
                di1[""] = MyEnum.enum2;
                var di2 = new Dictionary<string, MyEnum>();
                di2["abc"] = MyEnum.enum1;
                di2["qwerty"] = MyEnum.enum3;
                di2["Hello!!"] = MyEnum.enum2;

                var cb = new Callback();
                var ret = p.OpStringMyEnumDAsync(di1, di2).Result;
                cb.opStringMyEnumD(ret.ReturnValue, ret.p3);
            }

            {
                var di1 = new Dictionary<MyEnum, string>();
                di1[MyEnum.enum1] = "abc";
                var di2 = new Dictionary<MyEnum, string>();
                di2[MyEnum.enum2] = "Hello!!";
                di2[MyEnum.enum3] = "qwerty";

                var cb = new Callback();
                var ret = p.OpMyEnumStringDAsync(di1, di2).Result;
                cb.opMyEnumStringD(ret.ReturnValue, ret.p3);
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

                var cb = new Callback();
                var ret = p.OpMyStructMyEnumDAsync(di1, di2).Result;
                cb.opMyStructMyEnumD(ret.ReturnValue, ret.p3);
            }

            {
                Dictionary<byte, bool>[] dsi1 = new Dictionary<byte, bool>[2];
                Dictionary<byte, bool>[] dsi2 = new Dictionary<byte, bool>[1];

                Dictionary<byte, bool> di1 = new Dictionary<byte, bool>();
                di1[10] = true;
                di1[100] = false;
                Dictionary<byte, bool> di2 = new Dictionary<byte, bool>();
                di2[10] = true;
                di2[11] = false;
                di2[101] = true;
                Dictionary<byte, bool> di3 = new Dictionary<byte, bool>();
                di3[100] = false;
                di3[101] = false;

                dsi1[0] = di1;
                dsi1[1] = di2;
                dsi2[0] = di3;

                var cb = new Callback();
                var ret = p.OpByteBoolDSAsync(dsi1, dsi2).Result;
                cb.opByteBoolDS(ret.ReturnValue, ret.p3);
            }

            {
                Dictionary<long, float>[] dsi1 = new Dictionary<long, float>[2];
                Dictionary<long, float>[] dsi2 = new Dictionary<long, float>[1];

                Dictionary<long, float> di1 = new Dictionary<long, float>();
                di1[999999110L] = -1.1f;
                di1[999999111L] = 123123.2f;
                Dictionary<long, float> di2 = new Dictionary<long, float>();
                di2[999999110L] = -1.1f;
                di2[999999120L] = -100.4f;
                di2[999999130L] = 0.5f;
                Dictionary<long, float> di3 = new Dictionary<long, float>();
                di3[999999140L] = 3.14f;

                dsi1[0] = di1;
                dsi1[1] = di2;
                dsi2[0] = di3;

                var cb = new Callback();
                var ret = p.OpLongFloatDSAsync(dsi1, dsi2).Result;
                cb.opLongFloatDS(ret.ReturnValue, ret.p3);
            }

            {
                Dictionary<string, string>[] dsi1 = new Dictionary<string, string>[2];
                Dictionary<string, string>[] dsi2 = new Dictionary<string, string>[1];

                Dictionary<string, string> di1 = new Dictionary<string, string>();
                di1["foo"] = "abc -1.1";
                di1["bar"] = "abc 123123.2";
                Dictionary<string, string> di2 = new Dictionary<string, string>();
                di2["foo"] = "abc -1.1";
                di2["FOO"] = "abc -100.4";
                di2["BAR"] = "abc 0.5";
                Dictionary<string, string> di3 = new Dictionary<string, string>();
                di3["f00"] = "ABC -3.14";

                dsi1[0] = di1;
                dsi1[1] = di2;
                dsi2[0] = di3;

                var cb = new Callback();
                var ret = p.OpStringStringDSAsync(dsi1, dsi2).Result;
                cb.opStringStringDS(ret.ReturnValue, ret.p3);
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

                var cb = new Callback();
                var ret = p.OpStringMyEnumDSAsync(dsi1, dsi2).Result;
                cb.opStringMyEnumDS(ret.ReturnValue, ret.p3);
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

                var cb = new Callback();
                var ret = p.OpMyEnumStringDSAsync(dsi1, dsi2).Result;
                cb.opMyEnumStringDS(ret.ReturnValue, ret.p3);
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

                var cb = new Callback();
                var ret = p.OpMyStructMyEnumDSAsync(dsi1, dsi2).Result;
                cb.opMyStructMyEnumDS(ret.ReturnValue, ret.p3);
            }

            {
                Dictionary<byte, byte[]> sdi1 = new Dictionary<byte, byte[]>();
                Dictionary<byte, byte[]> sdi2 = new Dictionary<byte, byte[]>();

                byte[] si1 = new byte[] { 0x01, 0x11 };
                byte[] si2 = new byte[] { 0x12 };
                byte[] si3 = new byte[] { 0xf2, 0xf3 };

                sdi1[0x01] = si1;
                sdi1[0x22] = si2;
                sdi2[0xf1] = si3;

                var cb = new Callback();
                var ret = p.OpByteByteSDAsync(sdi1, sdi2).Result;
                cb.opByteByteSD(ret.ReturnValue, ret.p3);
            }

            {
                Dictionary<bool, bool[]> sdi1 = new Dictionary<bool, bool[]>();
                Dictionary<bool, bool[]> sdi2 = new Dictionary<bool, bool[]>();

                bool[] si1 = new bool[] { true, false };
                bool[] si2 = new bool[] { false, true, true };

                sdi1[false] = si1;
                sdi1[true] = si2;
                sdi2[false] = si1;

                var cb = new Callback();
                var ret = p.OpBoolBoolSDAsync(sdi1, sdi2).Result;
                cb.opBoolBoolSD(ret.ReturnValue, ret.p3);
            }

            {
                Dictionary<short, short[]> sdi1 = new Dictionary<short, short[]>();
                Dictionary<short, short[]> sdi2 = new Dictionary<short, short[]>();

                short[] si1 = new short[] { 1, 2, 3 };
                short[] si2 = new short[] { 4, 5 };
                short[] si3 = new short[] { 6, 7 };

                sdi1[1] = si1;
                sdi1[2] = si2;
                sdi2[4] = si3;

                var cb = new Callback();
                var ret = p.OpShortShortSDAsync(sdi1, sdi2).Result;
                cb.opShortShortSD(ret.ReturnValue, ret.p3);
            }

            {
                Dictionary<int, int[]> sdi1 = new Dictionary<int, int[]>();
                Dictionary<int, int[]> sdi2 = new Dictionary<int, int[]>();

                int[] si1 = new int[] { 100, 200, 300 };
                int[] si2 = new int[] { 400, 500 };
                int[] si3 = new int[] { 600, 700 };

                sdi1[100] = si1;
                sdi1[200] = si2;
                sdi2[400] = si3;

                var cb = new Callback();
                var ret = p.OpIntIntSDAsync(sdi1, sdi2).Result;
                cb.opIntIntSD(ret.ReturnValue, ret.p3);
            }

            {
                Dictionary<long, long[]> sdi1 = new Dictionary<long, long[]>();
                Dictionary<long, long[]> sdi2 = new Dictionary<long, long[]>();

                long[] si1 = new long[] { 999999110L, 999999111L, 999999110L };
                long[] si2 = new long[] { 999999120L, 999999130L };
                long[] si3 = new long[] { 999999110L, 999999120L };

                sdi1[999999990L] = si1;
                sdi1[999999991L] = si2;
                sdi2[999999992L] = si3;

                var cb = new Callback();
                var ret = p.OpLongLongSDAsync(sdi1, sdi2).Result;
                cb.opLongLongSD(ret.ReturnValue, ret.p3);
            }

            {
                Dictionary<string, float[]> sdi1 = new Dictionary<string, float[]>();
                Dictionary<string, float[]> sdi2 = new Dictionary<string, float[]>();

                float[] si1 = new float[] { -1.1f, 123123.2f, 100.0f };
                float[] si2 = new float[] { 42.24f, -1.61f };
                float[] si3 = new float[] { -3.14f, 3.14f };

                sdi1["abc"] = si1;
                sdi1["ABC"] = si2;
                sdi2["aBc"] = si3;

                var cb = new Callback();
                var ret = p.OpStringFloatSDAsync(sdi1, sdi2).Result;
                cb.opStringFloatSD(ret.ReturnValue, ret.p3);
            }

            {
                Dictionary<string, double[]> sdi1 = new Dictionary<string, double[]>();
                Dictionary<string, double[]> sdi2 = new Dictionary<string, double[]>();

                double[] si1 = new double[] { 1.1E10, 1.2E10, 1.3E10 };
                double[] si2 = new double[] { 1.4E10, 1.5E10 };
                double[] si3 = new double[] { 1.6E10, 1.7E10 };

                sdi1["Hello!!"] = si1;
                sdi1["Goodbye"] = si2;
                sdi2[""] = si3;

                var cb = new Callback();
                var ret = p.OpStringDoubleSDAsync(sdi1, sdi2).Result;
                cb.opStringDoubleSD(ret.ReturnValue, ret.p3);
            }

            {
                Dictionary<string, string[]> sdi1 = new Dictionary<string, string[]>();
                Dictionary<string, string[]> sdi2 = new Dictionary<string, string[]>();

                string[] si1 = new string[] { "abc", "de", "fghi" };
                string[] si2 = new string[] { "xyz", "or" };
                string[] si3 = new string[] { "and", "xor" };

                sdi1["abc"] = si1;
                sdi1["def"] = si2;
                sdi2["ghi"] = si3;

                var cb = new Callback();
                var ret = p.OpStringStringSDAsync(sdi1, sdi2).Result;
                cb.opStringStringSD(ret.ReturnValue, ret.p3);
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

                var cb = new Callback();
                var ret = p.OpMyEnumMyEnumSDAsync(sdi1, sdi2).Result;
                cb.opMyEnumMyEnumSD(ret.ReturnValue, ret.p3);
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

                    var cb = new Callback(lengths[l]);
                    cb.opIntS(p.OpIntSAsync(s).Result);
                }
            }

            {
                Dictionary<string, string> ctx = new Dictionary<string, string>();
                ctx["one"] = "ONE";
                ctx["two"] = "TWO";
                ctx["three"] = "THREE";
                {
                    TestHelper.Assert(p.Context.Count == 0);
                    var cb = new Callback(ctx);
                    cb.opContextNotEqual(p.OpContextAsync().Result);
                }
                {
                    TestHelper.Assert(p.Context.Count == 0);
                    var cb = new Callback(ctx);
                    cb.opContextEqual(p.OpContextAsync(ctx).Result);
                }
                {
                    var p2 = p.Clone(context: ctx);
                    TestHelper.Assert(p2.Context.DictionaryEqual(ctx));
                    var cb = new Callback(ctx);
                    cb.opContextEqual(p2.OpContextAsync().Result);
                }
                {
                    var p2 = p.Clone(context: ctx);
                    Callback cb = new Callback(ctx);
                    cb.opContextEqual(p2.OpContextAsync(ctx).Result);
                }
            }

            //
            // Test implicit context propagation with async task
            //
            if (p.GetConnection() != null)
            {
                communicator.CurrentContext["one"] = "ONE";
                communicator.CurrentContext["two"] = "TWO";
                communicator.CurrentContext["three"] = "THREE";

                var p3 = IMyClassPrx.Parse($"test:{helper.GetTestEndpoint(0)}", communicator);
                TestHelper.Assert(p3.OpContextAsync().Result.DictionaryEqual(communicator.CurrentContext));

                Dictionary<string, string> prxContext = new Dictionary<string, string>();
                prxContext["one"] = "UN";
                prxContext["four"] = "QUATRE";

                Dictionary<string, string> combined = new Dictionary<string, string>(prxContext);
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

                p3 = IMyClassPrx.Parse($"test:{helper.GetTestEndpoint(0)}", communicator);

                var ctx = new Dictionary<string, string>(communicator.CurrentContext);
                communicator.CurrentContext.Clear();
                TestHelper.Assert(p3.OpContextAsync().Result.DictionaryEqual(prxContext));

                communicator.CurrentContext = ctx;
                TestHelper.Assert(p3.OpContextAsync().Result.DictionaryEqual(combined));

                // Cleanup
                communicator.CurrentContext.Clear();
                communicator.DefaultContext = new Dictionary<string, string>();
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
                var derived = IMyDerivedClassPrx.CheckedCast(p);
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

            Func<Task> task = async () =>
            {
                {
                    var p1 = await p.OpMStruct1Async();

                    p1.E = MyEnum.enum3;
                    var r = await p.OpMStruct2Async(p1);
                    TestHelper.Assert(r.p2.Equals(p1) && r.ReturnValue.Equals(p1));
                }

                {
                    await p.OpMSeq1Async();

                    var p1 = new string[1];
                    p1[0] = "test";
                    var r = await p.OpMSeq2Async(p1);
                    TestHelper.Assert(r.p2.SequenceEqual(p1) && r.ReturnValue.SequenceEqual(p1));
                }

                {
                    await p.OpMDict1Async();

                    var p1 = new Dictionary<string, string>();
                    p1["test"] = "test";
                    var r = await p.OpMDict2Async(p1);
                    TestHelper.Assert(r.p2.DictionaryEqual(p1) && r.ReturnValue.DictionaryEqual(p1));
                }
            };
        }
    }
}
