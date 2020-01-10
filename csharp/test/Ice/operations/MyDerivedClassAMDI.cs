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

                public void IcePing(Current current)
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

                public Task shutdownAsync(Current current)
                {
                    while (_opVoidThread != null)
                    {
                        _opVoidThread.Join();
                        _opVoidThread = null;
                    }

                    current.Adapter.Communicator.shutdown();
                    return Task.CompletedTask;
                }

                public Task<bool> supportsCompressAsync(Current current)
                {
                    return Task.FromResult(true);
                }

                public Task opVoidAsync(Current current)
                {
                    test(current.Mode == OperationMode.Normal);

                    while (_opVoidThread != null)
                    {
                        _opVoidThread.Join();
                        _opVoidThread = null;
                    }

                    _opVoidThread = new Thread_opVoid();
                    _opVoidThread.Start();
                    return _opVoidThread.Task;
                }

                public Task<Test.MyClass.OpBoolReturnValue> opBoolAsync(bool p1, bool p2, Current current) =>
                    Task.FromResult(new Test.MyClass.OpBoolReturnValue(p2, p1));

                public Task<Test.MyClass.OpBoolSReturnValue> opBoolSAsync(bool[] p1, bool[] p2, Current current)
                {
                    bool[] p3 = new bool[p1.Length + p2.Length];
                    Array.Copy(p1, p3, p1.Length);
                    Array.Copy(p2, 0, p3, p1.Length, p2.Length);

                    bool[] r = new bool[p1.Length];
                    for (int i = 0; i < p1.Length; i++)
                    {
                        r[i] = p1[p1.Length - (i + 1)];
                    }

                    return Task.FromResult(new Test.MyClass.OpBoolSReturnValue(r, p3));
                }

                public Task<Test.MyClass.OpBoolSSReturnValue>
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

                    return Task.FromResult(new Test.MyClass.OpBoolSSReturnValue(r, p3));
                }

                public Task<Test.MyClass.OpByteReturnValue>
                opByteAsync(byte p1, byte p2, Current current) => Task.FromResult(new Test.MyClass.OpByteReturnValue(p1, (byte)(p1 ^ p2)));

                public Task<Test.MyClass.OpByteBoolDReturnValue>
                opByteBoolDAsync(Dictionary<byte, bool> p1, Dictionary<byte, bool> p2, Current current)
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
                    return Task.FromResult(new Test.MyClass.OpByteBoolDReturnValue(r, p3));
                }

                public Task<Test.MyClass.OpByteSReturnValue>
                opByteSAsync(byte[] p1, byte[] p2, Current current)
                {
                    byte[] p3 = new byte[p1.Length];
                    for (int i = 0; i < p1.Length; i++)
                    {
                        p3[i] = p1[p1.Length - (i + 1)];
                    }

                    byte[] r = new byte[p1.Length + p2.Length];
                    Array.Copy(p1, r, p1.Length);
                    Array.Copy(p2, 0, r, p1.Length, p2.Length);

                    return Task.FromResult(new Test.MyClass.OpByteSReturnValue(r, p3));
                }

                public Task<Test.MyClass.OpByteSSReturnValue>
                opByteSSAsync(byte[][] p1, byte[][] p2, Current current)
                {
                    byte[][] p3 = new byte[p1.Length][];
                    for (int i = 0; i < p1.Length; i++)
                    {
                        p3[i] = p1[p1.Length - (i + 1)];
                    }

                    byte[][] r = new byte[p1.Length + p2.Length][];
                    Array.Copy(p1, r, p1.Length);
                    Array.Copy(p2, 0, r, p1.Length, p2.Length);

                    return Task.FromResult(new Test.MyClass.OpByteSSReturnValue(r, p3));
                }

                public Task<Test.MyClass.OpFloatDoubleReturnValue>
                opFloatDoubleAsync(float p1, double p2, Current current) => Task.FromResult(new Test.MyClass.OpFloatDoubleReturnValue(p2, p1, p2));

                public Task<Test.MyClass.OpFloatDoubleSReturnValue>
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

                    return Task.FromResult(new Test.MyClass.OpFloatDoubleSReturnValue(r, p3, p4));
                }

                public Task<Test.MyClass.OpFloatDoubleSSReturnValue>
                opFloatDoubleSSAsync(float[][] p1, double[][] p2, Current current)
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

                    return Task.FromResult(new Test.MyClass.OpFloatDoubleSSReturnValue(r, p3, p4));
                }

                public Task<Test.MyClass.OpLongFloatDReturnValue>
                opLongFloatDAsync(Dictionary<long, float> p1, Dictionary<long, float> p2, Current current)
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

                    return Task.FromResult(new Test.MyClass.OpLongFloatDReturnValue(r, p3));
                }

                public Task<Test.MyClass.OpMyClassReturnValue>
                opMyClassAsync(Test.IMyClassPrx p1, Current current)
                {
                    var p2 = p1;
                    var p3 = Test.IMyClassPrx.UncheckedCast(current.Adapter.CreateProxy("noSuchIdentity"));
                    return Task.FromResult(new Test.MyClass.OpMyClassReturnValue(
                        Test.IMyClassPrx.UncheckedCast(current.Adapter.CreateProxy(current.Id)), p2, p3));
                }

                public Task<Test.MyClass.OpMyEnumReturnValue>
                opMyEnumAsync(Test.MyEnum p1, Current current) => Task.FromResult(new Test.MyClass.OpMyEnumReturnValue(Test.MyEnum.enum3, p1));

                public Task<Test.MyClass.OpShortIntDReturnValue>
                opShortIntDAsync(Dictionary<short, int> p1, Dictionary<short, int> p2, Current current)
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
                    return Task.FromResult(new Test.MyClass.OpShortIntDReturnValue(r, p3));
                }

                public Task<Test.MyClass.OpShortIntLongReturnValue>
                opShortIntLongAsync(short p1, int p2, long p3, Current current) =>
                    Task.FromResult(new Test.MyClass.OpShortIntLongReturnValue(p3, p1, p2, p3));

                public Task<Test.MyClass.OpShortIntLongSReturnValue>
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
                    return Task.FromResult(new Test.MyClass.OpShortIntLongSReturnValue(p3, p4, p5, p6));
                }

                public Task<Test.MyClass.OpShortIntLongSSReturnValue>
                opShortIntLongSSAsync(short[][] p1, int[][] p2, long[][] p3, Current current)
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
                    return Task.FromResult(new Test.MyClass.OpShortIntLongSSReturnValue(p3, p4, p5, p6));
                }

                public Task<Test.MyClass.OpStringReturnValue>
                opStringAsync(string p1, string p2, Current current) => Task.FromResult(new Test.MyClass.OpStringReturnValue($"{p1} {p2}", $"{p2} {p1}"));

                public Task<Test.MyClass.OpStringMyEnumDReturnValue>
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
                    return Task.FromResult(new Test.MyClass.OpStringMyEnumDReturnValue(r, p3));
                }

                public Task<Test.MyClass.OpMyEnumStringDReturnValue>
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
                    return Task.FromResult(new Test.MyClass.OpMyEnumStringDReturnValue(r, p3));
                }

                public Task<Test.MyClass.OpMyStructMyEnumDReturnValue>
                opMyStructMyEnumDAsync(Dictionary<Test.MyStruct, Test.MyEnum> p1,
                                       Dictionary<Test.MyStruct, Test.MyEnum> p2, Current current)
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
                    return Task.FromResult(new Test.MyClass.OpMyStructMyEnumDReturnValue(r, p3));
                }

                public Task<Test.MyClass.OpByteBoolDSReturnValue>
                opByteBoolDSAsync(Dictionary<byte, bool>[] p1, Dictionary<byte, bool>[] p2, Current current)
                {
                    var p3 = new Dictionary<byte, bool>[p1.Length + p2.Length];
                    Array.Copy(p2, p3, p2.Length);
                    Array.Copy(p1, 0, p3, p2.Length, p1.Length);

                    var r = new Dictionary<byte, bool>[p1.Length];
                    for (int i = 0; i < p1.Length; i++)
                    {
                        r[i] = p1[p1.Length - (i + 1)];
                    }
                    return Task.FromResult(new Test.MyClass.OpByteBoolDSReturnValue(r, p3));
                }

                public Task<Test.MyClass.OpShortIntDSReturnValue>
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
                    return Task.FromResult(new Test.MyClass.OpShortIntDSReturnValue(r, p3));
                }

                public Task<Test.MyClass.OpLongFloatDSReturnValue>
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
                    return Task.FromResult(new Test.MyClass.OpLongFloatDSReturnValue(r, p3));
                }

                public Task<Test.MyClass.OpStringStringDSReturnValue>
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
                    return Task.FromResult(new Test.MyClass.OpStringStringDSReturnValue(r, p3));
                }

                public Task<Test.MyClass.OpStringMyEnumDSReturnValue>
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
                    return Task.FromResult(new Test.MyClass.OpStringMyEnumDSReturnValue(r, p3));
                }

                public Task<Test.MyClass.OpMyEnumStringDSReturnValue>
                opMyEnumStringDSAsync(Dictionary<Test.MyEnum, string>[] p1, Dictionary<Test.MyEnum, string>[] p2, Current current)
                {
                    var p3 = new Dictionary<Test.MyEnum, string>[p1.Length + p2.Length];
                    Array.Copy(p2, p3, p2.Length);
                    Array.Copy(p1, 0, p3, p2.Length, p1.Length);

                    var r = new Dictionary<Test.MyEnum, string>[p1.Length];
                    for (int i = 0; i < p1.Length; i++)
                    {
                        r[i] = p1[p1.Length - (i + 1)];
                    }
                    return Task.FromResult(new Test.MyClass.OpMyEnumStringDSReturnValue(r, p3));
                }

                public Task<Test.MyClass.OpMyStructMyEnumDSReturnValue>
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
                    return Task.FromResult(new Test.MyClass.OpMyStructMyEnumDSReturnValue(r, p3));
                }

                public Task<Test.MyClass.OpByteByteSDReturnValue>
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
                    return Task.FromResult(new Test.MyClass.OpByteByteSDReturnValue(r, p3));
                }

                public Task<Test.MyClass.OpBoolBoolSDReturnValue>
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
                    return Task.FromResult(new Test.MyClass.OpBoolBoolSDReturnValue(r, p3));
                }

                public Task<Test.MyClass.OpShortShortSDReturnValue>
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
                    return Task.FromResult(new Test.MyClass.OpShortShortSDReturnValue(r, p3));
                }

                public Task<Test.MyClass.OpIntIntSDReturnValue>
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
                    return Task.FromResult(new Test.MyClass.OpIntIntSDReturnValue(r, p3));
                }

                public Task<Test.MyClass.OpLongLongSDReturnValue>
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
                    return Task.FromResult(new Test.MyClass.OpLongLongSDReturnValue(r, p3));
                }

                public Task<Test.MyClass.OpStringFloatSDReturnValue>
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
                    return Task.FromResult(new Test.MyClass.OpStringFloatSDReturnValue(r, p3));
                }

                public Task<Test.MyClass.OpStringDoubleSDReturnValue>
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
                    return Task.FromResult(new Test.MyClass.OpStringDoubleSDReturnValue(r, p3));
                }

                public Task<Test.MyClass.OpStringStringSDReturnValue>
                opStringStringSDAsync(Dictionary<string, string[]> p1, Dictionary<string, string[]> p2, Current current)
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
                    return Task.FromResult(new Test.MyClass.OpStringStringSDReturnValue(r, p3));
                }

                public Task<Test.MyClass.OpMyEnumMyEnumSDReturnValue>
                opMyEnumMyEnumSDAsync(Dictionary<Test.MyEnum, Test.MyEnum[]> p1, Dictionary<Test.MyEnum, Test.MyEnum[]> p2,
                    Current current)
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
                    return Task.FromResult(new Test.MyClass.OpMyEnumMyEnumSDReturnValue(r, p3));
                }

                public Task<int[]>
                opIntSAsync(int[] s, Current current)
                {
                    var r = new int[s.Length];
                    for (int i = 0; i < s.Length; ++i)
                    {
                        r[i] = -s[i];
                    }
                    return Task.FromResult(r);
                }

                public Task<Dictionary<string, string>>
                opContextAsync(Current current)
                {
                    return Task.FromResult(current.Context);
                }

                public Task
                opByteSOnewayAsync(byte[] s, Current current)
                {
                    lock (this)
                    {
                        ++_opByteSOnewayCallCount;
                    }
                    return Task.CompletedTask;
                }

                public Task<int>
                opByteSOnewayCallCountAsync(Current current)
                {
                    lock (this)
                    {
                        var count = _opByteSOnewayCallCount;
                        _opByteSOnewayCallCount = 0;
                        return Task.FromResult(count);
                    }
                }

                public Task
                opDoubleMarshalingAsync(double p1, double[] p2, Current current)
                {
                    var d = 1278312346.0 / 13.0;
                    test(p1 == d);
                    for (int i = 0; i < p2.Length; ++i)
                    {
                        test(p2[i] == d);
                    }
                    return Task.CompletedTask;
                }

                public Task<Test.MyClass.OpStringSReturnValue>
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
                    return Task.FromResult(new Test.MyClass.OpStringSReturnValue(r, p3));
                }

                public Task<Test.MyClass.OpStringSSReturnValue>
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
                    return Task.FromResult(new Test.MyClass.OpStringSSReturnValue(r, p3));
                }

                public Task<Test.MyClass.OpStringSSSReturnValue>
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
                    return Task.FromResult(new Test.MyClass.OpStringSSSReturnValue(r, p3));
                }

                public Task<Test.MyClass.OpStringStringDReturnValue>
                opStringStringDAsync(Dictionary<string, string> p1, Dictionary<string, string> p2, Current current)
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
                    return Task.FromResult(new Test.MyClass.OpStringStringDReturnValue(r, p3));
                }

                public Task<Test.MyClass.OpStructReturnValue>
                opStructAsync(Test.Structure p1, Test.Structure p2, Current current)
                {
                    var p3 = p1;
                    p3.s.s = "a new string";
                    return Task.FromResult(new Test.MyClass.OpStructReturnValue(p2, p3));
                }

                public Task
                opIdempotentAsync(Current current)
                {
                    test(current.Mode == OperationMode.Idempotent);
                    return Task.CompletedTask;
                }

                public Task
                opNonmutatingAsync(Current current)
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

                public async Task<Test.MyClass.OpMStruct1MarshaledReturnValue>
                opMStruct1Async(Current current)
                {
                    await Task.Delay(0);
                    return new Test.MyClass.OpMStruct1MarshaledReturnValue(new Test.Structure(), current);
                }

                public async Task<Test.MyClass.OpMStruct2MarshaledReturnValue>
                opMStruct2Async(Test.Structure p1, Current current)
                {
                    await Task.Delay(0);
                    return new Test.MyClass.OpMStruct2MarshaledReturnValue(p1, p1, current);
                }

                public async Task<Test.MyClass.OpMSeq1MarshaledReturnValue>
                opMSeq1Async(Current current)
                {
                    await Task.Delay(0);
                    return new Test.MyClass.OpMSeq1MarshaledReturnValue(new string[0], current);
                }

                public async Task<Test.MyClass.OpMSeq2MarshaledReturnValue>
                opMSeq2Async(string[] p1, Current current)
                {
                    await Task.Delay(0);
                    return new Test.MyClass.OpMSeq2MarshaledReturnValue(p1, p1, current);
                }

                public async Task<Test.MyClass.OpMDict1MarshaledReturnValue>
                opMDict1Async(Current current)
                {
                    await Task.Delay(0);
                    return new Test.MyClass.OpMDict1MarshaledReturnValue(new Dictionary<string, string>(), current);
                }

                public async Task<Test.MyClass.OpMDict2MarshaledReturnValue>
                opMDict2Async(Dictionary<string, string> p1, Current current)
                {
                    await Task.Delay(0);
                    return new Test.MyClass.OpMDict2MarshaledReturnValue(p1, p1, current);
                }

                private Thread_opVoid? _opVoidThread;
                private int _opByteSOnewayCallCount = 0;
            }
        }
    }
}
