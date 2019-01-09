// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

using System;
using System.Diagnostics;
using System.Threading;
using System.Threading.Tasks;
using System.Collections.Generic;

namespace Ice
{
    namespace operations
    {
        public class TwowaysAMI
        {
            private static void test(bool b)
            {
                if (!b)
                {
                    throw new System.Exception();
                }
            }

            private class CallbackBase
            {
                internal CallbackBase()
                {
                    _called = false;
                }

                public virtual void check()
                {
                    lock (this)
                    {
                        while (!_called)
                        {
                            Monitor.Wait(this);
                        }
                        _called = false;
                    }
                }

                public virtual void called()
                {
                    lock (this)
                    {
                        Debug.Assert(!_called);
                        _called = true;
                        Monitor.Pulse(this);
                    }
                }

                public bool _called;
            }

            private class GenericCallback<T> : CallbackBase
            {
                public GenericCallback(T value)
                {
                    this._value = value;
                }

                public void response(T value)
                {
                    _value = value;
                    _succeeded = true;
                    called();
                }

                public void exception(Ice.Exception ex)
                {
                    _succeeded = false;
                    called();
                }

                public bool succeeded()
                {
                    check();
                    return _succeeded;
                }

                public T value()
                {
                    return _value;
                }

                private T _value;
                private bool _succeeded = false;
            }

            private class Callback : CallbackBase
            {
                public Callback()
                {
                }

                public Callback(Ice.Communicator c)
                {
                    _communicator = c;
                }

                public Callback(int l)
                {
                    _l = l;
                }

                public Callback(Dictionary<string, string> d)
                {
                    _d = d;
                }

                public void ice_ping()
                {
                    called();
                }

                public void ice_isA(bool r)
                {
                    test(r);
                    called();
                }

                public void ice_ids(string[] ids)
                {
                    test(ids.Length == 3);
                    called();
                }

                public void ice_id(string id)
                {
                    test(id.Equals(Test.MyDerivedClassDisp_.ice_staticId()));
                    called();
                }

                public void opVoid()
                {
                    called();
                }

                public void opContext()
                {
                    called();
                }

                public void opByte(byte r, byte b)
                {
                    test(b == 0xf0);
                    test(r == 0xff);
                    called();
                }

                public void opBool(bool r, bool b)
                {
                    test(b);
                    test(!r);
                    called();
                }

                public void opShortIntLong(long r, short s, int i, long l)
                {
                    test(s == 10);
                    test(i == 11);
                    test(l == 12);
                    test(r == 12);
                    called();
                }

                public void opFloatDouble(double r, float f, double d)
                {
                    test(f == 3.14f);
                    test(d == 1.1e10);
                    test(r == 1.1e10);
                    called();
                }

                public void opString(string r, string s)
                {
                    test(s.Equals("world hello"));
                    test(r.Equals("hello world"));
                    called();
                }

                public void opMyEnum(Test.MyEnum r, Test.MyEnum e)
                {
                    test(e == Test.MyEnum.enum2);
                    test(r == Test.MyEnum.enum3);
                    called();
                }

                public void opMyClass(Test.MyClassPrx r, Test.MyClassPrx c1, Test.MyClassPrx c2)
                {
                    test(c1.ice_getIdentity().Equals(Ice.Util.stringToIdentity("test")));
                    test(c2.ice_getIdentity().Equals(Ice.Util.stringToIdentity("noSuchIdentity")));
                    test(r.ice_getIdentity().Equals(Ice.Util.stringToIdentity("test")));

                    //
                    // We can't do the callbacks below in connection serialization mode.
                    //
                    if (_communicator.getProperties().getPropertyAsInt("Ice.ThreadPool.Client.Serialize") == 0)
                    {
                        r.opVoid();
                        c1.opVoid();
                        try
                        {
                            c2.opVoid();
                            test(false);
                        }
                        catch (Ice.ObjectNotExistException)
                        {
                        }
                    }
                    called();
                }

                public void opStruct(Test.Structure rso, Test.Structure so)
                {
                    test(rso.p == null);
                    test(rso.e == Test.MyEnum.enum2);
                    test(rso.s.s.Equals("def"));
                    test(so.e == Test.MyEnum.enum3);
                    test(so.s.s.Equals("a new string"));

                    //
                    // We can't do the callbacks below in connection serialization mode.
                    //
                    if (_communicator.getProperties().getPropertyAsInt("Ice.ThreadPool.Client.Serialize") == 0)
                    {
                        so.p.opVoid();
                    }
                    called();
                }

                public void opByteS(byte[] rso, byte[] bso)
                {
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
                    called();
                }

                public void opBoolS(bool[] rso, bool[] bso)
                {
                    test(bso.Length == 4);
                    test(bso[0]);
                    test(bso[1]);
                    test(!bso[2]);
                    test(!bso[3]);
                    test(rso.Length == 3);
                    test(!rso[0]);
                    test(rso[1]);
                    test(rso[2]);
                    called();
                }

                public void opShortIntLongS(long[] rso, short[] sso, int[] iso, long[] lso)
                {
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
                    called();
                }

                public void opFloatDoubleS(double[] rso, float[] fso, double[] dso)
                {
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
                    called();
                }

                public void opStringS(string[] rso, string[] sso)
                {
                    test(sso.Length == 4);
                    test(sso[0].Equals("abc"));
                    test(sso[1].Equals("de"));
                    test(sso[2].Equals("fghi"));
                    test(sso[3].Equals("xyz"));
                    test(rso.Length == 3);
                    test(rso[0].Equals("fghi"));
                    test(rso[1].Equals("de"));
                    test(rso[2].Equals("abc"));
                    called();
                }

                public void opByteSS(byte[][] rso, byte[][] bso)
                {
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
                    called();
                }

                public void opBoolSS(bool[][] rso, bool[][] bso)
                {
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
                    called();
                }

                public void opShortIntLongSS(long[][] rso, short[][] sso, int[][] iso, long[][] lso)
                {
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
                    called();
                }

                public void opFloatDoubleSS(double[][] rso, float[][] fso, double[][] dso)
                {
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
                    called();
                }

                public void opStringSS(string[][] rso, string[][] sso)
                {
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
                    called();
                }

                public void opStringSSS(string[][][] rsso, string[][][] ssso)
                {
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
                    called();
                }

                public void opByteBoolD(Dictionary<byte, bool> ro, Dictionary<byte, bool> _do)
                {
                    Dictionary<byte, bool> di1 = new Dictionary<byte, bool>();
                    di1[10] = true;
                    di1[100] = false;
                    test(Ice.CollectionComparer.Equals(_do, di1));
                    test(ro.Count == 4);
                    // test(ro[10] == true); // Disabled since new dictionary mapping.
                    test(ro[11] == false);
                    test(ro[100] == false);
                    test(ro[101] == true);
                    called();
                }

                public void opShortIntD(Dictionary<short, int> ro, Dictionary<short, int> _do)
                {
                    Dictionary<short, int> di1 = new Dictionary<short, int>();
                    di1[110] = -1;
                    di1[1100] = 123123;
                    test(Ice.CollectionComparer.Equals(_do, di1));
                    test(ro.Count == 4);
                    // test(ro[110] == -1); // Disabled since new dictionary mapping.
                    test(ro[111] == -100);
                    test(ro[1100] == 123123);
                    test(ro[1101] == 0);
                    called();
                }

                public void opLongFloatD(Dictionary<long, float> ro, Dictionary<long, float> _do)
                {
                    Dictionary<long, float> di1 = new Dictionary<long, float>();
                    di1[999999110L] = -1.1f;
                    di1[999999111L] = 123123.2f;
                    test(Ice.CollectionComparer.Equals(_do, di1));
                    test(ro.Count == 4);
                    test(ro[999999110L] == -1.1f);
                    test(ro[999999120L] == -100.4f);
                    test(ro[999999111L] == 123123.2f);
                    test(ro[999999130L] == 0.5f);
                    called();
                }

                public void opStringStringD(Dictionary<string, string> ro, Dictionary<string, string> _do)
                {
                    Dictionary<string, string> di1 = new Dictionary<string, string>();
                    di1["foo"] = "abc -1.1";
                    di1["bar"] = "abc 123123.2";
                    test(Ice.CollectionComparer.Equals(_do, di1));
                    test(ro.Count == 4);
                    test(ro["foo"].Equals("abc -1.1"));
                    test(ro["FOO"].Equals("abc -100.4"));
                    test(ro["bar"].Equals("abc 123123.2"));
                    test(ro["BAR"].Equals("abc 0.5"));
                    called();
                }

                public void opStringMyEnumD(Dictionary<string, Test.MyEnum> ro, Dictionary<string, Test.MyEnum> _do)
                {
                    var di1 = new Dictionary<string, Test.MyEnum>();
                    di1["abc"] = Test.MyEnum.enum1;
                    di1[""] = Test.MyEnum.enum2;
                    test(Ice.CollectionComparer.Equals(_do, di1));
                    test(ro.Count == 4);
                    test(ro["abc"] == Test.MyEnum.enum1);
                    test(ro["qwerty"] == Test.MyEnum.enum3);
                    test(ro[""] == Test.MyEnum.enum2);
                    test(ro["Hello!!"] == Test.MyEnum.enum2);
                    called();
                }

                public void opMyEnumStringD(Dictionary<Test.MyEnum, string> ro, Dictionary<Test.MyEnum, string> _do)
                {
                    var di1 = new Dictionary<Test.MyEnum, string>();
                    di1[Test.MyEnum.enum1] = "abc";
                    test(Ice.CollectionComparer.Equals(_do, di1));
                    test(ro.Count == 3);
                    test(ro[Test.MyEnum.enum1].Equals("abc"));
                    test(ro[Test.MyEnum.enum2].Equals("Hello!!"));
                    test(ro[Test.MyEnum.enum3].Equals("qwerty"));
                    called();
                }

                public void opMyStructMyEnumD(Dictionary<Test.MyStruct, Test.MyEnum> ro,
                                              Dictionary<Test.MyStruct, Test.MyEnum> _do)
                {
                    var s11 = new Test.MyStruct(1, 1);
                    var s12 = new Test.MyStruct(1, 2);
                    var di1 = new Dictionary<Test.MyStruct, Test.MyEnum>();
                    di1[s11] = Test.MyEnum.enum1;
                    di1[s12] = Test.MyEnum.enum2;
                    test(Ice.CollectionComparer.Equals(_do, di1));
                    var s22 = new Test.MyStruct(2, 2);
                    var s23 = new Test.MyStruct(2, 3);
                    test(ro.Count == 4);
                    test(ro[s11] == Test.MyEnum.enum1);
                    test(ro[s12] == Test.MyEnum.enum2);
                    test(ro[s22] == Test.MyEnum.enum3);
                    test(ro[s23] == Test.MyEnum.enum2);
                    called();
                }

                public void opByteBoolDS(Dictionary<byte, bool>[] ro,
                                         Dictionary<byte, bool>[] _do)
                {
                    test(ro.Length == 2);
                    test(ro[0].Count == 3);
                    test(ro[0][10]);
                    test(!ro[0][11]);
                    test(ro[0][101]);
                    test(ro[1].Count == 2);
                    test(ro[1][10]);
                    test(!ro[1][100]);

                    test(_do.Length == 3);
                    test(_do[0].Count == 2);
                    test(!_do[0][100]);
                    test(!_do[0][101]);
                    test(_do[1].Count == 2);
                    test(_do[1][10]);
                    test(!_do[1][100]);
                    test(_do[2].Count == 3);
                    test(_do[2][10]);
                    test(!_do[2][11]);
                    test(_do[2][101]);
                    called();
                }

                public void opShortIntDS(Dictionary<short, int>[] ro,
                                         Dictionary<short, int>[] _do)
                {
                    test(ro.Length == 2);
                    test(ro[0].Count == 3);
                    test(ro[0][110] == -1);
                    test(ro[0][111] == -100);
                    test(ro[0][1101] == 0);
                    test(ro[1].Count == 2);
                    test(ro[1][110] == -1);
                    test(ro[1][1100] == 123123);

                    test(_do.Length == 3);
                    test(_do[0].Count == 1);
                    test(_do[0][100] == -1001);
                    test(_do[1].Count == 2);
                    test(_do[1][110] == -1);
                    test(_do[1][1100] == 123123);
                    test(_do[2].Count == 3);
                    test(_do[2][110] == -1);
                    test(_do[2][111] == -100);
                    test(_do[2][1101] == 0);
                    called();
                }

                public void opLongFloatDS(Dictionary<long, float>[] ro,
                                          Dictionary<long, float>[] _do)
                {
                    test(ro.Length == 2);
                    test(ro[0].Count == 3);
                    test(ro[0][999999110L] == -1.1f);
                    test(ro[0][999999120L] == -100.4f);
                    test(ro[0][999999130L] == 0.5f);
                    test(ro[1].Count == 2);
                    test(ro[1][999999110L] == -1.1f);
                    test(ro[1][999999111L] == 123123.2f);

                    test(_do.Length == 3);
                    test(_do[0].Count == 1);
                    test(_do[0][999999140L] == 3.14f);
                    test(_do[1].Count == 2);
                    test(_do[1][999999110L] == -1.1f);
                    test(_do[1][999999111L] == 123123.2f);
                    test(_do[2].Count == 3);
                    test(_do[2][999999110L] == -1.1f);
                    test(_do[2][999999120L] == -100.4f);
                    test(_do[2][999999130L] == 0.5f);
                    called();
                }

                public void opStringStringDS(Dictionary<string, string>[] ro,
                                             Dictionary<string, string>[] _do)
                {
                    test(ro.Length == 2);
                    test(ro[0].Count == 3);
                    test(ro[0]["foo"].Equals("abc -1.1"));
                    test(ro[0]["FOO"].Equals("abc -100.4"));
                    test(ro[0]["BAR"].Equals("abc 0.5"));
                    test(ro[1].Count == 2);
                    test(ro[1]["foo"] == "abc -1.1");
                    test(ro[1]["bar"] == "abc 123123.2");

                    test(_do.Length == 3);
                    test(_do[0].Count == 1);
                    test(_do[0]["f00"].Equals("ABC -3.14"));
                    test(_do[1].Count == 2);
                    test(_do[1]["foo"].Equals("abc -1.1"));
                    test(_do[1]["bar"].Equals("abc 123123.2"));
                    test(_do[2].Count == 3);
                    test(_do[2]["foo"].Equals("abc -1.1"));
                    test(_do[2]["FOO"].Equals("abc -100.4"));
                    test(_do[2]["BAR"].Equals("abc 0.5"));
                    called();
                }

                public void opStringMyEnumDS(Dictionary<string, Test.MyEnum>[] ro,
                                             Dictionary<string, Test.MyEnum>[] _do)
                {
                    test(ro.Length == 2);
                    test(ro[0].Count == 3);
                    test(ro[0]["abc"] == Test.MyEnum.enum1);
                    test(ro[0]["qwerty"] == Test.MyEnum.enum3);
                    test(ro[0]["Hello!!"] == Test.MyEnum.enum2);
                    test(ro[1].Count == 2);
                    test(ro[1]["abc"] == Test.MyEnum.enum1);
                    test(ro[1][""] == Test.MyEnum.enum2);

                    test(_do.Length == 3);
                    test(_do[0].Count == 1);
                    test(_do[0]["Goodbye"] == Test.MyEnum.enum1);
                    test(_do[1].Count == 2);
                    test(_do[1]["abc"] == Test.MyEnum.enum1);
                    test(_do[1][""] == Test.MyEnum.enum2);
                    test(_do[2].Count == 3);
                    test(_do[2]["abc"] == Test.MyEnum.enum1);
                    test(_do[2]["qwerty"] == Test.MyEnum.enum3);
                    test(_do[2]["Hello!!"] == Test.MyEnum.enum2);
                    called();
                }

                public void opMyEnumStringDS(Dictionary<Test.MyEnum, string>[] ro,
                                             Dictionary<Test.MyEnum, string>[] _do)
                {
                    test(ro.Length == 2);
                    test(ro[0].Count == 2);
                    test(ro[0][Test.MyEnum.enum2].Equals("Hello!!"));
                    test(ro[0][Test.MyEnum.enum3].Equals("qwerty"));
                    test(ro[1].Count == 1);
                    test(ro[1][Test.MyEnum.enum1].Equals("abc"));

                    test(_do.Length == 3);
                    test(_do[0].Count == 1);
                    test(_do[0][Test.MyEnum.enum1].Equals("Goodbye"));
                    test(_do[1].Count == 1);
                    test(_do[1][Test.MyEnum.enum1].Equals("abc"));
                    test(_do[2].Count == 2);
                    test(_do[2][Test.MyEnum.enum2].Equals("Hello!!"));
                    test(_do[2][Test.MyEnum.enum3].Equals("qwerty"));
                    called();
                }

                public void opMyStructMyEnumDS(Dictionary<Test.MyStruct, Test.MyEnum>[] ro,
                                               Dictionary<Test.MyStruct, Test.MyEnum>[] _do)
                {
                    var s11 = new Test.MyStruct(1, 1);
                    var s12 = new Test.MyStruct(1, 2);
                    var s22 = new Test.MyStruct(2, 2);
                    var s23 = new Test.MyStruct(2, 3);

                    test(ro.Length == 2);
                    test(ro[0].Count == 3);
                    test(ro[0][s11] == Test.MyEnum.enum1);
                    test(ro[0][s22] == Test.MyEnum.enum3);
                    test(ro[0][s23] == Test.MyEnum.enum2);
                    test(ro[1].Count == 2);
                    test(ro[1][s11] == Test.MyEnum.enum1);
                    test(ro[1][s12] == Test.MyEnum.enum2);

                    test(_do.Length == 3);
                    test(_do[0].Count == 1);
                    test(_do[0][s23] == Test.MyEnum.enum3);
                    test(_do[1].Count == 2);
                    test(_do[1][s11] == Test.MyEnum.enum1);
                    test(_do[1][s12] == Test.MyEnum.enum2);
                    test(_do[2].Count == 3);
                    test(_do[2][s11] == Test.MyEnum.enum1);
                    test(_do[2][s22] == Test.MyEnum.enum3);
                    test(_do[2][s23] == Test.MyEnum.enum2);
                    called();
                }

                public void opByteByteSD(Dictionary<byte, byte[]> ro,
                                         Dictionary<byte, byte[]> _do)
                {
                    test(_do.Count == 1);
                    test(_do[0xf1].Length == 2);
                    test(_do[0xf1][0] == 0xf2);
                    test(_do[0xf1][1] == 0xf3);

                    test(ro.Count == 3);
                    test(ro[0x01].Length == 2);
                    test(ro[0x01][0] == 0x01);
                    test(ro[0x01][1] == 0x11);
                    test(ro[0x22].Length == 1);
                    test(ro[0x22][0] == 0x12);
                    test(ro[0xf1].Length == 2);
                    test(ro[0xf1][0] == 0xf2);
                    test(ro[0xf1][1] == 0xf3);
                    called();
                }

                public void opBoolBoolSD(Dictionary<bool, bool[]> ro,
                                         Dictionary<bool, bool[]> _do)
                {
                    test(_do.Count == 1);
                    test(_do[false].Length == 2);
                    test(_do[false][0]);
                    test(!_do[false][1]);
                    test(ro.Count == 2);
                    test(ro[false].Length == 2);
                    test(ro[false][0]);
                    test(!ro[false][1]);
                    test(ro[true].Length == 3);
                    test(!ro[true][0]);
                    test(ro[true][1]);
                    test(ro[true][2]);
                    called();
                }

                public void opShortShortSD(Dictionary<short, short[]> ro,
                                           Dictionary<short, short[]> _do)
                {
                    test(_do.Count == 1);
                    test(_do[4].Length == 2);
                    test(_do[4][0] == 6);
                    test(_do[4][1] == 7);

                    test(ro.Count == 3);
                    test(ro[1].Length == 3);
                    test(ro[1][0] == 1);
                    test(ro[1][1] == 2);
                    test(ro[1][2] == 3);
                    test(ro[2].Length == 2);
                    test(ro[2][0] == 4);
                    test(ro[2][1] == 5);
                    test(ro[4].Length == 2);
                    test(ro[4][0] == 6);
                    test(ro[4][1] == 7);
                    called();
                }

                public void opIntIntSD(Dictionary<int, int[]> ro,
                                       Dictionary<int, int[]> _do)
                {
                    test(_do.Count == 1);
                    test(_do[400].Length == 2);
                    test(_do[400][0] == 600);
                    test(_do[400][1] == 700);

                    test(ro.Count == 3);
                    test(ro[100].Length == 3);
                    test(ro[100][0] == 100);
                    test(ro[100][1] == 200);
                    test(ro[100][2] == 300);
                    test(ro[200].Length == 2);
                    test(ro[200][0] == 400);
                    test(ro[200][1] == 500);
                    test(ro[400].Length == 2);
                    test(ro[400][0] == 600);
                    test(ro[400][1] == 700);
                    called();
                }

                public void opLongLongSD(Dictionary<long, long[]> ro,
                                         Dictionary<long, long[]> _do)
                {
                    test(_do.Count == 1);
                    test(_do[999999992L].Length == 2);
                    test(_do[999999992L][0] == 999999110L);
                    test(_do[999999992L][1] == 999999120L);
                    test(ro.Count == 3);
                    test(ro[999999990L].Length == 3);
                    test(ro[999999990L][0] == 999999110L);
                    test(ro[999999990L][1] == 999999111L);
                    test(ro[999999990L][2] == 999999110L);
                    test(ro[999999991L].Length == 2);
                    test(ro[999999991L][0] == 999999120L);
                    test(ro[999999991L][1] == 999999130L);
                    test(ro[999999992L].Length == 2);
                    test(ro[999999992L][0] == 999999110L);
                    test(ro[999999992L][1] == 999999120L);
                    called();
                }

                public void opStringFloatSD(Dictionary<string, float[]> ro,
                                            Dictionary<string, float[]> _do)
                {
                    test(_do.Count == 1);
                    test(_do["aBc"].Length == 2);
                    test(_do["aBc"][0] == -3.14f);
                    test(_do["aBc"][1] == 3.14f);

                    test(ro.Count == 3);
                    test(ro["abc"].Length == 3);
                    test(ro["abc"][0] == -1.1f);
                    test(ro["abc"][1] == 123123.2f);
                    test(ro["abc"][2] == 100.0f);
                    test(ro["ABC"].Length == 2);
                    test(ro["ABC"][0] == 42.24f);
                    test(ro["ABC"][1] == -1.61f);
                    test(ro["aBc"].Length == 2);
                    test(ro["aBc"][0] == -3.14f);
                    test(ro["aBc"][1] == 3.14f);
                    called();
                }

                public void opStringDoubleSD(Dictionary<string, double[]> ro,
                                             Dictionary<string, double[]> _do)
                {
                    test(_do.Count == 1);
                    test(_do[""].Length == 2);
                    test(_do[""][0] == 1.6E10);
                    test(_do[""][1] == 1.7E10);
                    test(ro.Count == 3);
                    test(ro["Hello!!"].Length == 3);
                    test(ro["Hello!!"][0] == 1.1E10);
                    test(ro["Hello!!"][1] == 1.2E10);
                    test(ro["Hello!!"][2] == 1.3E10);
                    test(ro["Goodbye"].Length == 2);
                    test(ro["Goodbye"][0] == 1.4E10);
                    test(ro["Goodbye"][1] == 1.5E10);
                    test(ro[""].Length == 2);
                    test(ro[""][0] == 1.6E10);
                    test(ro[""][1] == 1.7E10);
                    called();
                }

                public void opStringStringSD(Dictionary<string, string[]> ro,
                                             Dictionary<string, string[]> _do)
                {
                    test(_do.Count == 1);
                    test(_do["ghi"].Length == 2);
                    test(_do["ghi"][0].Equals("and"));
                    test(_do["ghi"][1].Equals("xor"));

                    test(ro.Count == 3);
                    test(ro["abc"].Length == 3);
                    test(ro["abc"][0].Equals("abc"));
                    test(ro["abc"][1].Equals("de"));
                    test(ro["abc"][2].Equals("fghi"));
                    test(ro["def"].Length == 2);
                    test(ro["def"][0].Equals("xyz"));
                    test(ro["def"][1].Equals("or"));
                    test(ro["ghi"].Length == 2);
                    test(ro["ghi"][0].Equals("and"));
                    test(ro["ghi"][1].Equals("xor"));
                    called();
                }

                public void opMyEnumMyEnumSD(Dictionary<Test.MyEnum, Test.MyEnum[]> ro,
                                             Dictionary<Test.MyEnum, Test.MyEnum[]> _do)
                {
                    test(_do.Count == 1);
                    test(_do[Test.MyEnum.enum1].Length == 2);
                    test(_do[Test.MyEnum.enum1][0] == Test.MyEnum.enum3);
                    test(_do[Test.MyEnum.enum1][1] == Test.MyEnum.enum3);
                    test(ro.Count == 3);
                    test(ro[Test.MyEnum.enum3].Length == 3);
                    test(ro[Test.MyEnum.enum3][0] == Test.MyEnum.enum1);
                    test(ro[Test.MyEnum.enum3][1] == Test.MyEnum.enum1);
                    test(ro[Test.MyEnum.enum3][2] == Test.MyEnum.enum2);
                    test(ro[Test.MyEnum.enum2].Length == 2);
                    test(ro[Test.MyEnum.enum2][0] == Test.MyEnum.enum1);
                    test(ro[Test.MyEnum.enum2][1] == Test.MyEnum.enum2);
                    test(ro[Test.MyEnum.enum1].Length == 2);
                    test(ro[Test.MyEnum.enum1][0] == Test.MyEnum.enum3);
                    test(ro[Test.MyEnum.enum1][1] == Test.MyEnum.enum3);
                    called();
                }

                public void opIntS(int[] r)
                {
                    test(r.Length == _l);
                    for (int j = 0; j < r.Length; ++j)
                    {
                        test(r[j] == -j);
                    }
                    called();
                }

                public void opContextNotEqual(Dictionary<string, string> r)
                {
                    test(!Ice.CollectionComparer.Equals(r, _d));
                    called();
                }

                public void opContextEqual(Dictionary<string, string> r)
                {
                    test(Ice.CollectionComparer.Equals(r, _d));
                    called();
                }

                public void opIdempotent()
                {
                    called();
                }

                public void opNonmutating()
                {
                    called();
                }

                public void opDerived()
                {
                    called();
                }

                public void exCB(Ice.Exception ex)
                {
                    test(false);
                }

                private Ice.Communicator _communicator;
                private int _l;
                private Dictionary<string, string> _d;
            }

            internal static void twowaysAMI(global::Test.TestHelper helper, Test.MyClassPrx p)
            {
                Ice.Communicator communicator = helper.communicator();

                {
                    p.ice_pingAsync().Wait();
                }

                {
                    Ice.AsyncResult r = p.begin_ice_ping();
                    p.end_ice_ping(r);
                }

                {
                    Callback cb = new Callback();
                    p.begin_ice_ping().whenCompleted(cb.ice_ping, cb.exCB);
                    cb.check();
                }

                {
                    Callback cb = new Callback();
                    p.begin_ice_ping().whenCompleted(
                        () =>
                        {
                            cb.ice_ping();
                        },
                        (Ice.Exception ex) =>
                        {
                            cb.exCB(ex);
                        });
                    cb.check();
                }

                {
                    test(p.ice_isAAsync(Test.MyClassDisp_.ice_staticId()).Result);
                }

                {
                    Ice.AsyncResult r = p.begin_ice_isA(Test.MyClassDisp_.ice_staticId());
                    test(p.end_ice_isA(r));
                }

                {
                    Callback cb = new Callback();
                    p.begin_ice_isA(Test.MyClassDisp_.ice_staticId()).whenCompleted(cb.ice_isA, cb.exCB);
                    cb.check();
                }

                {
                    Callback cb = new Callback();
                    p.begin_ice_isA(Test.MyClassDisp_.ice_staticId()).whenCompleted(
                        (bool v) =>
                        {
                            cb.ice_isA(v);
                        },
                        (Ice.Exception ex) =>
                        {
                            cb.exCB(ex);
                        });
                    cb.check();
                }

                {
                    test(p.ice_idsAsync().Result.Length == 3);
                }

                {
                    Ice.AsyncResult r = p.begin_ice_ids();
                    test(p.end_ice_ids(r).Length == 3);
                }

                {
                    Callback cb = new Callback();
                    p.begin_ice_ids().whenCompleted(cb.ice_ids, cb.exCB);
                    cb.check();
                }

                {
                    Callback cb = new Callback();
                    p.begin_ice_ids().whenCompleted(
                        (string[] ids) =>
                        {
                            cb.ice_ids(ids);
                        },
                        (Ice.Exception ex) =>
                        {
                            cb.exCB(ex);
                        });
                    cb.check();
                }

                {
                    test(p.ice_idAsync().Result.Equals(Test.MyDerivedClassDisp_.ice_staticId()));
                }

                {
                    Ice.AsyncResult r = p.begin_ice_id();
                    test(p.end_ice_id(r).Equals(Test.MyDerivedClassDisp_.ice_staticId()));
                }

                {
                    Callback cb = new Callback();
                    p.begin_ice_id().whenCompleted(cb.ice_id, cb.exCB);
                    cb.check();
                }

                {
                    Callback cb = new Callback();
                    p.begin_ice_id().whenCompleted(
                        (string id) =>
                        {
                            cb.ice_id(id);
                        },
                        (Ice.Exception ex) =>
                        {
                            cb.exCB(ex);
                        });
                    cb.check();
                }

                {
                    p.opVoidAsync().Wait();
                }

                {
                    Ice.AsyncResult r = p.begin_opVoid();
                    p.end_opVoid(r);
                }

                {
                    Callback cb = new Callback();
                    p.begin_opVoid().whenCompleted(cb.opVoid, cb.exCB);
                    cb.check();
                }

                {
                    Callback cb = new Callback();
                    p.begin_opVoid().whenCompleted(
                        () =>
                        {
                            cb.opVoid();
                        },
                        (Ice.Exception ex) =>
                        {
                            cb.exCB(ex);
                        });
                    cb.check();
                }

                {
                    var ret = p.opByteAsync(0xff, 0x0f).Result;
                    test(ret.p3 == 0xf0);
                    test(ret.returnValue == 0xff);
                }

                {
                    Ice.AsyncResult r = p.begin_opByte(0xff, 0x0f);
                    byte p3;
                    byte ret = p.end_opByte(out p3, r);
                    test(p3 == 0xf0);
                    test(ret == 0xff);
                }

                {
                    Callback cb = new Callback();
                    p.begin_opByte(0xff, 0x0f).whenCompleted(cb.opByte, cb.exCB);
                    cb.check();
                }

                {
                    Callback cb = new Callback();
                    p.begin_opByte(0xff, 0x0f).whenCompleted(
                        (byte r, byte b) =>
                        {
                            cb.opByte(r, b);
                        },
                        (Ice.Exception ex) =>
                        {
                            cb.exCB(ex);
                        });
                    cb.check();
                }

                {
                    var cb = new Callback();
                    var ret = p.opBoolAsync(true, false).Result;
                    cb.opBool(ret.returnValue, ret.p3);
                }

                {
                    Callback cb = new Callback();
                    p.begin_opBool(true, false).whenCompleted(cb.opBool, cb.exCB);
                    cb.check();
                }

                {
                    Callback cb = new Callback();
                    p.begin_opBool(true, false).whenCompleted(
                        (bool r, bool b) =>
                        {
                            cb.opBool(r, b);
                        },
                        (Ice.Exception ex) =>
                        {
                            cb.exCB(ex);
                        });
                    cb.check();
                }

                {
                    var cb = new Callback();
                    var ret = p.opShortIntLongAsync(10, 11, 12).Result;
                    cb.opShortIntLong(ret.returnValue, ret.p4, ret.p5, ret.p6);
                }

                {
                    Callback cb = new Callback();
                    p.begin_opShortIntLong(10, 11, 12).whenCompleted(cb.opShortIntLong, cb.exCB);
                    cb.check();
                }

                {
                    Callback cb = new Callback();
                    p.begin_opShortIntLong(10, 11, 12).whenCompleted(
                        (long r, short s, int i, long l) =>
                        {
                            cb.opShortIntLong(r, s, i, l);
                        },
                        (Ice.Exception ex) =>
                        {
                            cb.exCB(ex);
                        });
                    cb.check();
                }

                {
                    var cb = new Callback();
                    var ret = p.opFloatDoubleAsync(3.14f, 1.1E10).Result;
                    cb.opFloatDouble(ret.returnValue, ret.p3, ret.p4);
                }

                {
                    Callback cb = new Callback();
                    p.begin_opFloatDouble(3.14f, 1.1E10).whenCompleted(cb.opFloatDouble, cb.exCB);
                    cb.check();
                }

                {
                    Callback cb = new Callback();
                    p.begin_opFloatDouble(3.14f, 1.1E10).whenCompleted(
                        (double r, float f, double d) =>
                        {
                            cb.opFloatDouble(r, f, d);
                        },
                        (Ice.Exception ex) =>
                        {
                            cb.exCB(ex);
                        });
                    cb.check();
                }

                {
                    var cb = new Callback();
                    var ret = p.opStringAsync("hello", "world").Result;
                    cb.opString(ret.returnValue, ret.p3);
                }

                {
                    Callback cb = new Callback();
                    p.begin_opString("hello", "world").whenCompleted(cb.opString, cb.exCB);
                    cb.check();
                }

                {
                    Callback cb = new Callback();
                    p.begin_opString("hello", "world").whenCompleted(
                        (string r, string s) =>
                        {
                            cb.opString(r, s);
                        },
                        (Ice.Exception ex) =>
                        {
                            cb.exCB(ex);
                        });
                    cb.check();
                }

                {
                    var cb = new Callback();
                    var ret = p.opMyEnumAsync(Test.MyEnum.enum2).Result;
                    cb.opMyEnum(ret.returnValue, ret.p2);
                }

                {
                    Callback cb = new Callback();
                    p.begin_opMyEnum(Test.MyEnum.enum2).whenCompleted(cb.opMyEnum, cb.exCB);
                    cb.check();
                }

                {
                    Callback cb = new Callback();
                    p.begin_opMyEnum(Test.MyEnum.enum2).whenCompleted(
                        (Test.MyEnum r, Test.MyEnum e) =>
                        {
                            cb.opMyEnum(r, e);
                        },
                        (Ice.Exception ex) =>
                        {
                            cb.exCB(ex);
                        });
                    cb.check();
                }

                {
                    var cb = new Callback(communicator);
                    var ret = p.opMyClassAsync(p).Result;
                    cb.opMyClass(ret.returnValue, ret.p2, ret.p3);
                }

                {
                    Callback cb = new Callback(communicator);
                    p.begin_opMyClass(p).whenCompleted(cb.opMyClass, cb.exCB);
                    cb.check();
                }

                {
                    Callback cb = new Callback(communicator);
                    p.begin_opMyClass(p).whenCompleted(
                        (Test.MyClassPrx r, Test.MyClassPrx c1, Test.MyClassPrx c2) =>
                        {
                            cb.opMyClass(r, c1, c2);
                        },
                        (Ice.Exception ex) =>
                        {
                            cb.exCB(ex);
                        });
                    cb.check();
                }

                {
                    var si1 = new Test.Structure();
                    si1.p = p;
                    si1.e = Test.MyEnum.enum3;
                    si1.s = new Test.AnotherStruct();
                    si1.s.s = "abc";
                    var si2 = new Test.Structure();
                    si2.p = null;
                    si2.e = Test.MyEnum.enum2;
                    si2.s = new Test.AnotherStruct();
                    si2.s.s = "def";

                    var cb = new Callback(communicator);
                    var ret = p.opStructAsync(si1, si2).Result;
                    cb.opStruct(ret.returnValue, ret.p3);
                }

                {
                    var si1 = new Test.Structure();
                    si1.p = p;
                    si1.e = Test.MyEnum.enum3;
                    si1.s = new Test.AnotherStruct();
                    si1.s.s = "abc";
                    var si2 = new Test.Structure();
                    si2.p = null;
                    si2.e = Test.MyEnum.enum2;
                    si2.s = new Test.AnotherStruct();
                    si2.s.s = "def";

                    Callback cb = new Callback(communicator);
                    p.begin_opStruct(si1, si2).whenCompleted(cb.opStruct, cb.exCB);
                    cb.check();
                }

                {
                    var si1 = new Test.Structure();
                    si1.p = p;
                    si1.e = Test.MyEnum.enum3;
                    si1.s = new Test.AnotherStruct();
                    si1.s.s = "abc";
                    var si2 = new Test.Structure();
                    si2.p = null;
                    si2.e = Test.MyEnum.enum2;
                    si2.s = new Test.AnotherStruct();
                    si2.s.s = "def";

                    Callback cb = new Callback(communicator);
                    p.begin_opStruct(si1, si2).whenCompleted(
                        (Test.Structure rso, Test.Structure so) =>
                        {
                            cb.opStruct(rso, so);
                        },
                        (Ice.Exception ex) =>
                        {
                            cb.exCB(ex);
                        });
                    cb.check();
                }

                {
                    byte[] bsi1 = new byte[] { 0x01, 0x11, 0x12, 0x22 };
                    byte[] bsi2 = new byte[] { 0xf1, 0xf2, 0xf3, 0xf4 };

                    var cb = new Callback();
                    var ret = p.opByteSAsync(bsi1, bsi2).Result;
                    cb.opByteS(ret.returnValue, ret.p3);
                }

                {
                    byte[] bsi1 = new byte[] { 0x01, 0x11, 0x12, 0x22 };
                    byte[] bsi2 = new byte[] { 0xf1, 0xf2, 0xf3, 0xf4 };

                    Callback cb = new Callback();
                    p.begin_opByteS(bsi1, bsi2).whenCompleted(cb.opByteS, cb.exCB);
                    cb.check();
                }

                {
                    byte[] bsi1 = new byte[] { 0x01, 0x11, 0x12, 0x22 };
                    byte[] bsi2 = new byte[] { 0xf1, 0xf2, 0xf3, 0xf4 };

                    Callback cb = new Callback();
                    p.begin_opByteS(bsi1, bsi2).whenCompleted(
                        (byte[] rso, byte[] bso) =>
                        {
                            cb.opByteS(rso, bso);
                        },
                        (Ice.Exception ex) =>
                        {
                            cb.exCB(ex);
                        });
                    cb.check();
                }

                {
                    bool[] bsi1 = new bool[] { true, true, false };
                    bool[] bsi2 = new bool[] { false };

                    var cb = new Callback();
                    var ret = p.opBoolSAsync(bsi1, bsi2).Result;
                    cb.opBoolS(ret.returnValue, ret.p3);
                }

                {
                    bool[] bsi1 = new bool[] { true, true, false };
                    bool[] bsi2 = new bool[] { false };

                    Callback cb = new Callback();
                    p.begin_opBoolS(bsi1, bsi2).whenCompleted(cb.opBoolS, cb.exCB);
                    cb.check();
                }

                {
                    bool[] bsi1 = new bool[] { true, true, false };
                    bool[] bsi2 = new bool[] { false };

                    Callback cb = new Callback();
                    p.begin_opBoolS(bsi1, bsi2).whenCompleted(
                        (bool[] rso, bool[] bso) =>
                        {
                            cb.opBoolS(rso, bso);
                        },
                        (Ice.Exception ex) =>
                        {
                            cb.exCB(ex);
                        });
                    cb.check();
                }

                {
                    short[] ssi = new short[] { 1, 2, 3 };
                    int[] isi = new int[] { 5, 6, 7, 8 };
                    long[] lsi = new long[] { 10, 30, 20 };

                    var cb = new Callback();
                    var ret = p.opShortIntLongSAsync(ssi, isi, lsi).Result;
                    cb.opShortIntLongS(ret.returnValue, ret.p4, ret.p5, ret.p6);
                }

                {
                    short[] ssi = new short[] { 1, 2, 3 };
                    int[] isi = new int[] { 5, 6, 7, 8 };
                    long[] lsi = new long[] { 10, 30, 20 };

                    Callback cb = new Callback();
                    p.begin_opShortIntLongS(ssi, isi, lsi).whenCompleted(cb.opShortIntLongS, cb.exCB);
                    cb.check();
                }

                {
                    short[] ssi = new short[] { 1, 2, 3 };
                    int[] isi = new int[] { 5, 6, 7, 8 };
                    long[] lsi = new long[] { 10, 30, 20 };

                    Callback cb = new Callback();
                    p.begin_opShortIntLongS(ssi, isi, lsi).whenCompleted(
                        (long[] rso, short[] sso, int[] iso, long[] lso) =>
                        {
                            cb.opShortIntLongS(rso, sso, iso, lso);
                        },
                        (Ice.Exception ex) =>
                        {
                            cb.exCB(ex);
                        });
                    cb.check();
                }

                {
                    float[] fsi = new float[] { 3.14f, 1.11f };
                    double[] dsi = new double[] { 1.1e10, 1.2e10, 1.3e10 };

                    var cb = new Callback();
                    var ret = p.opFloatDoubleSAsync(fsi, dsi).Result;
                    cb.opFloatDoubleS(ret.returnValue, ret.p3, ret.p4);
                }

                {
                    float[] fsi = new float[] { 3.14f, 1.11f };
                    double[] dsi = new double[] { 1.1e10, 1.2e10, 1.3e10 };

                    Callback cb = new Callback();
                    p.begin_opFloatDoubleS(fsi, dsi).whenCompleted(cb.opFloatDoubleS, cb.exCB);
                    cb.check();
                }

                {
                    float[] fsi = new float[] { 3.14f, 1.11f };
                    double[] dsi = new double[] { 1.1e10, 1.2e10, 1.3e10 };

                    Callback cb = new Callback();
                    p.begin_opFloatDoubleS(fsi, dsi).whenCompleted(
                        (double[] rso, float[] fso, double[] dso) =>
                        {
                            cb.opFloatDoubleS(rso, fso, dso);
                        },
                        (Ice.Exception ex) =>
                        {
                            cb.exCB(ex);
                        });
                    cb.check();
                }

                {
                    string[] ssi1 = new string[] { "abc", "de", "fghi" };
                    string[] ssi2 = new string[] { "xyz" };

                    var cb = new Callback();
                    var ret = p.opStringSAsync(ssi1, ssi2).Result;
                    cb.opStringS(ret.returnValue, ret.p3);
                }

                {
                    string[] ssi1 = new string[] { "abc", "de", "fghi" };
                    string[] ssi2 = new string[] { "xyz" };

                    Callback cb = new Callback();
                    p.begin_opStringS(ssi1, ssi2).whenCompleted(cb.opStringS, cb.exCB);
                    cb.check();
                }

                {
                    string[] ssi1 = new string[] { "abc", "de", "fghi" };
                    string[] ssi2 = new string[] { "xyz" };

                    Callback cb = new Callback();
                    p.begin_opStringS(ssi1, ssi2).whenCompleted(
                        (string[] rso, string[] sso) =>
                        {
                            cb.opStringS(rso, sso);
                        },
                        (Ice.Exception ex) =>
                        {
                            cb.exCB(ex);
                        });
                    cb.check();
                }

                {
                    byte[] s11 = new byte[] { 0x01, 0x11, 0x12 };
                    byte[] s12 = new byte[] { 0xff };
                    byte[][] bsi1 = new byte[][] { s11, s12 };

                    byte[] s21 = new byte[] { 0x0e };
                    byte[] s22 = new byte[] { 0xf2, 0xf1 };
                    byte[][] bsi2 = new byte[][] { s21, s22 };

                    var cb = new Callback();
                    var ret = p.opByteSSAsync(bsi1, bsi2).Result;
                    cb.opByteSS(ret.returnValue, ret.p3);
                }

                {
                    byte[] s11 = new byte[] { 0x01, 0x11, 0x12 };
                    byte[] s12 = new byte[] { 0xff };
                    byte[][] bsi1 = new byte[][] { s11, s12 };

                    byte[] s21 = new byte[] { 0x0e };
                    byte[] s22 = new byte[] { 0xf2, 0xf1 };
                    byte[][] bsi2 = new byte[][] { s21, s22 };

                    Callback cb = new Callback();
                    p.begin_opByteSS(bsi1, bsi2).whenCompleted(cb.opByteSS, cb.exCB);
                    cb.check();
                }

                {
                    byte[] s11 = new byte[] { 0x01, 0x11, 0x12 };
                    byte[] s12 = new byte[] { 0xff };
                    byte[][] bsi1 = new byte[][] { s11, s12 };

                    byte[] s21 = new byte[] { 0x0e };
                    byte[] s22 = new byte[] { 0xf2, 0xf1 };
                    byte[][] bsi2 = new byte[][] { s21, s22 };

                    Callback cb = new Callback();
                    p.begin_opByteSS(bsi1, bsi2).whenCompleted(
                        (byte[][] rso, byte[][] bso) =>
                        {
                            cb.opByteSS(rso, bso);
                        },
                        (Ice.Exception ex) =>
                        {
                            cb.exCB(ex);
                        });
                    cb.check();
                }

                {
                    bool[] s11 = new bool[] { true };
                    bool[] s12 = new bool[] { false };
                    bool[] s13 = new bool[] { true, true };
                    bool[][] bsi1 = new bool[][] { s11, s12, s13 };

                    bool[] s21 = new bool[] { false, false, true };
                    bool[][] bsi2 = new bool[][] { s21 };

                    var cb = new Callback();
                    var ret = p.opBoolSSAsync(bsi1, bsi2).Result;
                    cb.opBoolSS(ret.returnValue, ret.p3);
                }

                {
                    bool[] s11 = new bool[] { true };
                    bool[] s12 = new bool[] { false };
                    bool[] s13 = new bool[] { true, true };
                    bool[][] bsi1 = new bool[][] { s11, s12, s13 };

                    bool[] s21 = new bool[] { false, false, true };
                    bool[][] bsi2 = new bool[][] { s21 };

                    Callback cb = new Callback();
                    p.begin_opBoolSS(bsi1, bsi2).whenCompleted(cb.opBoolSS, cb.exCB);
                    cb.check();
                }

                {
                    bool[] s11 = new bool[] { true };
                    bool[] s12 = new bool[] { false };
                    bool[] s13 = new bool[] { true, true };
                    bool[][] bsi1 = new bool[][] { s11, s12, s13 };

                    bool[] s21 = new bool[] { false, false, true };
                    bool[][] bsi2 = new bool[][] { s21 };

                    Callback cb = new Callback();
                    p.begin_opBoolSS(bsi1, bsi2).whenCompleted(
                        (bool[][] rso, bool[][] bso) =>
                        {
                            cb.opBoolSS(rso, bso);
                        },
                        (Ice.Exception ex) =>
                        {
                            cb.exCB(ex);
                        });
                    cb.check();
                }

                {
                    short[] s11 = new short[] { 1, 2, 5 };
                    short[] s12 = new short[] { 13 };
                    short[] s13 = new short[] { };
                    short[][] ssi = new short[][] { s11, s12, s13 };

                    int[] i11 = new int[] { 24, 98 };
                    int[] i12 = new int[] { 42 };
                    int[][] isi = new int[][] { i11, i12 };

                    long[] l11 = new long[] { 496, 1729 };
                    long[][] lsi = new long[][] { l11 };

                    var cb = new Callback();
                    var ret = p.opShortIntLongSSAsync(ssi, isi, lsi).Result;
                    cb.opShortIntLongSS(ret.returnValue, ret.p4, ret.p5, ret.p6);
                }

                {
                    short[] s11 = new short[] { 1, 2, 5 };
                    short[] s12 = new short[] { 13 };
                    short[] s13 = new short[] { };
                    short[][] ssi = new short[][] { s11, s12, s13 };

                    int[] i11 = new int[] { 24, 98 };
                    int[] i12 = new int[] { 42 };
                    int[][] isi = new int[][] { i11, i12 };

                    long[] l11 = new long[] { 496, 1729 };
                    long[][] lsi = new long[][] { l11 };

                    Callback cb = new Callback();
                    p.begin_opShortIntLongSS(ssi, isi, lsi).whenCompleted(cb.opShortIntLongSS, cb.exCB);
                    cb.check();
                }

                {
                    short[] s11 = new short[] { 1, 2, 5 };
                    short[] s12 = new short[] { 13 };
                    short[] s13 = new short[] { };
                    short[][] ssi = new short[][] { s11, s12, s13 };

                    int[] i11 = new int[] { 24, 98 };
                    int[] i12 = new int[] { 42 };
                    int[][] isi = new int[][] { i11, i12 };

                    long[] l11 = new long[] { 496, 1729 };
                    long[][] lsi = new long[][] { l11 };

                    Callback cb = new Callback();
                    p.begin_opShortIntLongSS(ssi, isi, lsi).whenCompleted(
                        (long[][] rso, short[][] sso, int[][] iso, long[][] lso) =>
                        {
                            cb.opShortIntLongSS(rso, sso, iso, lso);
                        },
                        (Ice.Exception ex) =>
                        {
                            cb.exCB(ex);
                        });
                    cb.check();
                }

                {
                    float[] f11 = new float[] { 3.14f };
                    float[] f12 = new float[] { 1.11f };
                    float[] f13 = new float[] { };
                    float[][] fsi = new float[][] { f11, f12, f13 };

                    double[] d11 = new double[] { 1.1e10, 1.2e10, 1.3e10 };
                    double[][] dsi = new double[][] { d11 };

                    var cb = new Callback();
                    var ret = p.opFloatDoubleSSAsync(fsi, dsi).Result;
                    cb.opFloatDoubleSS(ret.returnValue, ret.p3, ret.p4);
                }

                {
                    float[] f11 = new float[] { 3.14f };
                    float[] f12 = new float[] { 1.11f };
                    float[] f13 = new float[] { };
                    float[][] fsi = new float[][] { f11, f12, f13 };

                    double[] d11 = new double[] { 1.1e10, 1.2e10, 1.3e10 };
                    double[][] dsi = new double[][] { d11 };

                    Callback cb = new Callback();
                    p.begin_opFloatDoubleSS(fsi, dsi).whenCompleted(cb.opFloatDoubleSS, cb.exCB);
                    cb.check();
                }

                {
                    float[] f11 = new float[] { 3.14f };
                    float[] f12 = new float[] { 1.11f };
                    float[] f13 = new float[] { };
                    float[][] fsi = new float[][] { f11, f12, f13 };

                    double[] d11 = new double[] { 1.1e10, 1.2e10, 1.3e10 };
                    double[][] dsi = new double[][] { d11 };

                    Callback cb = new Callback();
                    p.begin_opFloatDoubleSS(fsi, dsi).whenCompleted(
                        (double[][] rso, float[][] fso, double[][] dso) =>
                        {
                            cb.opFloatDoubleSS(rso, fso, dso);
                        },
                        (Ice.Exception ex) =>
                        {
                            cb.exCB(ex);
                        });
                    cb.check();
                }

                {
                    string[] s11 = new string[] { "abc" };
                    string[] s12 = new string[] { "de", "fghi" };
                    string[][] ssi1 = new string[][] { s11, s12 };

                    string[] s21 = new string[] { };
                    string[] s22 = new string[] { };
                    string[] s23 = new string[] { "xyz" };
                    string[][] ssi2 = new string[][] { s21, s22, s23 };

                    var cb = new Callback();
                    var ret = p.opStringSSAsync(ssi1, ssi2).Result;
                    cb.opStringSS(ret.returnValue, ret.p3);
                }

                {
                    string[] s11 = new string[] { "abc" };
                    string[] s12 = new string[] { "de", "fghi" };
                    string[][] ssi1 = new string[][] { s11, s12 };

                    string[] s21 = new string[] { };
                    string[] s22 = new string[] { };
                    string[] s23 = new string[] { "xyz" };
                    string[][] ssi2 = new string[][] { s21, s22, s23 };

                    Callback cb = new Callback();
                    p.begin_opStringSS(ssi1, ssi2).whenCompleted(cb.opStringSS, cb.exCB);
                    cb.check();
                }

                {
                    string[] s11 = new string[] { "abc" };
                    string[] s12 = new string[] { "de", "fghi" };
                    string[][] ssi1 = new string[][] { s11, s12 };

                    string[] s21 = new string[] { };
                    string[] s22 = new string[] { };
                    string[] s23 = new string[] { "xyz" };
                    string[][] ssi2 = new string[][] { s21, s22, s23 };

                    Callback cb = new Callback();
                    p.begin_opStringSS(ssi1, ssi2).whenCompleted(
                        (string[][] rso, string[][] sso) =>
                        {
                            cb.opStringSS(rso, sso);
                        },
                        (Ice.Exception ex) =>
                        {
                            cb.exCB(ex);
                        });
                    cb.check();
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
                    string[][] ss23 = new string[][] { };
                    string[][][] sssi2 = new string[][][] { ss21, ss22, ss23 };

                    var cb = new Callback();
                    var ret = p.opStringSSSAsync(sssi1, sssi2).Result;
                    cb.opStringSSS(ret.returnValue, ret.p3);
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
                    string[][] ss23 = new string[][] { };
                    string[][][] sssi2 = new string[][][] { ss21, ss22, ss23 };

                    Callback cb = new Callback();
                    p.begin_opStringSSS(sssi1, sssi2).whenCompleted(cb.opStringSSS, cb.exCB);
                    cb.check();
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
                    string[][] ss23 = new string[][] { };
                    string[][][] sssi2 = new string[][][] { ss21, ss22, ss23 };

                    Callback cb = new Callback();
                    p.begin_opStringSSS(sssi1, sssi2).whenCompleted(
                        (string[][][] rsso, string[][][] ssso) =>
                        {
                            cb.opStringSSS(rsso, ssso);
                        },
                        (Ice.Exception ex) =>
                        {
                            cb.exCB(ex);
                        });
                    cb.check();
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
                    var ret = p.opByteBoolDAsync(di1, di2).Result;
                    cb.opByteBoolD(ret.returnValue, ret.p3);
                }

                {
                    Dictionary<byte, bool> di1 = new Dictionary<byte, bool>();
                    di1[10] = true;
                    di1[100] = false;
                    Dictionary<byte, bool> di2 = new Dictionary<byte, bool>();
                    di2[10] = true;
                    di2[11] = false;
                    di2[101] = true;

                    Callback cb = new Callback();
                    p.begin_opByteBoolD(di1, di2).whenCompleted(cb.opByteBoolD, cb.exCB);
                    cb.check();
                }

                {
                    Dictionary<byte, bool> di1 = new Dictionary<byte, bool>();
                    di1[10] = true;
                    di1[100] = false;
                    Dictionary<byte, bool> di2 = new Dictionary<byte, bool>();
                    di2[10] = true;
                    di2[11] = false;
                    di2[101] = true;

                    Callback cb = new Callback();
                    p.begin_opByteBoolD(di1, di2).whenCompleted(
                        (Dictionary<byte, bool> ro, Dictionary<byte, bool> _do) =>
                        {
                            cb.opByteBoolD(ro, _do);
                        },
                        (Ice.Exception ex) =>
                        {
                            cb.exCB(ex);
                        });
                    cb.check();
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
                    var ret = p.opShortIntDAsync(di1, di2).Result;
                    cb.opShortIntD(ret.returnValue, ret.p3);
                }

                {
                    Dictionary<short, int> di1 = new Dictionary<short, int>();
                    di1[110] = -1;
                    di1[1100] = 123123;
                    Dictionary<short, int> di2 = new Dictionary<short, int>();
                    di2[110] = -1;
                    di2[111] = -100;
                    di2[1101] = 0;

                    Callback cb = new Callback();
                    p.begin_opShortIntD(di1, di2).whenCompleted(cb.opShortIntD, cb.exCB);
                    cb.check();
                }

                {
                    Dictionary<short, int> di1 = new Dictionary<short, int>();
                    di1[110] = -1;
                    di1[1100] = 123123;
                    Dictionary<short, int> di2 = new Dictionary<short, int>();
                    di2[110] = -1;
                    di2[111] = -100;
                    di2[1101] = 0;

                    Callback cb = new Callback();
                    p.begin_opShortIntD(di1, di2).whenCompleted(
                        (Dictionary<short, int> ro, Dictionary<short, int> _do) =>
                        {
                            cb.opShortIntD(ro, _do);
                        },
                        (Ice.Exception ex) =>
                        {
                            cb.exCB(ex);
                        });
                    cb.check();
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
                    var ret = p.opLongFloatDAsync(di1, di2).Result;
                    cb.opLongFloatD(ret.returnValue, ret.p3);
                }

                {
                    Dictionary<long, float> di1 = new Dictionary<long, float>();
                    di1[999999110L] = -1.1f;
                    di1[999999111L] = 123123.2f;
                    Dictionary<long, float> di2 = new Dictionary<long, float>();
                    di2[999999110L] = -1.1f;
                    di2[999999120L] = -100.4f;
                    di2[999999130L] = 0.5f;

                    Callback cb = new Callback();
                    p.begin_opLongFloatD(di1, di2).whenCompleted(cb.opLongFloatD, cb.exCB);
                    cb.check();
                }

                {
                    Dictionary<long, float> di1 = new Dictionary<long, float>();
                    di1[999999110L] = -1.1f;
                    di1[999999111L] = 123123.2f;
                    Dictionary<long, float> di2 = new Dictionary<long, float>();
                    di2[999999110L] = -1.1f;
                    di2[999999120L] = -100.4f;
                    di2[999999130L] = 0.5f;

                    Callback cb = new Callback();
                    p.begin_opLongFloatD(di1, di2).whenCompleted(
                        (Dictionary<long, float> ro, Dictionary<long, float> _do) =>
                        {
                            cb.opLongFloatD(ro, _do);
                        },
                        (Ice.Exception ex) =>
                        {
                            cb.exCB(ex);
                        });
                    cb.check();
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
                    var ret = p.opStringStringDAsync(di1, di2).Result;
                    cb.opStringStringD(ret.returnValue, ret.p3);
                }

                {
                    Dictionary<string, string> di1 = new Dictionary<string, string>();
                    di1["foo"] = "abc -1.1";
                    di1["bar"] = "abc 123123.2";
                    Dictionary<string, string> di2 = new Dictionary<string, string>();
                    di2["foo"] = "abc -1.1";
                    di2["FOO"] = "abc -100.4";
                    di2["BAR"] = "abc 0.5";

                    Callback cb = new Callback();
                    p.begin_opStringStringD(di1, di2).whenCompleted(cb.opStringStringD, cb.exCB);
                    cb.check();
                }

                {
                    Dictionary<string, string> di1 = new Dictionary<string, string>();
                    di1["foo"] = "abc -1.1";
                    di1["bar"] = "abc 123123.2";
                    Dictionary<string, string> di2 = new Dictionary<string, string>();
                    di2["foo"] = "abc -1.1";
                    di2["FOO"] = "abc -100.4";
                    di2["BAR"] = "abc 0.5";

                    Callback cb = new Callback();
                    p.begin_opStringStringD(di1, di2).whenCompleted(
                        (Dictionary<string, string> ro, Dictionary<string, string> _do) =>
                        {
                            cb.opStringStringD(ro, _do);
                        },
                        (Ice.Exception ex) =>
                        {
                            cb.exCB(ex);
                        });
                    cb.check();
                }

                {
                    var di1 = new Dictionary<string, Test.MyEnum>();
                    di1["abc"] = Test.MyEnum.enum1;
                    di1[""] = Test.MyEnum.enum2;
                    var di2 = new Dictionary<string, Test.MyEnum>();
                    di2["abc"] = Test.MyEnum.enum1;
                    di2["qwerty"] = Test.MyEnum.enum3;
                    di2["Hello!!"] = Test.MyEnum.enum2;

                    var cb = new Callback();
                    var ret = p.opStringMyEnumDAsync(di1, di2).Result;
                    cb.opStringMyEnumD(ret.returnValue, ret.p3);
                }

                {
                    var di1 = new Dictionary<string, Test.MyEnum>();
                    di1["abc"] = Test.MyEnum.enum1;
                    di1[""] = Test.MyEnum.enum2;
                    var di2 = new Dictionary<string, Test.MyEnum>();
                    di2["abc"] = Test.MyEnum.enum1;
                    di2["qwerty"] = Test.MyEnum.enum3;
                    di2["Hello!!"] = Test.MyEnum.enum2;

                    Callback cb = new Callback();
                    p.begin_opStringMyEnumD(di1, di2).whenCompleted(cb.opStringMyEnumD, cb.exCB);
                    cb.check();
                }

                {
                    var di1 = new Dictionary<string, Test.MyEnum>();
                    di1["abc"] = Test.MyEnum.enum1;
                    di1[""] = Test.MyEnum.enum2;
                    var di2 = new Dictionary<string, Test.MyEnum>();
                    di2["abc"] = Test.MyEnum.enum1;
                    di2["qwerty"] = Test.MyEnum.enum3;
                    di2["Hello!!"] = Test.MyEnum.enum2;

                    Callback cb = new Callback();
                    p.begin_opStringMyEnumD(di1, di2).whenCompleted(
                        (Dictionary<string, Test.MyEnum> ro, Dictionary<string, Test.MyEnum> _do) =>
                        {
                            cb.opStringMyEnumD(ro, _do);
                        },
                        (Ice.Exception ex) =>
                        {
                            cb.exCB(ex);
                        });
                    cb.check();
                }

                {
                    var di1 = new Dictionary<Test.MyEnum, string>();
                    di1[Test.MyEnum.enum1] = "abc";
                    var di2 = new Dictionary<Test.MyEnum, string>();
                    di2[Test.MyEnum.enum2] = "Hello!!";
                    di2[Test.MyEnum.enum3] = "qwerty";

                    var cb = new Callback();
                    var ret = p.opMyEnumStringDAsync(di1, di2).Result;
                    cb.opMyEnumStringD(ret.returnValue, ret.p3);
                }

                {
                    var di1 = new Dictionary<Test.MyEnum, string>();
                    di1[Test.MyEnum.enum1] = "abc";
                    var di2 = new Dictionary<Test.MyEnum, string>();
                    di2[Test.MyEnum.enum2] = "Hello!!";
                    di2[Test.MyEnum.enum3] = "qwerty";

                    Callback cb = new Callback();
                    p.begin_opMyEnumStringD(di1, di2).whenCompleted(cb.opMyEnumStringD, cb.exCB);
                    cb.check();
                }

                {
                    var di1 = new Dictionary<Test.MyEnum, string>();
                    di1[Test.MyEnum.enum1] = "abc";
                    var di2 = new Dictionary<Test.MyEnum, string>();
                    di2[Test.MyEnum.enum2] = "Hello!!";
                    di2[Test.MyEnum.enum3] = "qwerty";

                    Callback cb = new Callback();
                    p.begin_opMyEnumStringD(di1, di2).whenCompleted(
                        (Dictionary<Test.MyEnum, string> ro, Dictionary<Test.MyEnum, string> _do) =>
                        {
                            cb.opMyEnumStringD(ro, _do);
                        },
                        (Ice.Exception ex) =>
                        {
                            cb.exCB(ex);
                        });
                    cb.check();
                }

                {
                    var s11 = new Test.MyStruct(1, 1);
                    var s12 = new Test.MyStruct(1, 2);
                    var di1 = new Dictionary<Test.MyStruct, Test.MyEnum>();
                    di1[s11] = Test.MyEnum.enum1;
                    di1[s12] = Test.MyEnum.enum2;

                    var s22 = new Test.MyStruct(2, 2);
                    var s23 = new Test.MyStruct(2, 3);
                    var di2 = new Dictionary<Test.MyStruct, Test.MyEnum>();
                    di2[s11] = Test.MyEnum.enum1;
                    di2[s22] = Test.MyEnum.enum3;
                    di2[s23] = Test.MyEnum.enum2;

                    var cb = new Callback();
                    var ret = p.opMyStructMyEnumDAsync(di1, di2).Result;
                    cb.opMyStructMyEnumD(ret.returnValue, ret.p3);
                }

                {
                    var s11 = new Test.MyStruct(1, 1);
                    var s12 = new Test.MyStruct(1, 2);
                    var di1 = new Dictionary<Test.MyStruct, Test.MyEnum>();
                    di1[s11] = Test.MyEnum.enum1;
                    di1[s12] = Test.MyEnum.enum2;

                    var s22 = new Test.MyStruct(2, 2);
                    var s23 = new Test.MyStruct(2, 3);
                    var di2 = new Dictionary<Test.MyStruct, Test.MyEnum>();
                    di2[s11] = Test.MyEnum.enum1;
                    di2[s22] = Test.MyEnum.enum3;
                    di2[s23] = Test.MyEnum.enum2;

                    Callback cb = new Callback();
                    p.begin_opMyStructMyEnumD(di1, di2).whenCompleted(cb.opMyStructMyEnumD, cb.exCB);
                    cb.check();
                }

                {
                    var s11 = new Test.MyStruct(1, 1);
                    var s12 = new Test.MyStruct(1, 2);
                    var di1 = new Dictionary<Test.MyStruct, Test.MyEnum>();
                    di1[s11] = Test.MyEnum.enum1;
                    di1[s12] = Test.MyEnum.enum2;

                    var s22 = new Test.MyStruct(2, 2);
                    var s23 = new Test.MyStruct(2, 3);
                    var di2 = new Dictionary<Test.MyStruct, Test.MyEnum>();
                    di2[s11] = Test.MyEnum.enum1;
                    di2[s22] = Test.MyEnum.enum3;
                    di2[s23] = Test.MyEnum.enum2;

                    Callback cb = new Callback();
                    p.begin_opMyStructMyEnumD(di1, di2).whenCompleted(
                        (Dictionary<Test.MyStruct, Test.MyEnum> ro,
                         Dictionary<Test.MyStruct, Test.MyEnum> _do) =>
                        {
                            cb.opMyStructMyEnumD(ro, _do);
                        },
                        (Ice.Exception ex) =>
                        {
                            cb.exCB(ex);
                        });
                    cb.check();
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
                    var ret = p.opByteBoolDSAsync(dsi1, dsi2).Result;
                    cb.opByteBoolDS(ret.returnValue, ret.p3);
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

                    Callback cb = new Callback();
                    p.begin_opByteBoolDS(dsi1, dsi2).whenCompleted(
                        (Dictionary<byte, bool>[] ro,
                         Dictionary<byte, bool>[] _do) =>
                        {
                            cb.opByteBoolDS(ro, _do);
                        },
                        (Ice.Exception ex) =>
                        {
                            cb.exCB(ex);
                        });
                    cb.check();
                }

                {
                    Dictionary<short, int>[] dsi1 = new Dictionary<short, int>[2];
                    Dictionary<short, int>[] dsi2 = new Dictionary<short, int>[1];

                    Dictionary<short, int> di1 = new Dictionary<short, int>();
                    di1[110] = -1;
                    di1[1100] = 123123;
                    Dictionary<short, int> di2 = new Dictionary<short, int>();
                    di2[110] = -1;
                    di2[111] = -100;
                    di2[1101] = 0;
                    Dictionary<short, int> di3 = new Dictionary<short, int>();
                    di3[100] = -1001;

                    dsi1[0] = di1;
                    dsi1[1] = di2;
                    dsi2[0] = di3;

                    var cb = new Callback();
                    var ret = p.opShortIntDSAsync(dsi1, dsi2).Result;
                    cb.opShortIntDS(ret.returnValue, ret.p3);
                }

                {
                    Dictionary<short, int>[] dsi1 = new Dictionary<short, int>[2];
                    Dictionary<short, int>[] dsi2 = new Dictionary<short, int>[1];

                    Dictionary<short, int> di1 = new Dictionary<short, int>();
                    di1[110] = -1;
                    di1[1100] = 123123;
                    Dictionary<short, int> di2 = new Dictionary<short, int>();
                    di2[110] = -1;
                    di2[111] = -100;
                    di2[1101] = 0;
                    Dictionary<short, int> di3 = new Dictionary<short, int>();
                    di3[100] = -1001;

                    dsi1[0] = di1;
                    dsi1[1] = di2;
                    dsi2[0] = di3;

                    Callback cb = new Callback();
                    p.begin_opShortIntDS(dsi1, dsi2).whenCompleted(
                        (Dictionary<short, int>[] ro,
                         Dictionary<short, int>[] _do) =>
                        {
                            cb.opShortIntDS(ro, _do);
                        },
                        (Ice.Exception ex) =>
                        {
                            cb.exCB(ex);
                        });
                    cb.check();
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
                    var ret = p.opLongFloatDSAsync(dsi1, dsi2).Result;
                    cb.opLongFloatDS(ret.returnValue, ret.p3);
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

                    Callback cb = new Callback();
                    p.begin_opLongFloatDS(dsi1, dsi2).whenCompleted(
                        (Dictionary<long, float>[] ro,
                         Dictionary<long, float>[] _do) =>
                        {
                            cb.opLongFloatDS(ro, _do);
                        },
                        (Ice.Exception ex) =>
                        {
                            cb.exCB(ex);
                        });
                    cb.check();
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
                    var ret = p.opStringStringDSAsync(dsi1, dsi2).Result;
                    cb.opStringStringDS(ret.returnValue, ret.p3);
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

                    Callback cb = new Callback();
                    p.begin_opStringStringDS(dsi1, dsi2).whenCompleted(
                        (Dictionary<string, string>[] ro,
                         Dictionary<string, string>[] _do) =>
                        {
                            cb.opStringStringDS(ro, _do);
                        },
                        (Ice.Exception ex) =>
                        {
                            cb.exCB(ex);
                        });
                    cb.check();
                }

                {
                    var dsi1 = new Dictionary<string, Test.MyEnum>[2];
                    var dsi2 = new Dictionary<string, Test.MyEnum>[1];

                    var di1 = new Dictionary<string, Test.MyEnum>();
                    di1["abc"] = Test.MyEnum.enum1;
                    di1[""] = Test.MyEnum.enum2;
                    var di2 = new Dictionary<string, Test.MyEnum>();
                    di2["abc"] = Test.MyEnum.enum1;
                    di2["qwerty"] = Test.MyEnum.enum3;
                    di2["Hello!!"] = Test.MyEnum.enum2;
                    var di3 = new Dictionary<string, Test.MyEnum>();
                    di3["Goodbye"] = Test.MyEnum.enum1;

                    dsi1[0] = di1;
                    dsi1[1] = di2;
                    dsi2[0] = di3;

                    var cb = new Callback();
                    var ret = p.opStringMyEnumDSAsync(dsi1, dsi2).Result;
                    cb.opStringMyEnumDS(ret.returnValue, ret.p3);
                }

                {
                    var dsi1 = new Dictionary<string, Test.MyEnum>[2];
                    var dsi2 = new Dictionary<string, Test.MyEnum>[1];

                    var di1 = new Dictionary<string, Test.MyEnum>();
                    di1["abc"] = Test.MyEnum.enum1;
                    di1[""] = Test.MyEnum.enum2;
                    var di2 = new Dictionary<string, Test.MyEnum>();
                    di2["abc"] = Test.MyEnum.enum1;
                    di2["qwerty"] = Test.MyEnum.enum3;
                    di2["Hello!!"] = Test.MyEnum.enum2;
                    var di3 = new Dictionary<string, Test.MyEnum>();
                    di3["Goodbye"] = Test.MyEnum.enum1;

                    dsi1[0] = di1;
                    dsi1[1] = di2;
                    dsi2[0] = di3;

                    Callback cb = new Callback();
                    p.begin_opStringMyEnumDS(dsi1, dsi2).whenCompleted(
                        (Dictionary<string, Test.MyEnum>[] ro,
                         Dictionary<string, Test.MyEnum>[] _do) =>
                        {
                            cb.opStringMyEnumDS(ro, _do);
                        },
                        (Ice.Exception ex) =>
                        {
                            cb.exCB(ex);
                        });
                    cb.check();
                }

                {
                    var dsi1 = new Dictionary<Test.MyEnum, string>[2];
                    var dsi2 = new Dictionary<Test.MyEnum, string>[1];

                    var di1 = new Dictionary<Test.MyEnum, string>();
                    di1[Test.MyEnum.enum1] = "abc";
                    var di2 = new Dictionary<Test.MyEnum, string>();
                    di2[Test.MyEnum.enum2] = "Hello!!";
                    di2[Test.MyEnum.enum3] = "qwerty";
                    var di3 = new Dictionary<Test.MyEnum, string>();
                    di3[Test.MyEnum.enum1] = "Goodbye";

                    dsi1[0] = di1;
                    dsi1[1] = di2;
                    dsi2[0] = di3;

                    var cb = new Callback();
                    var ret = p.opMyEnumStringDSAsync(dsi1, dsi2).Result;
                    cb.opMyEnumStringDS(ret.returnValue, ret.p3);
                }

                {
                    var dsi1 = new Dictionary<Test.MyEnum, string>[2];
                    var dsi2 = new Dictionary<Test.MyEnum, string>[1];

                    var di1 = new Dictionary<Test.MyEnum, string>();
                    di1[Test.MyEnum.enum1] = "abc";
                    var di2 = new Dictionary<Test.MyEnum, string>();
                    di2[Test.MyEnum.enum2] = "Hello!!";
                    di2[Test.MyEnum.enum3] = "qwerty";
                    var di3 = new Dictionary<Test.MyEnum, string>();
                    di3[Test.MyEnum.enum1] = "Goodbye";

                    dsi1[0] = di1;
                    dsi1[1] = di2;
                    dsi2[0] = di3;

                    Callback cb = new Callback();
                    p.begin_opMyEnumStringDS(dsi1, dsi2).whenCompleted(
                        (Dictionary<Test.MyEnum, string>[] ro,
                         Dictionary<Test.MyEnum, string>[] _do) =>
                        {
                            cb.opMyEnumStringDS(ro, _do);
                        },
                        (Ice.Exception ex) =>
                        {
                            cb.exCB(ex);
                        });
                    cb.check();
                }

                {
                    var dsi1 = new Dictionary<Test.MyStruct, Test.MyEnum>[2];
                    var dsi2 = new Dictionary<Test.MyStruct, Test.MyEnum>[1];

                    var s11 = new Test.MyStruct(1, 1);
                    var s12 = new Test.MyStruct(1, 2);
                    var di1 = new Dictionary<Test.MyStruct, Test.MyEnum>();
                    di1[s11] = Test.MyEnum.enum1;
                    di1[s12] = Test.MyEnum.enum2;

                    var s22 = new Test.MyStruct(2, 2);
                    var s23 = new Test.MyStruct(2, 3);
                    var di2 = new Dictionary<Test.MyStruct, Test.MyEnum>();
                    di2[s11] = Test.MyEnum.enum1;
                    di2[s22] = Test.MyEnum.enum3;
                    di2[s23] = Test.MyEnum.enum2;

                    var di3 = new Dictionary<Test.MyStruct, Test.MyEnum>();
                    di3[s23] = Test.MyEnum.enum3;

                    dsi1[0] = di1;
                    dsi1[1] = di2;
                    dsi2[0] = di3;

                    var cb = new Callback();
                    var ret = p.opMyStructMyEnumDSAsync(dsi1, dsi2).Result;
                    cb.opMyStructMyEnumDS(ret.returnValue, ret.p3);
                }

                {
                    var dsi1 = new Dictionary<Test.MyStruct, Test.MyEnum>[2];
                    var dsi2 = new Dictionary<Test.MyStruct, Test.MyEnum>[1];

                    var s11 = new Test.MyStruct(1, 1);
                    var s12 = new Test.MyStruct(1, 2);
                    var di1 = new Dictionary<Test.MyStruct, Test.MyEnum>();
                    di1[s11] = Test.MyEnum.enum1;
                    di1[s12] = Test.MyEnum.enum2;

                    var s22 = new Test.MyStruct(2, 2);
                    var s23 = new Test.MyStruct(2, 3);
                    var di2 = new Dictionary<Test.MyStruct, Test.MyEnum>();
                    di2[s11] = Test.MyEnum.enum1;
                    di2[s22] = Test.MyEnum.enum3;
                    di2[s23] = Test.MyEnum.enum2;

                    var di3 = new Dictionary<Test.MyStruct, Test.MyEnum>();
                    di3[s23] = Test.MyEnum.enum3;

                    dsi1[0] = di1;
                    dsi1[1] = di2;
                    dsi2[0] = di3;

                    Callback cb = new Callback();
                    p.begin_opMyStructMyEnumDS(dsi1, dsi2).whenCompleted(
                        (Dictionary<Test.MyStruct, Test.MyEnum>[] ro,
                         Dictionary<Test.MyStruct, Test.MyEnum>[] _do) =>
                        {
                            cb.opMyStructMyEnumDS(ro, _do);
                        },
                        (Ice.Exception ex) =>
                        {
                            cb.exCB(ex);
                        });
                    cb.check();
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
                    var ret = p.opByteByteSDAsync(sdi1, sdi2).Result;
                    cb.opByteByteSD(ret.returnValue, ret.p3);
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

                    Callback cb = new Callback();
                    p.begin_opByteByteSD(sdi1, sdi2).whenCompleted(
                        (Dictionary<byte, byte[]> ro,
                         Dictionary<byte, byte[]> _do) =>
                        {
                            cb.opByteByteSD(ro, _do);
                        },
                        (Ice.Exception ex) =>
                        {
                            cb.exCB(ex);
                        });
                    cb.check();
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
                    var ret = p.opBoolBoolSDAsync(sdi1, sdi2).Result;
                    cb.opBoolBoolSD(ret.returnValue, ret.p3);
                }

                {
                    Dictionary<bool, bool[]> sdi1 = new Dictionary<bool, bool[]>();
                    Dictionary<bool, bool[]> sdi2 = new Dictionary<bool, bool[]>();

                    bool[] si1 = new bool[] { true, false };
                    bool[] si2 = new bool[] { false, true, true };

                    sdi1[false] = si1;
                    sdi1[true] = si2;
                    sdi2[false] = si1;

                    Callback cb = new Callback();
                    p.begin_opBoolBoolSD(sdi1, sdi2).whenCompleted(
                        (Dictionary<bool, bool[]> ro,
                         Dictionary<bool, bool[]> _do) =>
                        {
                            cb.opBoolBoolSD(ro, _do);
                        },
                        (Ice.Exception ex) =>
                        {
                            cb.exCB(ex);
                        });
                    cb.check();
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
                    var ret = p.opShortShortSDAsync(sdi1, sdi2).Result;
                    cb.opShortShortSD(ret.returnValue, ret.p3);
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

                    Callback cb = new Callback();
                    p.begin_opShortShortSD(sdi1, sdi2).whenCompleted(
                        (Dictionary<short, short[]> ro,
                         Dictionary<short, short[]> _do) =>
                        {
                            cb.opShortShortSD(ro, _do);
                        },
                        (Ice.Exception ex) =>
                        {
                            cb.exCB(ex);
                        });
                    cb.check();
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
                    var ret = p.opIntIntSDAsync(sdi1, sdi2).Result;
                    cb.opIntIntSD(ret.returnValue, ret.p3);
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

                    Callback cb = new Callback();
                    p.begin_opIntIntSD(sdi1, sdi2).whenCompleted(
                        (Dictionary<int, int[]> ro,
                         Dictionary<int, int[]> _do) =>
                        {
                            cb.opIntIntSD(ro, _do);
                        },
                        (Ice.Exception ex) =>
                        {
                            cb.exCB(ex);
                        });
                    cb.check();
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
                    var ret = p.opLongLongSDAsync(sdi1, sdi2).Result;
                    cb.opLongLongSD(ret.returnValue, ret.p3);
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

                    Callback cb = new Callback();
                    p.begin_opLongLongSD(sdi1, sdi2).whenCompleted(
                        (Dictionary<long, long[]> ro,
                         Dictionary<long, long[]> _do) =>
                        {
                            cb.opLongLongSD(ro, _do);
                        },
                        (Ice.Exception ex) =>
                        {
                            cb.exCB(ex);
                        });
                    cb.check();
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
                    var ret = p.opStringFloatSDAsync(sdi1, sdi2).Result;
                    cb.opStringFloatSD(ret.returnValue, ret.p3);
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

                    Callback cb = new Callback();
                    p.begin_opStringFloatSD(sdi1, sdi2).whenCompleted(
                        (Dictionary<string, float[]> ro,
                         Dictionary<string, float[]> _do) =>
                        {
                            cb.opStringFloatSD(ro, _do);
                        },
                        (Ice.Exception ex) =>
                        {
                            cb.exCB(ex);
                        });
                    cb.check();
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
                    var ret = p.opStringDoubleSDAsync(sdi1, sdi2).Result;
                    cb.opStringDoubleSD(ret.returnValue, ret.p3);
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

                    Callback cb = new Callback();
                    p.begin_opStringDoubleSD(sdi1, sdi2).whenCompleted(
                        (Dictionary<string, double[]> ro,
                         Dictionary<string, double[]> _do) =>
                        {
                            cb.opStringDoubleSD(ro, _do);
                        },
                        (Ice.Exception ex) =>
                        {
                            cb.exCB(ex);
                        });
                    cb.check();
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
                    var ret = p.opStringStringSDAsync(sdi1, sdi2).Result;
                    cb.opStringStringSD(ret.returnValue, ret.p3);
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

                    Callback cb = new Callback();
                    p.begin_opStringStringSD(sdi1, sdi2).whenCompleted(
                        (Dictionary<string, string[]> ro,
                         Dictionary<string, string[]> _do) =>
                        {
                            cb.opStringStringSD(ro, _do);
                        },
                        (Ice.Exception ex) =>
                        {
                            cb.exCB(ex);
                        });
                    cb.check();
                }

                {
                    var sdi1 = new Dictionary<Test.MyEnum, Test.MyEnum[]>();
                    var sdi2 = new Dictionary<Test.MyEnum, Test.MyEnum[]>();

                    var si1 = new Test.MyEnum[] { Test.MyEnum.enum1, Test.MyEnum.enum1, Test.MyEnum.enum2 };
                    var si2 = new Test.MyEnum[] { Test.MyEnum.enum1, Test.MyEnum.enum2 };
                    var si3 = new Test.MyEnum[] { Test.MyEnum.enum3, Test.MyEnum.enum3 };

                    sdi1[Test.MyEnum.enum3] = si1;
                    sdi1[Test.MyEnum.enum2] = si2;
                    sdi2[Test.MyEnum.enum1] = si3;

                    var cb = new Callback();
                    var ret = p.opMyEnumMyEnumSDAsync(sdi1, sdi2).Result;
                    cb.opMyEnumMyEnumSD(ret.returnValue, ret.p3);
                }

                {
                    var sdi1 = new Dictionary<Test.MyEnum, Test.MyEnum[]>();
                    var sdi2 = new Dictionary<Test.MyEnum, Test.MyEnum[]>();

                    var si1 = new Test.MyEnum[] { Test.MyEnum.enum1, Test.MyEnum.enum1, Test.MyEnum.enum2 };
                    var si2 = new Test.MyEnum[] { Test.MyEnum.enum1, Test.MyEnum.enum2 };
                    var si3 = new Test.MyEnum[] { Test.MyEnum.enum3, Test.MyEnum.enum3 };

                    sdi1[Test.MyEnum.enum3] = si1;
                    sdi1[Test.MyEnum.enum2] = si2;
                    sdi2[Test.MyEnum.enum1] = si3;

                    Callback cb = new Callback();
                    p.begin_opMyEnumMyEnumSD(sdi1, sdi2).whenCompleted(
                        (Dictionary<Test.MyEnum, Test.MyEnum[]> ro,
                         Dictionary<Test.MyEnum, Test.MyEnum[]> _do) =>
                        {
                            cb.opMyEnumMyEnumSD(ro, _do);
                        },
                        (Ice.Exception ex) =>
                        {
                            cb.exCB(ex);
                        });
                    cb.check();
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
                        cb.opIntS(p.opIntSAsync(s).Result);
                    }
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

                        Callback cb = new Callback(lengths[l]);
                        p.begin_opIntS(s).whenCompleted(cb.opIntS, cb.exCB);
                        cb.check();
                    }
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

                        Callback cb = new Callback(lengths[l]);
                        p.begin_opIntS(s).whenCompleted(
                            (int[] r) =>
                            {
                                cb.opIntS(r);
                            },
                            (Ice.Exception ex) =>
                            {
                                cb.exCB(ex);
                            });
                        cb.check();
                    }
                }

                {
                    Dictionary<string, string> ctx = new Dictionary<string, string>();
                    ctx["one"] = "ONE";
                    ctx["two"] = "TWO";
                    ctx["three"] = "THREE";
                    {
                        test(p.ice_getContext().Count == 0);
                        var cb = new Callback(ctx);
                        cb.opContextNotEqual(p.opContextAsync().Result);
                    }
                    {
                        test(p.ice_getContext().Count == 0);
                        var cb = new Callback(ctx);
                        cb.opContextEqual(p.opContextAsync(ctx).Result);
                    }
                    {
                        var p2 = Test.MyClassPrxHelper.checkedCast(p.ice_context(ctx));
                        test(Ice.CollectionComparer.Equals(p2.ice_getContext(), ctx));
                        var cb = new Callback(ctx);
                        cb.opContextEqual(p2.opContextAsync().Result);
                    }
                    {
                        var p2 = Test.MyClassPrxHelper.checkedCast(p.ice_context(ctx));
                        Callback cb = new Callback(ctx);
                        cb.opContextEqual(p2.opContextAsync(ctx).Result);
                    }
                }

                {
                    Dictionary<string, string> ctx = new Dictionary<string, string>();
                    ctx["one"] = "ONE";
                    ctx["two"] = "TWO";
                    ctx["three"] = "THREE";
                    {
                        test(p.ice_getContext().Count == 0);
                        Callback cb = new Callback(ctx);
                        p.begin_opContext().whenCompleted(cb.opContextNotEqual, cb.exCB);
                        cb.check();
                    }
                    {
                        test(p.ice_getContext().Count == 0);
                        Callback cb = new Callback(ctx);
                        p.begin_opContext(ctx).whenCompleted(cb.opContextEqual, cb.exCB);
                        cb.check();
                    }
                    {
                        var p2 = Test.MyClassPrxHelper.checkedCast(p.ice_context(ctx));
                        test(Ice.CollectionComparer.Equals(p2.ice_getContext(), ctx));
                        Callback cb = new Callback(ctx);
                        p2.begin_opContext().whenCompleted(cb.opContextEqual, cb.exCB);
                        cb.check();
                    }
                    {
                        var p2 = Test.MyClassPrxHelper.checkedCast(p.ice_context(ctx));
                        Callback cb = new Callback(ctx);
                        p2.begin_opContext(ctx).whenCompleted(cb.opContextEqual, cb.exCB);
                        cb.check();
                    }
                }

                {
                    Dictionary<string, string> ctx = new Dictionary<string, string>();
                    ctx["one"] = "ONE";
                    ctx["two"] = "TWO";
                    ctx["three"] = "THREE";
                    {
                        test(p.ice_getContext().Count == 0);
                        Callback cb = new Callback(ctx);
                        p.begin_opContext().whenCompleted(
                            (Dictionary<string, string> r) =>
                            {
                                cb.opContextNotEqual(r);
                            },
                            (Ice.Exception ex) =>
                            {
                                cb.exCB(ex);
                            });
                        cb.check();
                    }
                    {
                        test(p.ice_getContext().Count == 0);
                        Callback cb = new Callback(ctx);
                        p.begin_opContext(ctx).whenCompleted(
                            (Dictionary<string, string> r) =>
                            {
                                cb.opContextEqual(r);
                            },
                            (Ice.Exception ex) =>
                            {
                                cb.exCB(ex);
                            });
                        cb.check();
                    }
                    {
                        var p2 = Test.MyClassPrxHelper.checkedCast(p.ice_context(ctx));
                        test(Ice.CollectionComparer.Equals(p2.ice_getContext(), ctx));
                        Callback cb = new Callback(ctx);
                        p2.begin_opContext().whenCompleted(
                            (Dictionary<string, string> r) =>
                            {
                                cb.opContextEqual(r);
                            },
                            (Ice.Exception ex) =>
                            {
                                cb.exCB(ex);
                            });
                        cb.check();
                    }
                    {
                        var p2 = Test.MyClassPrxHelper.checkedCast(p.ice_context(ctx));
                        Callback cb = new Callback(ctx);
                        p2.begin_opContext(ctx).whenCompleted(
                            (Dictionary<string, string> r) =>
                            {
                                cb.opContextEqual(r);
                            },
                            (Ice.Exception ex) =>
                            {
                                cb.exCB(ex);
                            });
                        cb.check();
                    }
                }

                //
                // Test implicit context propagation with async task
                //
                if (p.ice_getConnection() != null)
                {
                    string[] impls = { "Shared", "PerThread" };
                    for (int i = 0; i < 2; i++)
                    {
                        Ice.InitializationData initData = new Ice.InitializationData();
                        initData.properties = communicator.getProperties().ice_clone_();
                        initData.properties.setProperty("Ice.ImplicitContext", impls[i]);

                        Ice.Communicator ic = helper.initialize(initData);

                        Dictionary<string, string> ctx = new Dictionary<string, string>();
                        ctx["one"] = "ONE";
                        ctx["two"] = "TWO";
                        ctx["three"] = "THREE";

                        var p3 =
                            Test.MyClassPrxHelper.uncheckedCast(ic.stringToProxy("test:" + helper.getTestEndpoint(0)));

                        ic.getImplicitContext().setContext(ctx);
                        test(Ice.CollectionComparer.Equals(ic.getImplicitContext().getContext(), ctx));
                        {
                            test(Ice.CollectionComparer.Equals(p3.opContextAsync().Result, ctx));
                        }

                        ic.getImplicitContext().put("zero", "ZERO");

                        ctx = ic.getImplicitContext().getContext();
                        {
                            test(Ice.CollectionComparer.Equals(p3.opContextAsync().Result, ctx));
                        }

                        Dictionary<string, string> prxContext = new Dictionary<string, string>();
                        prxContext["one"] = "UN";
                        prxContext["four"] = "QUATRE";

                        Dictionary<string, string> combined = prxContext;
                        foreach (KeyValuePair<string, string> e in ctx)
                        {
                            try
                            {
                                combined.Add(e.Key, e.Value);
                            }
                            catch (System.ArgumentException)
                            {
                                // Ignore.
                            }
                        }
                        test(combined["one"].Equals("UN"));

                        p3 = Test.MyClassPrxHelper.uncheckedCast(p.ice_context(prxContext));

                        ic.getImplicitContext().setContext(null);
                        {
                            test(Ice.CollectionComparer.Equals(p3.opContextAsync().Result, prxContext));
                        }

                        ic.getImplicitContext().setContext(ctx);
                        {
                            test(Ice.CollectionComparer.Equals(p3.opContextAsync().Result, combined));
                        }

                        //ic.getImplicitContext().setContext(null);
                        ic.destroy();
                    }
                }

                //
                // Test implicit context propagation with async result
                //
                if (p.ice_getConnection() != null)
                {
                    string[] impls = { "Shared", "PerThread" };
                    for (int i = 0; i < 2; i++)
                    {
                        Ice.InitializationData initData = new Ice.InitializationData();
                        initData.properties = communicator.getProperties().ice_clone_();
                        initData.properties.setProperty("Ice.ImplicitContext", impls[i]);

                        Ice.Communicator ic = helper.initialize(initData);

                        Dictionary<string, string> ctx = new Dictionary<string, string>();
                        ctx["one"] = "ONE";
                        ctx["two"] = "TWO";
                        ctx["three"] = "THREE";

                        var p3 =
                            Test.MyClassPrxHelper.uncheckedCast(ic.stringToProxy("test:" + helper.getTestEndpoint(0)));

                        ic.getImplicitContext().setContext(ctx);
                        test(Ice.CollectionComparer.Equals(ic.getImplicitContext().getContext(), ctx));
                        {
                            Ice.AsyncResult r = p3.begin_opContext();
                            Dictionary<string, string> c = p3.end_opContext(r);
                            test(Ice.CollectionComparer.Equals(c, ctx));
                        }

                        ic.getImplicitContext().put("zero", "ZERO");

                        ctx = ic.getImplicitContext().getContext();
                        {
                            Ice.AsyncResult r = p3.begin_opContext();
                            Dictionary<string, string> c = p3.end_opContext(r);
                            test(Ice.CollectionComparer.Equals(c, ctx));
                        }

                        Dictionary<string, string> prxContext = new Dictionary<string, string>();
                        prxContext["one"] = "UN";
                        prxContext["four"] = "QUATRE";

                        Dictionary<string, string> combined = prxContext;
                        foreach (KeyValuePair<string, string> e in ctx)
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
                        test(combined["one"].Equals("UN"));

                        p3 = Test.MyClassPrxHelper.uncheckedCast(p.ice_context(prxContext));

                        ic.getImplicitContext().setContext(null);
                        {
                            Ice.AsyncResult r = p3.begin_opContext();
                            Dictionary<string, string> c = p3.end_opContext(r);
                            test(Ice.CollectionComparer.Equals(c, prxContext));
                        }

                        ic.getImplicitContext().setContext(ctx);
                        {
                            Ice.AsyncResult r = p3.begin_opContext();
                            Dictionary<string, string> c = p3.end_opContext(r);
                            test(Ice.CollectionComparer.Equals(c, combined));
                        }

                        //ic.getImplicitContext().setContext(null);
                        ic.destroy();
                    }
                }

                {
                    p.opIdempotentAsync().Wait();
                }

                {
                    Ice.AsyncResult r = p.begin_opIdempotent();
                    p.end_opIdempotent(r);
                }

                {
                    Callback cb = new Callback();
                    p.begin_opIdempotent().whenCompleted(cb.opIdempotent, cb.exCB);
                    cb.check();
                }

                {
                    Callback cb = new Callback();
                    p.begin_opIdempotent().whenCompleted(
                        () =>
                        {
                            cb.opIdempotent();
                        },
                        (Ice.Exception ex) =>
                        {
                            cb.exCB(ex);
                        });
                    cb.check();
                }

                {
                    p.opNonmutatingAsync().Wait();
                }

                {
                    Ice.AsyncResult r = p.begin_opNonmutating();
                    p.end_opNonmutating(r);
                }

                {
                    Callback cb = new Callback();
                    p.begin_opNonmutating().whenCompleted(cb.opNonmutating, cb.exCB);
                    cb.check();
                }

                {
                    Callback cb = new Callback();
                    p.begin_opNonmutating().whenCompleted(
                        () =>
                        {
                            cb.opNonmutating();
                        },
                        (Ice.Exception ex) =>
                        {
                            cb.exCB(ex);
                        });
                    cb.check();
                }

                {
                    var derived = Test.MyDerivedClassPrxHelper.checkedCast(p);
                    test(derived != null);
                    var cb = new Callback();
                    derived.opDerivedAsync().Wait();
                }

                {
                    var derived = Test.MyDerivedClassPrxHelper.checkedCast(p);
                    test(derived != null);
                    Callback cb = new Callback();
                    derived.begin_opDerived().whenCompleted(cb.opDerived, cb.exCB);
                    cb.check();
                }

                {
                    var derived = Test.MyDerivedClassPrxHelper.checkedCast(p);
                    test(derived != null);
                    Callback cb = new Callback();
                    derived.begin_opDerived().whenCompleted(
                        () =>
                        {
                            cb.opDerived();
                        },
                        (Ice.Exception ex) =>
                        {
                            cb.exCB(ex);
                        });
                    cb.check();
                }

                {
                    test(p.opByte1Async(0xFF).Result == 0xFF);
                }

                {
                    GenericCallback<byte> cb = new GenericCallback<byte>(0);
                    p.begin_opByte1(0xFF).whenCompleted(
                        (byte value) =>
                        {
                            cb.response(value);
                        },
                        (Ice.Exception ex) =>
                        {
                            cb.exception(ex);
                        });
                    test(cb.succeeded() && cb.value() == 0xFF);
                }

                {
                    test(p.opShort1Async(0x7FFF).Result == 0x7FFF);
                }

                {
                    GenericCallback<short> cb = new GenericCallback<short>(0);
                    p.begin_opShort1(0x7FFF).whenCompleted(
                        (short value) =>
                        {
                            cb.response(value);
                        },
                        (Ice.Exception ex) =>
                        {
                            cb.exception(ex);
                        });
                    test(cb.succeeded() && cb.value() == 0x7FFF);
                }

                {
                    test(p.opInt1Async(0x7FFFFFFF).Result == 0x7FFFFFFF);
                }

                {
                    GenericCallback<int> cb = new GenericCallback<int>(0);
                    p.begin_opInt1(0x7FFFFFFF).whenCompleted(
                        (int value) =>
                        {
                            cb.response(value);
                        },
                        (Ice.Exception ex) =>
                        {
                            cb.exception(ex);
                        });
                    test(cb.succeeded() && cb.value() == 0x7FFFFFFF);
                }

                {
                    test(p.opLong1Async(0x7FFFFFFFFFFFFFFF).Result == 0x7FFFFFFFFFFFFFFF);
                }

                {
                    GenericCallback<long> cb = new GenericCallback<long>(0);
                    p.begin_opLong1(0x7FFFFFFFFFFFFFFF).whenCompleted(
                        (long value) =>
                        {
                            cb.response(value);
                        },
                        (Ice.Exception ex) =>
                        {
                            cb.exception(ex);
                        });
                    test(cb.succeeded() && cb.value() == 0x7FFFFFFFFFFFFFFF);
                }

                {
                    test(p.opFloat1Async(1.0f).Result == 1.0f);
                }

                {
                    GenericCallback<float> cb = new GenericCallback<float>(0);
                    p.begin_opFloat1(1.0f).whenCompleted(
                        (float value) =>
                        {
                            cb.response(value);
                        },
                        (Ice.Exception ex) =>
                        {
                            cb.exception(ex);
                        });
                    test(cb.succeeded() && cb.value() == 1.0f);
                }

                {
                    test(p.opDouble1Async(1.0d).Result == 1.0d);
                }

                {
                    GenericCallback<double> cb = new GenericCallback<double>(0);
                    p.begin_opDouble1(1.0d).whenCompleted(
                        (double value) =>
                        {
                            cb.response(value);
                        },
                        (Ice.Exception ex) =>
                        {
                            cb.exception(ex);
                        });
                    test(cb.succeeded() && cb.value() == 1.0d);
                }

                {
                    test(p.opString1Async("opString1").Result.Equals("opString1"));
                }

                {
                    GenericCallback<string> cb = new GenericCallback<string>("");
                    p.begin_opString1("opString1").whenCompleted(
                        (string value) =>
                        {
                            cb.response(value);
                        },
                        (Ice.Exception ex) =>
                        {
                            cb.exception(ex);
                        });
                    test(cb.succeeded() && cb.value().Equals("opString1"));
                }

                {
                    test(p.opStringS1Async(null).Result.Length == 0);
                }

                {
                    GenericCallback<string[]> cb = new GenericCallback<string[]>(null);
                    p.begin_opStringS1(null).whenCompleted(
                        (string[] seq) =>
                        {
                            cb.response(seq);
                        },
                        (Ice.Exception ex) =>
                        {
                            cb.exception(ex);
                        });
                    test(cb.succeeded() && cb.value().Length == 0);
                }

                {
                    test(p.opByteBoolD1Async(null).Result.Count == 0);
                }

                {
                    GenericCallback<Dictionary<byte, bool>> cb = new GenericCallback<Dictionary<byte, bool>>(null);
                    p.begin_opByteBoolD1(null).whenCompleted(
                        (Dictionary<byte, bool> value) =>
                        {
                            cb.response(value);
                        },
                        (Ice.Exception ex) =>
                        {
                            cb.exception(ex);
                        });
                    test(cb.succeeded() && cb.value().Count == 0);
                }

                {
                    test(p.opStringS2Async(null).Result.Length == 0);
                }

                {
                    GenericCallback<string[]> cb = new GenericCallback<string[]>(null);
                    p.begin_opStringS2(null).whenCompleted(
                        (string[] seq) =>
                        {
                            cb.response(seq);
                        },
                        (Ice.Exception ex) =>
                        {
                            cb.exception(ex);
                        });
                    test(cb.succeeded() && cb.value().Length == 0);
                }

                {
                    test(p.opByteBoolD2Async(null).Result.Count == 0);
                }

                {
                    GenericCallback<Dictionary<byte, bool>> cb = new GenericCallback<Dictionary<byte, bool>>(null);
                    p.begin_opByteBoolD2(null).whenCompleted(
                        (Dictionary<byte, bool> value) =>
                        {
                            cb.response(value);
                        },
                        (Ice.Exception ex) =>
                        {
                            cb.exception(ex);
                        });
                    test(cb.succeeded() && cb.value().Count == 0);
                }

                Func<Task> task = async () =>
                {
                    {
                        var p1 = await p.opMStruct1Async();

                        p1.e = Test.MyEnum.enum3;
                        var r = await p.opMStruct2Async(p1);
                        test(r.p2.Equals(p1) && r.returnValue.Equals(p1));
                    }

                    {
                        await p.opMSeq1Async();

                        var p1 = new string[1];
                        p1[0] = "test";
                        var r = await p.opMSeq2Async(p1);
                        test(Ice.CollectionComparer.Equals(r.p2, p1) &&
                             Ice.CollectionComparer.Equals(r.returnValue, p1));
                    }

                    {
                        await p.opMDict1Async();

                        var p1 = new Dictionary<string, string>();
                        p1["test"] = "test";
                        var r = await p.opMDict2Async(p1);
                        test(Ice.CollectionComparer.Equals(r.p2, p1) &&
                             Ice.CollectionComparer.Equals(r.returnValue, p1));
                    }
                };
            }
        }
    }
}
