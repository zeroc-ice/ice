//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using System.Threading;
using System.Threading.Tasks;
using System.Linq;

namespace Ice
{
    namespace operations
    {
        namespace AMD
        {
            public sealed class MyDerivedClassI : Test.MyDerivedClass
            {
                private static void test(bool b)
                {
                    if (!b)
                    {
                        throw new System.Exception();
                    }
                }

                internal class Thread_opVoid : TaskCompletionSource<object>
                {
                    public Thread_opVoid()
                    {
                    }

                    public void Start()
                    {
                        lock (this)
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
                        lock (this)
                        {
                            _thread.Join();
                        }
                    }

                    private Thread _thread;
                }

                //
                // Override the Object "pseudo" operations to verify the operation mode.
                //
                public bool ice_isA(string id, Current current)
                {
                    test(current.Mode == Ice.OperationMode.Nonmutating);
                    Test.MyDerivedClassTraits myDerivedClassT = default;
                    return myDerivedClassT.Ids.Contains(id);
                }

                public void IcePing(Ice.Current current)
                {
                    test(current.Mode == Ice.OperationMode.Nonmutating);
                }

                public string[] ice_ids(Current current)
                {
                    test(current.Mode == OperationMode.Nonmutating);
                    Test.MyDerivedClassTraits myDerivedClassT = default;
                    return myDerivedClassT.Ids;
                }

                public string ice_id(Current current)
                {
                    test(current.Mode == Ice.OperationMode.Nonmutating);
                    Test.MyDerivedClassTraits myDerivedClassT = default;
                    return myDerivedClassT.Id;
                }

                public Task shutdownAsync(Ice.Current current)
                {
                    while (_opVoidThread != null)
                    {
                        _opVoidThread.Join();
                        _opVoidThread = null;
                    }

                    current.Adapter.Communicator.shutdown();
                    return null;
                }

                public Task<bool> supportsCompressAsync(Ice.Current current)
                {
                    return Task.FromResult<bool>(true);
                }

                public Task opVoidAsync(Ice.Current current)
                {
                    test(current.Mode == Ice.OperationMode.Normal);

                    while (_opVoidThread != null)
                    {
                        _opVoidThread.Join();
                        _opVoidThread = null;
                    }

                    _opVoidThread = new Thread_opVoid();
                    _opVoidThread.Start();
                    return _opVoidThread.Task;
                }

                public Task<Test.MyClass.opBoolResult> opBoolAsync(bool p1, bool p2, Current current) => Task.FromResult(new Test.MyClass.opBoolResult(p2, p1));

                public Task<Test.MyClass.opBoolSResult> opBoolSAsync(bool[] p1, bool[] p2, Current current)
                {
                    bool[] p3 = new bool[p1.Length + p2.Length];
                    Array.Copy(p1, p3, p1.Length);
                    Array.Copy(p2, 0, p3, p1.Length, p2.Length);

                    bool[] r = new bool[p1.Length];
                    for (int i = 0; i < p1.Length; i++)
                    {
                        r[i] = p1[p1.Length - (i + 1)];
                    }

                    return Task.FromResult(new Test.MyClass.opBoolSResult(r, p3));
                }

                public Task<Test.MyClass.opBoolSSResult>
                opBoolSSAsync(bool[][] p1, bool[][] p2, Current current)
                {
                    bool[][] p3 = new bool[p1.Length + p2.Length][];
                    Array.Copy(p1, p3, p1.Length);
                    Array.Copy(p2, 0, p3, p1.Length, p2.Length);

                    bool[][] r = new bool[p1.Length][];
                    for (int i = 0; i < p1.Length; i++)
                    {
                        r[i] = p1[p1.Length - (i + 1)];
                    }

                    return Task.FromResult(new Test.MyClass.opBoolSSResult(r, p3));
                }

                public Task<Test.MyClass.opByteResult>
                opByteAsync(byte p1, byte p2, Current current) => Task.FromResult(new Test.MyClass.opByteResult(p1, (byte)(p1 ^ p2)));


                public Task<Test.MyClass.opByteBoolDResult>
                opByteBoolDAsync(Dictionary<byte, bool> p1, Dictionary<byte, bool> p2, Ice.Current current)
                {
                    Dictionary<byte, bool> p3 = p1;
                    Dictionary<byte, bool> r = new Dictionary<byte, bool>();
                    foreach (KeyValuePair<byte, bool> e in p1)
                    {
                        r[e.Key] = e.Value;
                    }
                    foreach (KeyValuePair<byte, bool> e in p2)
                    {
                        r[e.Key] = e.Value;
                    }
                    return Task.FromResult(new Test.MyClass.opByteBoolDResult(r, p3));
                }

                public Task<Test.MyClass.opByteSResult>
                opByteSAsync(byte[] p1, byte[] p2, Ice.Current current)
                {
                    byte[] p3 = new byte[p1.Length];
                    for (int i = 0; i < p1.Length; i++)
                    {
                        p3[i] = p1[p1.Length - (i + 1)];
                    }

                    byte[] r = new byte[p1.Length + p2.Length];
                    Array.Copy(p1, r, p1.Length);
                    Array.Copy(p2, 0, r, p1.Length, p2.Length);

                    return Task.FromResult(new Test.MyClass.opByteSResult(r, p3));
                }

                public Task<Test.MyClass.opByteSSResult>
                opByteSSAsync(byte[][] p1, byte[][] p2, Ice.Current current)
                {
                    byte[][] p3 = new byte[p1.Length][];
                    for (int i = 0; i < p1.Length; i++)
                    {
                        p3[i] = p1[p1.Length - (i + 1)];
                    }

                    byte[][] r = new byte[p1.Length + p2.Length][];
                    Array.Copy(p1, r, p1.Length);
                    Array.Copy(p2, 0, r, p1.Length, p2.Length);

                    return Task.FromResult(new Test.MyClass.opByteSSResult(r, p3));
                }

                public Task<Test.MyClass.opFloatDoubleResult>
                opFloatDoubleAsync(float p1, double p2, Current current) => Task.FromResult(new Test.MyClass.opFloatDoubleResult(p2, p1, p2));


                public Task<Test.MyClass.opFloatDoubleSResult>
                opFloatDoubleSAsync(float[] p1, double[] p2, Current current)
                {
                    float[] p3 = p1;

                    double[] p4 = new double[p2.Length];
                    for (int i = 0; i < p2.Length; i++)
                    {
                        p4[i] = p2[p2.Length - (i + 1)];
                    }

                    double[] r = new double[p2.Length + p1.Length];
                    Array.Copy(p2, r, p2.Length);
                    for (int i = 0; i < p1.Length; i++)
                    {
                        r[p2.Length + i] = p1[i];
                    }

                    return Task.FromResult(new Test.MyClass.opFloatDoubleSResult(r, p3, p4));
                }

                public Task<Test.MyClass.opFloatDoubleSSResult>
                opFloatDoubleSSAsync(float[][] p1, double[][] p2, Ice.Current current)
                {
                    var p3 = p1;

                    var p4 = new double[p2.Length][];
                    for (int i = 0; i < p2.Length; i++)
                    {
                        p4[i] = p2[p2.Length - (i + 1)];
                    }

                    var r = new double[p2.Length + p2.Length][];
                    Array.Copy(p2, r, p2.Length);
                    for (int i = 0; i < p2.Length; i++)
                    {
                        r[p2.Length + i] = new double[p2[i].Length];
                        for (int j = 0; j < p2[i].Length; j++)
                        {
                            r[p2.Length + i][j] = p2[i][j];
                        }
                    }

                    return Task.FromResult(new Test.MyClass.opFloatDoubleSSResult(r, p3, p4));
                }

                public Task<Test.MyClass.opLongFloatDResult>
                opLongFloatDAsync(Dictionary<long, float> p1, Dictionary<long, float> p2, Ice.Current current)
                {
                    var p3 = p1;
                    var r = new Dictionary<long, float>();
                    foreach (KeyValuePair<long, float> e in p1)
                    {
                        r[e.Key] = e.Value;
                    }
                    foreach (KeyValuePair<long, float> e in p2)
                    {
                        r[e.Key] = e.Value;
                    }

                    return Task.FromResult(new Test.MyClass.opLongFloatDResult(r, p3));
                }

                public Task<Test.MyClass.opMyClassResult>
                opMyClassAsync(Test.MyClassPrx p1, Ice.Current current)
                {
                    var p2 = p1;
                    var p3 = Test.MyClassPrx.UncheckedCast(current.Adapter.CreateProxy("noSuchIdentity"));
                    return Task.FromResult(new Test.MyClass.opMyClassResult(
                        Test.MyClassPrx.UncheckedCast(current.Adapter.CreateProxy(current.Id)), p2, p3));
                }

                public Task<Test.MyClass.opMyEnumResult>
                opMyEnumAsync(Test.MyEnum p1, Ice.Current current) => Task.FromResult(new Test.MyClass.opMyEnumResult(Test.MyEnum.enum3, p1));

                public Task<Test.MyClass.opShortIntDResult>
                opShortIntDAsync(Dictionary<short, int> p1, Dictionary<short, int> p2, Ice.Current current)
                {
                    var p3 = p1;
                    var r = new Dictionary<short, int>();
                    foreach (KeyValuePair<short, int> e in p1)
                    {
                        r[e.Key] = e.Value;
                    }
                    foreach (KeyValuePair<short, int> e in p2)
                    {
                        r[e.Key] = e.Value;
                    }
                    return Task.FromResult(new Test.MyClass.opShortIntDResult(r, p3));
                }

                public Task<Test.MyClass.opShortIntLongResult>
                opShortIntLongAsync(short p1, int p2, long p3, Current current)
                {
                    return Task.FromResult(new Test.MyClass.opShortIntLongResult(p3, p1, p2, p3));
                }

                public Task<Test.MyClass.opShortIntLongSResult>
                opShortIntLongSAsync(short[] p1, int[] p2, long[] p3, Current current)
                {
                    var p4 = p1;
                    var p5 = new int[p2.Length];
                    for (int i = 0; i < p2.Length; i++)
                    {
                        p5[i] = p2[p2.Length - (i + 1)];
                    }
                    var p6 = new long[p3.Length + p3.Length];
                    Array.Copy(p3, p6, p3.Length);
                    Array.Copy(p3, 0, p6, p3.Length, p3.Length);
                    return Task.FromResult(new Test.MyClass.opShortIntLongSResult(p3, p4, p5, p6));
                }

                public Task<Test.MyClass.opShortIntLongSSResult>
                opShortIntLongSSAsync(short[][] p1, int[][] p2, long[][] p3, Ice.Current current)
                {
                    var p4 = p1;

                    var p5 = new int[p2.Length][];
                    for (int i = 0; i < p2.Length; i++)
                    {
                        p5[i] = p2[p2.Length - (i + 1)];
                    }

                    var p6 = new long[p3.Length + p3.Length][];
                    Array.Copy(p3, p6, p3.Length);
                    Array.Copy(p3, 0, p6, p3.Length, p3.Length);
                    return Task.FromResult(new Test.MyClass.opShortIntLongSSResult(p3, p4, p5, p6));
                }

                public Task<Test.MyClass.opStringResult>
                opStringAsync(string p1, string p2, Current current) => Task.FromResult(new Test.MyClass.opStringResult($"{p1} {p2}", $"{p2} {p1}"));

                public Task<Test.MyClass.opStringMyEnumDResult>
                opStringMyEnumDAsync(Dictionary<string, Test.MyEnum> p1, Dictionary<string, Test.MyEnum> p2, Current current)
                {
                    var p3 = p1;
                    var r = new Dictionary<string, Test.MyEnum>();
                    foreach (KeyValuePair<string, Test.MyEnum> e in p1)
                    {
                        r[e.Key] = e.Value;
                    }
                    foreach (KeyValuePair<string, Test.MyEnum> e in p2)
                    {
                        r[e.Key] = e.Value;
                    }
                    return Task.FromResult(new Test.MyClass.opStringMyEnumDResult(r, p3));
                }

                public Task<Test.MyClass.opMyEnumStringDResult>
                opMyEnumStringDAsync(Dictionary<Test.MyEnum, string> p1, Dictionary<Test.MyEnum, string> p2, Current current)
                {
                    var p3 = p1;
                    var r = new Dictionary<Test.MyEnum, string>();
                    foreach (var e in p1)
                    {
                        r[e.Key] = e.Value;
                    }
                    foreach (KeyValuePair<Test.MyEnum, string> e in p2)
                    {
                        r[e.Key] = e.Value;
                    }
                    return Task.FromResult(new Test.MyClass.opMyEnumStringDResult(r, p3));
                }

                public Task<Test.MyClass.opMyStructMyEnumDResult>
                opMyStructMyEnumDAsync(Dictionary<Test.MyStruct, Test.MyEnum> p1,
                                       Dictionary<Test.MyStruct, Test.MyEnum> p2, Ice.Current current)
                {
                    var p3 = p1;
                    var r = new Dictionary<Test.MyStruct, Test.MyEnum>();
                    foreach (var e in p1)
                    {
                        r[e.Key] = e.Value;
                    }
                    foreach (var e in p2)
                    {
                        r[e.Key] = e.Value;
                    }
                    return Task.FromResult(new Test.MyClass.opMyStructMyEnumDResult(r, p3));
                }

                public Task<Test.MyClass.opByteBoolDSResult>
                opByteBoolDSAsync(Dictionary<byte, bool>[] p1, Dictionary<byte, bool>[] p2, Ice.Current current)
                {
                    var p3 = new Dictionary<byte, bool>[p1.Length + p2.Length];
                    Array.Copy(p2, p3, p2.Length);
                    Array.Copy(p1, 0, p3, p2.Length, p1.Length);

                    var r = new Dictionary<byte, bool>[p1.Length];
                    for (int i = 0; i < p1.Length; i++)
                    {
                        r[i] = p1[p1.Length - (i + 1)];
                    }
                    return Task.FromResult(new Test.MyClass.opByteBoolDSResult(r, p3));
                }

                public Task<Test.MyClass.opShortIntDSResult>
                opShortIntDSAsync(Dictionary<short, int>[] p1, Dictionary<short, int>[] p2, Current current)
                {
                    var p3 = new Dictionary<short, int>[p1.Length + p2.Length];
                    Array.Copy(p2, p3, p2.Length);
                    Array.Copy(p1, 0, p3, p2.Length, p1.Length);

                    var r = new Dictionary<short, int>[p1.Length];
                    for (int i = 0; i < p1.Length; i++)
                    {
                        r[i] = p1[p1.Length - (i + 1)];
                    }
                    return Task.FromResult(new Test.MyClass.opShortIntDSResult(r, p3));
                }

                public Task<Test.MyClass.opLongFloatDSResult>
                opLongFloatDSAsync(Dictionary<long, float>[] p1, Dictionary<long, float>[] p2, Current current)
                {
                    var p3 = new Dictionary<long, float>[p1.Length + p2.Length];
                    Array.Copy(p2, p3, p2.Length);
                    Array.Copy(p1, 0, p3, p2.Length, p1.Length);

                    var r = new Dictionary<long, float>[p1.Length];
                    for (int i = 0; i < p1.Length; i++)
                    {
                        r[i] = p1[p1.Length - (i + 1)];
                    }
                    return Task.FromResult(new Test.MyClass.opLongFloatDSResult(r, p3));
                }

                public Task<Test.MyClass.opStringStringDSResult>
                opStringStringDSAsync(Dictionary<string, string>[] p1, Dictionary<string, string>[] p2, Current current)
                {
                    var p3 = new Dictionary<string, string>[p1.Length + p2.Length];
                    Array.Copy(p2, p3, p2.Length);
                    Array.Copy(p1, 0, p3, p2.Length, p1.Length);

                    var r = new Dictionary<string, string>[p1.Length];
                    for (int i = 0; i < p1.Length; i++)
                    {
                        r[i] = p1[p1.Length - (i + 1)];
                    }
                    return Task.FromResult(new Test.MyClass.opStringStringDSResult(r, p3));
                }

                public Task<Test.MyClass.opStringMyEnumDSResult>
                opStringMyEnumDSAsync(Dictionary<string, Test.MyEnum>[] p1, Dictionary<string, Test.MyEnum>[] p2, Current current)
                {
                    var p3 = new Dictionary<string, Test.MyEnum>[p1.Length + p2.Length];
                    Array.Copy(p2, p3, p2.Length);
                    Array.Copy(p1, 0, p3, p2.Length, p1.Length);

                    var r = new Dictionary<string, Test.MyEnum>[p1.Length];
                    for (int i = 0; i < p1.Length; i++)
                    {
                        r[i] = p1[p1.Length - (i + 1)];
                    }
                    return Task.FromResult(new Test.MyClass.opStringMyEnumDSResult(r, p3));
                }

                public Task<Test.MyClass.opMyEnumStringDSResult>
                opMyEnumStringDSAsync(Dictionary<Test.MyEnum, string>[] p1, Dictionary<Test.MyEnum, string>[] p2, Ice.Current current)
                {
                    var p3 = new Dictionary<Test.MyEnum, string>[p1.Length + p2.Length];
                    Array.Copy(p2, p3, p2.Length);
                    Array.Copy(p1, 0, p3, p2.Length, p1.Length);

                    var r = new Dictionary<Test.MyEnum, string>[p1.Length];
                    for (int i = 0; i < p1.Length; i++)
                    {
                        r[i] = p1[p1.Length - (i + 1)];
                    }
                    return Task.FromResult(new Test.MyClass.opMyEnumStringDSResult(r, p3));
                }

                public Task<Test.MyClass.opMyStructMyEnumDSResult>
                opMyStructMyEnumDSAsync(Dictionary<Test.MyStruct, Test.MyEnum>[] p1,
                                        Dictionary<Test.MyStruct, Test.MyEnum>[] p2,
                                        Current current)
                {
                    var p3 = new Dictionary<Test.MyStruct, Test.MyEnum>[p1.Length + p2.Length];
                    Array.Copy(p2, p3, p2.Length);
                    Array.Copy(p1, 0, p3, p2.Length, p1.Length);

                    var r = new Dictionary<Test.MyStruct, Test.MyEnum>[p1.Length];
                    for (int i = 0; i < p1.Length; i++)
                    {
                        r[i] = p1[p1.Length - (i + 1)];
                    }
                    return Task.FromResult(new Test.MyClass.opMyStructMyEnumDSResult(r, p3));
                }

                public Task<Test.MyClass.opByteByteSDResult>
                opByteByteSDAsync(Dictionary<byte, byte[]> p1, Dictionary<byte, byte[]> p2, Current current)
                {
                    var p3 = p2;
                    var r = new Dictionary<byte, byte[]>();
                    foreach (var e in p1)
                    {
                        r[e.Key] = e.Value;
                    }
                    foreach (var e in p2)
                    {
                        r[e.Key] = e.Value;
                    }
                    return Task.FromResult(new Test.MyClass.opByteByteSDResult(r, p3));
                }

                public Task<Test.MyClass.opBoolBoolSDResult>
                opBoolBoolSDAsync(Dictionary<bool, bool[]> p1, Dictionary<bool, bool[]> p2, Current current)
                {
                    var p3 = p2;
                    var r = new Dictionary<bool, bool[]>();
                    foreach (KeyValuePair<bool, bool[]> e in p1)
                    {
                        r[e.Key] = e.Value;
                    }
                    foreach (KeyValuePair<bool, bool[]> e in p2)
                    {
                        r[e.Key] = e.Value;
                    }
                    return Task.FromResult(new Test.MyClass.opBoolBoolSDResult(r, p3));
                }

                public Task<Test.MyClass.opShortShortSDResult>
                opShortShortSDAsync(Dictionary<short, short[]> p1, Dictionary<short, short[]> p2, Current current)
                {
                    var p3 = p2;
                    var r = new Dictionary<short, short[]>();
                    foreach (var e in p1)
                    {
                        r[e.Key] = e.Value;
                    }
                    foreach (var e in p2)
                    {
                        r[e.Key] = e.Value;
                    }
                    return Task.FromResult(new Test.MyClass.opShortShortSDResult(r, p3));
                }

                public Task<Test.MyClass.opIntIntSDResult>
                opIntIntSDAsync(Dictionary<int, int[]> p1, Dictionary<int, int[]> p2, Current current)
                {
                    var p3 = p2;
                    var r = new Dictionary<int, int[]>();
                    foreach (var e in p1)
                    {
                        r[e.Key] = e.Value;
                    }
                    foreach (var e in p2)
                    {
                        r[e.Key] = e.Value;
                    }
                    return Task.FromResult(new Test.MyClass.opIntIntSDResult(r, p3));
                }

                public Task<Test.MyClass.opLongLongSDResult>
                opLongLongSDAsync(Dictionary<long, long[]> p1, Dictionary<long, long[]> p2, Current current)
                {
                    var p3 = p2;
                    var r = new Dictionary<long, long[]>();
                    foreach (var e in p1)
                    {
                        r[e.Key] = e.Value;
                    }
                    foreach (var e in p2)
                    {
                        r[e.Key] = e.Value;
                    }
                    return Task.FromResult(new Test.MyClass.opLongLongSDResult(r, p3));
                }

                public Task<Test.MyClass.opStringFloatSDResult>
                opStringFloatSDAsync(Dictionary<string, float[]> p1, Dictionary<string, float[]> p2, Current current)
                {
                    var p3 = p2;
                    var r = new Dictionary<string, float[]>();
                    foreach (var e in p1)
                    {
                        r[e.Key] = e.Value;
                    }
                    foreach (var e in p2)
                    {
                        r[e.Key] = e.Value;
                    }
                    return Task.FromResult(new Test.MyClass.opStringFloatSDResult(r, p3));
                }

                public Task<Test.MyClass.opStringDoubleSDResult>
                opStringDoubleSDAsync(Dictionary<string, double[]> p1, Dictionary<string, double[]> p2, Current current)
                {
                    var p3 = p2;
                    var r = new Dictionary<string, double[]>();
                    foreach (var e in p1)
                    {
                        r[e.Key] = e.Value;
                    }
                    foreach (var e in p2)
                    {
                        r[e.Key] = e.Value;
                    }
                    return Task.FromResult(new Test.MyClass.opStringDoubleSDResult(r, p3));
                }

                public Task<Test.MyClass.opStringStringSDResult>
                opStringStringSDAsync(Dictionary<string, string[]> p1, Dictionary<string, string[]> p2, Ice.Current current)
                {
                    var p3 = p2;
                    var r = new Dictionary<string, string[]>();
                    foreach (var e in p1)
                    {
                        r[e.Key] = e.Value;
                    }
                    foreach (var e in p2)
                    {
                        r[e.Key] = e.Value;
                    }
                    return Task.FromResult(new Test.MyClass.opStringStringSDResult(r, p3));
                }

                public Task<Test.MyClass.opMyEnumMyEnumSDResult>
                opMyEnumMyEnumSDAsync(Dictionary<Test.MyEnum, Test.MyEnum[]> p1, Dictionary<Test.MyEnum, Test.MyEnum[]> p2,
                    Ice.Current current)
                {
                    var p3 = p2;
                    var r = new Dictionary<Test.MyEnum, Test.MyEnum[]>();
                    foreach (var e in p1)
                    {
                        r[e.Key] = e.Value;
                    }
                    foreach (var e in p2)
                    {
                        r[e.Key] = e.Value;
                    }
                    return Task.FromResult(new Test.MyClass.opMyEnumMyEnumSDResult(r, p3));
                }

                public Task<int[]>
                opIntSAsync(int[] s, Ice.Current current)
                {
                    var r = new int[s.Length];
                    for (int i = 0; i < s.Length; ++i)
                    {
                        r[i] = -s[i];
                    }
                    return Task.FromResult(r);
                }

                public Task<Dictionary<string, string>>
                opContextAsync(Ice.Current current)
                {
                    return Task.FromResult(current.Context);
                }

                public Task
                opByteSOnewayAsync(byte[] s, Ice.Current current)
                {
                    lock (this)
                    {
                        ++_opByteSOnewayCallCount;
                    }
                    return Task.CompletedTask;
                }

                public Task<int>
                opByteSOnewayCallCountAsync(Ice.Current current)
                {
                    lock (this)
                    {
                        var count = _opByteSOnewayCallCount;
                        _opByteSOnewayCallCount = 0;
                        return Task.FromResult(count);
                    }
                }

                public Task
                opDoubleMarshalingAsync(double p1, double[] p2, Ice.Current current)
                {
                    var d = 1278312346.0 / 13.0;
                    test(p1 == d);
                    for (int i = 0; i < p2.Length; ++i)
                    {
                        test(p2[i] == d);
                    }
                    return Task.CompletedTask;
                }

                public Task<Test.MyClass.opStringSResult>
                opStringSAsync(string[] p1, string[] p2, Current current)
                {
                    var p3 = new string[p1.Length + p2.Length];
                    Array.Copy(p1, p3, p1.Length);
                    Array.Copy(p2, 0, p3, p1.Length, p2.Length);

                    var r = new string[p1.Length];
                    for (int i = 0; i < p1.Length; i++)
                    {
                        r[i] = p1[p1.Length - (i + 1)];
                    }
                    return Task.FromResult(new Test.MyClass.opStringSResult(r, p3));
                }

                public Task<Test.MyClass.opStringSSResult>
                opStringSSAsync(string[][] p1, string[][] p2, Current current)
                {
                    var p3 = new string[p1.Length + p2.Length][];
                    Array.Copy(p1, p3, p1.Length);
                    Array.Copy(p2, 0, p3, p1.Length, p2.Length);
                    var r = new string[p2.Length][];
                    for (int i = 0; i < p2.Length; i++)
                    {
                        r[i] = p2[p2.Length - (i + 1)];
                    }
                    return Task.FromResult(new Test.MyClass.opStringSSResult(r, p3));
                }

                public Task<Test.MyClass.opStringSSSResult>
                opStringSSSAsync(string[][][] p1, string[][][] p2, Current current)
                {
                    var p3 = new string[p1.Length + p2.Length][][];
                    Array.Copy(p1, p3, p1.Length);
                    Array.Copy(p2, 0, p3, p1.Length, p2.Length);

                    var r = new string[p2.Length][][];
                    for (int i = 0; i < p2.Length; i++)
                    {
                        r[i] = p2[p2.Length - (i + 1)];
                    }
                    return Task.FromResult(new Test.MyClass.opStringSSSResult(r, p3));
                }

                public Task<Test.MyClass.opStringStringDResult>
                opStringStringDAsync(Dictionary<string, string> p1, Dictionary<string, string> p2, Ice.Current current)
                {
                    var p3 = p1;
                    var r = new Dictionary<string, string>();
                    foreach (var e in p1)
                    {
                        r[e.Key] = e.Value;
                    }
                    foreach (var e in p2)
                    {
                        r[e.Key] = e.Value;
                    }
                    return Task.FromResult(new Test.MyClass.opStringStringDResult(r, p3));
                }

                public Task<Test.MyClass.opStructResult>
                opStructAsync(Test.Structure p1, Test.Structure p2, Ice.Current current)
                {
                    var p3 = p1;
                    p3.s.s = "a new string";
                    return Task.FromResult(new Test.MyClass.opStructResult(p2, p3));
                }

                public Task
                opIdempotentAsync(Current current)
                {
                    test(current.Mode == OperationMode.Idempotent);
                    return Task.CompletedTask;
                }

                public Task
                opNonmutatingAsync(Ice.Current current)
                {
                    test(current.Mode == OperationMode.Nonmutating);
                    return Task.CompletedTask;
                }

                public Task
                opDerivedAsync(Current current)
                {
                    return Task.CompletedTask;
                }

                public Task<byte>
                opByte1Async(byte value, Current current)
                {
                    return Task.FromResult(value);
                }

                public Task<short>
                opShort1Async(short value, Current current)
                {
                    return Task.FromResult(value);
                }

                public Task<int>
                opInt1Async(int value, Current current)
                {
                    return Task.FromResult(value);
                }

                public Task<long>
                opLong1Async(long value, Current current)
                {
                    return Task.FromResult(value);
                }

                public Task<float>
                opFloat1Async(float value, Current current)
                {
                    return Task.FromResult(value);
                }

                public Task<double>
                opDouble1Async(double value, Current current)
                {
                    return Task.FromResult(value);
                }

                public Task<string>
                opString1Async(string value, Current current)
                {
                    return Task.FromResult(value);
                }

                public Task<string[]>
                opStringS1Async(string[] value, Current current)
                {
                    return Task.FromResult(value);
                }

                public Task<Dictionary<byte, bool>>
                opByteBoolD1Async(Dictionary<byte, bool> value, Current current)
                {
                    return Task.FromResult(value);
                }

                public Task<string[]>
                opStringS2Async(string[] value, Current current)
                {
                    return Task.FromResult(value);
                }

                public Task<Dictionary<byte, bool>>
                opByteBoolD2Async(Dictionary<byte, bool> value, Current current)
                {
                    return Task.FromResult(value);
                }

                public Task<Test.MyClass1>
                opMyClass1Async(Test.MyClass1 value, Current current)
                {
                    return Task.FromResult(value);
                }

                public Task<Test.MyStruct1>
                opMyStruct1Async(Test.MyStruct1 value, Current current)
                {
                    return Task.FromResult(value);
                }

                public Task<string[]>
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

                public Task<string[]>
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

                public async Task<Test.MyClass.opMStruct1MarshaledResult>
                opMStruct1Async(Current current)
                {
                    await Task.Delay(0);
                    return new Test.MyClass.opMStruct1MarshaledResult(new Test.Structure(), current);
                }

                public async Task<Test.MyClass.opMStruct2MarshaledResult>
                opMStruct2Async(Test.Structure p1, Current current)
                {
                    await Task.Delay(0);
                    return new Test.MyClass.opMStruct2MarshaledResult(p1, p1, current);
                }

                public async Task<Test.MyClass.opMSeq1MarshaledResult>
                opMSeq1Async(Current current)
                {
                    await Task.Delay(0);
                    return new Test.MyClass.opMSeq1MarshaledResult(new string[0], current);
                }

                public async Task<Test.MyClass.opMSeq2MarshaledResult>
                opMSeq2Async(string[] p1, Current current)
                {
                    await Task.Delay(0);
                    return new Test.MyClass.opMSeq2MarshaledResult(p1, p1, current);
                }

                public async Task<Test.MyClass.opMDict1MarshaledResult>
                opMDict1Async(Current current)
                {
                    await Task.Delay(0);
                    return new Test.MyClass.opMDict1MarshaledResult(new Dictionary<string, string>(), current);
                }

                public async Task<Test.MyClass.opMDict2MarshaledResult>
                opMDict2Async(Dictionary<string, string> p1, Ice.Current current)
                {
                    await Task.Delay(0);
                    return new Test.MyClass.opMDict2MarshaledResult(p1, p1, current);
                }

                private Thread_opVoid? _opVoidThread;
                private int _opByteSOnewayCallCount = 0;
            }
        }
    }
}
