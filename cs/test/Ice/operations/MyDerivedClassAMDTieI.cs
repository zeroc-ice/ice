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

public sealed class MyDerivedClassI : Test.MyDerivedClassTie_
{
    public MyDerivedClassI() : base(new MyDerivedClassTieI())
    {
    }
}

public sealed class MyDerivedClassTieI : Test.MyDerivedClassOperations_
{
    private static void test(bool b)
    {
        if (!b)
        {
            throw new Exception();
        }
    }

    internal class Thread_opVoid
    {
        public Thread_opVoid(Test.AMD_MyClass_opVoid cb)
        {
            _cb = cb;
        }

        public void Start()
        {
            lock(this)
            {
                _thread = new Thread(new ThreadStart(Run));
                _thread.Start();
            }
        }

        public void Run()
        {
            _cb.ice_response();
        }

        public void Join()
        {
            lock(this)
            {
                _thread.Join();
            }
        }

        private Test.AMD_MyClass_opVoid _cb;
        private Thread _thread;
    }

    public void shutdown_async(Test.AMD_MyClass_shutdown cb, Ice.Current current)
    {
        while(_opVoidThread != null)
        {
            _opVoidThread.Join();
            _opVoidThread = null;
        }

        current.adapter.getCommunicator().shutdown();
        cb.ice_response();
    }

    public void delay_async(Test.AMD_MyClass_delay cb, int ms, Ice.Current current)
    {
        System.Threading.Thread.Sleep(ms);
        cb.ice_response();
    }

    public void opVoid_async(Test.AMD_MyClass_opVoid cb, Ice.Current current)
    {
        test(current.mode == Ice.OperationMode.Normal);

        while(_opVoidThread != null)
        {
            _opVoidThread.Join();
            _opVoidThread = null;
        }

        _opVoidThread = new Thread_opVoid(cb);
        _opVoidThread.Start();
    }

    public void opBool_async(Test.AMD_MyClass_opBool cb, bool p1, bool p2, Ice.Current current)
    {
        cb.ice_response(p2, p1);
    }

    public void opBoolS_async(Test.AMD_MyClass_opBoolS cb, Test.BoolS p1, Test.BoolS p2, Ice.Current current)
    {
        Test.BoolS p3 = new Test.BoolS();
        p3.AddRange(p1);
        p3.AddRange(p2);

        Test.BoolS r = new Test.BoolS();
        for(int i = 0; i < p1.Count; i++)
        {
            r.Add(p1[p1.Count - (i + 1)]);
        }
        cb.ice_response(r, p3);
    }

    public void opBoolSS_async(Test.AMD_MyClass_opBoolSS cb, Test.BoolSS p1, Test.BoolSS p2, Ice.Current current)
    {
        Test.BoolSS p3 = new Test.BoolSS();
        p3.AddRange(p1);
        p3.AddRange(p2);

        Test.BoolSS r = new Test.BoolSS();
        for(int i = 0; i < p1.Count; i++)
        {
            r.Add(p1[p1.Count - (i + 1)]);
        }
        cb.ice_response(r, p3);
    }

    public void opByte_async(Test.AMD_MyClass_opByte cb, byte p1, byte p2, Ice.Current current)
    {
        cb.ice_response(p1, (byte)(p1 ^ p2));
    }

    public void opByteBoolD_async(Test.AMD_MyClass_opByteBoolD cb, Dictionary<byte, bool> p1,
                                  Dictionary<byte, bool> p2, Ice.Current current)
    {
        Dictionary<byte, bool> p3 = p1;
        Dictionary<byte, bool> r = new Dictionary<byte, bool>();
        foreach(KeyValuePair<byte, bool> e in p1)
        {
            r[e.Key] = e.Value;
        }
        foreach(KeyValuePair<byte, bool> e in p2)
        {
            r[e.Key] = e.Value;
        }
        cb.ice_response(r, p3);
    }

    public void opByteS_async(Test.AMD_MyClass_opByteS cb, Test.ByteS p1, Test.ByteS p2, Ice.Current current)
    {
        Test.ByteS p3 = new Test.ByteS();
        for(int i = 0; i < p1.Count; i++)
        {
            p3.Add(p1[p1.Count - (i + 1)]);
        }

        Test.ByteS r = new Test.ByteS();
        r.AddRange(p1);
        r.AddRange(p2);
        cb.ice_response(r, p3);
    }

    public void opByteSS_async(Test.AMD_MyClass_opByteSS cb, Test.ByteSS p1, Test.ByteSS p2,
                               Ice.Current current)
    {
        Test.ByteSS p3 = new Test.ByteSS();
        for(int i = 0; i < p1.Count; i++)
        {
            p3.Add(p1[p1.Count - (i + 1)]);
        }

        Test.ByteSS r = new Test.ByteSS();
        r.AddRange(p1);
        r.AddRange(p2);
        cb.ice_response(r, p3);
    }

    public void opFloatDouble_async(Test.AMD_MyClass_opFloatDouble cb, float p1, double p2,
                                    Ice.Current current)
    {
        cb.ice_response(p2, p1, p2);
    }

    public void opFloatDoubleS_async(Test.AMD_MyClass_opFloatDoubleS cb, Test.FloatS p1, Test.DoubleS p2,
                                     Ice.Current current)
    {
        Test.FloatS p3 = p1;
        Test.DoubleS p4 = new Test.DoubleS();
        for(int i = 0; i < p2.Count; i++)
        {
            p4.Add(p2[p2.Count - (i + 1)]);
        }
        Test.DoubleS r = new Test.DoubleS();
        r.AddRange(p2);
        for(int i = 0; i < p1.Count; i++)
        {
            r.Add(p1[i]);
        }
        cb.ice_response(r, p3, p4);
    }

    public void opFloatDoubleSS_async(Test.AMD_MyClass_opFloatDoubleSS cb, Test.FloatSS p1, Test.DoubleSS p2,
                                      Ice.Current current)
    {
        Test.FloatSS p3 = p1;
        Test.DoubleSS p4 = new Test.DoubleSS();
        for(int i = 0; i < p2.Count; i++)
        {
            p4.Add(p2[p2.Count - (i + 1)]);
        }
        Test.DoubleSS r = new Test.DoubleSS();
        r.AddRange(p2);
        r.AddRange(p2);
        cb.ice_response(r, p3, p4);
    }

    public void opLongFloatD_async(Test.AMD_MyClass_opLongFloatD cb, Dictionary<long, float> p1,
                                   Dictionary<long, float> p2, Ice.Current current)
    {
        Dictionary<long, float> p3 = p1;
        Dictionary<long, float> r = new Dictionary<long, float>();
        foreach(KeyValuePair<long, float> e in p1)
        {
            r[e.Key] = e.Value;
        }
        foreach(KeyValuePair<long, float> e in p2)
        {
            r[e.Key] = e.Value;
        }
        cb.ice_response(r, p3);
    }

    public void opMyClass_async(Test.AMD_MyClass_opMyClass cb, Test.MyClassPrx p1, Ice.Current current)
    {
        Test.MyClassPrx p2 = p1;
        Test.MyClassPrx p3 = Test.MyClassPrxHelper.uncheckedCast(current.adapter.createProxy(
                                                current.adapter.getCommunicator().stringToIdentity("noSuchIdentity")));
        cb.ice_response(Test.MyClassPrxHelper.uncheckedCast(current.adapter.createProxy(current.id)), p2, p3);
    }

    public void opMyEnum_async(Test.AMD_MyClass_opMyEnum cb, Test.MyEnum p1, Ice.Current current)
    {
        cb.ice_response(Test.MyEnum.enum3, p1);
    }

    public void opShortIntD_async(Test.AMD_MyClass_opShortIntD cb, Dictionary<short, int> p1,
                                  Dictionary<short, int> p2, Ice.Current current)
    {
        Dictionary<short, int> p3 = p1;
        Dictionary<short, int> r = new Dictionary<short, int>();
        foreach(KeyValuePair<short, int> e in p1)
        {
            r[e.Key] = e.Value;
        }
        foreach(KeyValuePair<short, int> e in p2)
        {
            r[e.Key] = e.Value;
        }
        cb.ice_response(r, p3);
    }

    public void opShortIntLong_async(Test.AMD_MyClass_opShortIntLong cb, short p1, int p2, long p3,
                                     Ice.Current current)
    {
        cb.ice_response(p3, p1, p2, p3);
    }

    public void opShortIntLongS_async(Test.AMD_MyClass_opShortIntLongS cb, Test.ShortS p1, Test.IntS p2,
                                      Test.LongS p3, Ice.Current current)
    {
        Test.ShortS p4 = p1;
        Test.IntS p5 = new Test.IntS();
        for(int i = 0; i < p2.Count; i++)
        {
            p5.Add(p2[p2.Count - (i + 1)]);
        }
        Test.LongS p6 = new Test.LongS();
        p6.AddRange(p3);
        p6.AddRange(p3);
        cb.ice_response(p3, p4, p5, p6);
    }

    public void opShortIntLongSS_async(Test.AMD_MyClass_opShortIntLongSS cb, Test.ShortSS p1,
                                       Test.IntSS p2, Test.LongSS p3, Ice.Current current)
    {
        Test.ShortSS p4 = p1;
        Test.IntSS p5 = new Test.IntSS();
        for(int i = 0; i < p2.Count; i++)
        {
            p5.Add(p2[p2.Count - (i + 1)]);
        }
        Test.LongSS p6 = new Test.LongSS();
        p6.AddRange(p3);
        p6.AddRange(p3);
        cb.ice_response(p3, p4, p5, p6);
    }

    public void opString_async(Test.AMD_MyClass_opString cb, string p1, string p2, Ice.Current current)
    {
        cb.ice_response(p1 + " " + p2, p2 + " " + p1);
    }

    public void opStringMyEnumD_async(Test.AMD_MyClass_opStringMyEnumD cb, Dictionary<string, Test.MyEnum> p1,
                                      Dictionary<string, Test.MyEnum> p2, Ice.Current current)
    {
        Dictionary<string, Test.MyEnum> p3 = p1;
        Dictionary<string, Test.MyEnum> r = new Dictionary<string, Test.MyEnum>();
        foreach(KeyValuePair<string, Test.MyEnum> e in p1)
        {
            r[e.Key] = e.Value;
        }
        foreach(KeyValuePair<string, Test.MyEnum> e in p2)
        {
            r[e.Key] = e.Value;
        }
        cb.ice_response(r, p3);
    }

    public void opMyEnumStringD_async(Test.AMD_MyClass_opMyEnumStringD cb, Dictionary<Test.MyEnum, string> p1,
                                      Dictionary<Test.MyEnum, string> p2, Ice.Current current)
    {
        Dictionary<Test.MyEnum, string> p3 = p1;
        Dictionary<Test.MyEnum, string> r = new Dictionary<Test.MyEnum, string>();
        foreach(KeyValuePair<Test.MyEnum, string> e in p1)
        {
            r[e.Key] = e.Value;
        }
        foreach(KeyValuePair<Test.MyEnum, string> e in p2)
        {
            r[e.Key] = e.Value;
        }
        cb.ice_response(r, p3);
    }

    public void opMyStructMyEnumD_async(Test.AMD_MyClass_opMyStructMyEnumD cb,
                                        Dictionary<Test.MyStruct, Test.MyEnum> p1,
                                        Dictionary<Test.MyStruct, Test.MyEnum> p2, Ice.Current current)
    {
        Dictionary<Test.MyStruct, Test.MyEnum> p3 = p1;
        Dictionary<Test.MyStruct, Test.MyEnum> r = new Dictionary<Test.MyStruct, Test.MyEnum>();
        foreach(KeyValuePair<Test.MyStruct, Test.MyEnum> e in p1)
        {
            r[e.Key] = e.Value;
        }
        foreach(KeyValuePair<Test.MyStruct, Test.MyEnum> e in p2)
        {
            r[e.Key] = e.Value;
        }
        cb.ice_response(r, p3);
    }

    public void opIntS_async(Test.AMD_MyClass_opIntS cb, Test.IntS s, Ice.Current current)
    {
        Test.IntS r = new Test.IntS();
        for(int i = 0; i < s.Count; ++i)
        {
            r.Add(-s[i]);
        }
        cb.ice_response(r);
    }

    public void opContext_async(Test.AMD_MyClass_opContext cb, Ice.Current current)
    {
        cb.ice_response(current.ctx);
    }

    public void opByteSOneway_async(Test.AMD_MyClass_opByteSOneway cb, Test.ByteS s, Ice.Current current)
    {
        cb.ice_response();
    }

    public void opDoubleMarshaling_async(Test.AMD_MyClass_opDoubleMarshaling cb, double p1, Test.DoubleS p2,
                                         Ice.Current current)
    {
        double d = 1278312346.0 / 13.0;
        test(p1 == d);
        for(int i = 0; i < p2.Count; ++i)
        {
            test(p2[i] == d);
        }
        cb.ice_response();
    }

    public void opStringS_async(Test.AMD_MyClass_opStringS cb, Test.StringS p1, Test.StringS p2,
                                Ice.Current current)
    {
        Test.StringS p3 = new Test.StringS();
        p3.AddRange(p1);
        p3.AddRange(p2);

        Test.StringS r = new Test.StringS();
        for(int i = 0; i < p1.Count; i++)
        {
            r.Add(p1[p1.Count - (i + 1)]);
        }
        cb.ice_response(r, p3);
    }

    public void opStringSS_async(Test.AMD_MyClass_opStringSS cb, Test.StringSS p1, Test.StringSS p2,
                                 Ice.Current current)
    {
        Test.StringSS p3 = new Test.StringSS();
        p3.AddRange(p1);
        p3.AddRange(p2);

        Test.StringSS r = new Test.StringSS();
        for(int i = 0; i < p2.Count; i++)
        {
            r.Add(p2[p2.Count - (i + 1)]);
        }
        cb.ice_response(r, p3);
    }

    public void opStringSSS_async(Test.AMD_MyClass_opStringSSS cb, Test.StringSS[] p1, Test.StringSS[] p2,
                                  Ice.Current current)
    {
        Test.StringSS[] p3 = new Test.StringSS[p1.Length + p2.Length];
        Array.Copy(p1, 0, p3, 0, p1.Length);
        Array.Copy(p2, 0, p3, p1.Length, p2.Length);

        Test.StringSS[] r = new Test.StringSS[p2.Length];
        for(int i = 0; i < p2.Length; i++)
        {
            r[i] = p2[p2.Length - (i + 1)];
        }
        cb.ice_response(r, p3);
    }

    public void opStringStringD_async(Test.AMD_MyClass_opStringStringD cb, Dictionary<string, string> p1,
                                      Dictionary<string, string> p2, Ice.Current current)
    {
        Dictionary<string, string> p3 = p1;
        Dictionary<string, string> r = new Dictionary<string, string>();
        foreach(KeyValuePair<string, string> e in p1)
        {
            r[e.Key] = e.Value;
        }
        foreach(KeyValuePair<string, string> e in p2)
        {
            r[e.Key] = e.Value;
        }
        cb.ice_response(r, p3);
    }

    public void opStruct_async(Test.AMD_MyClass_opStruct cb, Test.Structure p1, Test.Structure p2, Ice.Current current)
    {
        Test.Structure p3 = p1;
        p3.s.s = "a new string";
        cb.ice_response(p2, p3);
    }

    public void opIdempotent_async(Test.AMD_MyClass_opIdempotent cb, Ice.Current current)
    {
        test(current.mode == Ice.OperationMode.Idempotent);
        cb.ice_response();
    }

    public void opNonmutating_async(Test.AMD_MyClass_opNonmutating cb, Ice.Current current)
    {
        test(current.mode == Ice.OperationMode.Nonmutating);
        cb.ice_response();
    }

    public void opDerived_async(Test.AMD_MyDerivedClass_opDerived cb, Ice.Current current)
    {
        cb.ice_response();
    }

    private Thread_opVoid _opVoidThread;
}
