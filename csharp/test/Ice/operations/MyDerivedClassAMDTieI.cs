// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
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

public sealed class MyDerivedClassI : MyDerivedClassTie_
{
    public MyDerivedClassI() : base(new MyDerivedClassTieI())
    {
    }
}

public sealed class MyDerivedClassTieI : MyDerivedClassOperations_
{
    private static void test(bool b)
    {
        if (!b)
        {
            throw new Exception();
        }
    }

    internal class Thread_opVoid : TaskCompletionSource<object>
    {
        public Thread_opVoid()
        {
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
            SetResult(null);
        }

        public void Join()
        {
            lock(this)
            {
                _thread.Join();
            }
        }

        private Thread _thread;
    }

    public Task shutdownAsync(Ice.Current current)
    {
        while(_opVoidThread != null)
        {
            _opVoidThread.Join();
            _opVoidThread = null;
        }

        current.adapter.getCommunicator().shutdown();
        return null;
    }

    public Task opVoidAsync(Ice.Current current)
    {
        test(current.mode == Ice.OperationMode.Normal);

        while(_opVoidThread != null)
        {
            _opVoidThread.Join();
            _opVoidThread = null;
        }

        _opVoidThread = new Thread_opVoid();
        _opVoidThread.Start();
        return _opVoidThread.Task;
    }

    public Task<MyClass_OpBoolResult> opBoolAsync(bool p1, bool p2, Ice.Current current)
    {
        return Task.FromResult<MyClass_OpBoolResult>(new MyClass_OpBoolResult(p2, p1));
    }

    public Task<MyClass_OpBoolSResult> opBoolSAsync(bool[] p1, bool[] p2, Ice.Current current)
    {
        bool[] p3 = new bool[p1.Length + p2.Length];
        Array.Copy(p1, p3, p1.Length);
        Array.Copy(p2, 0, p3, p1.Length, p2.Length);

        bool[] r = new bool[p1.Length];
        for(int i = 0; i < p1.Length; i++)
        {
            r[i] = p1[p1.Length - (i + 1)];
        }

        return Task.FromResult<MyClass_OpBoolSResult>(new MyClass_OpBoolSResult(r, p3));
    }

    public Task<MyClass_OpBoolSSResult>
    opBoolSSAsync(bool[][] p1, bool[][] p2, Ice.Current current)
    {
        bool[][] p3 = new bool[p1.Length + p2.Length][];
        Array.Copy(p1, p3, p1.Length);
        Array.Copy(p2, 0, p3, p1.Length, p2.Length);

        bool[][] r = new bool[p1.Length][];
        for(int i = 0; i < p1.Length; i++)
        {
            r[i] = p1[p1.Length - (i + 1)];
        }

        return Task.FromResult<MyClass_OpBoolSSResult>(new MyClass_OpBoolSSResult(r, p3));
    }

    public Task<MyClass_OpByteResult>
    opByteAsync(byte p1, byte p2, Ice.Current current)
    {
        return Task.FromResult<MyClass_OpByteResult>(new MyClass_OpByteResult(p1, (byte)(p1 ^ p2)));
    }

    public Task<MyClass_OpByteBoolDResult>
    opByteBoolDAsync(Dictionary<byte, bool> p1, Dictionary<byte, bool> p2, Ice.Current current)
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

        return Task.FromResult<MyClass_OpByteBoolDResult>(new MyClass_OpByteBoolDResult(r, p3));
    }

    public Task<MyClass_OpByteSResult>
    opByteSAsync(byte[] p1, byte[] p2, Ice.Current current)
    {
        byte[] p3 = new byte[p1.Length];
        for(int i = 0; i < p1.Length; i++)
        {
            p3[i] = p1[p1.Length - (i + 1)];
        }

        byte[] r = new byte[p1.Length + p2.Length];
        Array.Copy(p1, r, p1.Length);
        Array.Copy(p2, 0, r, p1.Length, p2.Length);

        return Task.FromResult<MyClass_OpByteSResult>(new MyClass_OpByteSResult(r, p3));
    }

    public Task<MyClass_OpByteSSResult>
    opByteSSAsync(byte[][] p1, byte[][] p2, Ice.Current current)
    {
        byte[][] p3 = new byte[p1.Length][];
        for(int i = 0; i < p1.Length; i++)
        {
            p3[i] = p1[p1.Length - (i + 1)];
        }

        byte[][] r = new byte[p1.Length + p2.Length][];
        Array.Copy(p1, r, p1.Length);
        Array.Copy(p2, 0, r, p1.Length, p2.Length);

        return Task.FromResult<MyClass_OpByteSSResult>(new MyClass_OpByteSSResult(r, p3));
    }

    public Task<MyClass_OpFloatDoubleResult>
    opFloatDoubleAsync(float p1, double p2, Ice.Current current)
    {
        return Task.FromResult<MyClass_OpFloatDoubleResult>(new MyClass_OpFloatDoubleResult(p2, p1, p2));
    }

    public Task<MyClass_OpFloatDoubleSResult>
    opFloatDoubleSAsync(float[] p1, double[] p2, Ice.Current current)
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

        return Task.FromResult<MyClass_OpFloatDoubleSResult>(new MyClass_OpFloatDoubleSResult(r, p3, p4));
    }

    public Task<MyClass_OpFloatDoubleSSResult>
    opFloatDoubleSSAsync(float[][] p1, double[][] p2, Ice.Current current)
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

        return Task.FromResult<MyClass_OpFloatDoubleSSResult>(new MyClass_OpFloatDoubleSSResult(r, p3, p4));
    }

    public Task<MyClass_OpLongFloatDResult>
    opLongFloatDAsync(Dictionary<long, float> p1, Dictionary<long, float> p2, Ice.Current current)
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

        return Task.FromResult<MyClass_OpLongFloatDResult>(new MyClass_OpLongFloatDResult(r, p3));
    }

    public Task<MyClass_OpMyClassResult>
    opMyClassAsync(MyClassPrx p1, Ice.Current current)
    {
        var p2 = p1;
        var p3 = MyClassPrxHelper.uncheckedCast(current.adapter.createProxy(
                                                Ice.Util.stringToIdentity("noSuchIdentity")));
        return Task.FromResult<MyClass_OpMyClassResult>(new MyClass_OpMyClassResult(
            MyClassPrxHelper.uncheckedCast(current.adapter.createProxy(current.id)), p2, p3));
    }

    public Task<MyClass_OpMyEnumResult>
    opMyEnumAsync(MyEnum p1, Ice.Current current)
    {
        return Task.FromResult<MyClass_OpMyEnumResult>(new MyClass_OpMyEnumResult(MyEnum.enum3, p1));
    }

    public Task<MyClass_OpShortIntDResult>
    opShortIntDAsync(Dictionary<short, int> p1, Dictionary<short, int> p2, Ice.Current current)
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
        return Task.FromResult<MyClass_OpShortIntDResult>(new MyClass_OpShortIntDResult(r, p3));
    }

    public Task<MyClass_OpShortIntLongResult>
    opShortIntLongAsync(short p1, int p2, long p3, Ice.Current current)
    {
        return Task.FromResult<MyClass_OpShortIntLongResult>(new MyClass_OpShortIntLongResult(p3, p1, p2, p3));
    }

    public Task<MyClass_OpShortIntLongSResult>
    opShortIntLongSAsync(short[] p1, int[] p2, long[] p3, Ice.Current current)
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
        return Task.FromResult<MyClass_OpShortIntLongSResult>(new MyClass_OpShortIntLongSResult(p3, p4, p5, p6));
    }

    public Task<MyClass_OpShortIntLongSSResult>
    opShortIntLongSSAsync(short[][] p1, int[][] p2, long[][] p3, Ice.Current current)
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
        return Task.FromResult<MyClass_OpShortIntLongSSResult>(new MyClass_OpShortIntLongSSResult(p3, p4, p5, p6));
    }

    public Task<MyClass_OpStringResult>
    opStringAsync(string p1, string p2, Ice.Current current)
    {
        return Task.FromResult<MyClass_OpStringResult>(new MyClass_OpStringResult(p1 + " " + p2, p2 + " " + p1));
    }

    public Task<MyClass_OpStringMyEnumDResult>
    opStringMyEnumDAsync(Dictionary<string, MyEnum> p1, Dictionary<string, MyEnum> p2, Ice.Current current)
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
        return Task.FromResult<MyClass_OpStringMyEnumDResult>(new MyClass_OpStringMyEnumDResult(r, p3));
    }

    public Task<MyClass_OpMyEnumStringDResult>
    opMyEnumStringDAsync(Dictionary<MyEnum, string> p1, Dictionary<MyEnum, string> p2, Ice.Current current)
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
        return Task.FromResult<MyClass_OpMyEnumStringDResult>(new MyClass_OpMyEnumStringDResult(r, p3));
    }

    public Task<MyClass_OpMyStructMyEnumDResult>
    opMyStructMyEnumDAsync(Dictionary<MyStruct, MyEnum> p1,
                           Dictionary<MyStruct, MyEnum> p2, Ice.Current current)
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
        return Task.FromResult<MyClass_OpMyStructMyEnumDResult>(new MyClass_OpMyStructMyEnumDResult(r, p3));
    }

    public Task<MyClass_OpByteBoolDSResult>
    opByteBoolDSAsync(Dictionary<byte, bool>[] p1, Dictionary<byte, bool>[] p2, Ice.Current current)
    {
        var p3 = new Dictionary<byte, bool>[p1.Length + p2.Length];
        Array.Copy(p2, p3, p2.Length);
        Array.Copy(p1, 0, p3, p2.Length, p1.Length);

        var r = new Dictionary<byte, bool>[p1.Length];
        for(int i = 0; i < p1.Length; i++)
        {
            r[i] = p1[p1.Length - (i + 1)];
        }
        return Task.FromResult<MyClass_OpByteBoolDSResult>(new MyClass_OpByteBoolDSResult(r, p3));
    }

    public Task<MyClass_OpShortIntDSResult>
    opShortIntDSAsync(Dictionary<short, int>[] p1, Dictionary<short, int>[] p2, Ice.Current current)
    {
        var p3 = new Dictionary<short, int>[p1.Length + p2.Length];
        Array.Copy(p2, p3, p2.Length);
        Array.Copy(p1, 0, p3, p2.Length, p1.Length);

        var r = new Dictionary<short, int>[p1.Length];
        for(int i = 0; i < p1.Length; i++)
        {
            r[i] = p1[p1.Length - (i + 1)];
        }
        return Task.FromResult<MyClass_OpShortIntDSResult>(new MyClass_OpShortIntDSResult(r, p3));
    }

    public Task<MyClass_OpLongFloatDSResult>
    opLongFloatDSAsync(Dictionary<long, float>[] p1, Dictionary<long, float>[] p2, Ice.Current current)
    {
        var p3 = new Dictionary<long, float>[p1.Length + p2.Length];
        Array.Copy(p2, p3, p2.Length);
        Array.Copy(p1, 0, p3, p2.Length, p1.Length);

        var r = new Dictionary<long, float>[p1.Length];
        for(int i = 0; i < p1.Length; i++)
        {
            r[i] = p1[p1.Length - (i + 1)];
        }
        return Task.FromResult<MyClass_OpLongFloatDSResult>(new MyClass_OpLongFloatDSResult(r, p3));
    }

    public Task<MyClass_OpStringStringDSResult>
    opStringStringDSAsync(Dictionary<string, string>[] p1, Dictionary<string, string>[] p2, Ice.Current current)
    {
        var p3 = new Dictionary<string, string>[p1.Length + p2.Length];
        Array.Copy(p2, p3, p2.Length);
        Array.Copy(p1, 0, p3, p2.Length, p1.Length);

        var r = new Dictionary<string, string>[p1.Length];
        for(int i = 0; i < p1.Length; i++)
        {
            r[i] = p1[p1.Length - (i + 1)];
        }
        return Task.FromResult<MyClass_OpStringStringDSResult>(new MyClass_OpStringStringDSResult(r, p3));
    }

    public Task<MyClass_OpStringMyEnumDSResult>
    opStringMyEnumDSAsync(Dictionary<string, MyEnum>[] p1, Dictionary<string, MyEnum>[] p2, Ice.Current current)
    {
        var p3 = new Dictionary<string, MyEnum>[p1.Length + p2.Length];
        Array.Copy(p2, p3, p2.Length);
        Array.Copy(p1, 0, p3, p2.Length, p1.Length);

        var r = new Dictionary<string, MyEnum>[p1.Length];
        for(int i = 0; i < p1.Length; i++)
        {
            r[i] = p1[p1.Length - (i + 1)];
        }
        return Task.FromResult<MyClass_OpStringMyEnumDSResult>(new MyClass_OpStringMyEnumDSResult(r, p3));
    }

    public Task<MyClass_OpMyEnumStringDSResult>
    opMyEnumStringDSAsync(Dictionary<MyEnum, string>[] p1, Dictionary<MyEnum, string>[] p2, Ice.Current current)
    {
        var p3 = new Dictionary<MyEnum, string>[p1.Length + p2.Length];
        Array.Copy(p2, p3, p2.Length);
        Array.Copy(p1, 0, p3, p2.Length, p1.Length);

        var r = new Dictionary<MyEnum, string>[p1.Length];
        for(int i = 0; i < p1.Length; i++)
        {
            r[i] = p1[p1.Length - (i + 1)];
        }
        return Task.FromResult<MyClass_OpMyEnumStringDSResult>(new MyClass_OpMyEnumStringDSResult(r, p3));
    }

    public Task<MyClass_OpMyStructMyEnumDSResult>
    opMyStructMyEnumDSAsync(Dictionary<MyStruct, MyEnum>[] p1, Dictionary<MyStruct, MyEnum>[] p2, Ice.Current current)
    {
        var p3 = new Dictionary<MyStruct, MyEnum>[p1.Length + p2.Length];
        Array.Copy(p2, p3, p2.Length);
        Array.Copy(p1, 0, p3, p2.Length, p1.Length);

        var r = new Dictionary<MyStruct, MyEnum>[p1.Length];
        for(int i = 0; i < p1.Length; i++)
        {
            r[i] = p1[p1.Length - (i + 1)];
        }
        return Task.FromResult<MyClass_OpMyStructMyEnumDSResult>(new MyClass_OpMyStructMyEnumDSResult(r, p3));
    }

    public Task<MyClass_OpByteByteSDResult>
    opByteByteSDAsync(Dictionary<byte, byte[]> p1, Dictionary<byte, byte[]> p2, Ice.Current current)
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
        return Task.FromResult<MyClass_OpByteByteSDResult>(new MyClass_OpByteByteSDResult(r, p3));
    }

    public Task<MyClass_OpBoolBoolSDResult>
    opBoolBoolSDAsync(Dictionary<bool, bool[]> p1, Dictionary<bool, bool[]> p2, Ice.Current current)
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
        return Task.FromResult<MyClass_OpBoolBoolSDResult>(new MyClass_OpBoolBoolSDResult(r, p3));
    }

    public Task<MyClass_OpShortShortSDResult>
    opShortShortSDAsync(Dictionary<short, short[]> p1, Dictionary<short, short[]> p2, Ice.Current current)
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
        return Task.FromResult<MyClass_OpShortShortSDResult>(new MyClass_OpShortShortSDResult(r, p3));
    }

    public Task<MyClass_OpIntIntSDResult>
    opIntIntSDAsync(Dictionary<int, int[]> p1, Dictionary<int, int[]> p2, Ice.Current current)
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
        return Task.FromResult<MyClass_OpIntIntSDResult>(new MyClass_OpIntIntSDResult(r, p3));
    }

    public Task<MyClass_OpLongLongSDResult>
    opLongLongSDAsync(Dictionary<long, long[]> p1, Dictionary<long, long[]> p2, Ice.Current current)
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
        return Task.FromResult<MyClass_OpLongLongSDResult>(new MyClass_OpLongLongSDResult(r, p3));
    }

    public Task<MyClass_OpStringFloatSDResult>
    opStringFloatSDAsync(Dictionary<string, float[]> p1, Dictionary<string, float[]> p2, Ice.Current current)
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
        return Task.FromResult<MyClass_OpStringFloatSDResult>(new MyClass_OpStringFloatSDResult(r, p3));
    }

    public Task<MyClass_OpStringDoubleSDResult>
    opStringDoubleSDAsync(Dictionary<string, double[]> p1, Dictionary<string, double[]> p2, Ice.Current current)
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
        return Task.FromResult<MyClass_OpStringDoubleSDResult>(new MyClass_OpStringDoubleSDResult(r, p3));
    }

    public Task<MyClass_OpStringStringSDResult>
    opStringStringSDAsync(Dictionary<string, string[]> p1, Dictionary<string, string[]> p2, Ice.Current current)
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
        return Task.FromResult<MyClass_OpStringStringSDResult>(new MyClass_OpStringStringSDResult(r, p3));
    }

    public Task<MyClass_OpMyEnumMyEnumSDResult>
    opMyEnumMyEnumSDAsync(Dictionary<MyEnum, MyEnum[]> p1, Dictionary<MyEnum, MyEnum[]> p2, Ice.Current current)
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
        return Task.FromResult<MyClass_OpMyEnumMyEnumSDResult>(new MyClass_OpMyEnumMyEnumSDResult(r, p3));
    }

    public Task<int[]>
    opIntSAsync(int[] s, Ice.Current current)
    {
        var r = new int[s.Length];
        for(int i = 0; i < s.Length; ++i)
        {
            r[i] = -s[i];
        }
        return Task.FromResult<int[]>(r);
    }

    public Task<Dictionary<string, string>>
    opContextAsync(Ice.Current current)
    {
        return Task.FromResult<Dictionary<string, string>>(current.ctx);
    }

    public Task
    opByteSOnewayAsync(byte[] s, Ice.Current current)
    {
        lock(this)
        {
            ++_opByteSOnewayCallCount;
        }
        return null;
    }

    public Task<int>
    opByteSOnewayCallCountAsync(Ice.Current current)
    {
        lock(this)
        {
            var count = _opByteSOnewayCallCount;
            _opByteSOnewayCallCount = 0;
            return Task.FromResult<int>(count);
        }
    }

    public Task
    opDoubleMarshalingAsync(double p1, double[] p2, Ice.Current current)
    {
        var d = 1278312346.0 / 13.0;
        test(p1 == d);
        for(int i = 0; i < p2.Length; ++i)
        {
            test(p2[i] == d);
        }
        return null;
    }

    public Task<MyClass_OpStringSResult>
    opStringSAsync(string[] p1, string[] p2, Ice.Current current)
    {
        var p3 = new string[p1.Length + p2.Length];
        Array.Copy(p1, p3, p1.Length);
        Array.Copy(p2, 0, p3, p1.Length, p2.Length);

        var r = new string[p1.Length];
        for(int i = 0; i < p1.Length; i++)
        {
            r[i] = p1[p1.Length - (i + 1)];
        }
        return Task.FromResult<MyClass_OpStringSResult>(new MyClass_OpStringSResult(r, p3));
    }

    public Task<MyClass_OpStringSSResult>
    opStringSSAsync(string[][] p1, string[][] p2, Ice.Current current)
    {
        var p3 = new string[p1.Length + p2.Length][];
        Array.Copy(p1, p3, p1.Length);
        Array.Copy(p2, 0, p3, p1.Length, p2.Length);
        var r = new string[p2.Length][];
        for(int i = 0; i < p2.Length; i++)
        {
            r[i] = p2[p2.Length - (i + 1)];
        }
        return Task.FromResult<MyClass_OpStringSSResult>(new MyClass_OpStringSSResult(r, p3));
    }

    public Task<MyClass_OpStringSSSResult>
    opStringSSSAsync(string[][][] p1, string[][][] p2, Ice.Current current)
    {
        var p3 = new string[p1.Length + p2.Length][][];
        Array.Copy(p1, p3, p1.Length);
        Array.Copy(p2, 0, p3, p1.Length, p2.Length);

        var r = new string[p2.Length][][];
        for(int i = 0; i < p2.Length; i++)
        {
            r[i] = p2[p2.Length - (i + 1)];
        }
        return Task.FromResult<MyClass_OpStringSSSResult>(new MyClass_OpStringSSSResult(r, p3));
    }

    public Task<MyClass_OpStringStringDResult>
    opStringStringDAsync(Dictionary<string, string> p1, Dictionary<string, string> p2, Ice.Current current)
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
        return Task.FromResult<MyClass_OpStringStringDResult>(new MyClass_OpStringStringDResult(r, p3));
    }

    public Task<MyClass_OpStructResult>
    opStructAsync(Structure p1, Structure p2, Ice.Current current)
    {
        var p3 = p1;
        p3.s.s = "a new string";
        return Task.FromResult<MyClass_OpStructResult>(new MyClass_OpStructResult(p2, p3));
    }

    public Task
    opIdempotentAsync(Ice.Current current)
    {
        test(current.mode == Ice.OperationMode.Idempotent);
        return null;
    }

    public Task
    opNonmutatingAsync(Ice.Current current)
    {
        test(current.mode == Ice.OperationMode.Nonmutating);
        return null;
    }

    public Task
    opDerivedAsync(Ice.Current current)
    {
        return null;
    }

    public Task<byte>
    opByte1Async(byte value, Ice.Current current)
    {
        return Task.FromResult<byte>(value);
    }

    public Task<short>
    opShort1Async(short value, Ice.Current current)
    {
        return Task.FromResult<short>(value);
    }

    public Task<int>
    opInt1Async(int value, Ice.Current current)
    {
        return Task.FromResult<int>(value);
    }

    public Task<long>
    opLong1Async(long value, Ice.Current current)
    {
        return Task.FromResult<long>(value);
    }

    public Task<float>
    opFloat1Async(float value, Ice.Current current)
    {
        return Task.FromResult<float>(value);
    }

    public Task<double>
    opDouble1Async(double value, Ice.Current current)
    {
        return Task.FromResult<double>(value);
    }

    public Task<string>
    opString1Async(string value, Ice.Current current)
    {
        return Task.FromResult<string>(value);
    }

    public Task<string[]>
    opStringS1Async(string[] value, Ice.Current current)
    {
        return Task.FromResult<string[]>(value);
    }

    public Task<Dictionary<byte, bool>>
    opByteBoolD1Async(Dictionary<byte, bool> value, Ice.Current current)
    {
        return Task.FromResult<Dictionary<byte, bool>>(value);
    }

    public Task<string[]>
    opStringS2Async(string[] value, Ice.Current current)
    {
        return Task.FromResult<string[]>(value);
    }

    public Task<Dictionary<byte, bool>>
    opByteBoolD2Async(Dictionary<byte, bool> value, Ice.Current current)
    {
        return Task.FromResult<Dictionary<byte, bool>>(value);
    }

    public Task<MyClass1>
    opMyClass1Async(MyClass1 value, Ice.Current current)
    {
        return Task.FromResult<MyClass1>(value);
    }

    public Task<MyStruct1>
    opMyStruct1Async(MyStruct1 value, Ice.Current current)
    {
        return Task.FromResult<MyStruct1>(value);
    }

    public Task<string[]>
    opStringLiteralsAsync(Ice.Current current)
    {
        return Task.FromResult<string[]>(new string[]
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

    public Task<string[]>
    opWStringLiteralsAsync(Ice.Current current)
    {
        return Task.FromResult<string[]>(new string[]
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

    public async Task<Test.MyClass_OpMStruct1MarshaledResult>
    opMStruct1Async(Ice.Current current)
    {
        await Task.Delay(0);
        return new Test.MyClass_OpMStruct1MarshaledResult(new Test.Structure(), current);
    }

    public async Task<Test.MyClass_OpMStruct2MarshaledResult>
    opMStruct2Async(Test.Structure p1, Ice.Current current)
    {
        await Task.Delay(0);
        return new Test.MyClass_OpMStruct2MarshaledResult(p1, p1, current);
    }

    public async Task<Test.MyClass_OpMSeq1MarshaledResult>
    opMSeq1Async(Ice.Current current)
    {
        await Task.Delay(0);
        return new Test.MyClass_OpMSeq1MarshaledResult(new string[0], current);
    }

    public async Task<Test.MyClass_OpMSeq2MarshaledResult>
    opMSeq2Async(string[] p1, Ice.Current current)
    {
        await Task.Delay(0);
        return new Test.MyClass_OpMSeq2MarshaledResult(p1, p1, current);
    }

    public async Task<Test.MyClass_OpMDict1MarshaledResult>
    opMDict1Async(Ice.Current current)
    {
        await Task.Delay(0);
        return new Test.MyClass_OpMDict1MarshaledResult(new Dictionary<string, string>(), current);
    }

    public async Task<Test.MyClass_OpMDict2MarshaledResult>
    opMDict2Async(Dictionary<string, string> p1, Ice.Current current)
    {
        await Task.Delay(0);
        return new Test.MyClass_OpMDict2MarshaledResult(p1, p1, current);
    }

    private Thread_opVoid _opVoidThread;
    private int _opByteSOnewayCallCount = 0;
}
