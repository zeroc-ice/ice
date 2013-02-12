// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Diagnostics;
using System.Threading;
using System.Collections.Generic;

public class TwowaysAMI
{
    private static void test(bool b)
    {
        if(!b)
        {
            throw new System.SystemException();
        }
    }

    private class Callback
    {
        internal Callback()
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

    private class AMI_MyClass_opVoidI : Test.AMI_MyClass_opVoid
    {
        public override void ice_response()
        {
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual void check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opIdempotentI : Test.AMI_MyClass_opIdempotent
    {
        public override void ice_response()
        {
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual void check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opNonmutatingI : Test.AMI_MyClass_opNonmutating
    {
        public override void ice_response()
        {
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual void check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opVoidExI : Test.AMI_MyClass_opVoid
    {
        public override void ice_response()
        {
            test(false);
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(ex is Ice.NoEndpointException);
            callback.called();
        }

        public virtual void check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opByteI : Test.AMI_MyClass_opByte
    {
        public override void ice_response(byte r, byte b)
        {
            test(b == 0xf0);
            test(r == 0xff);
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual void check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opByteExI : Test.AMI_MyClass_opByte
    {
        public override void ice_response(byte r, byte b)
        {
            test(false);
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(ex is Ice.NoEndpointException);
            callback.called();
        }

        public virtual void check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opBoolI : Test.AMI_MyClass_opBool
    {
        public override void ice_response(bool r, bool b)
        {
            test(b);
            test(!r);
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual void check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opShortIntLongI : Test.AMI_MyClass_opShortIntLong
    {
        public override void ice_response(long r, short s, int i, long l)
        {
            test(s == 10);
            test(i == 11);
            test(l == 12);
            test(r == 12);
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual void check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opFloatDoubleI : Test.AMI_MyClass_opFloatDouble
    {
        public override void ice_response(double r, float f, double d)
        {
            test(f == 3.14f);
            test(d == 1.1e10);
            test(r == 1.1e10);
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual void check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opStringI : Test.AMI_MyClass_opString
    {
        public override void ice_response(string r, string s)
        {
            test(s.Equals("world hello"));
            test(r.Equals("hello world"));
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual void check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opMyEnumI : Test.AMI_MyClass_opMyEnum
    {
        public override void ice_response(Test.MyEnum r, Test.MyEnum e)
        {
            test(e == Test.MyEnum.enum2);
            test(r == Test.MyEnum.enum3);
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual void check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opMyClassI : Test.AMI_MyClass_opMyClass
    {
        public AMI_MyClass_opMyClassI(Ice.Communicator comunicator)
        {
            _communicator = comunicator;
        }

        public override void ice_response(Test.MyClassPrx r, Test.MyClassPrx c1, Test.MyClassPrx c2)
        {
            test(c1.ice_getIdentity().Equals(_communicator.stringToIdentity("test")));
            test(c2.ice_getIdentity().Equals(_communicator.stringToIdentity("noSuchIdentity")));
            test(r.ice_getIdentity().Equals(_communicator.stringToIdentity("test")));
            // We can't do the callbacks below in thread per connection mode.
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
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual void check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
        private Ice.Communicator _communicator;
    }

    private class AMI_MyClass_opStructI : Test.AMI_MyClass_opStruct
    {
        public AMI_MyClass_opStructI(Ice.Communicator comunicator)
        {
            _communicator = comunicator;
        }

        public override void ice_response(Test.Structure rso, Test.Structure so)
        {
            test(rso.p == null);
            test(rso.e == Test.MyEnum.enum2);
            test(rso.s.s.Equals("def"));
            test(so.e == Test.MyEnum.enum3);
            test(so.s.s.Equals("a new string"));
            // We can't do the callbacks below in thread per connection mode.
            if(_communicator.getProperties().getPropertyAsInt("Ice.ThreadPool.Client.Serialize") == 0)
            {
                so.p.opVoid();
            }
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual void check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
        private Ice.Communicator _communicator;
    }

    private class AMI_MyClass_opByteSI : Test.AMI_MyClass_opByteS
    {
        public override void ice_response(Test.ByteS rso, Test.ByteS bso)
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
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual void check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opBoolSI : Test.AMI_MyClass_opBoolS
    {
        public override void ice_response(Test.BoolS rso, Test.BoolS bso)
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
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual void check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opShortIntLongSI : Test.AMI_MyClass_opShortIntLongS
    {
        public override void ice_response(Test.LongS rso, Test.ShortS sso, Test.IntS iso, Test.LongS lso)
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
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual void check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opFloatDoubleSI : Test.AMI_MyClass_opFloatDoubleS
    {
        public override void ice_response(Test.DoubleS rso, Test.FloatS fso, Test.DoubleS dso)
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
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual void check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opStringSI : Test.AMI_MyClass_opStringS
    {
        public override void ice_response(Test.StringS rso, Test.StringS sso)
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
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual void check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opByteSSI : Test.AMI_MyClass_opByteSS
    {
        public override void ice_response(Test.ByteSS rso, Test.ByteSS bso)
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
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual void check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opBoolSSI : Test.AMI_MyClass_opBoolSS
    {
        public override void ice_response(Test.BoolSS rso, Test.BoolSS bso)
        {
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual void check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opShortIntLongSSI : Test.AMI_MyClass_opShortIntLongSS
    {
        public override void ice_response(Test.LongSS rso, Test.ShortSS sso, Test.IntSS iso, Test.LongSS lso)
        {
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual void check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opFloatDoubleSSI : Test.AMI_MyClass_opFloatDoubleSS
    {
        public override void ice_response(Test.DoubleSS rso, Test.FloatSS fso, Test.DoubleSS dso)
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
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual void check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opStringSSI : Test.AMI_MyClass_opStringSS
    {
        public override void ice_response(Test.StringSS rso, Test.StringSS sso)
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
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual void check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opStringSSSI : Test.AMI_MyClass_opStringSSS
    {
        public override void ice_response(Test.StringSS[] rsso, Test.StringSS[] ssso)
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
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual void check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opByteBoolDI : Test.AMI_MyClass_opByteBoolD
    {
        public override void ice_response(Dictionary<byte, bool> ro, Dictionary<byte, bool> _do)
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
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual void check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opShortIntDI : Test.AMI_MyClass_opShortIntD
    {
        public override void ice_response(Dictionary<short, int> ro, Dictionary<short, int> _do)
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
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual void check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opLongFloatDI : Test.AMI_MyClass_opLongFloatD
    {
        public override void ice_response(Dictionary<long, float> ro, Dictionary<long, float> _do)
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
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual void check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opStringStringDI : Test.AMI_MyClass_opStringStringD
    {
        public override void ice_response(Dictionary<string, string> ro, Dictionary<string, string> _do)
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
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual void check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opStringMyEnumDI : Test.AMI_MyClass_opStringMyEnumD
    {
        public override void ice_response(Dictionary<string, Test.MyEnum> ro, Dictionary<string, Test.MyEnum> _do)
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
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual void check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opMyEnumStringDI : Test.AMI_MyClass_opMyEnumStringD
    {
        public override void ice_response(Dictionary<Test.MyEnum, string> ro, Dictionary<Test.MyEnum, string> _do)
        {
            Dictionary<Test.MyEnum, string> di1 = new Dictionary<Test.MyEnum, string>();
            di1[Test.MyEnum.enum1] = "abc";
            test(Ice.CollectionComparer.Equals(_do, di1));
            test(ro.Count == 3);
            test(ro[Test.MyEnum.enum1].Equals("abc"));
            test(ro[Test.MyEnum.enum2].Equals("Hello!!"));
            test(ro[Test.MyEnum.enum3].Equals("qwerty"));
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual void check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opMyStructMyEnumDI : Test.AMI_MyClass_opMyStructMyEnumD
    {
        public override void ice_response(Dictionary<Test.MyStruct, Test.MyEnum> ro,
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
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual void check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opIntSI : Test.AMI_MyClass_opIntS
    {
        internal AMI_MyClass_opIntSI(int l)
        {
            _l = l;
            callback = new Callback();
        }

        public override void ice_response(Test.IntS r)
        {
            test(r.Count == _l);
            for(int j = 0; j < r.Count; ++j)
            {
                test(r[j] == -j);
            }
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual void check()
        {
            callback.check();
        }

        private int _l;
        private Callback callback;
    }

    private class AMI_MyClass_opContextEqualI : Test.AMI_MyClass_opContext
    {
        internal AMI_MyClass_opContextEqualI(Dictionary<string, string> d)
        {
            _d = d;
            callback = new Callback();
        }

        public override void ice_response(Dictionary<string, string> r)
        {
            test(Ice.CollectionComparer.Equals(r, _d));
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual void check()
        {
            callback.check();
        }

        private Dictionary<string, string> _d;
        private Callback callback;
    }

    private class AMI_MyClass_opContextNotEqualI : Test.AMI_MyClass_opContext
    {
        internal AMI_MyClass_opContextNotEqualI(Dictionary<string, string> d)
        {
            _d = d;
            callback = new Callback();
        }

        public override void ice_response(Dictionary<string, string> r)
        {
            test(!Ice.CollectionComparer.Equals(r, _d));
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual void check()
        {
            callback.check();
        }

        private Dictionary<string, string> _d;
        private Callback callback;
    }

    private class AMI_MyDerivedClass_opDerivedI : Test.AMI_MyDerivedClass_opDerived
    {
        public override void ice_response()
        {
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual void check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    internal static void twowaysAMI(Ice.Communicator communicator, Test.MyClassPrx p)
    {
        {
            // Check that a call to a void operation raises NoEndpointException
            // in the ice_exception() callback instead of at the point of call.
            Test.MyClassPrx indirect = Test.MyClassPrxHelper.uncheckedCast(p.ice_adapterId("dummy"));
            AMI_MyClass_opVoidExI cb = new AMI_MyClass_opVoidExI();
            try
            {
                test(!indirect.opVoid_async(cb));
            }
            catch(Ice.Exception)
            {
                test(false);
            }
            cb.check();
        }

        {
            // Check that a call to a twoway operation raises NoEndpointException
            // in the ice_exception() callback instead of at the point of call.
            Test.MyClassPrx indirect = Test.MyClassPrxHelper.uncheckedCast(p.ice_adapterId("dummy"));
            AMI_MyClass_opByteExI cb = new AMI_MyClass_opByteExI();
            try
            {
                test(!indirect.opByte_async(cb, (byte)0, (byte)0));
            }
            catch(Ice.Exception)
            {
                test(false);
            }
            cb.check();
        }

        {
            AMI_MyClass_opVoidI cb = new AMI_MyClass_opVoidI();
            p.opVoid_async(cb);
            cb.check();
            // Let's check if we can reuse the callback object for another call.
            p.opVoid_async(cb);
            cb.check();
        }

        {
            Ice.InitializationData initData = new Ice.InitializationData();
            initData.properties = communicator.getProperties().ice_clone_();
            Ice.Communicator ic = Ice.Util.initialize(initData);

            Ice.ObjectPrx obj = ic.stringToProxy(p.ToString());
            Test.MyClassPrx p2 = Test.MyClassPrxHelper.checkedCast(obj);

            ic.destroy();

            AMI_MyClass_opVoidI cb = new AMI_MyClass_opVoidI();
            try
            {
                test(!p2.opVoid_async(cb));
                test(false);
            }
            catch(Ice.CommunicatorDestroyedException)
            {
                // Expected.
            }
        }

        {
            AMI_MyClass_opByteI cb = new AMI_MyClass_opByteI();
            p.opByte_async(cb, 0xff, 0x0f);
            cb.check();
        }

        {
            AMI_MyClass_opBoolI cb = new AMI_MyClass_opBoolI();
            p.opBool_async(cb, true, false);
            cb.check();
        }

        {
            AMI_MyClass_opShortIntLongI cb = new AMI_MyClass_opShortIntLongI();
            p.opShortIntLong_async(cb, (short) 10, 11, 12L);
            cb.check();
        }

        {
            AMI_MyClass_opFloatDoubleI cb = new AMI_MyClass_opFloatDoubleI();
            p.opFloatDouble_async(cb, 3.14f, 1.1e10);
            cb.check();
        }

        {
            AMI_MyClass_opStringI cb = new AMI_MyClass_opStringI();
            p.opString_async(cb, "hello", "world");
            cb.check();
        }

        {
            AMI_MyClass_opMyEnumI cb = new AMI_MyClass_opMyEnumI();
            p.opMyEnum_async(cb, Test.MyEnum.enum2);
            cb.check();
        }

        {
            AMI_MyClass_opMyClassI cb = new AMI_MyClass_opMyClassI(communicator);
            p.opMyClass_async(cb, p);
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

            AMI_MyClass_opStructI cb = new AMI_MyClass_opStructI(communicator);
            p.opStruct_async(cb, si1, si2);
            cb.check();
        }

        {
            Test.ByteS bsi1 = new Test.ByteS(new byte[] { 0x01, 0x11, 0x12, 0x22 });
            Test.ByteS bsi2 = new Test.ByteS(new byte[] { 0xf1, 0xf2, 0xf3, 0xf4 });

            AMI_MyClass_opByteSI cb = new AMI_MyClass_opByteSI();
            p.opByteS_async(cb, bsi1, bsi2);
            cb.check();
        }

        {
            Test.BoolS bsi1 = new Test.BoolS(new bool[] { true, true, false });
            Test.BoolS bsi2 = new Test.BoolS(new bool[] { false });

            AMI_MyClass_opBoolSI cb = new AMI_MyClass_opBoolSI();
            p.opBoolS_async(cb, bsi1, bsi2);
            cb.check();
        }

        {
            Test.ShortS ssi = new Test.ShortS(new short[] { 1, 2, 3 });
            Test.IntS isi = new Test.IntS(new int[] { 5, 6, 7, 8 });
            Test.LongS lsi = new Test.LongS(new long[] { 10, 30, 20 });

            AMI_MyClass_opShortIntLongSI cb = new AMI_MyClass_opShortIntLongSI();
            p.opShortIntLongS_async(cb, ssi, isi, lsi);
            cb.check();
        }

        {
            Test.FloatS fsi = new Test.FloatS(new float[] { 3.14f, 1.11f });
            Test.DoubleS dsi = new Test.DoubleS(new double[] { 1.1e10, 1.2e10, 1.3e10 });

            AMI_MyClass_opFloatDoubleSI cb = new AMI_MyClass_opFloatDoubleSI();
            p.opFloatDoubleS_async(cb, fsi, dsi);
            cb.check();
        }

        {
            Test.StringS ssi1 = new Test.StringS(new string[] { "abc", "de", "fghi" });
            Test.StringS ssi2 = new Test.StringS(new string[] { "xyz" });

            AMI_MyClass_opStringSI cb = new AMI_MyClass_opStringSI();
            p.opStringS_async(cb, ssi1, ssi2);
            cb.check();
        }

        {
            Test.ByteSS bsi1 = new Test.ByteSS();
            bsi1.Add(new Test.ByteS());
            bsi1.Add(new Test.ByteS());
            bsi1[0].Add(0x01);
            bsi1[0].Add(0x11);
            bsi1[0].Add(0x12);
            bsi1[1].Add(0xff);
            Test.ByteSS bsi2 = new Test.ByteSS();
            bsi2.Add(new Test.ByteS());
            bsi2.Add(new Test.ByteS());
            bsi2[0].Add(0x0e);
            bsi2[1].Add(0xf2);
            bsi2[1].Add(0xf1);

            AMI_MyClass_opByteSSI cb = new AMI_MyClass_opByteSSI();
            p.opByteSS_async(cb, bsi1, bsi2);
            cb.check();
        }

        {
            Test.FloatSS fsi = new Test.FloatSS();
            fsi.Add(new Test.FloatS(new float[] { 3.14f }));
            fsi.Add(new Test.FloatS(new float[] { 1.11f }));
            fsi.Add(new Test.FloatS(new float[] { }));

            Test.DoubleSS dsi = new Test.DoubleSS();
            dsi.Add(new Test.DoubleS(new double[] { 1.1E10, 1.2E10, 1.3E10 }));

            AMI_MyClass_opFloatDoubleSSI cb = new AMI_MyClass_opFloatDoubleSSI();
            p.opFloatDoubleSS_async(cb, fsi, dsi);
            cb.check();
        }

        {
            Test.StringSS ssi1 = new Test.StringSS();
            ssi1.Add(new Test.StringS());
            ssi1.Add(new Test.StringS());
            ssi1[0].Add("abc");
            ssi1[1].Add("de");
            ssi1[1].Add("fghi");
            Test.StringSS ssi2 = new Test.StringSS();
            ssi2.Add(new Test.StringS());
            ssi2.Add(new Test.StringS());
            ssi2.Add(new Test.StringS());
            ssi2[2].Add("xyz");

            AMI_MyClass_opStringSSI cb = new AMI_MyClass_opStringSSI();
            p.opStringSS_async(cb, ssi1, ssi2);
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

            AMI_MyClass_opStringSSSI cb = new AMI_MyClass_opStringSSSI();
            p.opStringSSS_async(cb, sssi1, sssi2);
            cb.check();
        }

        {
            Dictionary<byte, bool> di1 = new Dictionary<byte, bool>();
            di1[10] = true;
            di1[100] = false;
            Dictionary<byte, bool> di2 = new Dictionary<byte, bool>();
            // di2[10] = true; // Disabled since new dictionary mapping.
            di2[11] = false;
            di2[101] = true;

            AMI_MyClass_opByteBoolDI cb = new AMI_MyClass_opByteBoolDI();
            p.opByteBoolD_async(cb, di1, di2);
            cb.check();
        }

        {
            Dictionary<short, int> di1 = new Dictionary<short, int>();
            di1[110] = -1;
            di1[1100] = 123123;
            Dictionary<short, int> di2 = new Dictionary<short, int>();
            // di2[110] = -1; // Disabled since new dictionary mapping.
            di2[111] = -100;
            di2[1101] = 0;

            AMI_MyClass_opShortIntDI cb = new AMI_MyClass_opShortIntDI();
            p.opShortIntD_async(cb, di1, di2);
            cb.check();
        }

        {
            Dictionary<long, float> di1 = new Dictionary<long, float>();
            di1[999999110L] = -1.1f;
            di1[999999111L] = 123123.2f;
            Dictionary<long, float> di2 = new Dictionary<long, float>();
            // di2[999999110L] = -1.1f; // Disabled since new dictionary mapping.
            di2[999999120L] = -100.4f;
            di2[999999130L] = 0.5f;

            AMI_MyClass_opLongFloatDI cb = new AMI_MyClass_opLongFloatDI();
            p.opLongFloatD_async(cb, di1, di2);
            cb.check();
        }

        {
            Dictionary<string, string> di1 = new Dictionary<string, string>();
            di1["foo"] = "abc -1.1";
            di1["bar"] = "abc 123123.2";
            Dictionary<string, string> di2 = new Dictionary<string, string>();
            // di2["foo"] = "abc -1.1"; // Disabled since new dictionary mapping
            di2["FOO"] = "abc -100.4";
            di2["BAR"] = "abc 0.5";

            AMI_MyClass_opStringStringDI cb = new AMI_MyClass_opStringStringDI();
            p.opStringStringD_async(cb, di1, di2);
            cb.check();
        }

        {
            Dictionary<string, Test.MyEnum> di1 = new Dictionary<string, Test.MyEnum>();
            di1["abc"] = Test.MyEnum.enum1;
            di1[""] = Test.MyEnum.enum2;
            Dictionary<string, Test.MyEnum> di2 = new Dictionary<string, Test.MyEnum>();
            // di2["abc"] = Test.MyEnum.enum1; // Disabled since new dictionary mapping
            di2["qwerty"] = Test.MyEnum.enum3;
            di2["Hello!!"] = Test.MyEnum.enum2;

            AMI_MyClass_opStringMyEnumDI cb = new AMI_MyClass_opStringMyEnumDI();
            p.opStringMyEnumD_async(cb, di1, di2);
            cb.check();
        }

        {
            Dictionary<Test.MyEnum, string> di1 = new Dictionary<Test.MyEnum, string>();
            di1[Test.MyEnum.enum1] = "abc";
            Dictionary<Test.MyEnum, string> di2 = new Dictionary<Test.MyEnum, string>();
            di2[Test.MyEnum.enum2] = "Hello!!";
            di2[Test.MyEnum.enum3] = "qwerty";

            AMI_MyClass_opMyEnumStringDI cb = new AMI_MyClass_opMyEnumStringDI();
            p.opMyEnumStringD_async(cb, di1, di2);
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
            di2[s22] = Test.MyEnum.enum3;
            di2[s23] = Test.MyEnum.enum2;

            AMI_MyClass_opMyStructMyEnumDI cb = new AMI_MyClass_opMyStructMyEnumDI();
            p.opMyStructMyEnumD_async(cb, di1, di2);
            cb.check();
        }

        {
            int[] lengths = new int[] {0, 1, 2, 126, 127, 128, 129, 253, 254, 255, 256, 257, 1000};

            for(int l = 0; l < lengths.Length; ++l)
            {
                Test.IntS s = new Test.IntS();
                for(int i = 0; i < lengths[l]; ++i)
                {
                    s.Add(i);
                }
                AMI_MyClass_opIntSI cb = new AMI_MyClass_opIntSI(lengths[l]);
                p.opIntS_async(cb, s);
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
                AMI_MyClass_opContextNotEqualI cb = new AMI_MyClass_opContextNotEqualI(ctx);
                p.opContext_async(cb);
                cb.check();
            }
            {
                test(p.ice_getContext().Count == 0);
                AMI_MyClass_opContextEqualI cb = new AMI_MyClass_opContextEqualI(ctx);
                p.opContext_async(cb, ctx);
                cb.check();
            }
            Test.MyClassPrx p2 = Test.MyClassPrxHelper.checkedCast(p.ice_context(ctx));
            test(Ice.CollectionComparer.Equals(p2.ice_getContext(), ctx));
            {
                AMI_MyClass_opContextEqualI cb = new AMI_MyClass_opContextEqualI(ctx);
                p2.opContext_async(cb);
                cb.check();
            }
            {
                AMI_MyClass_opContextEqualI cb = new AMI_MyClass_opContextEqualI(ctx);
                p2.opContext_async(cb, ctx);
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
                    AMI_MyClass_opContextEqualI cb = new AMI_MyClass_opContextEqualI(ctx);
                    p3.opContext_async(cb);
                    cb.check();
                }

                ic.getImplicitContext().put("zero", "ZERO");

                ctx = ic.getImplicitContext().getContext();
                {
                    AMI_MyClass_opContextEqualI cb = new AMI_MyClass_opContextEqualI(ctx);
                    p3.opContext_async(cb);
                    cb.check();
                }

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
                {
                    AMI_MyClass_opContextEqualI cb = new AMI_MyClass_opContextEqualI(prxContext);
                    p3.opContext_async(cb);
                    cb.check();
                }

                ic.getImplicitContext().setContext(ctx);
                {
                    AMI_MyClass_opContextEqualI cb = new AMI_MyClass_opContextEqualI(combined);
                    p3.opContext_async(cb);
                    cb.check();
                }

                ic.getImplicitContext().setContext(null);
                ic.destroy();
            }
        }

        {
            AMI_MyClass_opIdempotentI cb = new AMI_MyClass_opIdempotentI();
            p.opIdempotent_async(cb);
            cb.check();
        }

        {
            AMI_MyClass_opNonmutatingI cb = new AMI_MyClass_opNonmutatingI();
            p.opNonmutating_async(cb);
            cb.check();
        }

        {
            Test.MyDerivedClassPrx derived = Test.MyDerivedClassPrxHelper.checkedCast(p);
            test(derived != null);
            AMI_MyDerivedClass_opDerivedI cb = new AMI_MyDerivedClass_opDerivedI();
            derived.opDerived_async(cb);
            cb.check();
        }
    }
}
