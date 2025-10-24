// Copyright (c) ZeroC, Inc.

namespace Ice.operations.AMD;

public sealed class MyDerivedClassI : Test.AsyncMyDerivedClassDisp_
{
    private static void test(bool b) => global::Test.TestHelper.test(b);

    internal class Thread_opVoid : TaskCompletionSource<object>
    {
        public Thread_opVoid()
        {
        }

        public void Start()
        {
            lock (_mutex)
            {
                _thread = new Thread(new ThreadStart(Run));
                _thread.Start();
            }
        }

        public void Run() => SetResult(null);

        public void Join()
        {
            lock (_mutex)
            {
                _thread.Join();
            }
        }

        private Thread _thread;
        private readonly object _mutex = new();
    }

    public override Task shutdownAsync(Current current)
    {
        while (_opVoidThread != null)
        {
            _opVoidThread.Join();
            _opVoidThread = null;
        }

        current.adapter.getCommunicator().shutdown();
        return Task.CompletedTask;
    }

    public override Task<bool> supportsCompressAsync(Current current) => Task.FromResult<bool>(true);

    public override Task opVoidAsync(Current current)
    {
        test(current.mode == Ice.OperationMode.Normal);

        while (_opVoidThread != null)
        {
            _opVoidThread.Join();
            _opVoidThread = null;
        }

        _opVoidThread = new Thread_opVoid();
        _opVoidThread.Start();
        return _opVoidThread.Task;
    }

    public override Task<Test.MyClass_OpBoolResult> opBoolAsync(bool p1, bool p2, Current current) =>
        Task.FromResult(new Test.MyClass_OpBoolResult(p2, p1));

    public override Task<Test.MyClass_OpBoolSResult> opBoolSAsync(bool[] p1, bool[] p2, Current current)
    {
        bool[] p3 = new bool[p1.Length + p2.Length];
        Array.Copy(p1, p3, p1.Length);
        Array.Copy(p2, 0, p3, p1.Length, p2.Length);

        bool[] r = new bool[p1.Length];
        for (int i = 0; i < p1.Length; i++)
        {
            r[i] = p1[^(i + 1)];
        }

        return Task.FromResult(new Test.MyClass_OpBoolSResult(r, p3));
    }

    public override Task<Test.MyClass_OpBoolSSResult>
    opBoolSSAsync(bool[][] p1, bool[][] p2, Current current)
    {
        bool[][] p3 = new bool[p1.Length + p2.Length][];
        Array.Copy(p1, p3, p1.Length);
        Array.Copy(p2, 0, p3, p1.Length, p2.Length);

        bool[][] r = new bool[p1.Length][];
        for (int i = 0; i < p1.Length; i++)
        {
            r[i] = p1[^(i + 1)];
        }

        return Task.FromResult(new Test.MyClass_OpBoolSSResult(r, p3));
    }

    public override Task<Test.MyClass_OpByteResult>
    opByteAsync(byte p1, byte p2, Current current) =>
        Task.FromResult(new Test.MyClass_OpByteResult(p1, (byte)(p1 ^ p2)));

    public override Task<Test.MyClass_OpByteBoolDResult>
    opByteBoolDAsync(Dictionary<byte, bool> p1, Dictionary<byte, bool> p2, Current current)
    {
        Dictionary<byte, bool> p3 = p1;
        var r = new Dictionary<byte, bool>();
        foreach (KeyValuePair<byte, bool> e in p1)
        {
            r[e.Key] = e.Value;
        }
        foreach (KeyValuePair<byte, bool> e in p2)
        {
            r[e.Key] = e.Value;
        }

        return Task.FromResult(new Test.MyClass_OpByteBoolDResult(r, p3));
    }

    public override Task<Test.MyClass_OpByteSResult>
    opByteSAsync(byte[] p1, byte[] p2, Current current)
    {
        byte[] p3 = new byte[p1.Length];
        for (int i = 0; i < p1.Length; i++)
        {
            p3[i] = p1[^(i + 1)];
        }

        byte[] r = new byte[p1.Length + p2.Length];
        Array.Copy(p1, r, p1.Length);
        Array.Copy(p2, 0, r, p1.Length, p2.Length);

        return Task.FromResult(new Test.MyClass_OpByteSResult(r, p3));
    }

    public override Task<Test.MyClass_OpByteSSResult>
    opByteSSAsync(byte[][] p1, byte[][] p2, Current current)
    {
        byte[][] p3 = new byte[p1.Length][];
        for (int i = 0; i < p1.Length; i++)
        {
            p3[i] = p1[^(i + 1)];
        }

        byte[][] r = new byte[p1.Length + p2.Length][];
        Array.Copy(p1, r, p1.Length);
        Array.Copy(p2, 0, r, p1.Length, p2.Length);

        return Task.FromResult(new Test.MyClass_OpByteSSResult(r, p3));
    }

    public override Task<Test.MyClass_OpFloatDoubleResult>
    opFloatDoubleAsync(float p1, double p2, Current current) =>
        Task.FromResult(new Test.MyClass_OpFloatDoubleResult(p2, p1, p2));

    public override Task<Test.MyClass_OpFloatDoubleSResult>
    opFloatDoubleSAsync(float[] p1, double[] p2, Current current)
    {
        float[] p3 = p1;

        double[] p4 = new double[p2.Length];
        for (int i = 0; i < p2.Length; i++)
        {
            p4[i] = p2[^(i + 1)];
        }

        double[] r = new double[p2.Length + p1.Length];
        Array.Copy(p2, r, p2.Length);
        for (int i = 0; i < p1.Length; i++)
        {
            r[p2.Length + i] = p1[i];
        }

        return Task.FromResult(new Test.MyClass_OpFloatDoubleSResult(r, p3, p4));
    }

    public override Task<Test.MyClass_OpFloatDoubleSSResult>
    opFloatDoubleSSAsync(float[][] p1, double[][] p2, Current current)
    {
        float[][] p3 = p1;

        double[][] p4 = new double[p2.Length][];
        for (int i = 0; i < p2.Length; i++)
        {
            p4[i] = p2[^(i + 1)];
        }

        double[][] r = new double[p2.Length + p2.Length][];
        Array.Copy(p2, r, p2.Length);
        for (int i = 0; i < p2.Length; i++)
        {
            r[p2.Length + i] = new double[p2[i].Length];
            for (int j = 0; j < p2[i].Length; j++)
            {
                r[p2.Length + i][j] = p2[i][j];
            }
        }

        return Task.FromResult(new Test.MyClass_OpFloatDoubleSSResult(r, p3, p4));
    }

    public override Task<Test.MyClass_OpLongFloatDResult>
    opLongFloatDAsync(Dictionary<long, float> p1, Dictionary<long, float> p2, Current current)
    {
        Dictionary<long, float> p3 = p1;
        var r = new Dictionary<long, float>();
        foreach (KeyValuePair<long, float> e in p1)
        {
            r[e.Key] = e.Value;
        }
        foreach (KeyValuePair<long, float> e in p2)
        {
            r[e.Key] = e.Value;
        }

        return Task.FromResult(new Test.MyClass_OpLongFloatDResult(r, p3));
    }

    public override Task<Test.MyClass_OpMyClassResult>
    opMyClassAsync(Test.MyClassPrx p1, Current current)
    {
        Test.MyClassPrx p2 = p1;
        Test.MyClassPrx p3 = Test.MyClassPrxHelper.uncheckedCast(current.adapter.createProxy(
                                                Ice.Util.stringToIdentity("noSuchIdentity")));
        return Task.FromResult(new Test.MyClass_OpMyClassResult(
            Test.MyClassPrxHelper.uncheckedCast(current.adapter.createProxy(current.id)), p2, p3));
    }

    public override Task<Test.MyClass_OpMyEnumResult>
    opMyEnumAsync(Test.MyEnum p1, Current current) =>
        Task.FromResult(new Test.MyClass_OpMyEnumResult(Test.MyEnum.enum3, p1));

    public override Task<Test.MyClass_OpShortIntDResult>
    opShortIntDAsync(Dictionary<short, int> p1, Dictionary<short, int> p2, Current current)
    {
        Dictionary<short, int> p3 = p1;
        var r = new Dictionary<short, int>();
        foreach (KeyValuePair<short, int> e in p1)
        {
            r[e.Key] = e.Value;
        }
        foreach (KeyValuePair<short, int> e in p2)
        {
            r[e.Key] = e.Value;
        }
        return Task.FromResult(new Test.MyClass_OpShortIntDResult(r, p3));
    }

    public override Task<Test.MyClass_OpShortIntLongResult>
    opShortIntLongAsync(short p1, int p2, long p3, Current current) =>
        Task.FromResult(new Test.MyClass_OpShortIntLongResult(p3, p1, p2, p3));

    public override Task<Test.MyClass_OpShortIntLongSResult>
    opShortIntLongSAsync(short[] p1, int[] p2, long[] p3, Current current)
    {
        short[] p4 = p1;
        int[] p5 = new int[p2.Length];
        for (int i = 0; i < p2.Length; i++)
        {
            p5[i] = p2[^(i + 1)];
        }
        long[] p6 = new long[p3.Length + p3.Length];
        Array.Copy(p3, p6, p3.Length);
        Array.Copy(p3, 0, p6, p3.Length, p3.Length);
        return Task.FromResult(new Test.MyClass_OpShortIntLongSResult(p3, p4, p5, p6));
    }

    public override Task<Test.MyClass_OpShortIntLongSSResult>
    opShortIntLongSSAsync(short[][] p1, int[][] p2, long[][] p3, Current current)
    {
        short[][] p4 = p1;

        int[][] p5 = new int[p2.Length][];
        for (int i = 0; i < p2.Length; i++)
        {
            p5[i] = p2[^(i + 1)];
        }

        long[][] p6 = new long[p3.Length + p3.Length][];
        Array.Copy(p3, p6, p3.Length);
        Array.Copy(p3, 0, p6, p3.Length, p3.Length);
        return Task.FromResult(new Test.MyClass_OpShortIntLongSSResult(p3, p4, p5, p6));
    }

    public override Task<Test.MyClass_OpStringResult>
    opStringAsync(string p1, string p2, Current current) =>
        Task.FromResult(new Test.MyClass_OpStringResult(p1 + " " + p2, p2 + " " + p1));

    public override Task<Test.MyClass_OpStringMyEnumDResult>
    opStringMyEnumDAsync(Dictionary<string, Test.MyEnum> p1, Dictionary<string, Test.MyEnum> p2, Current current)
    {
        Dictionary<string, Test.MyEnum> p3 = p1;
        var r = new Dictionary<string, Test.MyEnum>();
        foreach (KeyValuePair<string, Test.MyEnum> e in p1)
        {
            r[e.Key] = e.Value;
        }
        foreach (KeyValuePair<string, Test.MyEnum> e in p2)
        {
            r[e.Key] = e.Value;
        }
        return Task.FromResult(new Test.MyClass_OpStringMyEnumDResult(r, p3));
    }

    public override Task<Test.MyClass_OpMyEnumStringDResult>
    opMyEnumStringDAsync(Dictionary<Test.MyEnum, string> p1, Dictionary<Test.MyEnum, string> p2, Current current)
    {
        Dictionary<Test.MyEnum, string> p3 = p1;
        var r = new Dictionary<Test.MyEnum, string>();
        foreach (KeyValuePair<Test.MyEnum, string> e in p1)
        {
            r[e.Key] = e.Value;
        }
        foreach (KeyValuePair<Test.MyEnum, string> e in p2)
        {
            r[e.Key] = e.Value;
        }
        return Task.FromResult(new Test.MyClass_OpMyEnumStringDResult(r, p3));
    }

    public override Task<Test.MyClass_OpMyStructMyEnumDResult> opMyStructMyEnumDAsync(
        Dictionary<Test.MyStruct, Test.MyEnum> p1,
        Dictionary<Test.MyStruct, Test.MyEnum> p2,
        Current current)
    {
        Dictionary<Test.MyStruct, Test.MyEnum> p3 = p1;
        var r = new Dictionary<Test.MyStruct, Test.MyEnum>();
        foreach (KeyValuePair<Test.MyStruct, Test.MyEnum> e in p1)
        {
            r[e.Key] = e.Value;
        }
        foreach (KeyValuePair<Test.MyStruct, Test.MyEnum> e in p2)
        {
            r[e.Key] = e.Value;
        }
        return Task.FromResult(new Test.MyClass_OpMyStructMyEnumDResult(r, p3));
    }

    public override Task<Test.MyClass_OpByteBoolDSResult>
    opByteBoolDSAsync(Dictionary<byte, bool>[] p1, Dictionary<byte, bool>[] p2, Current current)
    {
        var p3 = new Dictionary<byte, bool>[p1.Length + p2.Length];
        Array.Copy(p2, p3, p2.Length);
        Array.Copy(p1, 0, p3, p2.Length, p1.Length);

        var r = new Dictionary<byte, bool>[p1.Length];
        for (int i = 0; i < p1.Length; i++)
        {
            r[i] = p1[^(i + 1)];
        }
        return Task.FromResult(new Test.MyClass_OpByteBoolDSResult(r, p3));
    }

    public override Task<Test.MyClass_OpShortIntDSResult>
    opShortIntDSAsync(Dictionary<short, int>[] p1, Dictionary<short, int>[] p2, Current current)
    {
        var p3 = new Dictionary<short, int>[p1.Length + p2.Length];
        Array.Copy(p2, p3, p2.Length);
        Array.Copy(p1, 0, p3, p2.Length, p1.Length);

        var r = new Dictionary<short, int>[p1.Length];
        for (int i = 0; i < p1.Length; i++)
        {
            r[i] = p1[^(i + 1)];
        }
        return Task.FromResult(new Test.MyClass_OpShortIntDSResult(r, p3));
    }

    public override Task<Test.MyClass_OpLongFloatDSResult>
    opLongFloatDSAsync(Dictionary<long, float>[] p1, Dictionary<long, float>[] p2, Current current)
    {
        var p3 = new Dictionary<long, float>[p1.Length + p2.Length];
        Array.Copy(p2, p3, p2.Length);
        Array.Copy(p1, 0, p3, p2.Length, p1.Length);

        var r = new Dictionary<long, float>[p1.Length];
        for (int i = 0; i < p1.Length; i++)
        {
            r[i] = p1[^(i + 1)];
        }
        return Task.FromResult(new Test.MyClass_OpLongFloatDSResult(r, p3));
    }

    public override Task<Test.MyClass_OpStringStringDSResult>
    opStringStringDSAsync(Dictionary<string, string>[] p1, Dictionary<string, string>[] p2, Current current)
    {
        var p3 = new Dictionary<string, string>[p1.Length + p2.Length];
        Array.Copy(p2, p3, p2.Length);
        Array.Copy(p1, 0, p3, p2.Length, p1.Length);

        var r = new Dictionary<string, string>[p1.Length];
        for (int i = 0; i < p1.Length; i++)
        {
            r[i] = p1[^(i + 1)];
        }
        return Task.FromResult(new Test.MyClass_OpStringStringDSResult(r, p3));
    }

    public override Task<Test.MyClass_OpStringMyEnumDSResult>
    opStringMyEnumDSAsync(Dictionary<string, Test.MyEnum>[] p1, Dictionary<string, Test.MyEnum>[] p2, Current current)
    {
        var p3 = new Dictionary<string, Test.MyEnum>[p1.Length + p2.Length];
        Array.Copy(p2, p3, p2.Length);
        Array.Copy(p1, 0, p3, p2.Length, p1.Length);

        var r = new Dictionary<string, Test.MyEnum>[p1.Length];
        for (int i = 0; i < p1.Length; i++)
        {
            r[i] = p1[^(i + 1)];
        }
        return Task.FromResult(new Test.MyClass_OpStringMyEnumDSResult(r, p3));
    }

    public override Task<Test.MyClass_OpMyEnumStringDSResult>
    opMyEnumStringDSAsync(Dictionary<Test.MyEnum, string>[] p1, Dictionary<Test.MyEnum, string>[] p2, Current current)
    {
        var p3 = new Dictionary<Test.MyEnum, string>[p1.Length + p2.Length];
        Array.Copy(p2, p3, p2.Length);
        Array.Copy(p1, 0, p3, p2.Length, p1.Length);

        var r = new Dictionary<Test.MyEnum, string>[p1.Length];
        for (int i = 0; i < p1.Length; i++)
        {
            r[i] = p1[^(i + 1)];
        }
        return Task.FromResult(new Test.MyClass_OpMyEnumStringDSResult(r, p3));
    }

    public override Task<Test.MyClass_OpMyStructMyEnumDSResult> opMyStructMyEnumDSAsync(
        Dictionary<Test.MyStruct, Test.MyEnum>[] p1,
        Dictionary<Test.MyStruct, Test.MyEnum>[] p2,
        Current current)
    {
        var p3 = new Dictionary<Test.MyStruct, Test.MyEnum>[p1.Length + p2.Length];
        Array.Copy(p2, p3, p2.Length);
        Array.Copy(p1, 0, p3, p2.Length, p1.Length);

        var r = new Dictionary<Test.MyStruct, Test.MyEnum>[p1.Length];
        for (int i = 0; i < p1.Length; i++)
        {
            r[i] = p1[^(i + 1)];
        }
        return Task.FromResult(new Test.MyClass_OpMyStructMyEnumDSResult(r, p3));
    }

    public override Task<Test.MyClass_OpByteByteSDResult>
    opByteByteSDAsync(Dictionary<byte, byte[]> p1, Dictionary<byte, byte[]> p2, Current current)
    {
        Dictionary<byte, byte[]> p3 = p2;
        var r = new Dictionary<byte, byte[]>();
        foreach (KeyValuePair<byte, byte[]> e in p1)
        {
            r[e.Key] = e.Value;
        }
        foreach (KeyValuePair<byte, byte[]> e in p2)
        {
            r[e.Key] = e.Value;
        }
        return Task.FromResult(new Test.MyClass_OpByteByteSDResult(r, p3));
    }

    public override Task<Test.MyClass_OpBoolBoolSDResult>
    opBoolBoolSDAsync(Dictionary<bool, bool[]> p1, Dictionary<bool, bool[]> p2, Current current)
    {
        Dictionary<bool, bool[]> p3 = p2;
        var r = new Dictionary<bool, bool[]>();
        foreach (KeyValuePair<bool, bool[]> e in p1)
        {
            r[e.Key] = e.Value;
        }
        foreach (KeyValuePair<bool, bool[]> e in p2)
        {
            r[e.Key] = e.Value;
        }
        return Task.FromResult(new Test.MyClass_OpBoolBoolSDResult(r, p3));
    }

    public override Task<Test.MyClass_OpShortShortSDResult>
    opShortShortSDAsync(Dictionary<short, short[]> p1, Dictionary<short, short[]> p2, Current current)
    {
        Dictionary<short, short[]> p3 = p2;
        var r = new Dictionary<short, short[]>();
        foreach (KeyValuePair<short, short[]> e in p1)
        {
            r[e.Key] = e.Value;
        }
        foreach (KeyValuePair<short, short[]> e in p2)
        {
            r[e.Key] = e.Value;
        }
        return Task.FromResult(new Test.MyClass_OpShortShortSDResult(r, p3));
    }

    public override Task<Test.MyClass_OpIntIntSDResult>
    opIntIntSDAsync(Dictionary<int, int[]> p1, Dictionary<int, int[]> p2, Current current)
    {
        Dictionary<int, int[]> p3 = p2;
        var r = new Dictionary<int, int[]>();
        foreach (KeyValuePair<int, int[]> e in p1)
        {
            r[e.Key] = e.Value;
        }
        foreach (KeyValuePair<int, int[]> e in p2)
        {
            r[e.Key] = e.Value;
        }
        return Task.FromResult(new Test.MyClass_OpIntIntSDResult(r, p3));
    }

    public override Task<Test.MyClass_OpLongLongSDResult>
    opLongLongSDAsync(Dictionary<long, long[]> p1, Dictionary<long, long[]> p2, Current current)
    {
        Dictionary<long, long[]> p3 = p2;
        var r = new Dictionary<long, long[]>();
        foreach (KeyValuePair<long, long[]> e in p1)
        {
            r[e.Key] = e.Value;
        }
        foreach (KeyValuePair<long, long[]> e in p2)
        {
            r[e.Key] = e.Value;
        }
        return Task.FromResult(new Test.MyClass_OpLongLongSDResult(r, p3));
    }

    public override Task<Test.MyClass_OpStringFloatSDResult>
    opStringFloatSDAsync(Dictionary<string, float[]> p1, Dictionary<string, float[]> p2, Current current)
    {
        Dictionary<string, float[]> p3 = p2;
        var r = new Dictionary<string, float[]>();
        foreach (KeyValuePair<string, float[]> e in p1)
        {
            r[e.Key] = e.Value;
        }
        foreach (KeyValuePair<string, float[]> e in p2)
        {
            r[e.Key] = e.Value;
        }
        return Task.FromResult(new Test.MyClass_OpStringFloatSDResult(r, p3));
    }

    public override Task<Test.MyClass_OpStringDoubleSDResult>
    opStringDoubleSDAsync(Dictionary<string, double[]> p1, Dictionary<string, double[]> p2, Current current)
    {
        Dictionary<string, double[]> p3 = p2;
        var r = new Dictionary<string, double[]>();
        foreach (KeyValuePair<string, double[]> e in p1)
        {
            r[e.Key] = e.Value;
        }
        foreach (KeyValuePair<string, double[]> e in p2)
        {
            r[e.Key] = e.Value;
        }
        return Task.FromResult<Test.MyClass_OpStringDoubleSDResult>(new Test.MyClass_OpStringDoubleSDResult(r, p3));
    }

    public override Task<Test.MyClass_OpStringStringSDResult>
    opStringStringSDAsync(Dictionary<string, string[]> p1, Dictionary<string, string[]> p2, Current current)
    {
        Dictionary<string, string[]> p3 = p2;
        var r = new Dictionary<string, string[]>();
        foreach (KeyValuePair<string, string[]> e in p1)
        {
            r[e.Key] = e.Value;
        }
        foreach (KeyValuePair<string, string[]> e in p2)
        {
            r[e.Key] = e.Value;
        }
        return Task.FromResult<Test.MyClass_OpStringStringSDResult>(new Test.MyClass_OpStringStringSDResult(r, p3));
    }

    public override Task<Test.MyClass_OpMyEnumMyEnumSDResult> opMyEnumMyEnumSDAsync(
        Dictionary<Test.MyEnum, Test.MyEnum[]> p1,
        Dictionary<Test.MyEnum, Test.MyEnum[]> p2,
        Current current)
    {
        Dictionary<Test.MyEnum, Test.MyEnum[]> p3 = p2;
        var r = new Dictionary<Test.MyEnum, Test.MyEnum[]>();
        foreach (KeyValuePair<Test.MyEnum, Test.MyEnum[]> e in p1)
        {
            r[e.Key] = e.Value;
        }
        foreach (KeyValuePair<Test.MyEnum, Test.MyEnum[]> e in p2)
        {
            r[e.Key] = e.Value;
        }
        return Task.FromResult(new Test.MyClass_OpMyEnumMyEnumSDResult(r, p3));
    }

    public override Task<int[]>
    opIntSAsync(int[] s, Current current)
    {
        int[] r = new int[s.Length];
        for (int i = 0; i < s.Length; ++i)
        {
            r[i] = -s[i];
        }
        return Task.FromResult(r);
    }

    public override Task<Dictionary<string, string>>
    opContextAsync(Current current) => Task.FromResult(current.ctx);

    public override Task
    opByteSOnewayAsync(byte[] s, Current current)
    {
        lock (_mutex)
        {
            ++_opByteSOnewayCallCount;
        }
        return Task.CompletedTask;
    }

    public override Task<int>
    opByteSOnewayCallCountAsync(Current current)
    {
        lock (_mutex)
        {
            int count = _opByteSOnewayCallCount;
            _opByteSOnewayCallCount = 0;
            return Task.FromResult<int>(count);
        }
    }

    public override Task
    opDoubleMarshalingAsync(double p1, double[] p2, Current current)
    {
        double d = 1278312346.0 / 13.0;
        test(p1 == d);
        for (int i = 0; i < p2.Length; ++i)
        {
            test(p2[i] == d);
        }
        return Task.CompletedTask;
    }

    public override Task<Test.MyClass_OpStringSResult>
    opStringSAsync(string[] p1, string[] p2, Current current)
    {
        string[] p3 = new string[p1.Length + p2.Length];
        Array.Copy(p1, p3, p1.Length);
        Array.Copy(p2, 0, p3, p1.Length, p2.Length);

        string[] r = new string[p1.Length];
        for (int i = 0; i < p1.Length; i++)
        {
            r[i] = p1[^(i + 1)];
        }
        return Task.FromResult(new Test.MyClass_OpStringSResult(r, p3));
    }

    public override Task<Test.MyClass_OpStringSSResult>
    opStringSSAsync(string[][] p1, string[][] p2, Current current)
    {
        string[][] p3 = new string[p1.Length + p2.Length][];
        Array.Copy(p1, p3, p1.Length);
        Array.Copy(p2, 0, p3, p1.Length, p2.Length);
        string[][] r = new string[p2.Length][];
        for (int i = 0; i < p2.Length; i++)
        {
            r[i] = p2[^(i + 1)];
        }
        return Task.FromResult(new Test.MyClass_OpStringSSResult(r, p3));
    }

    public override Task<Test.MyClass_OpStringSSSResult>
    opStringSSSAsync(string[][][] p1, string[][][] p2, Current current)
    {
        string[][][] p3 = new string[p1.Length + p2.Length][][];
        Array.Copy(p1, p3, p1.Length);
        Array.Copy(p2, 0, p3, p1.Length, p2.Length);

        string[][][] r = new string[p2.Length][][];
        for (int i = 0; i < p2.Length; i++)
        {
            r[i] = p2[^(i + 1)];
        }
        return Task.FromResult(new Test.MyClass_OpStringSSSResult(r, p3));
    }

    public override Task<Test.MyClass_OpStringStringDResult>
    opStringStringDAsync(Dictionary<string, string> p1, Dictionary<string, string> p2, Current current)
    {
        Dictionary<string, string> p3 = p1;
        var r = new Dictionary<string, string>();
        foreach (KeyValuePair<string, string> e in p1)
        {
            r[e.Key] = e.Value;
        }
        foreach (KeyValuePair<string, string> e in p2)
        {
            r[e.Key] = e.Value;
        }
        return Task.FromResult(new Test.MyClass_OpStringStringDResult(r, p3));
    }

    public override Task<Test.MyClass_OpStructResult>
    opStructAsync(Test.Structure p1, Test.Structure p2, Current current)
    {
        Test.Structure p3 = p1;
        p3.s.s = "a new string";
        return Task.FromResult(new Test.MyClass_OpStructResult(p2, p3));
    }

    public override Task
    opIdempotentAsync(Current current)
    {
        test(current.mode == Ice.OperationMode.Idempotent);
        return Task.CompletedTask;
    }

    public override Task
    opDerivedAsync(Current current) => Task.CompletedTask;

    public override Task<byte>
    opByte1Async(byte opByte1, Current current) => Task.FromResult(opByte1);

    public override Task<short>
    opShort1Async(short opShort1, Current current) => Task.FromResult(opShort1);

    public override Task<int>
    opInt1Async(int opInt1, Current current) => Task.FromResult<int>(opInt1);

    public override Task<long>
    opLong1Async(long opLong1, Current current) => Task.FromResult(opLong1);

    public override Task<float>
    opFloat1Async(float opFloat1, Current current) => Task.FromResult(opFloat1);

    public override Task<double>
    opDouble1Async(double opDouble1, Current current) => Task.FromResult(opDouble1);

    public override Task<string>
    opString1Async(string opString1, Current current) => Task.FromResult<string>(opString1);

    public override Task<string[]>
    opStringS1Async(string[] opStringS1, Current current) => Task.FromResult(opStringS1);

    public override Task<Dictionary<byte, bool>>
    opByteBoolD1Async(Dictionary<byte, bool> opByteBoolD1, Current current) => Task.FromResult(opByteBoolD1);

    public override Task<string[]>
    opStringS2Async(string[] stringS, Current current) => Task.FromResult(stringS);

    public override Task<Dictionary<byte, bool>>
    opByteBoolD2Async(Dictionary<byte, bool> byteBoolD, Current current) => Task.FromResult(byteBoolD);

    public override Task<Test.MyClass1>
    opMyClass1Async(Test.MyClass1 opMyClass1, Current current) => Task.FromResult<Test.MyClass1>(opMyClass1);

    public override Task<Test.MyStruct1>
    opMyStruct1Async(Test.MyStruct1 opMyStruct1, Current current) => Task.FromResult(opMyStruct1);

    public override Task<string[]>
    opStringLiteralsAsync(Current current)
    {
        return Task.FromResult(new string[]
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

    public override Task<string[]>
    opWStringLiteralsAsync(Current current)
    {
        return Task.FromResult(new string[]
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

    public override Task<Test.MyClass_OpMStruct1MarshaledResult> opMStruct1Async(Current current) =>
        Task.FromResult(
            new Test.MyClass_OpMStruct1MarshaledResult(new Test.Structure(new Test.AnotherStruct()), current));

    public override Task<Test.MyClass_OpMStruct2MarshaledResult> opMStruct2Async(Test.Structure p1, Current current) =>
        Task.FromResult(new Test.MyClass_OpMStruct2MarshaledResult(p1, p1, current));

    public override Task<Test.MyClass_OpMSeq1MarshaledResult> opMSeq1Async(Current current) =>
        Task.FromResult(new Test.MyClass_OpMSeq1MarshaledResult([], current));

    public override Task<Test.MyClass_OpMSeq2MarshaledResult> opMSeq2Async(string[] p1, Current current) =>
        Task.FromResult(new Test.MyClass_OpMSeq2MarshaledResult(p1, p1, current));

    public override Task<Test.MyClass_OpMDict1MarshaledResult> opMDict1Async(Current current) =>
        Task.FromResult(new Test.MyClass_OpMDict1MarshaledResult([], current));

    public override Task<Test.MyClass_OpMDict2MarshaledResult> opMDict2Async(
        Dictionary<string, string> p1,
        Current current) =>
        Task.FromResult(new Test.MyClass_OpMDict2MarshaledResult(p1, p1, current));

    private Thread_opVoid _opVoidThread;
    private int _opByteSOnewayCallCount;
    private readonly object _mutex = new();
}
