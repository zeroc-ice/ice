// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Threading;

public sealed class MyDerivedClassI : Test.MyDerivedClass
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
    
    public override void shutdown_async(Test.AMD_MyClass_shutdown cb, Ice.Current current)
    {
        while(_opVoidThread != null)
        {
            _opVoidThread.Join();
            _opVoidThread = null;
        }
        
        current.adapter.getCommunicator().shutdown();
        cb.ice_response();
    }
    
    public override void opVoid_async(Test.AMD_MyClass_opVoid cb, Ice.Current current)
    {
        while(_opVoidThread != null)
        {
            _opVoidThread.Join();
            _opVoidThread = null;
        }
        
        _opVoidThread = new Thread_opVoid(cb);
        _opVoidThread.Start();
    }
    
    public override void opSleep_async(Test.AMD_MyClass_opSleep cb, int duration, Ice.Current current)
    {
        System.Threading.Thread.Sleep(duration);
        cb.ice_response();
    }

    public override void opBool_async(Test.AMD_MyClass_opBool cb, bool p1, bool p2, Ice.Current current)
    {
        cb.ice_response(p2, p1);
    }
    
    public override void opBoolS_async(Test.AMD_MyClass_opBoolS cb, Test.BoolS p1, Test.BoolS p2, Ice.Current current)
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
    
    public override void opBoolSS_async(Test.AMD_MyClass_opBoolSS cb, Test.BoolSS p1, Test.BoolSS p2, Ice.Current current)
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
    
    public override void opByte_async(Test.AMD_MyClass_opByte cb, byte p1, byte p2, Ice.Current current)
    {
        cb.ice_response(p1, (byte)(p1 ^ p2));
    }
    
    public override void opByteBoolD_async(Test.AMD_MyClass_opByteBoolD cb, Test.ByteBoolD p1, Test.ByteBoolD p2,
                                  Ice.Current current)
    {
        Test.ByteBoolD p3 = p1;
        Test.ByteBoolD r = new Test.ByteBoolD();
        r.AddRange(p1);
        r.AddRange(p2);
        cb.ice_response(r, p3);
    }
    
    public override void opByteS_async(Test.AMD_MyClass_opByteS cb, Test.ByteS p1, Test.ByteS p2, Ice.Current current)
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
    
    public override void opByteSS_async(Test.AMD_MyClass_opByteSS cb, Test.ByteSS p1, Test.ByteSS p2, Ice.Current current)
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
    
    public override void opFloatDouble_async(Test.AMD_MyClass_opFloatDouble cb, float p1, double p2, Ice.Current current)
    {
        cb.ice_response(p2, p1, p2);
    }
    
    public override void opFloatDoubleS_async(Test.AMD_MyClass_opFloatDoubleS cb, Test.FloatS p1, Test.DoubleS p2, Ice.Current current)
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
    
    public override void opFloatDoubleSS_async(Test.AMD_MyClass_opFloatDoubleSS cb, Test.FloatSS p1, Test.DoubleSS p2, Ice.Current current)
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
    
    public override void opLongFloatD_async(Test.AMD_MyClass_opLongFloatD cb, Test.LongFloatD p1, Test.LongFloatD p2,
                                   Ice.Current current)
    {
        Test.LongFloatD p3 = p1;
        Test.LongFloatD r = new Test.LongFloatD();
        r.AddRange(p1);
        r.AddRange(p2);
        cb.ice_response(r, p3);
    }
    
    public override void opMyClass_async(Test.AMD_MyClass_opMyClass cb, Test.MyClassPrx p1, Ice.Current current)
    {
        Test.MyClassPrx p2 = p1;
        Test.MyClassPrx p3 = Test.MyClassPrxHelper.uncheckedCast(current.adapter.createProxy(
                                                current.adapter.getCommunicator().stringToIdentity("noSuchIdentity")));
        cb.ice_response(Test.MyClassPrxHelper.uncheckedCast(current.adapter.createProxy(current.id)), p2, p3);
    }
    
    public override void opMyEnum_async(Test.AMD_MyClass_opMyEnum cb, Test.MyEnum p1, Ice.Current current)
    {
        cb.ice_response(Test.MyEnum.enum3, p1);
    }
    
    public override void opShortIntD_async(Test.AMD_MyClass_opShortIntD cb, Test.ShortIntD p1, Test.ShortIntD p2,
                                  Ice.Current current)
    {
        Test.ShortIntD p3 = p1;
        Test.ShortIntD r = new Test.ShortIntD();
        r.AddRange(p1);
        r.AddRange(p2);
        cb.ice_response(r, p3);
    }
    
    public override void opShortIntLong_async(Test.AMD_MyClass_opShortIntLong cb, short p1, int p2, long p3, Ice.Current current)
    {
        cb.ice_response(p3, p1, p2, p3);
    }
    
    public override void opShortIntLongS_async(Test.AMD_MyClass_opShortIntLongS cb, Test.ShortS p1, Test.IntS p2, Test.LongS p3, Ice.Current current)
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
    
    public override void opShortIntLongSS_async(Test.AMD_MyClass_opShortIntLongSS cb, Test.ShortSS p1, Test.IntSS p2, Test.LongSS p3, Ice.Current current)
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
    
    public override void opString_async(Test.AMD_MyClass_opString cb, string p1, string p2, Ice.Current current)
    {
        cb.ice_response(p1 + " " + p2, p2 + " " + p1);
    }
    
    public override void opStringMyEnumD_async(Test.AMD_MyClass_opStringMyEnumD cb, Test.StringMyEnumD p1, Test.StringMyEnumD p2,
                                      Ice.Current current)
    {
        Test.StringMyEnumD p3 = p1;
        Test.StringMyEnumD r = new Test.StringMyEnumD();
        r.AddRange(p1);
        r.AddRange(p2);
        cb.ice_response(r, p3);
    }
    
    public override void opIntS_async(Test.AMD_MyClass_opIntS cb, Test.IntS s, Ice.Current current)
    {
        Test.IntS r = new Test.IntS();
        for(int i = 0; i < s.Count; ++i)
        {
            r.Add(-s[i]);
        }
        cb.ice_response(r);
    }
    
    public override void opContext_async(Test.AMD_MyClass_opContext cb, Ice.Current current)
    {
        cb.ice_response(current.ctx);
    }
    
    public override void opByteSOneway_async(Test.AMD_MyClass_opByteSOneway cb, Test.ByteS s, Ice.Current current)
    {
        cb.ice_response();
    }

    public override void opDoubleMarshaling_async(Test.AMD_MyClass_opDoubleMarshaling cb, double p1, Test.DoubleS p2, 
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

    public override void opStringS_async(Test.AMD_MyClass_opStringS cb, Test.StringS p1, Test.StringS p2, Ice.Current current)
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
    
    public override void opStringSS_async(Test.AMD_MyClass_opStringSS cb, Test.StringSS p1, Test.StringSS p2, Ice.Current current)
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
    
    public override void opStringSSS_async(Test.AMD_MyClass_opStringSSS cb, Test.StringSS[] p1, Test.StringSS[] p2, Ice.Current current)
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
    
    public override void opStringStringD_async(Test.AMD_MyClass_opStringStringD cb, Test.StringStringD p1, Test.StringStringD p2, Ice.Current current)
    {
        Test.StringStringD p3 = p1;
        Test.StringStringD r = new Test.StringStringD();
        r.AddRange(p1);
        r.AddRange(p2);
        cb.ice_response(r, p3);
    }
    
    public override void opStruct_async(Test.AMD_MyClass_opStruct cb, Test.Structure p1, Test.Structure p2, Ice.Current current)
    {
        Test.Structure p3 = p1;
        p3.s.s = "a new string";
        cb.ice_response(p2, p3);
    }
    
    public override void opDerived_async(Test.AMD_MyDerivedClass_opDerived cb, Ice.Current current)
    {
        cb.ice_response();
    }
    
    private Thread_opVoid _opVoidThread;
}
