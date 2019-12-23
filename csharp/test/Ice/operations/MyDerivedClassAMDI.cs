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

                public Task<(bool returnValue, bool p3)> opBoolAsync(bool p1, bool p2, Current current)
                {
                    return Task.FromResult((p2, p1));
                }

                public Task<(bool[] returnValue, bool[] p3)> opBoolSAsync(bool[] p1, bool[] p2, Current current)
                {
                    bool[] p3 = new bool[p1.Length + p2.Length];
                    Array.Copy(p1, p3, p1.Length);
                    Array.Copy(p2, 0, p3, p1.Length, p2.Length);

                    bool[] r = new bool[p1.Length];
                    for (int i = 0; i < p1.Length; i++)
                    {
                        r[i] = p1[p1.Length - (i + 1)];
                    }

                    return Task.FromResult((r, p3));
                }

                public Task<(bool[][] returnValue, bool[][] p3)>
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

                    return Task.FromResult((r, p3));
                }

                public Task<(byte returnValue, byte p3)>
                opByteAsync(byte p1, byte p2, Current current)
                {
                    return Task.FromResult((p1, (byte)(p1 ^ p2)));
                }

                public Task<(Dictionary<byte, bool> returnValue, Dictionary<byte, bool> p3)>
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

                    return Task.FromResult((r, p3));
                }

                public Task<(byte[] returnValue, byte[] p3)>
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

                    return Task.FromResult((r, p3));
                }

                public Task<(byte[][] returnValue, byte[][] p3)>
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

                    return Task.FromResult((r, p3));
                }

                public Task<(double returnValue, float p3, double p4)>
                opFloatDoubleAsync(float p1, double p2, Current current)
                {
                    return Task.FromResult((p2, p1, p2));
                }

                public Task<(double[] returnValue, float[] p3, double[] p4)>
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

                    return Task.FromResult((r, p3, p4));
                }

                public Task<(double[][] returnValue, float[][] p3, double[][] p4)>
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

                    return Task.FromResult((r, p3, p4));
                }

                public Task<(Dictionary<long, float> returnValue, Dictionary<long, float> p3)>
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

                    return Task.FromResult((r, p3));
                }

                public Task<(Test.MyClassPrx returnValue, Test.MyClassPrx p2, Test.MyClassPrx p3)>
                opMyClassAsync(Test.MyClassPrx p1, Ice.Current current)
                {
                    var p2 = p1;
                    var p3 = Test.MyClassPrx.UncheckedCast(current.Adapter.CreateProxy("noSuchIdentity"));
                    return Task.FromResult((Test.MyClassPrx.UncheckedCast(current.Adapter.CreateProxy(current.Id)), p2, p3));
                }

                public Task<(Test.MyEnum returnValue, Test.MyEnum p2)>
                opMyEnumAsync(Test.MyEnum p1, Ice.Current current)
                {
                    return Task.FromResult((Test.MyEnum.enum3, p1));
                }

                public Task<(Dictionary<short, int> returnValue, Dictionary<short, int> p3)>
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
                    return Task.FromResult((r, p3));
                }

                public Task<(long returnValue, short p4, int p5, long p6)>
                opShortIntLongAsync(short p1, int p2, long p3, Current current)
                {
                    return Task.FromResult((p3, p1, p2, p3));
                }

                public Task<(long[] returnValue, short[] p4, int[] p5, long[] p6)>
                opShortIntLongSAsync(short[] p1, int[] p2, long[] p3, Ice.Current current)
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
                    return Task.FromResult((p3, p4, p5, p6));
                }

                public Task<(long[][] returnValue, short[][] p4, int[][] p5, long[][] p6)>
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
                    return Task.FromResult((p3, p4, p5, p6));
                }

                public Task<(string returnValue, string p3)>
                opStringAsync(string p1, string p2, Ice.Current current)
                {
                    return Task.FromResult((p1 + " " + p2, p2 + " " + p1));
                }

                public Task<(Dictionary<string, Test.MyEnum> returnValue, Dictionary<string, Test.MyEnum> p3)>
                opStringMyEnumDAsync(Dictionary<string, Test.MyEnum> p1, Dictionary<string, Test.MyEnum> p2, Ice.Current current)
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
                    return Task.FromResult((r, p3));
                }

                public Task<(Dictionary<Test.MyEnum, string> returnValue, Dictionary<Test.MyEnum, string> p3)>
                opMyEnumStringDAsync(Dictionary<Test.MyEnum, string> p1, Dictionary<Test.MyEnum, string> p2, Ice.Current current)
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
                    return Task.FromResult((r, p3));
                }

                public Task<(Dictionary<Test.MyStruct, Test.MyEnum> returnValue, Dictionary<Test.MyStruct, Test.MyEnum> p3)>
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
                    return Task.FromResult((r, p3));
                }

                public Task<(Dictionary<byte, bool>[] returnValue, Dictionary<byte, bool>[] p3)>
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
                    return Task.FromResult((r, p3));
                }

                public Task<(Dictionary<short, int>[] returnValue, Dictionary<short, int>[] p3)>
                opShortIntDSAsync(Dictionary<short, int>[] p1, Dictionary<short, int>[] p2, Ice.Current current)
                {
                    var p3 = new Dictionary<short, int>[p1.Length + p2.Length];
                    Array.Copy(p2, p3, p2.Length);
                    Array.Copy(p1, 0, p3, p2.Length, p1.Length);

                    var r = new Dictionary<short, int>[p1.Length];
                    for (int i = 0; i < p1.Length; i++)
                    {
                        r[i] = p1[p1.Length - (i + 1)];
                    }
                    return Task.FromResult((r, p3));
                }

                public Task<(Dictionary<long, float>[] returnValue, Dictionary<long, float>[] p3)>
                opLongFloatDSAsync(Dictionary<long, float>[] p1, Dictionary<long, float>[] p2, Ice.Current current)
                {
                    var p3 = new Dictionary<long, float>[p1.Length + p2.Length];
                    Array.Copy(p2, p3, p2.Length);
                    Array.Copy(p1, 0, p3, p2.Length, p1.Length);

                    var r = new Dictionary<long, float>[p1.Length];
                    for (int i = 0; i < p1.Length; i++)
                    {
                        r[i] = p1[p1.Length - (i + 1)];
                    }
                    return Task.FromResult((r, p3));
                }

                public Task<(Dictionary<string, string>[] returnValue, Dictionary<string, string>[] p3)>
                opStringStringDSAsync(Dictionary<string, string>[] p1, Dictionary<string, string>[] p2, Ice.Current current)
                {
                    var p3 = new Dictionary<string, string>[p1.Length + p2.Length];
                    Array.Copy(p2, p3, p2.Length);
                    Array.Copy(p1, 0, p3, p2.Length, p1.Length);

                    var r = new Dictionary<string, string>[p1.Length];
                    for (int i = 0; i < p1.Length; i++)
                    {
                        r[i] = p1[p1.Length - (i + 1)];
                    }
                    return Task.FromResult((r, p3));
                }

                public Task<(Dictionary<string, Test.MyEnum>[] returnValue, Dictionary<string, Test.MyEnum>[] p3)>
                opStringMyEnumDSAsync(Dictionary<string, Test.MyEnum>[] p1, Dictionary<string, Test.MyEnum>[] p2, Ice.Current current)
                {
                    var p3 = new Dictionary<string, Test.MyEnum>[p1.Length + p2.Length];
                    Array.Copy(p2, p3, p2.Length);
                    Array.Copy(p1, 0, p3, p2.Length, p1.Length);

                    var r = new Dictionary<string, Test.MyEnum>[p1.Length];
                    for (int i = 0; i < p1.Length; i++)
                    {
                        r[i] = p1[p1.Length - (i + 1)];
                    }
                    return Task.FromResult((r, p3));
                }

                public Task<(Dictionary<Test.MyEnum, string>[] returnValue, Dictionary<Test.MyEnum, string>[] p3)>
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
                    return Task.FromResult((r, p3));
                }

                public Task<(Dictionary<Test.MyStruct, Test.MyEnum>[] returnValue, Dictionary<Test.MyStruct, Test.MyEnum>[] p3)>
                opMyStructMyEnumDSAsync(Dictionary<Test.MyStruct, Test.MyEnum>[] p1,
                                        Dictionary<Test.MyStruct, Test.MyEnum>[] p2,
                                        Ice.Current current)
                {
                    var p3 = new Dictionary<Test.MyStruct, Test.MyEnum>[p1.Length + p2.Length];
                    Array.Copy(p2, p3, p2.Length);
                    Array.Copy(p1, 0, p3, p2.Length, p1.Length);

                    var r = new Dictionary<Test.MyStruct, Test.MyEnum>[p1.Length];
                    for (int i = 0; i < p1.Length; i++)
                    {
                        r[i] = p1[p1.Length - (i + 1)];
                    }
                    return Task.FromResult((r, p3));
                }

                public Task<(Dictionary<byte, byte[]> returnValue, Dictionary<byte, byte[]> p3)>
                opByteByteSDAsync(Dictionary<byte, byte[]> p1, Dictionary<byte, byte[]> p2, Ice.Current current)
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
                    return Task.FromResult((r, p3));
                }

                public Task<(Dictionary<bool, bool[]> returnValue, Dictionary<bool, bool[]> p3)>
                opBoolBoolSDAsync(Dictionary<bool, bool[]> p1, Dictionary<bool, bool[]> p2, Ice.Current current)
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
                    return Task.FromResult((r, p3));
                }

                public Task<(Dictionary<short, short[]> returnValue, Dictionary<short, short[]> p3)>
                opShortShortSDAsync(Dictionary<short, short[]> p1, Dictionary<short, short[]> p2, Ice.Current current)
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
                    return Task.FromResult((r, p3));
                }

                public Task<(Dictionary<int, int[]> returnValue, Dictionary<int, int[]> p3)>
                opIntIntSDAsync(Dictionary<int, int[]> p1, Dictionary<int, int[]> p2, Ice.Current current)
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
                    return Task.FromResult((r, p3));
                }

                public Task<(Dictionary<long, long[]> returnValue, Dictionary<long, long[]> p3)>
                opLongLongSDAsync(Dictionary<long, long[]> p1, Dictionary<long, long[]> p2, Ice.Current current)
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
                    return Task.FromResult((r, p3));
                }

                public Task<(Dictionary<string, float[]> returnValue, Dictionary<string, float[]> p3)>
                opStringFloatSDAsync(Dictionary<string, float[]> p1, Dictionary<string, float[]> p2, Ice.Current current)
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
                    return Task.FromResult((r, p3));
                }

                public Task<(Dictionary<string, double[]> returnValue, Dictionary<string, double[]> p3)>
                opStringDoubleSDAsync(Dictionary<string, double[]> p1, Dictionary<string, double[]> p2, Ice.Current current)
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
                    return Task.FromResult((r, p3));
                }

                public Task<(Dictionary<string, string[]> returnValue, Dictionary<string, string[]> p3)>
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
                    return Task.FromResult((r, p3));
                }

                public Task<(Dictionary<Test.MyEnum, Test.MyEnum[]> returnValue, Dictionary<Test.MyEnum, Test.MyEnum[]> p3)>
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
                    return Task.FromResult((r, p3));
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

                public Task<(string[] returnValue, string[] p3)>
                opStringSAsync(string[] p1, string[] p2, Ice.Current current)
                {
                    var p3 = new string[p1.Length + p2.Length];
                    Array.Copy(p1, p3, p1.Length);
                    Array.Copy(p2, 0, p3, p1.Length, p2.Length);

                    var r = new string[p1.Length];
                    for (int i = 0; i < p1.Length; i++)
                    {
                        r[i] = p1[p1.Length - (i + 1)];
                    }
                    return Task.FromResult((r, p3));
                }

                public Task<(string[][] returnValue, string[][] p3)>
                opStringSSAsync(string[][] p1, string[][] p2, Ice.Current current)
                {
                    var p3 = new string[p1.Length + p2.Length][];
                    Array.Copy(p1, p3, p1.Length);
                    Array.Copy(p2, 0, p3, p1.Length, p2.Length);
                    var r = new string[p2.Length][];
                    for (int i = 0; i < p2.Length; i++)
                    {
                        r[i] = p2[p2.Length - (i + 1)];
                    }
                    return Task.FromResult((r, p3));
                }

                public Task<(string[][][] returnValue, string[][][] p3)>
                opStringSSSAsync(string[][][] p1, string[][][] p2, Ice.Current current)
                {
                    var p3 = new string[p1.Length + p2.Length][][];
                    Array.Copy(p1, p3, p1.Length);
                    Array.Copy(p2, 0, p3, p1.Length, p2.Length);

                    var r = new string[p2.Length][][];
                    for (int i = 0; i < p2.Length; i++)
                    {
                        r[i] = p2[p2.Length - (i + 1)];
                    }
                    return Task.FromResult((r, p3));
                }

                public Task<(Dictionary<string, string> returnValue, Dictionary<string, string> p3)>
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
                    return Task.FromResult((r, p3));
                }

                public Task<(Test.Structure returnValue, Test.Structure p3)>
                opStructAsync(Test.Structure p1, Test.Structure p2, Ice.Current current)
                {
                    var p3 = p1;
                    p3.s.s = "a new string";
                    return Task.FromResult((p2, p3));
                }

                public Task
                opIdempotentAsync(Ice.Current current)
                {
                    test(current.Mode == Ice.OperationMode.Idempotent);
                    return null;
                }

                public Task
                opNonmutatingAsync(Ice.Current current)
                {
                    test(current.Mode == Ice.OperationMode.Nonmutating);
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
                    return Task.FromResult(value);
                }

                public Task<short>
                opShort1Async(short value, Ice.Current current)
                {
                    return Task.FromResult(value);
                }

                public Task<int>
                opInt1Async(int value, Ice.Current current)
                {
                    return Task.FromResult<int>(value);
                }

                public Task<long>
                opLong1Async(long value, Ice.Current current)
                {
                    return Task.FromResult(value);
                }

                public Task<float>
                opFloat1Async(float value, Ice.Current current)
                {
                    return Task.FromResult(value);
                }

                public Task<double>
                opDouble1Async(double value, Ice.Current current)
                {
                    return Task.FromResult(value);
                }

                public Task<string>
                opString1Async(string value, Ice.Current current)
                {
                    return Task.FromResult<string>(value);
                }

                public Task<string[]>
                opStringS1Async(string[] value, Ice.Current current)
                {
                    return Task.FromResult(value);
                }

                public Task<Dictionary<byte, bool>>
                opByteBoolD1Async(Dictionary<byte, bool> value, Ice.Current current)
                {
                    return Task.FromResult(value);
                }

                public Task<string[]>
                opStringS2Async(string[] value, Ice.Current current)
                {
                    return Task.FromResult(value);
                }

                public Task<Dictionary<byte, bool>>
                opByteBoolD2Async(Dictionary<byte, bool> value, Ice.Current current)
                {
                    return Task.FromResult(value);
                }

                public Task<Test.MyClass1>
                opMyClass1Async(Test.MyClass1 value, Ice.Current current)
                {
                    return Task.FromResult<Test.MyClass1>(value);
                }

                public Task<Test.MyStruct1>
                opMyStruct1Async(Test.MyStruct1 value, Ice.Current current)
                {
                    return Task.FromResult(value);
                }

                public Task<string[]>
                opStringLiteralsAsync(Ice.Current current)
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
                opWStringLiteralsAsync(Ice.Current current)
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

                private Thread_opVoid? _opVoidThread;
                private int _opByteSOnewayCallCount = 0;
            }
        }
    }
}
