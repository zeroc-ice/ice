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

public sealed class MyDerivedClassI : Test.MyDerivedClass
{
    private static void test(bool b)
    {
        if (!b)
        {
            throw new Exception();
        }
    }

    //
    // Override the Object "pseudo" operations to verify the operation mode.
    //

    public override bool ice_isA(String id, Ice.Current current)
    {
        test(current.mode == Ice.OperationMode.Nonmutating);
        return base.ice_isA(id, current);
    }

    public override void ice_ping(Ice.Current current)
    {
        test(current.mode == Ice.OperationMode.Nonmutating);
        base.ice_ping(current);
    }

    public override string[] ice_ids(Ice.Current current)
    {
        test(current.mode == Ice.OperationMode.Nonmutating);
        return base.ice_ids(current);
    }

    public override string ice_id(Ice.Current current)
    {
        test(current.mode == Ice.OperationMode.Nonmutating);
        return base.ice_id(current);
    }

    public override void shutdown(Ice.Current current)
    {
        current.adapter.getCommunicator().shutdown();
    }

    public override void delay(int ms, Ice.Current current)
    {
        System.Threading.Thread.Sleep(ms);
    }

    public override void opVoid(Ice.Current current)
    {
        test(current.mode == Ice.OperationMode.Normal);
    }

    public override bool opBool(bool p1, bool p2, out bool p3, Ice.Current current)
    {
        p3 = p1;
        return p2;
    }

    public override Test.BoolS opBoolS(Test.BoolS p1, Test.BoolS p2, out Test.BoolS p3, Ice.Current current)
    {
        p3 = new Test.BoolS();
        p3.AddRange(p1);
        p3.AddRange(p2);
        Test.BoolS r = new Test.BoolS();
        for(int i = 0; i < p1.Count; i++)
        {
            r.Add(p1[p1.Count - (i + 1)]);
        }
        return r;
    }

    public override Test.BoolSS opBoolSS(Test.BoolSS p1, Test.BoolSS p2, out Test.BoolSS p3, Ice.Current current)
    {
        p3 = new Test.BoolSS();
        p3.AddRange(p1);
        p3.AddRange(p2);
        Test.BoolSS r = new Test.BoolSS();
        for(int i = 0; i < p1.Count; i++)
        {
            r.Add(p1[p1.Count - (i + 1)]);
        }
        return r;
    }

    public override byte opByte(byte p1, byte p2, out byte p3, Ice.Current current)
    {
        p3 = (byte)(p1 ^ p2);
        return p1;
    }

    public override Dictionary<byte, bool> opByteBoolD(Dictionary<byte, bool> p1, Dictionary<byte, bool> p2,
                                                       out Dictionary<byte, bool> p3,
                                                       Ice.Current current)
    {
        p3 = p1;
        Dictionary<byte, bool> r = new Dictionary<byte, bool>();
        foreach(KeyValuePair<byte, bool> e in p1)
        {
            r[e.Key] = e.Value;
        }
        foreach(KeyValuePair<byte, bool> e in p2)
        {
            r[e.Key] = e.Value;
        }
        return r;
    }

    public override Test.ByteS opByteS(Test.ByteS p1, Test.ByteS p2, out Test.ByteS p3, Ice.Current current)
    {
        p3 = new Test.ByteS();
        for(int i = 0; i < p1.Count; i++)
        {
            p3.Add(p1[p1.Count - (i + 1)]);
        }

        Test.ByteS r = new Test.ByteS(p1.ToArray());
        r.AddRange(p2);
        return r;
    }

    public override Test.ByteSS opByteSS(Test.ByteSS p1, Test.ByteSS p2, out Test.ByteSS p3, Ice.Current current)
    {
        p3 = new Test.ByteSS();
        for(int i = 0; i < p1.Count; i++)
        {
            p3.Add(p1[p1.Count - (i + 1)]);
        }

        Test.ByteSS r = new Test.ByteSS();
        r.AddRange(p1);
        r.AddRange(p2);
        return r;
    }

    public override double opFloatDouble(float p1, double p2, out float p3, out double p4, Ice.Current current)
    {
        p3 = p1;
        p4 = p2;
        return p2;
    }

    public override Test.DoubleS opFloatDoubleS(Test.FloatS p1, Test.DoubleS p2,
                                                out Test.FloatS p3, out Test.DoubleS p4,
                                                Ice.Current current)
    {
        p3 = p1;
        p4 = new Test.DoubleS();
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
        return r;
    }

    public override Test.DoubleSS opFloatDoubleSS(Test.FloatSS p1, Test.DoubleSS p2,
                                                  out Test.FloatSS p3, out Test.DoubleSS p4,
                                                  Ice.Current current)
    {
        p3 = p1;
        p4 = new Test.DoubleSS();
        for(int i = 0; i < p2.Count; i++)
        {
            p4.Add(p2[p2.Count - (i + 1)]);
        }
        Test.DoubleSS r = new Test.DoubleSS();
        r.AddRange(p2);
        r.AddRange(p2);
        return r;
    }

    public override Dictionary<long, float> opLongFloatD(Dictionary<long, float> p1, Dictionary<long, float> p2,
                                                         out Dictionary<long, float> p3,
                                                 Ice.Current current)
    {
        p3 = p1;
        Dictionary<long, float> r = new Dictionary<long, float>();
        foreach(KeyValuePair<long, float> e in p1)
        {
            r[e.Key] = e.Value;
        }
        foreach(KeyValuePair<long, float> e in p2)
        {
            r[e.Key] = e.Value;
        }
        return r;
    }

    public override Test.MyClassPrx opMyClass(Test.MyClassPrx p1, out Test.MyClassPrx p2, out Test.MyClassPrx p3,
                                              Ice.Current current)
    {
        p2 = p1;
        p3 = Test.MyClassPrxHelper.uncheckedCast(current.adapter.createProxy(
                                                current.adapter.getCommunicator().stringToIdentity("noSuchIdentity")));
        return Test.MyClassPrxHelper.uncheckedCast(current.adapter.createProxy(current.id));
    }

    public override Test.MyEnum opMyEnum(Test.MyEnum p1, out Test.MyEnum p2, Ice.Current current)
    {
        p2 = p1;
        return Test.MyEnum.enum3;
    }

    public override Dictionary<short, int> opShortIntD(Dictionary<short, int> p1, Dictionary<short, int> p2,
                                                       out Dictionary<short, int> p3, Ice.Current current)
    {
        p3 = p1;
        Dictionary<short, int> r = new Dictionary<short, int>();
        foreach(KeyValuePair<short, int> e in p1)
        {
            r[e.Key] = e.Value;
        }
        foreach(KeyValuePair<short, int> e in p2)
        {
            r[e.Key] = e.Value;
        }
        return r;
    }

    public override long opShortIntLong(short p1, int p2, long p3, out short p4, out int p5, out long p6,
                                        Ice.Current current)
    {
        p4 = p1;
        p5 = p2;
        p6 = p3;
        return p3;
    }

    public override Test.LongS opShortIntLongS(Test.ShortS p1, Test.IntS p2, Test.LongS p3,
                                               out Test.ShortS p4, out Test.IntS p5, out Test.LongS p6,
                                               Ice.Current current)
    {
        p4 = p1;
        p5 = new Test.IntS();
        for(int i = 0; i < p2.Count; i++)
        {
            p5.Add(p2[p2.Count - (i + 1)]);
        }
        p6 = new Test.LongS();
        p6.AddRange(p3);
        p6.AddRange(p3);
        return p3;
    }

    public override Test.LongSS opShortIntLongSS(Test.ShortSS p1, Test.IntSS p2, Test.LongSS p3,
                                                 out Test.ShortSS p4, out Test.IntSS p5, out Test.LongSS p6,
                                                 Ice.Current current)
    {
        p4 = p1;
        p5 = new Test.IntSS();
        for(int i = 0; i < p2.Count; i++)
        {
            p5.Add(p2[p2.Count - (i + 1)]);
        }
        p6 = new Test.LongSS();
        p6.AddRange(p3);
        p6.AddRange(p3);
        return p3;
    }

    public override string opString(string p1, string p2, out string p3, Ice.Current current)
    {
        p3 = p2 + " " + p1;
        return p1 + " " + p2;
    }

    public override Dictionary<string, Test.MyEnum> opStringMyEnumD(Dictionary<string, Test.MyEnum> p1,
                                                                    Dictionary<string, Test.MyEnum> p2,
                                                                    out Dictionary<string, Test.MyEnum> p3,
                                                                    Ice.Current current)
    {
        p3 = p1;
        Dictionary<string, Test.MyEnum> r = new Dictionary<string, Test.MyEnum>();
        foreach(KeyValuePair<string, Test.MyEnum> e in p1)
        {
            r[e.Key] = e.Value;
        }
        foreach(KeyValuePair<string, Test.MyEnum> e in p2)
        {
            r[e.Key] = e.Value;
        }
        return r;
    }

    public override Dictionary<Test.MyEnum, string> opMyEnumStringD(Dictionary<Test.MyEnum, string> p1,
                                                                    Dictionary<Test.MyEnum, string> p2,
                                                                    out Dictionary<Test.MyEnum, string> p3,
                                                                    Ice.Current current)
    {
        p3 = p1;
        Dictionary<Test.MyEnum, string> r = new Dictionary<Test.MyEnum, string>();
        foreach(KeyValuePair<Test.MyEnum, string> e in p1)
        {
            r[e.Key] = e.Value;
        }
        foreach(KeyValuePair<Test.MyEnum, string> e in p2)
        {
            r[e.Key] = e.Value;
        }
        return r;
    }

    public override Dictionary<Test.MyStruct, Test.MyEnum> opMyStructMyEnumD(
                                                Dictionary<Test.MyStruct, Test.MyEnum> p1,
                                                Dictionary<Test.MyStruct, Test.MyEnum> p2,
                                                out Dictionary<Test.MyStruct, Test.MyEnum> p3,
                                                Ice.Current current)
    {
        p3 = p1;
        Dictionary<Test.MyStruct, Test.MyEnum> r = new Dictionary<Test.MyStruct, Test.MyEnum>();
        foreach(KeyValuePair<Test.MyStruct, Test.MyEnum> e in p1)
        {
            r[e.Key] = e.Value;
        }
        foreach(KeyValuePair<Test.MyStruct, Test.MyEnum> e in p2)
        {
            r[e.Key] = e.Value;
        }
        return r;
    }

    public override Test.IntS opIntS(Test.IntS s, Ice.Current current)
    {
        Test.IntS r = new Test.IntS();
        for(int i = 0; i < s.Count; ++i)
        {
            r.Add(-s[i]);
        }
        return r;
    }

    public override void opByteSOneway(Test.ByteS s, Ice.Current current)
    {
    }

    public override Dictionary<string, string> opContext(Ice.Current current)
    {
        return current.ctx == null ? new Dictionary<string, string>() : new Dictionary<string, string>(current.ctx);
    }

    public override void opDoubleMarshaling(double p1, Test.DoubleS p2, Ice.Current current)
    {
        double d = 1278312346.0 / 13.0;
        test(p1 == d);
        for(int i = 0; i < p2.Count; ++i)
        {
            test(p2[i] == d);
        }
    }

    public override Test.StringS opStringS(Test.StringS p1, Test.StringS p2, out Test.StringS p3, Ice.Current current)
    {
        p3 = new Test.StringS();
        p3.AddRange(p1);
        p3.AddRange(p2);

        Test.StringS r = new Test.StringS();
        for(int i = 0; i < p1.Count; i++)
        {
            r.Add(p1[p1.Count - (i + 1)]);
        }
        return r;
    }

    public override Test.StringSS opStringSS(Test.StringSS p1, Test.StringSS p2, out Test.StringSS p3,
                                             Ice.Current current)
    {
        p3 = new Test.StringSS();
        p3.AddRange(p1);
        p3.AddRange(p2);

        Test.StringSS r = new Test.StringSS();
        for(int i = 0; i < p2.Count; i++)
        {
            r.Add(p2[p2.Count - (i + 1)]);
        }
        return r;
    }

    public override Test.StringSS[] opStringSSS(Test.StringSS[] p1, Test.StringSS[] p2, out Test.StringSS[] p3,
                                                Ice.Current current)
    {
        p3 = new Test.StringSS[p1.Length + p2.Length];
        p1.CopyTo(p3, 0);
        p2.CopyTo(p3, p1.Length);

        Test.StringSS[] r = new Test.StringSS[p2.Length];
        for(int i = 0; i < p2.Length; i++)
        {
            r[i] = p2[p2.Length - (i + 1)];
        }
        return r;
    }

    public override Dictionary<string, string> opStringStringD(Dictionary<string, string> p1,
                                                               Dictionary<string, string> p2,
                                                               out Dictionary<string, string> p3,
                                                               Ice.Current current)
    {
        p3 = p1;
        Dictionary<string, string> r = new Dictionary<string, string>();
        foreach(KeyValuePair<string, string> e in p1)
        {
            r[e.Key] = e.Value;
        }
        foreach(KeyValuePair<string, string> e in p2)
        {
            r[e.Key] = e.Value;
        }
        return r;
    }

    public override Test.Structure opStruct(Test.Structure p1, Test.Structure p2,
                                            out Test.Structure p3, Ice.Current current)
    {
        p3 = p1;
        p3.s.s = "a new string";
        return p2;
    }

    public override void opIdempotent(Ice.Current current)
    {
        test(current.mode == Ice.OperationMode.Idempotent);
    }

    public override void opNonmutating(Ice.Current current)
    {
        test(current.mode == Ice.OperationMode.Nonmutating);
    }

    public override void opDerived(Ice.Current current)
    {
    }
}
