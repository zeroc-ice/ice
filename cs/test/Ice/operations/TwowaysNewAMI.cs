// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Diagnostics;
using System.Threading;
using System.Collections.Generic;

public class TwowaysNewAMI
{
    private static void test(bool b)
    {
        if(!b)
        {
            throw new System.SystemException();
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
            _m.Lock();
            try
            {
                while(!_called)
                {
                    _m.Wait();
                }
                _called = false;
            }
            finally
            {
                _m.Unlock();
            }
        }

        public virtual void called()
        {
            _m.Lock();
            try
            {
                Debug.Assert(!_called);
                _called = true;
                _m.Notify();
            }
            finally
            {
                _m.Unlock();
            }
        }

        private bool _called;
        private readonly IceUtilInternal.Monitor _m = new IceUtilInternal.Monitor();
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
            test(id.Equals(Test.MyDerivedClass.ice_staticId()));
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
            test(c1.ice_getIdentity().Equals(_communicator.stringToIdentity("test")));
            test(c2.ice_getIdentity().Equals(_communicator.stringToIdentity("noSuchIdentity")));
            test(r.ice_getIdentity().Equals(_communicator.stringToIdentity("test")));

            //
            // We can't do the callbacks below in connection serialization mode.
            //
            if(_communicator.getProperties().getPropertyAsInt("Ice.ThreadPool.Client.Serialize") == 0)
            {
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
            if(_communicator.getProperties().getPropertyAsInt("Ice.ThreadPool.Client.Serialize") == 0)
            {
                so.p.opVoid();
            }
            called();
        }

        public void opByteS(Test.ByteS rso, Test.ByteS bso)
        {
            test(bso.Count == 4);
            test(bso[0] == 0x22);
            test(bso[1] == 0x12);
            test(bso[2] == 0x11);
            test(bso[3] == 0x01);
            test(rso.Count == 8);
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

        public void opBoolS(Test.BoolS rso, Test.BoolS bso)
        {
            test(bso.Count == 4);
            test(bso[0]);
            test(bso[1]);
            test(!bso[2]);
            test(!bso[3]);
            test(rso.Count == 3);
            test(!rso[0]);
            test(rso[1]);
            test(rso[2]);
            called();
        }

        public void opShortIntLongS(Test.LongS rso, Test.ShortS sso, Test.IntS iso, Test.LongS lso)
        {
            test(sso.Count == 3);
            test(sso[0] == 1);
            test(sso[1] == 2);
            test(sso[2] == 3);
            test(iso.Count == 4);
            test(iso[0] == 8);
            test(iso[1] == 7);
            test(iso[2] == 6);
            test(iso[3] == 5);
            test(lso.Count == 6);
            test(lso[0] == 10);
            test(lso[1] == 30);
            test(lso[2] == 20);
            test(lso[3] == 10);
            test(lso[4] == 30);
            test(lso[5] == 20);
            test(rso.Count == 3);
            test(rso[0] == 10);
            test(rso[1] == 30);
            test(rso[2] == 20);
            called();
        }

        public void opFloatDoubleS(Test.DoubleS rso, Test.FloatS fso, Test.DoubleS dso)
        {
            test(fso.Count == 2);
            test(fso[0] == 3.14f);
            test(fso[1] == 1.11f);
            test(dso.Count == 3);
            test(dso[0] == 1.3e10);
            test(dso[1] == 1.2e10);
            test(dso[2] == 1.1e10);
            test(rso.Count == 5);
            test(rso[0] == 1.1e10);
            test(rso[1] == 1.2e10);
            test(rso[2] == 1.3e10);
            test((float) rso[3] == 3.14f);
            test((float) rso[4] == 1.11f);
            called();
        }

        public void opStringS(Test.StringS rso, Test.StringS sso)
        {
            test(sso.Count == 4);
            test(sso[0].Equals("abc"));
            test(sso[1].Equals("de"));
            test(sso[2].Equals("fghi"));
            test(sso[3].Equals("xyz"));
            test(rso.Count == 3);
            test(rso[0].Equals("fghi"));
            test(rso[1].Equals("de"));
            test(rso[2].Equals("abc"));
            called();
        }

        public void opByteSS(Test.ByteSS rso, Test.ByteSS bso)
        {
            test(bso.Count == 2);
            test(bso[0].Count == 1);
            test(bso[0][0] == 0xff);
            test(bso[1].Count == 3);
            test(bso[1][0] == 0x01);
            test(bso[1][1] == 0x11);
            test(bso[1][2] == 0x12);
            test(rso.Count == 4);
            test(rso[0].Count == 3);
            test(rso[0][0] == 0x01);
            test(rso[0][1] == 0x11);
            test(rso[0][2] == 0x12);
            test(rso[1].Count == 1);
            test(rso[1][0] == 0xff);
            test(rso[2].Count == 1);
            test(rso[2][0] == 0x0e);
            test(rso[3].Count == 2);
            test(rso[3][0] == 0xf2);
            test(rso[3][1] == 0xf1);
            called();
        }

        public void opFloatDoubleSS(Test.DoubleSS rso, Test.FloatSS fso, Test.DoubleSS dso)
        {
            test(fso.Count == 3);
            test(fso[0].Count == 1);
            test(fso[0][0] == 3.14f);
            test(fso[1].Count == 1);
            test(fso[1][0] == 1.11f);
            test(fso[2].Count == 0);
            test(dso.Count == 1);
            test(dso[0].Count == 3);
            test(dso[0][0] == 1.1e10);
            test(dso[0][1] == 1.2e10);
            test(dso[0][2] == 1.3e10);
            test(rso.Count == 2);
            test(rso[0].Count == 3);
            test(rso[0][0] == 1.1e10);
            test(rso[0][1] == 1.2e10);
            test(rso[0][2] == 1.3e10);
            test(rso[1].Count == 3);
            test(rso[1][0] == 1.1e10);
            test(rso[1][1] == 1.2e10);
            test(rso[1][2] == 1.3e10);
            called();
        }

        public void opStringSS(Test.StringSS rso, Test.StringSS sso)
        {
            test(sso.Count == 5);
            test(sso[0].Count == 1);
            test(sso[0][0].Equals("abc"));
            test(sso[1].Count == 2);
            test(sso[1][0].Equals("de"));
            test(sso[1][1].Equals("fghi"));
            test(sso[2].Count == 0);
            test(sso[3].Count == 0);
            test(sso[4].Count == 1);
            test(sso[4][0].Equals("xyz"));
            test(rso.Count == 3);
            test(rso[0].Count == 1);
            test(rso[0][0].Equals("xyz"));
            test(rso[1].Count == 0);
            test(rso[2].Count == 0);
            called();
        }

        public void opStringSSS(Test.StringSS[] rsso, Test.StringSS[] ssso)
        {
            test(ssso.Length == 5);
            test(ssso[0].Count == 2);
            test(ssso[0][0].Count == 2);
            test(ssso[0][1].Count == 1);
            test(ssso[1].Count == 1);
            test(ssso[1][0].Count == 1);
            test(ssso[2].Count == 2);
            test(ssso[2][0].Count == 2);
            test(ssso[2][1].Count == 1);
            test(ssso[3].Count == 1);
            test(ssso[3][0].Count == 1);
            test(ssso[4].Count == 0);
            test(ssso[0][0][0].Equals("abc"));
            test(ssso[0][0][1].Equals("de"));
            test(ssso[0][1][0].Equals("xyz"));
            test(ssso[1][0][0].Equals("hello"));
            test(ssso[2][0][0].Equals(""));
            test(ssso[2][0][1].Equals(""));
            test(ssso[2][1][0].Equals("abcd"));
            test(ssso[3][0][0].Equals(""));

            test(rsso.Length == 3);
            test(rsso[0].Count == 0);
            test(rsso[1].Count == 1);
            test(rsso[1][0].Count == 1);
            test(rsso[2].Count == 2);
            test(rsso[2][0].Count == 2);
            test(rsso[2][1].Count == 1);
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
            Dictionary<string, Test.MyEnum> di1 = new Dictionary<string, Test.MyEnum>();
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
            Dictionary<Test.MyEnum, string> di1 = new Dictionary<Test.MyEnum, string>();
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
            Test.MyStruct s11 = new Test.MyStruct(1, 1);
            Test.MyStruct s12 = new Test.MyStruct(1, 2);
            Dictionary<Test.MyStruct, Test.MyEnum> di1 = new Dictionary<Test.MyStruct, Test.MyEnum>();
            di1[s11] = Test.MyEnum.enum1;
            di1[s12] = Test.MyEnum.enum2;
            test(Ice.CollectionComparer.Equals(_do, di1));
            Test.MyStruct s22 = new Test.MyStruct(2, 2);
            Test.MyStruct s23 = new Test.MyStruct(2, 3);
            test(ro.Count == 4);
            test(ro[s11] == Test.MyEnum.enum1);
            test(ro[s12] == Test.MyEnum.enum2);
            test(ro[s22] == Test.MyEnum.enum3);
            test(ro[s23] == Test.MyEnum.enum2);
            called();
        }

        public void opIntS(Test.IntS r)
        {
            test(r.Count == _l);
            for(int j = 0; j < r.Count; ++j)
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

    internal static void twowaysNewAMI(Ice.Communicator communicator, Test.MyClassPrx p)
    {
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
            Ice.AsyncResult r = p.begin_ice_isA(Test.MyClass.ice_staticId());
            test(p.end_ice_isA(r));
        }

        {
            Callback cb = new Callback();
            p.begin_ice_isA(Test.MyClass.ice_staticId()).whenCompleted(cb.ice_isA, cb.exCB);
            cb.check();
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
            Ice.AsyncResult r = p.begin_ice_id();
            test(p.end_ice_id(r).Equals(Test.MyDerivedClass.ice_staticId()));
        }

        {
            Callback cb = new Callback();
            p.begin_ice_id().whenCompleted(cb.ice_id, cb.exCB);
            cb.check();
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
            p.begin_opBool(true, false).whenCompleted(cb.opBool, cb.exCB);
            cb.check();
        }

        {
            Callback cb = new Callback();
            p.begin_opShortIntLong(10, 11, 12).whenCompleted(cb.opShortIntLong, cb.exCB);
            cb.check();
        }

        {
            Callback cb = new Callback();
            p.begin_opFloatDouble(3.14f, 1.1E10).whenCompleted(cb.opFloatDouble, cb.exCB);
            cb.check();
        }

        {
            Callback cb = new Callback();
            p.begin_opString("hello", "world").whenCompleted(cb.opString, cb.exCB);
            cb.check();
        }

        {
            Callback cb = new Callback();
            p.begin_opMyEnum(Test.MyEnum.enum2).whenCompleted(cb.opMyEnum, cb.exCB);
            cb.check();
        }

        {
            Callback cb = new Callback(communicator);
            p.begin_opMyClass(p).whenCompleted(cb.opMyClass, cb.exCB);
            cb.check();
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

            Callback cb = new Callback(communicator);
            p.begin_opStruct(si1, si2).whenCompleted(cb.opStruct, cb.exCB);
            cb.check();
        }

        {
            byte[] tbsi1 = new byte[] { 0x01, 0x11, 0x12, 0x22 };
            Test.ByteS bsi1 = new Test.ByteS(tbsi1);
            byte[] tbsi2 = new byte[] { 0xf1, 0xf2, 0xf3, 0xf4 };
            Test.ByteS bsi2 = new Test.ByteS(tbsi2);

            Callback cb = new Callback();
            p.begin_opByteS(bsi1, bsi2).whenCompleted(cb.opByteS, cb.exCB);
            cb.check();
        }

        {
            bool[] tbsi1 = new bool[] { true, true, false };
            Test.BoolS bsi1 = new Test.BoolS(tbsi1);
            bool[] tbsi2 = new bool[] { false };
            Test.BoolS bsi2 = new Test.BoolS(tbsi2);

            Callback cb = new Callback();
            p.begin_opBoolS(bsi1, bsi2).whenCompleted(cb.opBoolS, cb.exCB);
            cb.check();
        }

        {
            short[] tssi = new short[] { 1, 2, 3 };
            Test.ShortS ssi = new Test.ShortS(tssi);
            int[] tisi = new int[] { 5, 6, 7, 8 };
            Test.IntS isi = new Test.IntS(tisi);
            long[] tlsi = new long[] { 10, 30, 20 };
            Test.LongS lsi = new Test.LongS(tlsi);

            Callback cb = new Callback();
            p.begin_opShortIntLongS(ssi, isi, lsi).whenCompleted(cb.opShortIntLongS, cb.exCB);
            cb.check();
        }

        {
            float[] tfsi = new float[] { 3.14f, 1.11f };
            Test.FloatS fsi = new Test.FloatS(tfsi);
            double[] tdsi = new double[] { 1.1e10, 1.2e10, 1.3e10 };
            Test.DoubleS dsi = new Test.DoubleS(tdsi);

            Callback cb = new Callback();
            p.begin_opFloatDoubleS(fsi, dsi).whenCompleted(cb.opFloatDoubleS, cb.exCB);
            cb.check();
        }

        {
            string[] tssi1 = new string[] { "abc", "de", "fghi" };
            Test.StringS ssi1 = new Test.StringS(tssi1);
            string[] tssi2 = new string[] { "xyz" };
            Test.StringS ssi2 = new Test.StringS(tssi2);

            Callback cb = new Callback();
            p.begin_opStringS(ssi1, ssi2).whenCompleted(cb.opStringS, cb.exCB);
            cb.check();
        }

        {
            Test.ByteS s11 = new Test.ByteS(new byte[] { 0x01, 0x11, 0x12 });
            Test.ByteS s12 = new Test.ByteS(new byte[] { 0xff });
            Test.ByteSS bsi1 = new Test.ByteSS(new Test.ByteS[] { s11, s12 });

            Test.ByteS s21 = new Test.ByteS(new byte[] { 0x0e });
            Test.ByteS s22 = new Test.ByteS(new byte[] { 0xf2, 0xf1 });
            Test.ByteSS bsi2 = new Test.ByteSS(new Test.ByteS[] { s21, s22 });

            Callback cb = new Callback();
            p.begin_opByteSS(bsi1, bsi2).whenCompleted(cb.opByteSS, cb.exCB);
            cb.check();
        }

        {
            Test.FloatS f11 = new Test.FloatS(new float[] { 3.14f });
            Test.FloatS f12 = new Test.FloatS(new float[] { 1.11f });
            Test.FloatS f13 = new Test.FloatS(new float[] { });
            Test.FloatSS fsi = new Test.FloatSS(new Test.FloatS[] { f11, f12, f13 });

            Test.DoubleS d11 = new Test.DoubleS(new double[] { 1.1e10, 1.2e10, 1.3e10 });
            Test.DoubleSS dsi = new Test.DoubleSS(new Test.DoubleS[] { d11 });

            Callback cb = new Callback();
            p.begin_opFloatDoubleSS(fsi, dsi).whenCompleted(cb.opFloatDoubleSS, cb.exCB);
            cb.check();
        }

        {
            Test.StringS s11 = new Test.StringS();
            s11.Add("abc");
            Test.StringS s12 = new Test.StringS();
            s12.Add("de");
            s12.Add("fghi");
            Test.StringSS ssi1 = new Test.StringSS();
            ssi1.Add(s11);
            ssi1.Add(s12);

            Test.StringS s21 = new Test.StringS();
            Test.StringS s22 = new Test.StringS();
            Test.StringS s23 = new Test.StringS();
            s23.Add("xyz");
            Test.StringSS ssi2 = new Test.StringSS();
            ssi2.Add(s21);
            ssi2.Add(s22);
            ssi2.Add(s23);

            Callback cb = new Callback();
            p.begin_opStringSS(ssi1, ssi2).whenCompleted(cb.opStringSS, cb.exCB);
            cb.check();
        }

        {
            Test.StringSS[] sssi1 = { new Test.StringSS(), new Test.StringSS() };
            sssi1[0].Add(new Test.StringS());
            sssi1[0].Add(new Test.StringS());
            sssi1[0][0].Add("abc");
            sssi1[0][0].Add("de");
            sssi1[0][1].Add("xyz");
            sssi1[1].Add(new Test.StringS());
            sssi1[1][0].Add("hello");
            Test.StringSS[] sssi2 = { new Test.StringSS(), new Test.StringSS(), new Test.StringSS() };
            sssi2[0].Add(new Test.StringS());
            sssi2[0].Add(new Test.StringS());
            sssi2[0][0].Add("");
            sssi2[0][0].Add("");
            sssi2[0][1].Add("abcd");
            sssi2[1].Add(new Test.StringS());
            sssi2[1][0].Add("");

            Callback cb = new Callback();
            p.begin_opStringSSS(sssi1, sssi2).whenCompleted(cb.opStringSSS, cb.exCB);
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
            p.begin_opByteBoolD(di1, di2).whenCompleted(cb.opByteBoolD, cb.exCB);
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
            p.begin_opShortIntD(di1, di2).whenCompleted(cb.opShortIntD, cb.exCB);
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
            p.begin_opLongFloatD(di1, di2).whenCompleted(cb.opLongFloatD, cb.exCB);
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
            p.begin_opStringStringD(di1, di2).whenCompleted(cb.opStringStringD, cb.exCB);
            cb.check();
        }

        {
            Dictionary<string, Test.MyEnum> di1 = new Dictionary<string, Test.MyEnum>();
            di1["abc"] = Test.MyEnum.enum1;
            di1[""] = Test.MyEnum.enum2;
            Dictionary<string, Test.MyEnum> di2 = new Dictionary<string, Test.MyEnum>();
            di2["abc"] = Test.MyEnum.enum1;
            di2["qwerty"] = Test.MyEnum.enum3;
            di2["Hello!!"] = Test.MyEnum.enum2;

            Callback cb = new Callback();
            p.begin_opStringMyEnumD(di1, di2).whenCompleted(cb.opStringMyEnumD, cb.exCB);
            cb.check();
        }

        {
            Dictionary<Test.MyEnum, string> di1 = new Dictionary<Test.MyEnum, string>();
            di1[Test.MyEnum.enum1] = "abc";
            Dictionary<Test.MyEnum, string> di2 = new Dictionary<Test.MyEnum, string>();
            di2[Test.MyEnum.enum2] = "Hello!!";
            di2[Test.MyEnum.enum3] = "qwerty";

            Callback cb = new Callback();
            p.begin_opMyEnumStringD(di1, di2).whenCompleted(cb.opMyEnumStringD, cb.exCB);
            cb.check();
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

            Callback cb = new Callback();
            p.begin_opMyStructMyEnumD(di1, di2).whenCompleted(cb.opMyStructMyEnumD, cb.exCB);
            cb.check();
        }

        {
            int[] lengths = new int[] { 0, 1, 2, 126, 127, 128, 129, 253, 254, 255, 256, 257, 1000 };

            for(int l = 0; l < lengths.Length; ++l)
            {
                Test.IntS s = new Test.IntS();
                for(int i = 0; i < lengths[l]; ++i)
                {
                    s.Add(i);
                }

                Callback cb = new Callback(lengths[l]);
                p.begin_opIntS(s).whenCompleted(cb.opIntS, cb.exCB);
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
                Test.MyClassPrx p2 = Test.MyClassPrxHelper.checkedCast(p.ice_context(ctx));
                test(Ice.CollectionComparer.Equals(p2.ice_getContext(), ctx));
                Callback cb = new Callback(ctx);
                p2.begin_opContext().whenCompleted(cb.opContextEqual, cb.exCB);
                cb.check();
            }
            {
                Test.MyClassPrx p2 = Test.MyClassPrxHelper.checkedCast(p.ice_context(ctx));
                Callback cb = new Callback(ctx);
                p2.begin_opContext(ctx).whenCompleted(cb.opContextEqual, cb.exCB);
                cb.check();
            }
        }

        {
            //
            // Test implicit context propagation
            //

            string[] impls = {"Shared", "PerThread"};
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
            Ice.AsyncResult r = p.begin_opIdempotent();
            p.end_opIdempotent(r);
        }

        {
            Callback cb = new Callback();
            p.begin_opIdempotent().whenCompleted(cb.opIdempotent, cb.exCB);
            cb.check();
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
            Test.MyDerivedClassPrx derived = Test.MyDerivedClassPrxHelper.checkedCast(p);
            test(derived != null);
            Callback cb = new Callback();
            derived.begin_opDerived().whenCompleted(cb.opDerived, cb.exCB);
            cb.check();
        }
    }
}
