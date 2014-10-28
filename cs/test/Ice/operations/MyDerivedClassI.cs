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

    public override bool[] opBoolS(bool[] p1, bool[] p2, out bool[] p3, Ice.Current current)
    {
        p3 = new bool[p1.Length + p2.Length];
        Array.Copy(p1, p3, p1.Length);
        Array.Copy(p2, 0, p3, p1.Length, p2.Length);

        bool[] r = new bool[p1.Length];
        for(int i = 0; i < p1.Length; i++)
        {
            r[i] = p1[p1.Length - (i + 1)];
        }
        return r;
    }

    public override bool[][] opBoolSS(bool[][] p1, bool[][] p2, out bool[][] p3, Ice.Current current)
    {
        p3 = new bool[p1.Length + p2.Length][];
        Array.Copy(p1, p3, p1.Length);
        Array.Copy(p2, 0, p3, p1.Length, p2.Length);

        bool[][] r = new bool[p1.Length][];
        for(int i = 0; i < p1.Length; i++)
        {
            r[i] = p1[p1.Length - (i + 1)];
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

    public override byte[] opByteS(byte[] p1, byte[] p2, out byte[] p3, Ice.Current current)
    {
        p3 = new byte[p1.Length];
        for(int i = 0; i < p1.Length; i++)
        {
            p3[i] = p1[p1.Length - (i + 1)];
        }

        byte[] r = new byte[p1.Length + p2.Length];
        Array.Copy(p1, r, p1.Length);
        Array.Copy(p2, 0, r, p1.Length, p2.Length);
        return r;
    }

    public override byte[][] opByteSS(byte[][] p1, byte[][] p2, out byte[][] p3, Ice.Current current)
    {
        p3 = new byte[p1.Length][];
        for(int i = 0; i < p1.Length; i++)
        {
            p3[i] = p1[p1.Length - (i + 1)];
        }

        byte[][] r = new byte[p1.Length + p2.Length][];
        Array.Copy(p1, r, p1.Length);
        Array.Copy(p2, 0, r, p1.Length, p2.Length);
        return r;
    }

    public override double opFloatDouble(float p1, double p2, out float p3, out double p4, Ice.Current current)
    {
        p3 = p1;
        p4 = p2;
        return p2;
    }

    public override double[] opFloatDoubleS(float[] p1, double[] p2,
                                            out float[] p3, out double[] p4,
                                            Ice.Current current)
    {
        p3 = p1;

        p4 = new double[p2.Length];
        for(int i = 0; i < p2.Length; i++)
        {
            p4[i] = p2[p2.Length - (i + 1)];
        }

        double[] r = new double[p2.Length + p1.Length];
        Array.Copy(p2, r, p2.Length);
        for(int i = 0; i < p1.Length; i++)
        {
            r[p2.Length + i] = (double)p1[i];
        }
        return r;
    }

    public override double[][] opFloatDoubleSS(float[][] p1, double[][] p2,
                                               out float[][] p3, out double[][] p4,
                                               Ice.Current current)
    {
        p3 = p1;

        p4 = new double[p2.Length][];
        for(int i = 0; i < p2.Length; i++)
        {
            p4[i] = p2[p2.Length - (i + 1)];
        }

        double[][] r = new double[p2.Length + p2.Length][];
        Array.Copy(p2, r, p2.Length);
        for(int i = 0; i < p2.Length; i++)
        {
            r[p2.Length + i] = new double[p2[i].Length];
            for(int j = 0; j < p2[i].Length; j++)
            {
                r[p2.Length + i][j] = (double)p2[i][j];
            }
        }
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

    public override long[] opShortIntLongS(short[] p1, int[] p2, long[] p3,
                                           out short[] p4, out int[] p5, out long[] p6,
                                           Ice.Current current)
    {
        p4 = p1;

        p5 = new int[p2.Length];
        for(int i = 0; i < p2.Length; i++)
        {
            p5[i] = p2[p2.Length - (i + 1)];
        }

        p6 = new long[p3.Length + p3.Length];
        Array.Copy(p3, p6, p3.Length);
        Array.Copy(p3, 0, p6, p3.Length, p3.Length);

        return p3;
    }

    public override long[][] opShortIntLongSS(short[][] p1, int[][] p2, long[][] p3,
                                              out short[][] p4, out int[][] p5, out long[][] p6,
                                              Ice.Current current)
    {
        p4 = p1;

        p5 = new int[p2.Length][];
        for(int i = 0; i < p2.Length; i++)
        {
            p5[i] = p2[p2.Length - (i + 1)];
        }

        p6 = new long[p3.Length + p3.Length][];
        Array.Copy(p3, p6, p3.Length);
        Array.Copy(p3, 0, p6, p3.Length, p3.Length);

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

    public override int[] opIntS(int[] s, Ice.Current current)
    {
        int[] r = new int[s.Length];
        for(int i = 0; i < s.Length; ++i)
        {
            r[i] = -s[i];
        }
        return r;
    }

    public override void opByteSOneway(byte[] s, Ice.Current current)
    {
    }

    public override Dictionary<string, string> opContext(Ice.Current current)
    {
        return current.ctx == null ? new Dictionary<string, string>() : new Dictionary<string, string>(current.ctx);
    }

    public override void opDoubleMarshaling(double p1, double[] p2, Ice.Current current)
    {
        double d = 1278312346.0 / 13.0;
        test(p1 == d);
        for(int i = 0; i < p2.Length; ++i)
        {
            test(p2[i] == d);
        }
    }

    public override string[] opStringS(string[] p1, string[] p2, out string[] p3, Ice.Current current)
    {
        p3 = new string[p1.Length + p2.Length];
        Array.Copy(p1, p3, p1.Length);
        Array.Copy(p2, 0, p3, p1.Length, p2.Length);

        string[] r = new string[p1.Length];
        for(int i = 0; i < p1.Length; i++)
        {
            r[i] = p1[p1.Length - (i + 1)];
        }
        return r;
    }

    public override string[][] opStringSS(string[][] p1, string[][] p2, out string[][] p3, Ice.Current current)
    {
        p3 = new string[p1.Length + p2.Length][];
        Array.Copy(p1, p3, p1.Length);
        Array.Copy(p2, 0, p3, p1.Length, p2.Length);

        string[][] r = new string[p2.Length][];
        for(int i = 0; i < p2.Length; i++)
        {
            r[i] = p2[p2.Length - (i + 1)];
        }
        return r;
    }

    public override string[][][] opStringSSS(string[][][] p1, string[][][] p2, out string[][][] p3, Ice.Current current)
    {
        p3 = new string[p1.Length + p2.Length][][];
        Array.Copy(p1, p3, p1.Length);
        Array.Copy(p2, 0, p3, p1.Length, p2.Length);

        string[][][] r = new string[p2.Length][][];
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
