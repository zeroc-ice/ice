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
using System.Threading.Tasks;

using Test;

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
        public Thread_opVoid(Action response)
        {
            _response = response;
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
            _response();
        }

        public void Join()
        {
            lock(this)
            {
                _thread.Join();
            }
        }

        private Action _response;
        private Thread _thread;
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

    public override void shutdownAsync(Action response, Action<Exception> exception, Ice.Current current)
    {
        while(_opVoidThread != null)
        {
            _opVoidThread.Join();
            _opVoidThread = null;
        }

        current.adapter.getCommunicator().shutdown();
        response();
    }

    public override async void delayAsync(int ms, Action response, Action<Exception> exception, Ice.Current current)
    {
        await Task.Delay(ms); 
        response();
    }

    public override void opVoidAsync(Action response, Action<Exception> exception, Ice.Current current)
    {
        test(current.mode == Ice.OperationMode.Normal);

        while(_opVoidThread != null)
        {
            _opVoidThread.Join();
            _opVoidThread = null;
        }

        _opVoidThread = new Thread_opVoid(response);
        _opVoidThread.Start();
    }

    public override void opBoolAsync(bool p1, bool p2, 
                                     Action<MyClass_OpBoolResult> response,
                                     Action<Exception> exception,
                                     Ice.Current current)
    {
        response(new MyClass_OpBoolResult(p2, p1));
    }

    public override void opBoolSAsync(bool[] p1, bool[] p2,
                                      Action<MyClass_OpBoolSResult> response,
                                      Action<Exception> exception,
                                      Ice.Current current)
    {
        bool[] p3 = new bool[p1.Length + p2.Length];
        Array.Copy(p1, p3, p1.Length);
        Array.Copy(p2, 0, p3, p1.Length, p2.Length);

        bool[] r = new bool[p1.Length];
        for(int i = 0; i < p1.Length; i++)
        {
            r[i] = p1[p1.Length - (i + 1)];
        }

        response(new MyClass_OpBoolSResult(r, p3));
    }

    public override void
    opBoolSSAsync(bool[][] p1, bool[][] p2,
                  Action<MyClass_OpBoolSSResult> response,
                  Action<Exception> exception,
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

        response(new MyClass_OpBoolSSResult(r, p3));
    }

    public override void opByteAsync(byte p1, byte p2,
                                     Action<MyClass_OpByteResult> response,
                                     Action<Exception> exception,
                                     Ice.Current current)
    {
        response(new MyClass_OpByteResult(p1, (byte)(p1 ^ p2)));
    }

    public override void
    opByteBoolDAsync(Dictionary<byte, bool> p1, 
                     Dictionary<byte, bool> p2,
                     Action<MyClass_OpByteBoolDResult> response,
                     Action<Exception> exception,
                     Ice.Current current)
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

        response(new MyClass_OpByteBoolDResult(r, p3));
    }

    public override void
    opByteSAsync(byte[] p1, byte[] p2,
                 Action<MyClass_OpByteSResult> response,
                 Action<Exception> exception,
                 Ice.Current current)
    {
        byte[] p3 = new byte[p1.Length];
        for(int i = 0; i < p1.Length; i++)
        {
            p3[i] = p1[p1.Length - (i + 1)];
        }

        byte[] r = new byte[p1.Length + p2.Length];
        Array.Copy(p1, r, p1.Length);
        Array.Copy(p2, 0, r, p1.Length, p2.Length);

        response(new MyClass_OpByteSResult(r, p3));
    }

    public override void
    opByteSSAsync(byte[][] p1, byte[][] p2,
                  Action<MyClass_OpByteSSResult> response,
                  Action<Exception> exception,
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

        response(new MyClass_OpByteSSResult(r, p3));
    }

    public override void opFloatDoubleAsync(float p1, double p2,
                                            Action<MyClass_OpFloatDoubleResult> response,
                                            Action<Exception> exception,
                                            Ice.Current current)
    {
        response(new MyClass_OpFloatDoubleResult(p2, p1, p2));
    }

    public override void
    opFloatDoubleSAsync(float[] p1, double[] p2,
                        Action<MyClass_OpFloatDoubleSResult> response,
                        Action<Exception> exception,
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
            r[p2.Length + i] = p1[i];
        }

        response(new MyClass_OpFloatDoubleSResult(r, p3, p4));
    }

    public override void
    opFloatDoubleSSAsync(float[][] p1, double[][] p2,
                         Action<MyClass_OpFloatDoubleSSResult> response,
                         Action<Exception> exception,
                         Ice.Current current)
    {
        var p3 = p1;

        var p4 = new double[p2.Length][];
        for(int i = 0; i < p2.Length; i++)
        {
            p4[i] = p2[p2.Length - (i + 1)];
        }

        var r = new double[p2.Length + p2.Length][];
        Array.Copy(p2, r, p2.Length);
        for(int i = 0; i < p2.Length; i++)
        {
            r[p2.Length + i] = new double[p2[i].Length];
            for(int j = 0; j < p2[i].Length; j++)
            {
                r[p2.Length + i][j] = p2[i][j];
            }
        }

        response(new MyClass_OpFloatDoubleSSResult(r, p3, p4));
    }

    public override void
    opLongFloatDAsync(Dictionary<long, float> p1, Dictionary<long, float> p2,
                      Action<MyClass_OpLongFloatDResult> response,
                      Action<Exception> exception,
                      Ice.Current current)
    {
        var p3 = p1;
        var r = new Dictionary<long, float>();
        foreach(KeyValuePair<long, float> e in p1)
        {
            r[e.Key] = e.Value;
        }
        foreach(KeyValuePair<long, float> e in p2)
        {
            r[e.Key] = e.Value;
        }

        response(new MyClass_OpLongFloatDResult(r, p3));
    }

    public override void
    opMyClassAsync(MyClassPrx p1, Action<MyClass_OpMyClassResult> response,
                   Action<Exception> exception, Ice.Current current)
    {
        var p2 = p1;
        var p3 = MyClassPrxHelper.uncheckedCast(current.adapter.createProxy(
                                                Ice.Util.stringToIdentity("noSuchIdentity")));
        response(new MyClass_OpMyClassResult(
            MyClassPrxHelper.uncheckedCast(current.adapter.createProxy(current.id)), p2, p3));
    }

    public override void
    opMyEnumAsync(MyEnum p1, Action<MyClass_OpMyEnumResult> response, Action<Exception> exception, Ice.Current current)
    {
        response(new MyClass_OpMyEnumResult(MyEnum.enum3, p1));
    }

    public override void
    opShortIntDAsync(Dictionary<short, int> p1, Dictionary<short, int> p2,
                     Action<MyClass_OpShortIntDResult> response,
                     Action<Exception> exception,
                     Ice.Current current)
    {
        var p3 = p1;
        var r = new Dictionary<short, int>();
        foreach(KeyValuePair<short, int> e in p1)
        {
            r[e.Key] = e.Value;
        }
        foreach(KeyValuePair<short, int> e in p2)
        {
            r[e.Key] = e.Value;
        }
        response(new MyClass_OpShortIntDResult(r, p3));
    }

    public override void
    opShortIntLongAsync(short p1, int p2, long p3,
                        Action<MyClass_OpShortIntLongResult> response, 
                        Action<Exception> exception,
                        Ice.Current current)
    {
        response(new MyClass_OpShortIntLongResult(p3, p1, p2, p3));
    }

    public override void
    opShortIntLongSAsync(short[] p1, int[] p2, long[] p3,
                         Action<MyClass_OpShortIntLongSResult> response,
                         Action<Exception> exception,
                         Ice.Current current)
    {
        var p4 = p1;
        var p5 = new int[p2.Length];
        for(int i = 0; i < p2.Length; i++)
        {
            p5[i] = p2[p2.Length - (i + 1)];
        }
        var p6 = new long[p3.Length + p3.Length];
        Array.Copy(p3, p6, p3.Length);
        Array.Copy(p3, 0, p6, p3.Length, p3.Length);
        response(new MyClass_OpShortIntLongSResult(p3, p4, p5, p6));
    }

    public override void
    opShortIntLongSSAsync(short[][] p1, int[][] p2, long[][] p3,
                          Action<MyClass_OpShortIntLongSSResult> response,
                          Action<Exception> exception,
                          Ice.Current current)
    {
        var p4 = p1;

        var p5 = new int[p2.Length][];
        for(int i = 0; i < p2.Length; i++)
        {
            p5[i] = p2[p2.Length - (i + 1)];
        }

        var p6 = new long[p3.Length + p3.Length][];
        Array.Copy(p3, p6, p3.Length);
        Array.Copy(p3, 0, p6, p3.Length, p3.Length);
        response(new MyClass_OpShortIntLongSSResult(p3, p4, p5, p6));
    }

    public override void
    opStringAsync(string p1, string p2,
                  Action<MyClass_OpStringResult> response,
                  Action<Exception> exception,
                  Ice.Current current)
    {
        response(new MyClass_OpStringResult(p1 + " " + p2, p2 + " " + p1));
    }

    public override void
    opStringMyEnumDAsync(Dictionary<string, MyEnum> p1, Dictionary<string, MyEnum> p2,
                         Action<MyClass_OpStringMyEnumDResult> response,
                         Action<Exception> exception,
                         Ice.Current current)
    {
        var p3 = p1;
        var r = new Dictionary<string, Test.MyEnum>();
        foreach(KeyValuePair<string, Test.MyEnum> e in p1)
        {
            r[e.Key] = e.Value;
        }
        foreach(KeyValuePair<string, Test.MyEnum> e in p2)
        {
            r[e.Key] = e.Value;
        }
        response(new MyClass_OpStringMyEnumDResult(r, p3));
    }

    public override void
    opMyEnumStringDAsync(Dictionary<MyEnum, string> p1, Dictionary<MyEnum, string> p2,
                         Action<MyClass_OpMyEnumStringDResult> response,
                         Action<Exception> exception,
                         Ice.Current current)
    {
        var p3 = p1;
        var r = new Dictionary<MyEnum, string>();
        foreach(var e in p1)
        {
            r[e.Key] = e.Value;
        }
        foreach(KeyValuePair<MyEnum, string> e in p2)
        {
            r[e.Key] = e.Value;
        }
        response(new MyClass_OpMyEnumStringDResult(r, p3));
    }

    public override void
    opMyStructMyEnumDAsync(Dictionary<MyStruct, MyEnum> p1,
                           Dictionary<MyStruct, MyEnum> p2,
                           Action<MyClass_OpMyStructMyEnumDResult> response,
                           Action<Exception> exception,
                           Ice.Current current)
    {
        var p3 = p1;
        var r = new Dictionary<MyStruct, MyEnum>();
        foreach(var e in p1)
        {
            r[e.Key] = e.Value;
        }
        foreach(var e in p2)
        {
            r[e.Key] = e.Value;
        }
        response(new MyClass_OpMyStructMyEnumDResult(r, p3));
    }

    public override void
    opByteBoolDSAsync(Dictionary<byte, bool>[] p1,
                      Dictionary<byte, bool>[] p2,
                      Action<MyClass_OpByteBoolDSResult> response,
                      Action<Exception> exception,
                      Ice.Current current)
    {
        var p3 = new Dictionary<byte, bool>[p1.Length + p2.Length];
        Array.Copy(p2, p3, p2.Length);
        Array.Copy(p1, 0, p3, p2.Length, p1.Length);

        var r = new Dictionary<byte, bool>[p1.Length];
        for(int i = 0; i < p1.Length; i++)
        {
            r[i] = p1[p1.Length - (i + 1)];
        }
        response(new MyClass_OpByteBoolDSResult(r, p3));
    }

    public override void
    opShortIntDSAsync(Dictionary<short, int>[] p1, Dictionary<short, int>[] p2,
                      Action<MyClass_OpShortIntDSResult> response,
                      Action<Exception> exception,
                      Ice.Current current)
    {
        var p3 = new Dictionary<short, int>[p1.Length + p2.Length];
        Array.Copy(p2, p3, p2.Length);
        Array.Copy(p1, 0, p3, p2.Length, p1.Length);

        var r = new Dictionary<short, int>[p1.Length];
        for(int i = 0; i < p1.Length; i++)
        {
            r[i] = p1[p1.Length - (i + 1)];
        }
        response(new MyClass_OpShortIntDSResult(r, p3));
    }

    public override void
    opLongFloatDSAsync(Dictionary<long, float>[] p1,
                       Dictionary<long, float>[] p2,
                       Action<MyClass_OpLongFloatDSResult> response,
                       Action<Exception> exception,
                       Ice.Current current)
    {
        var p3 = new Dictionary<long, float>[p1.Length + p2.Length];
        Array.Copy(p2, p3, p2.Length);
        Array.Copy(p1, 0, p3, p2.Length, p1.Length);

        var r = new Dictionary<long, float>[p1.Length];
        for(int i = 0; i < p1.Length; i++)
        {
            r[i] = p1[p1.Length - (i + 1)];
        }
        response(new MyClass_OpLongFloatDSResult(r, p3));
    }

    public override void
    opStringStringDSAsync(Dictionary<string, string>[] p1,
                          Dictionary<string, string>[] p2,
                          Action<MyClass_OpStringStringDSResult> response,
                          Action<Exception> exception,
                          Ice.Current current)
    {
        var p3 = new Dictionary<string, string>[p1.Length + p2.Length];
        Array.Copy(p2, p3, p2.Length);
        Array.Copy(p1, 0, p3, p2.Length, p1.Length);

        var r = new Dictionary<string, string>[p1.Length];
        for(int i = 0; i < p1.Length; i++)
        {
            r[i] = p1[p1.Length - (i + 1)];
        }
        response(new MyClass_OpStringStringDSResult(r, p3));
    }

    public override void
    opStringMyEnumDSAsync(Dictionary<string, MyEnum>[] p1,
                          Dictionary<string, MyEnum>[] p2,
                          Action<MyClass_OpStringMyEnumDSResult> response,
                          Action<Exception> exception,
                          Ice.Current current)
    {
        var p3 = new Dictionary<string, MyEnum>[p1.Length + p2.Length];
        Array.Copy(p2, p3, p2.Length);
        Array.Copy(p1, 0, p3, p2.Length, p1.Length);

        var r = new Dictionary<string, MyEnum>[p1.Length];
        for(int i = 0; i < p1.Length; i++)
        {
            r[i] = p1[p1.Length - (i + 1)];
        }
        response(new MyClass_OpStringMyEnumDSResult(r, p3));
    }

    public override void
    opMyEnumStringDSAsync(Dictionary<MyEnum, string>[] p1,
                          Dictionary<MyEnum, string>[] p2,
                          Action<MyClass_OpMyEnumStringDSResult> response,
                          Action<Exception> exception,
                          Ice.Current current)
    {
        var p3 = new Dictionary<MyEnum, string>[p1.Length + p2.Length];
        Array.Copy(p2, p3, p2.Length);
        Array.Copy(p1, 0, p3, p2.Length, p1.Length);

        var r = new Dictionary<MyEnum, string>[p1.Length];
        for(int i = 0; i < p1.Length; i++)
        {
            r[i] = p1[p1.Length - (i + 1)];
        }
        response(new MyClass_OpMyEnumStringDSResult(r, p3));
    }

    public override void
    opMyStructMyEnumDSAsync(Dictionary<MyStruct, MyEnum>[] p1,
                            Dictionary<MyStruct, MyEnum>[] p2,
                            Action<MyClass_OpMyStructMyEnumDSResult> response,
                            Action<Exception> exception,
                            Ice.Current current)
    {
        var p3 = new Dictionary<MyStruct, MyEnum>[p1.Length + p2.Length];
        Array.Copy(p2, p3, p2.Length);
        Array.Copy(p1, 0, p3, p2.Length, p1.Length);

        var r = new Dictionary<MyStruct, MyEnum>[p1.Length];
        for(int i = 0; i < p1.Length; i++)
        {
            r[i] = p1[p1.Length - (i + 1)];
        }
        response(new MyClass_OpMyStructMyEnumDSResult(r, p3));
    }

    public override void
    opByteByteSDAsync(Dictionary<byte, byte[]> p1,
                      Dictionary<byte, byte[]> p2,
                      Action<MyClass_OpByteByteSDResult> response,
                      Action<Exception> exception,
                      Ice.Current current)
    {
        var p3 = p2;
        var r = new Dictionary<byte, byte[]>();
        foreach(var e in p1)
        {
            r[e.Key] = e.Value;
        }
        foreach(var e in p2)
        {
            r[e.Key] = e.Value;
        }
        response(new MyClass_OpByteByteSDResult(r, p3));
    }

    public override void
    opBoolBoolSDAsync(Dictionary<bool, bool[]> p1,
                      Dictionary<bool, bool[]> p2,
                      Action<MyClass_OpBoolBoolSDResult> response,
                      Action<Exception> exception,
                      Ice.Current current)
    {
        var  p3 = p2;
        var r = new Dictionary<bool, bool[]>();
        foreach(KeyValuePair<bool, bool[]> e in p1)
        {
            r[e.Key] = e.Value;
        }
        foreach(KeyValuePair<bool, bool[]> e in p2)
        {
            r[e.Key] = e.Value;
        }
        response(new MyClass_OpBoolBoolSDResult(r, p3));
    }

    public override void
    opShortShortSDAsync(Dictionary<short, short[]> p1,
                        Dictionary<short, short[]> p2,
                        Action<MyClass_OpShortShortSDResult> response,
                        Action<Exception> exception,
                        Ice.Current current)
    {
        var p3 = p2;
        var r = new Dictionary<short, short[]>();
        foreach(var e in p1)
        {
            r[e.Key] = e.Value;
        }
        foreach(var e in p2)
        {
            r[e.Key] = e.Value;
        }
        response(new MyClass_OpShortShortSDResult(r, p3));
    }

    public override void
    opIntIntSDAsync(Dictionary<int, int[]> p1, 
                    Dictionary<int, int[]> p2,
                    Action<MyClass_OpIntIntSDResult> response,
                    Action<Exception> exception,
                    Ice.Current current)
    {
        var p3 = p2;
        var r = new Dictionary<int, int[]>();
        foreach(var e in p1)
        {
            r[e.Key] = e.Value;
        }
        foreach(var e in p2)
        {
            r[e.Key] = e.Value;
        }
        response(new MyClass_OpIntIntSDResult(r, p3));
    }

    public override void
    opLongLongSDAsync(Dictionary<long, long[]> p1,
                      Dictionary<long, long[]> p2,
                      Action<MyClass_OpLongLongSDResult> response,
                      Action<Exception> exception,
                      Ice.Current current)
    {
        var p3 = p2;
        var r = new Dictionary<long, long[]>();
        foreach(var e in p1)
        {
            r[e.Key] = e.Value;
        }
        foreach(var e in p2)
        {
            r[e.Key] = e.Value;
        }
        response(new MyClass_OpLongLongSDResult(r, p3));
    }

    public override void
    opStringFloatSDAsync(Dictionary<string, float[]> p1,
                         Dictionary<string, float[]> p2,
                         Action<MyClass_OpStringFloatSDResult> response,
                         Action<Exception> exception,
                         Ice.Current current)
    {
        var p3 = p2;
        var r = new Dictionary<string, float[]>();
        foreach(var e in p1)
        {
            r[e.Key] = e.Value;
        }
        foreach(var e in p2)
        {
            r[e.Key] = e.Value;
        }
        response(new MyClass_OpStringFloatSDResult(r, p3));
    }

    public override void
    opStringDoubleSDAsync(Dictionary<string, double[]> p1,
                          Dictionary<string, double[]> p2,
                          Action<MyClass_OpStringDoubleSDResult> response,
                          Action<Exception> exception,
                          Ice.Current current)
    {
        var p3 = p2;
        var r = new Dictionary<string, double[]>();
        foreach(var e in p1)
        {
            r[e.Key] = e.Value;
        }
        foreach(var e in p2)
        {
            r[e.Key] = e.Value;
        }
        response(new MyClass_OpStringDoubleSDResult(r, p3));
    }

    public override void
    opStringStringSDAsync(Dictionary<string, string[]> p1,
                          Dictionary<string, string[]> p2,
                          Action<MyClass_OpStringStringSDResult> response,
                          Action<Exception> exception,
                          Ice.Current current)
    {
        var p3 = p2;
        var r = new Dictionary<string, string[]>();
        foreach(var e in p1)
        {
            r[e.Key] = e.Value;
        }
        foreach(var e in p2)
        {
            r[e.Key] = e.Value;
        }
        response(new MyClass_OpStringStringSDResult(r, p3));
    }

    public override void
    opMyEnumMyEnumSDAsync(Dictionary<MyEnum, MyEnum[]> p1, 
                          Dictionary<MyEnum, MyEnum[]> p2,
                          Action<MyClass_OpMyEnumMyEnumSDResult> response,
                          Action<Exception> exception,
                          Ice.Current current)
    {
        var p3 = p2;
        var r = new Dictionary<Test.MyEnum, Test.MyEnum[]>();
        foreach(var e in p1)
        {
            r[e.Key] = e.Value;
        }
        foreach(var e in p2)
        {
            r[e.Key] = e.Value;
        }
        response(new MyClass_OpMyEnumMyEnumSDResult(r, p3));
    }

    public override void opIntSAsync(int[] s,
                                     Action<int[]> response,
                                     Action<Exception> exception,
                                     Ice.Current current)
    {
        var r = new int[s.Length];
        for(int i = 0; i < s.Length; ++i)
        {
            r[i] = -s[i];
        }
        response(r);
    }

    public override void opContextAsync(Action<Dictionary<string, string>> response,
                                        Action<Exception> exception, 
                                        Ice.Current current)
    {
        response(current.ctx);
    }

    public override void opByteSOnewayAsync(byte[] s, Action response,
                                            Action<Exception> exception,
                                            Ice.Current current)
    {
        lock(this)
        {
            ++_opByteSOnewayCallCount;
        }
        response();
    }

    public override void opByteSOnewayCallCountAsync(Action<int> response,
                                                     Action<Exception> exeption,
                                                     Ice.Current current)
    {
        lock(this)
        {
            var count = _opByteSOnewayCallCount;
            _opByteSOnewayCallCount = 0;
            response(count);
        }
    }

    public override void opDoubleMarshalingAsync(double p1, double[] p2, Action response, 
                                                 Action<Exception> exception, Ice.Current current)
    {
        var d = 1278312346.0 / 13.0;
        test(p1 == d);
        for(int i = 0; i < p2.Length; ++i)
        {
            test(p2[i] == d);
        }
        response();
    }

    public override void
    opStringSAsync(string[] p1, string[] p2,
                   Action<MyClass_OpStringSResult> response,
                   Action<Exception> exception,
                   Ice.Current current)
    {
        var p3 = new string[p1.Length + p2.Length];
        Array.Copy(p1, p3, p1.Length);
        Array.Copy(p2, 0, p3, p1.Length, p2.Length);

        var r = new string[p1.Length];
        for(int i = 0; i < p1.Length; i++)
        {
            r[i] = p1[p1.Length - (i + 1)];
        }
        response(new MyClass_OpStringSResult(r, p3));
    }

    public override void
    opStringSSAsync(string[][] p1, string[][] p2, Action<MyClass_OpStringSSResult> response,
                    Action<Exception> exception, Ice.Current current)
    {
        var p3 = new string[p1.Length + p2.Length][];
        Array.Copy(p1, p3, p1.Length);
        Array.Copy(p2, 0, p3, p1.Length, p2.Length);
        var r = new string[p2.Length][];
        for(int i = 0; i < p2.Length; i++)
        {
            r[i] = p2[p2.Length - (i + 1)];
        }
        response(new MyClass_OpStringSSResult(r, p3));
    }

    public override void
    opStringSSSAsync(string[][][] p1, string[][][] p2, Action<MyClass_OpStringSSSResult> response,
                     Action<Exception> exception, Ice.Current current)
    {
        var p3 = new string[p1.Length + p2.Length][][];
        Array.Copy(p1, p3, p1.Length);
        Array.Copy(p2, 0, p3, p1.Length, p2.Length);

        var r = new string[p2.Length][][];
        for(int i = 0; i < p2.Length; i++)
        {
            r[i] = p2[p2.Length - (i + 1)];
        }
        response(new MyClass_OpStringSSSResult(r, p3));
    }

    public override void
    opStringStringDAsync(Dictionary<string, string> p1, Dictionary<string, string> p2,
                         Action<MyClass_OpStringStringDResult> response, Action<Exception> exception,
                         Ice.Current current)
    {
        var p3 = p1;
        var r = new Dictionary<string, string>();
        foreach(var e in p1)
        {
            r[e.Key] = e.Value;
        }
        foreach(var e in p2)
        {
            r[e.Key] = e.Value;
        }
        response(new MyClass_OpStringStringDResult(r, p3));
    }

    public override void
    opStructAsync(Structure p1, Structure p2, Action<MyClass_OpStructResult> response,
                  Action<Exception> exception, Ice.Current current)
    {
        var p3 = p1;
        p3.s.s = "a new string";
        response(new MyClass_OpStructResult(p2, p3));
    }

    public override void opIdempotentAsync(Action response, Action<Exception> exception, Ice.Current current)
    {
        test(current.mode == Ice.OperationMode.Idempotent);
        response();
    }

    public override void opNonmutatingAsync(Action response, Action<Exception> exception, Ice.Current current)
    {
        test(current.mode == Ice.OperationMode.Nonmutating);
        response();
    }

    public override void opDerivedAsync(Action response, Action<Exception> exception, Ice.Current current)
    {
        response();
    }
    
    public override void opByte1Async(byte value, Action<byte> response, Action<Exception> exception, 
                                      Ice.Current current)
    {
        response(value);
    }
    
    public override void opShort1Async(short value, Action<short> response, Action<Exception> exception,
                                       Ice.Current current)
    {
        response(value);
    }
    
    public override void opInt1Async(int value, Action<int> response, Action<Exception> exception,
                                     Ice.Current current)
    {
        response(value);
    }
    
    public override void opLong1Async(long value, Action<long> response, Action<Exception> exception,
                                      Ice.Current current)
    {
        response(value);
    }
    
    public override void opFloat1Async(float value, Action<float> response, Action<Exception> exception,
                                       Ice.Current current)
    {
        response(value);
    }
    
    public override void opDouble1Async(double value, Action<double> response, Action<Exception> exception,
                                        Ice.Current current)
    {
        response(value);
    }
    
    public override void opString1Async(string value, Action<string> response, Action<Exception> exception,
                                        Ice.Current current)
    {
        response(value);
    }
    
    public override void opStringS1Async(string[] value, Action<string[]> response, Action<Exception> exception,
                                                   Ice.Current current)
    {
        response(value);
    }
    
    public override void
    opByteBoolD1Async(Dictionary<byte, bool> value, Action<Dictionary<byte, bool>> response, Action<Exception> exception,
                      Ice.Current current)
    {
        response(value);
    }
    
    public override void
    opStringS2Async(string[] value, Action<string[]> response, Action<Exception> exception, Ice.Current current)
    {
        response(value);
    }
    
    public override void
    opByteBoolD2Async(Dictionary<byte, bool> value, Action<Dictionary<byte, bool>> response, Action<Exception> exception,
                      Ice.Current current)
    {
        response(value);
    }
    
    public override void
    opMyClass1Async(MyClass1 value, Action<MyClass1> response, Action<Exception> exception, Ice.Current current)
    {
        response(value);
    }
    
    public override void
    opMyStruct1Async(MyStruct1 value, Action<MyStruct1> response, Action<Exception> exception, Ice.Current current)
    {
        response(value);
    }
    
    
    public override void
    opStringLiteralsAsync(Action<string[]> response, Action<Exception> exception, Ice.Current current)
    {
        response(new string[]
            {
                s0.value,
                s1.value,
                s2.value,
                s3.value,
                s4.value,
                s5.value,
                s6.value,
                s7.value,
                s8.value,
                s9.value,
                s10.value,
                
                sw0.value,
                sw1.value,
                sw2.value,
                sw3.value,
                sw4.value,
                sw5.value,
                sw6.value,
                sw7.value,
                sw8.value,
                sw9.value,
                sw10.value,
                
                ss0.value,
                ss1.value,
                ss2.value,
                ss3.value,
                ss4.value,
                ss5.value,
                
                su0.value,
                su1.value,
                su2.value
            });
    }
    
    public override void
    opWStringLiteralsAsync(Action<string[]> response, Action<Exception> exception, Ice.Current current)
    {
        response(new string[]
            {
                s0.value,
                s1.value,
                s2.value,
                s3.value,
                s4.value,
                s5.value,
                s6.value,
                s7.value,
                s8.value,
                s9.value,
                s10.value,
                
                sw0.value,
                sw1.value,
                sw2.value,
                sw3.value,
                sw4.value,
                sw5.value,
                sw6.value,
                sw7.value,
                sw8.value,
                sw9.value,
                sw10.value,
                
                ss0.value,
                ss1.value,
                ss2.value,
                ss3.value,
                ss4.value,
                ss5.value,
                
                su0.value,
                su1.value,
                su2.value
            });
    }

    private Thread_opVoid _opVoidThread;
    private int _opByteSOnewayCallCount = 0;
}
