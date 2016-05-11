// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
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
        Thread.Sleep(ms);
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

    public void opBoolS_async(Test.AMD_MyClass_opBoolS cb, bool[] p1, bool[] p2, Ice.Current current)
    {
        bool[] p3 = new bool[p1.Length + p2.Length];
        Array.Copy(p1, p3, p1.Length);
        Array.Copy(p2, 0, p3, p1.Length, p2.Length);

        bool[] r = new bool[p1.Length];
        for(int i = 0; i < p1.Length; i++)
        {
            r[i] = p1[p1.Length - (i + 1)];
        }
        cb.ice_response(r, p3);
    }

    public void opBoolSS_async(Test.AMD_MyClass_opBoolSS cb, bool[][] p1, bool[][] p2,
                                        Ice.Current current)
    {
        bool[][] p3 = new bool[p1.Length + p2.Length][];
        Array.Copy(p1, p3, p1.Length);
        Array.Copy(p2, 0, p3, p1.Length, p2.Length);

        bool[][] r = new bool[p1.Length][];
        for(int i = 0; i < p1.Length; i++)
        {
            r[i] = p1[p1.Length - (i + 1)];
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

    public void opByteS_async(Test.AMD_MyClass_opByteS cb, byte[] p1, byte[] p2, Ice.Current current)
    {
        byte[] p3 = new byte[p1.Length];
        for(int i = 0; i < p1.Length; i++)
        {
            p3[i] = p1[p1.Length - (i + 1)];
        }

        byte[] r = new byte[p1.Length + p2.Length];
        Array.Copy(p1, r, p1.Length);
        Array.Copy(p2, 0, r, p1.Length, p2.Length);
        cb.ice_response(r, p3);
    }

    public void opByteSS_async(Test.AMD_MyClass_opByteSS cb, byte[][] p1, byte[][] p2,
                                        Ice.Current current)
    {
        byte[][] p3 = new byte[p1.Length][];
        for(int i = 0; i < p1.Length; i++)
        {
            p3[i] = p1[p1.Length - (i + 1)];
        }

        byte[][] r = new byte[p1.Length + p2.Length][];
        Array.Copy(p1, r, p1.Length);
        Array.Copy(p2, 0, r, p1.Length, p2.Length);
        cb.ice_response(r, p3);
    }

    public void opFloatDouble_async(Test.AMD_MyClass_opFloatDouble cb, float p1, double p2,
                                    Ice.Current current)
    {
        cb.ice_response(p2, p1, p2);
    }

    public void opFloatDoubleS_async(Test.AMD_MyClass_opFloatDoubleS cb, float[] p1, double[] p2,
                                              Ice.Current current)
    {
        float[] p3 = p1;

        double[] p4 = new double[p2.Length];
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
        cb.ice_response(r, p3, p4);
    }

    public void opFloatDoubleSS_async(Test.AMD_MyClass_opFloatDoubleSS cb, float[][] p1, double[][] p2,
                                               Ice.Current current)
    {
        float[][] p3 = p1;

        double[][] p4 = new double[p2.Length][];
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


    public void opShortIntLongS_async(Test.AMD_MyClass_opShortIntLongS cb, short[] p1, int[] p2,
                                               long[] p3, Ice.Current current)
    {
        short[] p4 = p1;

        int[] p5 = new int[p2.Length];
        for(int i = 0; i < p2.Length; i++)
        {
            p5[i] = p2[p2.Length - (i + 1)];
        }

        long[] p6 = new long[p3.Length + p3.Length];
        Array.Copy(p3, p6, p3.Length);
        Array.Copy(p3, 0, p6, p3.Length, p3.Length);
        cb.ice_response(p3, p4, p5, p6);
    }

    public void opShortIntLongSS_async(Test.AMD_MyClass_opShortIntLongSS cb, short[][] p1,
                                                int[][] p2, long[][] p3, Ice.Current current)
    {
        short[][] p4 = p1;

        int[][] p5 = new int[p2.Length][];
        for(int i = 0; i < p2.Length; i++)
        {
            p5[i] = p2[p2.Length - (i + 1)];
        }

        long[][] p6 = new long[p3.Length + p3.Length][];
        Array.Copy(p3, p6, p3.Length);
        Array.Copy(p3, 0, p6, p3.Length, p3.Length);
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

    public void opByteBoolDS_async(Test.AMD_MyClass_opByteBoolDS cb,
                                   Dictionary<byte, bool>[] p1,
                                   Dictionary<byte, bool>[] p2,
                                   Ice.Current current)
    {
        Dictionary<byte, bool>[] p3 = new Dictionary<byte, bool>[p1.Length + p2.Length];
        Array.Copy(p2, p3, p2.Length);
        Array.Copy(p1, 0, p3, p2.Length, p1.Length);

        Dictionary<byte, bool>[] r = new Dictionary<byte, bool>[p1.Length];
        for(int i = 0; i < p1.Length; i++)
        {
            r[i] = p1[p1.Length - (i + 1)];
        }
        cb.ice_response(r,p3);
    }

    public void opShortIntDS_async(Test.AMD_MyClass_opShortIntDS cb,
                                   Dictionary<short, int>[] p1,
                                   Dictionary<short, int>[] p2,
                                   Ice.Current current)
    {
        Dictionary<short, int>[] p3 = new Dictionary<short, int>[p1.Length + p2.Length];
        Array.Copy(p2, p3, p2.Length);
        Array.Copy(p1, 0, p3, p2.Length, p1.Length);

        Dictionary<short, int>[] r = new Dictionary<short, int>[p1.Length];
        for(int i = 0; i < p1.Length; i++)
        {
            r[i] = p1[p1.Length - (i + 1)];
        }
        cb.ice_response(r,p3);
    }

    public void opLongFloatDS_async(Test.AMD_MyClass_opLongFloatDS cb,
                                    Dictionary<long, float>[] p1,
                                    Dictionary<long, float>[] p2,
                                    Ice.Current current)
    {
        Dictionary<long, float>[] p3 = new Dictionary<long, float>[p1.Length + p2.Length];
        Array.Copy(p2, p3, p2.Length);
        Array.Copy(p1, 0, p3, p2.Length, p1.Length);

        Dictionary<long, float>[] r = new Dictionary<long, float>[p1.Length];
        for(int i = 0; i < p1.Length; i++)
        {
            r[i] = p1[p1.Length - (i + 1)];
        }
        cb.ice_response(r,p3);
    }

    public void opStringStringDS_async(Test.AMD_MyClass_opStringStringDS cb,
                                       Dictionary<string, string>[] p1,
                                       Dictionary<string, string>[] p2,
                                       Ice.Current current)
    {
        Dictionary<string, string>[] p3 = new Dictionary<string, string>[p1.Length + p2.Length];
        Array.Copy(p2, p3, p2.Length);
        Array.Copy(p1, 0, p3, p2.Length, p1.Length);

        Dictionary<string, string>[] r = new Dictionary<string, string>[p1.Length];
        for(int i = 0; i < p1.Length; i++)
        {
            r[i] = p1[p1.Length - (i + 1)];
        }
        cb.ice_response(r,p3);
    }

    public void opStringMyEnumDS_async(Test.AMD_MyClass_opStringMyEnumDS cb,
                                       Dictionary<string, Test.MyEnum>[] p1,
                                       Dictionary<string, Test.MyEnum>[] p2,
                                       Ice.Current current)
    {
        Dictionary<string, Test.MyEnum>[] p3 = new Dictionary<string, Test.MyEnum>[p1.Length + p2.Length];
        Array.Copy(p2, p3, p2.Length);
        Array.Copy(p1, 0, p3, p2.Length, p1.Length);

        Dictionary<string, Test.MyEnum>[] r = new Dictionary<string, Test.MyEnum>[p1.Length];
        for(int i = 0; i < p1.Length; i++)
        {
            r[i] = p1[p1.Length - (i + 1)];
        }
        cb.ice_response(r,p3);
    }

    public void opMyEnumStringDS_async(Test.AMD_MyClass_opMyEnumStringDS cb,
                                       Dictionary<Test.MyEnum, string>[] p1,
                                       Dictionary<Test.MyEnum, string>[] p2,
                                       Ice.Current current)
    {
        Dictionary<Test.MyEnum, string>[] p3 = new Dictionary<Test.MyEnum, string>[p1.Length + p2.Length];
        Array.Copy(p2, p3, p2.Length);
        Array.Copy(p1, 0, p3, p2.Length, p1.Length);

        Dictionary<Test.MyEnum, string>[] r = new Dictionary<Test.MyEnum, string>[p1.Length];
        for(int i = 0; i < p1.Length; i++)
        {
            r[i] = p1[p1.Length - (i + 1)];
        }
        cb.ice_response(r,p3);
    }

    public void opMyStructMyEnumDS_async(Test.AMD_MyClass_opMyStructMyEnumDS cb,
                                         Dictionary<Test.MyStruct, Test.MyEnum>[] p1,
                                         Dictionary<Test.MyStruct, Test.MyEnum>[] p2,
                                         Ice.Current current)
    {
        Dictionary<Test.MyStruct, Test.MyEnum>[] p3 = new Dictionary<Test.MyStruct, Test.MyEnum>[p1.Length + p2.Length];
        Array.Copy(p2, p3, p2.Length);
        Array.Copy(p1, 0, p3, p2.Length, p1.Length);

        Dictionary<Test.MyStruct, Test.MyEnum>[] r = new Dictionary<Test.MyStruct, Test.MyEnum>[p1.Length];
        for(int i = 0; i < p1.Length; i++)
        {
            r[i] = p1[p1.Length - (i + 1)];
        }
        cb.ice_response(r,p3);
    }

    public void opByteByteSD_async(Test.AMD_MyClass_opByteByteSD cb,
                                   Dictionary<byte, byte[]> p1,
                                   Dictionary<byte, byte[]> p2,
                                   Ice.Current current)
    {
        Dictionary<byte, byte[]> p3 = p2;
        Dictionary<byte, byte[]> r = new Dictionary<byte, byte[]>();
        foreach(KeyValuePair<byte, byte[]> e in p1)
        {
            r[e.Key] = e.Value;
        }
        foreach(KeyValuePair<byte, byte[]> e in p2)
        {
            r[e.Key] = e.Value;
        }
        cb.ice_response(r,p3);
    }

    public void opBoolBoolSD_async(Test.AMD_MyClass_opBoolBoolSD cb,
                                   Dictionary<bool, bool[]> p1,
                                   Dictionary<bool, bool[]> p2,
                                   Ice.Current current)
    {
        Dictionary<bool, bool[]>  p3 = p2;
        Dictionary<bool, bool[]> r = new Dictionary<bool, bool[]>();
        foreach(KeyValuePair<bool, bool[]> e in p1)
        {
            r[e.Key] = e.Value;
        }
        foreach(KeyValuePair<bool, bool[]> e in p2)
        {
            r[e.Key] = e.Value;
        }
        cb.ice_response(r,p3);
    }

    public void opShortShortSD_async(Test.AMD_MyClass_opShortShortSD cb,
                                     Dictionary<short, short[]> p1,
                                     Dictionary<short, short[]> p2,
                                     Ice.Current current)
    {
        Dictionary<short, short[]> p3 = p2;
        Dictionary<short, short[]> r = new Dictionary<short, short[]>();
        foreach(KeyValuePair<short, short[]> e in p1)
        {
            r[e.Key] = e.Value;
        }
        foreach(KeyValuePair<short, short[]> e in p2)
        {
            r[e.Key] = e.Value;
        }
        cb.ice_response(r,p3);
    }

    public void opIntIntSD_async(Test.AMD_MyClass_opIntIntSD cb,
                                 Dictionary<int, int[]> p1,
                                 Dictionary<int, int[]> p2,
                                 Ice.Current current)
    {
        Dictionary<int, int[]> p3 = p2;
        Dictionary<int, int[]> r = new Dictionary<int, int[]>();
        foreach(KeyValuePair<int, int[]> e in p1)
        {
            r[e.Key] = e.Value;
        }
        foreach(KeyValuePair<int, int[]> e in p2)
        {
            r[e.Key] = e.Value;
        }
        cb.ice_response(r,p3);
    }

    public void opLongLongSD_async(Test.AMD_MyClass_opLongLongSD cb,
                                   Dictionary<long, long[]> p1,
                                   Dictionary<long, long[]> p2,
                                   Ice.Current current)
    {
        Dictionary<long, long[]> p3 = p2;
        Dictionary<long, long[]> r = new Dictionary<long, long[]>();
        foreach(KeyValuePair<long, long[]> e in p1)
        {
            r[e.Key] = e.Value;
        }
        foreach(KeyValuePair<long, long[]> e in p2)
        {
            r[e.Key] = e.Value;
        }
        cb.ice_response(r,p3);
    }

    public void opStringFloatSD_async(Test.AMD_MyClass_opStringFloatSD cb,
                                      Dictionary<string, float[]> p1,
                                      Dictionary<string, float[]> p2,
                                      Ice.Current current)
    {
        Dictionary<string, float[]> p3 = p2;
        Dictionary<string, float[]> r = new Dictionary<string, float[]>();
        foreach(KeyValuePair<string, float[]> e in p1)
        {
            r[e.Key] = e.Value;
        }
        foreach(KeyValuePair<string, float[]> e in p2)
        {
            r[e.Key] = e.Value;
        }
        cb.ice_response(r,p3);
    }

    public void opStringDoubleSD_async(Test.AMD_MyClass_opStringDoubleSD cb,
                                       Dictionary<string, double[]> p1,
                                       Dictionary<string, double[]> p2,
                                       Ice.Current current)
    {
        Dictionary<string, double[]> p3 = p2;
        Dictionary<string, double[]> r = new Dictionary<string, double[]>();
        foreach(KeyValuePair<string, double[]> e in p1)
        {
            r[e.Key] = e.Value;
        }
        foreach(KeyValuePair<string, double[]> e in p2)
        {
            r[e.Key] = e.Value;
        }
        cb.ice_response(r,p3);
    }

    public void opStringStringSD_async(Test.AMD_MyClass_opStringStringSD cb,
                                       Dictionary<string, string[]> p1,
                                       Dictionary<string, string[]> p2,
                                       Ice.Current current)
    {
        Dictionary<string, string[]> p3 = p2;
        Dictionary<string, string[]> r = new Dictionary<string, string[]>();
        foreach(KeyValuePair<string, string[]> e in p1)
        {
            r[e.Key] = e.Value;
        }
        foreach(KeyValuePair<string, string[]> e in p2)
        {
            r[e.Key] = e.Value;
        }
        cb.ice_response(r,p3);
    }

    public void opMyEnumMyEnumSD_async(Test.AMD_MyClass_opMyEnumMyEnumSD cb,
                                       Dictionary<Test.MyEnum, Test.MyEnum[]> p1,
                                       Dictionary<Test.MyEnum, Test.MyEnum[]> p2,
                                       Ice.Current ice)
    {
        Dictionary<Test.MyEnum, Test.MyEnum[]> p3 = p2;
        Dictionary<Test.MyEnum, Test.MyEnum[]> r = new Dictionary<Test.MyEnum, Test.MyEnum[]>();
        foreach(KeyValuePair<Test.MyEnum, Test.MyEnum[]> e in p1)
        {
            r[e.Key] = e.Value;
        }
        foreach(KeyValuePair<Test.MyEnum, Test.MyEnum[]> e in p2)
        {
            r[e.Key] = e.Value;
        }
        cb.ice_response(r,p3);
    }

    public void opIntS_async(Test.AMD_MyClass_opIntS cb, int[] s, Ice.Current current)
    {
        int[] r = new int[s.Length];
        for(int i = 0; i < s.Length; ++i)
        {
            r[i] = -s[i];
        }
        cb.ice_response(r);
    }

    public void opContext_async(Test.AMD_MyClass_opContext cb, Ice.Current current)
    {
        cb.ice_response(current.ctx);
    }

    public void opByteSOneway_async(Test.AMD_MyClass_opByteSOneway cb, byte[] s, Ice.Current current)
    {
        lock(this)
        {
            ++_opByteSOnewayCallCount;
        }
        cb.ice_response();
    }

    public void opByteSOnewayCallCount_async(Test.AMD_MyClass_opByteSOnewayCallCount cb, Ice.Current current)
    {
        lock(this)
        {
            int count = _opByteSOnewayCallCount;
            _opByteSOnewayCallCount = 0;
            cb.ice_response(count);
        }
    }

    public void opDoubleMarshaling_async(Test.AMD_MyClass_opDoubleMarshaling cb, double p1, double[] p2,
                                                  Ice.Current current)
    {
        double d = 1278312346.0 / 13.0;
        test(p1 == d);
        for(int i = 0; i < p2.Length; ++i)
        {
            test(p2[i] == d);
        }
        cb.ice_response();
    }

    public void opStringS_async(Test.AMD_MyClass_opStringS cb, string[] p1, string[] p2,
                                         Ice.Current current)
    {
        string[] p3 = new string[p1.Length + p2.Length];
        Array.Copy(p1, p3, p1.Length);
        Array.Copy(p2, 0, p3, p1.Length, p2.Length);

        string[] r = new string[p1.Length];
        for(int i = 0; i < p1.Length; i++)
        {
            r[i] = p1[p1.Length - (i + 1)];
        }
        cb.ice_response(r, p3);
    }

    public void opStringSS_async(Test.AMD_MyClass_opStringSS cb, string[][] p1, string[][] p2,
                                          Ice.Current current)
    {
        string[][] p3 = new string[p1.Length + p2.Length][];
        Array.Copy(p1, p3, p1.Length);
        Array.Copy(p2, 0, p3, p1.Length, p2.Length);

        string[][] r = new string[p2.Length][];
        for(int i = 0; i < p2.Length; i++)
        {
            r[i] = p2[p2.Length - (i + 1)];
        }
        cb.ice_response(r, p3);
    }

    public void opStringSSS_async(Test.AMD_MyClass_opStringSSS cb, string[][][] p1, string[][][] p2,
                                           Ice.Current current)
    {
        string[][][] p3 = new string[p1.Length + p2.Length][][];
        Array.Copy(p1, p3, p1.Length);
        Array.Copy(p2, 0, p3, p1.Length, p2.Length);

        string[][][] r = new string[p2.Length][][];
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
    
    public void opByte1_async(Test.AMD_MyClass_opByte1 cb, byte value, Ice.Current current)
    {
        cb.ice_response(value);
    }
    
    public void opShort1_async(Test.AMD_MyClass_opShort1 cb, short value, Ice.Current current)
    {
        cb.ice_response(value);
    }
    
    public void opInt1_async(Test.AMD_MyClass_opInt1 cb, int value, Ice.Current current)
    {
        cb.ice_response(value);
    }
    
    public void opLong1_async(Test.AMD_MyClass_opLong1 cb, long value, Ice.Current current)
    {
        cb.ice_response(value);
    }
    
    public void opFloat1_async(Test.AMD_MyClass_opFloat1 cb, float value, Ice.Current current)
    {
        cb.ice_response(value);
    }
    
    public void opDouble1_async(Test.AMD_MyClass_opDouble1 cb, double value, Ice.Current current)
    {
        cb.ice_response(value);
    }
    
    public void opString1_async(Test.AMD_MyClass_opString1 cb, string value, Ice.Current current)
    {
        cb.ice_response(value);
    }
    
    public void opStringS1_async(Test.AMD_MyClass_opStringS1 cb, string[] value, Ice.Current current)
    {
        cb.ice_response(value);
    }
    
    public void opByteBoolD1_async(Test.AMD_MyClass_opByteBoolD1 cb, Dictionary<byte, bool> value,
                                   Ice.Current current)
    {
        cb.ice_response(value);
    }
    
    public void opStringS2_async(Test.AMD_MyClass_opStringS2 cb, string[] value, Ice.Current current)
    {
        cb.ice_response(value);
    }
    
    public void opByteBoolD2_async(Test.AMD_MyClass_opByteBoolD2 cb, Dictionary<byte, bool> value,
                                   Ice.Current current)
    {
        cb.ice_response(value);
    }
    
    public void opMyClass1_async(Test.AMD_MyDerivedClass_opMyClass1 cb, Test.MyClass1 value,
                                 Ice.Current current)
    {
        cb.ice_response(value);
    }
    
    public void opMyStruct1_async(Test.AMD_MyDerivedClass_opMyStruct1 cb, Test.MyStruct1 value,
                                  Ice.Current current)
    {
        cb.ice_response(value);
    }
    
    public void opStringLiterals_async(Test.AMD_MyClass_opStringLiterals cb, Ice.Current current)
    {
        cb.ice_response(new string[]
            {
                Test.s0.value,
                Test.s1.value,
                Test.s2.value,
                Test.s3.value,
                Test.s4.value,
                Test.s5.value,
                Test.s6.value,
                Test.s7.value,
                Test.s8.value,
                Test.s9.value,
                Test.s10.value,
                
                Test.sw0.value,
                Test.sw1.value,
                Test.sw2.value,
                Test.sw3.value,
                Test.sw4.value,
                Test.sw5.value,
                Test.sw6.value,
                Test.sw7.value,
                Test.sw8.value,
                Test.sw9.value,
                Test.sw10.value,
                
                Test.ss0.value,
                Test.ss1.value,
                Test.ss2.value,
                Test.ss3.value,
                Test.ss4.value,
                Test.ss5.value,
                
                Test.su0.value,
                Test.su1.value,
                Test.su2.value
            });
    }
    
    public void opWStringLiterals_async(Test.AMD_MyClass_opWStringLiterals cb, Ice.Current current)
    {
        cb.ice_response(new string[]
            {
                Test.s0.value,
                Test.s1.value,
                Test.s2.value,
                Test.s3.value,
                Test.s4.value,
                Test.s5.value,
                Test.s6.value,
                Test.s7.value,
                Test.s8.value,
                Test.s9.value,
                Test.s10.value,
                
                Test.sw0.value,
                Test.sw1.value,
                Test.sw2.value,
                Test.sw3.value,
                Test.sw4.value,
                Test.sw5.value,
                Test.sw6.value,
                Test.sw7.value,
                Test.sw8.value,
                Test.sw9.value,
                Test.sw10.value,
                
                Test.ss0.value,
                Test.ss1.value,
                Test.ss2.value,
                Test.ss3.value,
                Test.ss4.value,
                Test.ss5.value,
                
                Test.su0.value,
                Test.su1.value,
                Test.su2.value
            });
    }

    private Thread_opVoid _opVoidThread;
    private int _opByteSOnewayCallCount = 0;
}
