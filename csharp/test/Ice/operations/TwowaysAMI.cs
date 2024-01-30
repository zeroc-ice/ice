//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

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
                    test(s == "world hello");
                    test(r == "hello world");
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
                    test(rso.s.s == "def");
                    test(so.e == Test.MyEnum.enum3);
                    test(so.s.s == "a new string");

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
                    test(sso[0] == "abc");
                    test(sso[1] == "de");
                    test(sso[2] == "fghi");
                    test(sso[3] == "xyz");
                    test(rso.Length == 3);
                    test(rso[0] == "fghi");
                    test(rso[1] == "de");
                    test(rso[2] == "abc");
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
                    test(sso[0][0] == "abc");
                    test(sso[1].Length == 2);
                    test(sso[1][0] == "de");
                    test(sso[1][1] == "fghi");
                    test(sso[2].Length == 0);
                    test(sso[3].Length == 0);
                    test(sso[4].Length == 1);
                    test(sso[4][0] == "xyz");
                    test(rso.Length == 3);
                    test(rso[0].Length == 1);
                    test(rso[0][0] == "xyz");
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
                    test(ssso[0][0][0] == "abc");
                    test(ssso[0][0][1] == "de");
                    test(ssso[0][1][0] == "xyz");
                    test(ssso[1][0][0] == "hello");
                    test(ssso[2][0][0] == "");
                    test(ssso[2][0][1] == "");
                    test(ssso[2][1][0] == "abcd");
                    test(ssso[3][0][0] == "");

                    test(rsso.Length == 3);
                    test(rsso[0].Length == 0);
                    test(rsso[1].Length == 1);
                    test(rsso[1][0].Length == 1);
                    test(rsso[2].Length == 2);
                    test(rsso[2][0].Length == 2);
                    test(rsso[2][1].Length == 1);
                    test(rsso[1][0][0] == "");
                    test(rsso[2][0][0] == "");
                    test(rsso[2][0][1] == "");
                    test(rsso[2][1][0] == "abcd");
                    called();
                }

                public void opByteBoolD(Dictionary<byte, bool> ro, Dictionary<byte, bool> _do)
                {
                    Dictionary<byte, bool> di1 = new Dictionary<byte, bool>();
                    di1[10] = true;
                    di1[100] = false;
                    test(Ice.CollectionComparer.Equals(_do, di1));
                    test(ro.Count == 4);
                    test(ro[10] == true);
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
                    test(ro[110] == -1);
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
                    test(ro["foo"] == "abc -1.1");
                    test(ro["FOO"] == "abc -100.4");
                    test(ro["bar"] == "abc 123123.2");
                    test(ro["BAR"] == "abc 0.5");
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
                    test(ro[Test.MyEnum.enum1] == "abc");
                    test(ro[Test.MyEnum.enum2] == "Hello!!");
                    test(ro[Test.MyEnum.enum3] == "qwerty");
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
                    test(ro[0]["foo"] == "abc -1.1");
                    test(ro[0]["FOO"] == "abc -100.4");
                    test(ro[0]["BAR"] == "abc 0.5");
                    test(ro[1].Count == 2);
                    test(ro[1]["foo"] == "abc -1.1");
                    test(ro[1]["bar"] == "abc 123123.2");

                    test(_do.Length == 3);
                    test(_do[0].Count == 1);
                    test(_do[0]["f00"] == "ABC -3.14");
                    test(_do[1].Count == 2);
                    test(_do[1]["foo"] == "abc -1.1");
                    test(_do[1]["bar"] == "abc 123123.2");
                    test(_do[2].Count == 3);
                    test(_do[2]["foo"] == "abc -1.1");
                    test(_do[2]["FOO"] == "abc -100.4");
                    test(_do[2]["BAR"] == "abc 0.5");
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
                    test(ro[0][Test.MyEnum.enum2] == "Hello!!");
                    test(ro[0][Test.MyEnum.enum3] == "qwerty");
                    test(ro[1].Count == 1);
                    test(ro[1][Test.MyEnum.enum1] == "abc");

                    test(_do.Length == 3);
                    test(_do[0].Count == 1);
                    test(_do[0][Test.MyEnum.enum1] == "Goodbye");
                    test(_do[1].Count == 1);
                    test(_do[1][Test.MyEnum.enum1] == "abc");
                    test(_do[2].Count == 2);
                    test(_do[2][Test.MyEnum.enum2] == "Hello!!");
                    test(_do[2][Test.MyEnum.enum3] == "qwerty");
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
                    test(_do["ghi"][0] == "and");
                    test(_do["ghi"][1] == "xor");

                    test(ro.Count == 3);
                    test(ro["abc"].Length == 3);
                    test(ro["abc"][0] == "abc");
                    test(ro["abc"][1] == "de");
                    test(ro["abc"][2] == "fghi");
                    test(ro["def"].Length == 2);
                    test(ro["def"][0] == "xyz");
                    test(ro["def"][1] == "or");
                    test(ro["ghi"].Length == 2);
                    test(ro["ghi"][0] == "and");
                    test(ro["ghi"][1] == "xor");
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

            internal static async Task twowaysAMIAsync(global::Test.TestHelper helper, Test.MyClassPrx p)
            {
                Ice.Communicator communicator = helper.communicator();

                {
                    await p.ice_pingAsync();
                }

                {
                    test((await p.ice_idsAsync()).Length == 3);
                }

                {
                    test(await p.ice_idAsync() == Test.MyDerivedClassDisp_.ice_staticId());
                }

                {
                    var ret = await p.opByteAsync(0xff, 0x0f);
                    test(ret.p3 == 0xf0);
                    test(ret.returnValue == 0xff);
                }

                {
                    var cb = new Callback();
                    var ret = await p.opBoolAsync(true, false);
                    cb.opBool(ret.returnValue, ret.p3);
                }

                {
                    var cb = new Callback();
                    var ret = await p.opShortIntLongAsync(10, 11, 12);
                    cb.opShortIntLong(ret.returnValue, ret.p4, ret.p5, ret.p6);
                }
                {
                    var cb = new Callback();
                    var ret = await p.opFloatDoubleAsync(3.14f, 1.1E10);
                    cb.opFloatDouble(ret.returnValue, ret.p3, ret.p4);
                }

                {
                    var cb = new Callback();
                    var ret = await p.opStringAsync("hello", "world");
                    cb.opString(ret.returnValue, ret.p3);
                }

                {
                    var cb = new Callback();
                    var ret = await p.opMyEnumAsync(Test.MyEnum.enum2);
                    cb.opMyEnum(ret.returnValue, ret.p2);
                }

                {
                    var cb = new Callback(communicator);
                    var ret = await p.opMyClassAsync(p);
                    cb.opMyClass(ret.returnValue, ret.p2, ret.p3);
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
                    var ret = await p.opStructAsync(si1, si2);
                    cb.opStruct(ret.returnValue, ret.p3);
                }

                {
                    byte[] bsi1 = new byte[] { 0x01, 0x11, 0x12, 0x22 };
                    byte[] bsi2 = new byte[] { 0xf1, 0xf2, 0xf3, 0xf4 };

                    var cb = new Callback();
                    var ret = await p.opByteSAsync(bsi1, bsi2);
                    cb.opByteS(ret.returnValue, ret.p3);
                }

                {
                    bool[] bsi1 = new bool[] { true, true, false };
                    bool[] bsi2 = new bool[] { false };

                    var cb = new Callback();
                    var ret = await p.opBoolSAsync(bsi1, bsi2);
                    cb.opBoolS(ret.returnValue, ret.p3);
                }

                {
                    short[] ssi = new short[] { 1, 2, 3 };
                    int[] isi = new int[] { 5, 6, 7, 8 };
                    long[] lsi = new long[] { 10, 30, 20 };

                    var cb = new Callback();
                    var ret = await p.opShortIntLongSAsync(ssi, isi, lsi);
                    cb.opShortIntLongS(ret.returnValue, ret.p4, ret.p5, ret.p6);
                }

                {
                    float[] fsi = new float[] { 3.14f, 1.11f };
                    double[] dsi = new double[] { 1.1e10, 1.2e10, 1.3e10 };

                    var cb = new Callback();
                    var ret = await p.opFloatDoubleSAsync(fsi, dsi);
                    cb.opFloatDoubleS(ret.returnValue, ret.p3, ret.p4);
                }

                {
                    string[] ssi1 = new string[] { "abc", "de", "fghi" };
                    string[] ssi2 = new string[] { "xyz" };

                    var cb = new Callback();
                    var ret = await p.opStringSAsync(ssi1, ssi2);
                    cb.opStringS(ret.returnValue, ret.p3);
                }

                {
                    byte[] s11 = new byte[] { 0x01, 0x11, 0x12 };
                    byte[] s12 = new byte[] { 0xff };
                    byte[][] bsi1 = new byte[][] { s11, s12 };

                    byte[] s21 = new byte[] { 0x0e };
                    byte[] s22 = new byte[] { 0xf2, 0xf1 };
                    byte[][] bsi2 = new byte[][] { s21, s22 };

                    var cb = new Callback();
                    var ret = await p.opByteSSAsync(bsi1, bsi2);
                    cb.opByteSS(ret.returnValue, ret.p3);
                }

                {
                    bool[] s11 = new bool[] { true };
                    bool[] s12 = new bool[] { false };
                    bool[] s13 = new bool[] { true, true };
                    bool[][] bsi1 = new bool[][] { s11, s12, s13 };

                    bool[] s21 = new bool[] { false, false, true };
                    bool[][] bsi2 = new bool[][] { s21 };

                    var cb = new Callback();
                    var ret = await p.opBoolSSAsync(bsi1, bsi2);
                    cb.opBoolSS(ret.returnValue, ret.p3);
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
                    var ret = await p.opShortIntLongSSAsync(ssi, isi, lsi);
                    cb.opShortIntLongSS(ret.returnValue, ret.p4, ret.p5, ret.p6);
                }

                {
                    float[] f11 = new float[] { 3.14f };
                    float[] f12 = new float[] { 1.11f };
                    float[] f13 = new float[] { };
                    float[][] fsi = new float[][] { f11, f12, f13 };

                    double[] d11 = new double[] { 1.1e10, 1.2e10, 1.3e10 };
                    double[][] dsi = new double[][] { d11 };

                    var cb = new Callback();
                    var ret = await p.opFloatDoubleSSAsync(fsi, dsi);
                    cb.opFloatDoubleSS(ret.returnValue, ret.p3, ret.p4);
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
                    var ret = await p.opStringSSAsync(ssi1, ssi2);
                    cb.opStringSS(ret.returnValue, ret.p3);
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
                    var ret = await p.opStringSSSAsync(sssi1, sssi2);
                    cb.opStringSSS(ret.returnValue, ret.p3);
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
                    var ret = await p.opByteBoolDAsync(di1, di2);
                    cb.opByteBoolD(ret.returnValue, ret.p3);
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
                    var ret = await p.opShortIntDAsync(di1, di2);
                    cb.opShortIntD(ret.returnValue, ret.p3);
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
                    var ret = await p.opLongFloatDAsync(di1, di2);
                    cb.opLongFloatD(ret.returnValue, ret.p3);
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
                    var ret = await p.opStringStringDAsync(di1, di2);
                    cb.opStringStringD(ret.returnValue, ret.p3);
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
                    var ret = await p.opStringMyEnumDAsync(di1, di2);
                    cb.opStringMyEnumD(ret.returnValue, ret.p3);
                }

                {
                    var di1 = new Dictionary<Test.MyEnum, string>();
                    di1[Test.MyEnum.enum1] = "abc";
                    var di2 = new Dictionary<Test.MyEnum, string>();
                    di2[Test.MyEnum.enum2] = "Hello!!";
                    di2[Test.MyEnum.enum3] = "qwerty";

                    var cb = new Callback();
                    var ret = await p.opMyEnumStringDAsync(di1, di2);
                    cb.opMyEnumStringD(ret.returnValue, ret.p3);
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
                    var ret = await p.opMyStructMyEnumDAsync(di1, di2);
                    cb.opMyStructMyEnumD(ret.returnValue, ret.p3);
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
                    var ret = await p.opByteBoolDSAsync(dsi1, dsi2);
                    cb.opByteBoolDS(ret.returnValue, ret.p3);
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
                    var ret = await p.opShortIntDSAsync(dsi1, dsi2);
                    cb.opShortIntDS(ret.returnValue, ret.p3);
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
                    var ret = await p.opLongFloatDSAsync(dsi1, dsi2);
                    cb.opLongFloatDS(ret.returnValue, ret.p3);
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
                    var ret = await p.opStringStringDSAsync(dsi1, dsi2);
                    cb.opStringStringDS(ret.returnValue, ret.p3);
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
                    var ret = await p.opStringMyEnumDSAsync(dsi1, dsi2);
                    cb.opStringMyEnumDS(ret.returnValue, ret.p3);
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
                    var ret = await p.opMyEnumStringDSAsync(dsi1, dsi2);
                    cb.opMyEnumStringDS(ret.returnValue, ret.p3);
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
                    var ret = await p.opMyStructMyEnumDSAsync(dsi1, dsi2);
                    cb.opMyStructMyEnumDS(ret.returnValue, ret.p3);
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
                    var ret = await p.opByteByteSDAsync(sdi1, sdi2);
                    cb.opByteByteSD(ret.returnValue, ret.p3);
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
                    var ret = await p.opBoolBoolSDAsync(sdi1, sdi2);
                    cb.opBoolBoolSD(ret.returnValue, ret.p3);
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
                    var ret = await p.opShortShortSDAsync(sdi1, sdi2);
                    cb.opShortShortSD(ret.returnValue, ret.p3);
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
                    var ret = await p.opIntIntSDAsync(sdi1, sdi2);
                    cb.opIntIntSD(ret.returnValue, ret.p3);
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
                    var ret = await p.opLongLongSDAsync(sdi1, sdi2);
                    cb.opLongLongSD(ret.returnValue, ret.p3);
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
                    var ret = await p.opStringFloatSDAsync(sdi1, sdi2);
                    cb.opStringFloatSD(ret.returnValue, ret.p3);
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
                    var ret = await p.opStringDoubleSDAsync(sdi1, sdi2);
                    cb.opStringDoubleSD(ret.returnValue, ret.p3);
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
                    var ret = await p.opStringStringSDAsync(sdi1, sdi2);
                    cb.opStringStringSD(ret.returnValue, ret.p3);
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
                    var ret = await p.opMyEnumMyEnumSDAsync(sdi1, sdi2);
                    cb.opMyEnumMyEnumSD(ret.returnValue, ret.p3);
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
                        cb.opIntS(await p.opIntSAsync(s));
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
                        cb.opContextNotEqual(await p.opContextAsync());
                    }
                    {
                        test(p.ice_getContext().Count == 0);
                        var cb = new Callback(ctx);
                        cb.opContextEqual(await p.opContextAsync(ctx));
                    }
                    {
                        var p2 = Test.MyClassPrxHelper.checkedCast(p.ice_context(ctx));
                        test(Ice.CollectionComparer.Equals(p2.ice_getContext(), ctx));
                        var cb = new Callback(ctx);
                        cb.opContextEqual(await p2.opContextAsync());
                    }
                    {
                        var p2 = Test.MyClassPrxHelper.checkedCast(p.ice_context(ctx));
                        Callback cb = new Callback(ctx);
                        cb.opContextEqual(await p2.opContextAsync(ctx));
                    }
                }

                //
                // Test implicit context propagation with async task
                //
                if (p.ice_getConnection() != null)
                {
                    // TODO: revise this test and use await instead of Result.
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
                        test(combined["one"] == "UN");

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

                await p.opIdempotentAsync();
                await p.opNonmutatingAsync();

                {
                    var derived = Test.MyDerivedClassPrxHelper.checkedCast(p);
                    test(derived != null);
                    await derived.opDerivedAsync();
                }

                test(await p.opByte1Async(0xFF) == 0xFF);
                test(await p.opShort1Async(0x7FFF) == 0x7FFF);
                test(await p.opInt1Async(0x7FFFFFFF) == 0x7FFFFFFF);
                test(await p.opLong1Async(0x7FFFFFFFFFFFFFFF) == 0x7FFFFFFFFFFFFFFF);
                test(await p.opFloat1Async(1.0f) == 1.0f);
                test(await p.opDouble1Async(1.0d) == 1.0d);
                test(await p.opString1Async("opString1") == "opString1");
                test((await p.opStringS1Async(null)).Length == 0);
                test((await p.opByteBoolD1Async(null)).Count == 0);
                test((await p.opStringS2Async(null)).Length == 0);
                test((await p.opByteBoolD2Async(null)).Count == 0);

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
            }
        }
    }
}
