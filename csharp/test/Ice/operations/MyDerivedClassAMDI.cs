//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading;
using System.Threading.Tasks;
using Test;

namespace Ice.operations.AMD
{
    public sealed class MyDerivedClass : Test.IMyDerivedClass
    {

        internal class Thread_opVoid : TaskCompletionSource<object?>
        {
            public void Start()
            {
                lock (this)
                {
                    _thread = new Thread(new ThreadStart(Run));
                    _thread.Start();
                }
            }

            public void Run() => SetResult(null);

            public void Join()
            {
                lock (this)
                {
                    _thread!.Join();
                }
            }

            private Thread? _thread;
        }

        //
        // Override the Object "pseudo" operations to verify the operation mode.
        //
        public bool IceIsA(string id, Current current)
        {
            TestHelper.Assert(current.IsIdempotent);
            return typeof(Test.IMyDerivedClass).GetAllIceTypeIds().Contains(id);
        }

        public void IcePing(Current current) => TestHelper.Assert(current.IsIdempotent);

        public IEnumerable<string> IceIds(Current current)
        {
            TestHelper.Assert(current.IsIdempotent);
            return typeof(Test.IMyDerivedClass).GetAllIceTypeIds();
        }

        public string IceId(Current current)
        {
            TestHelper.Assert(current.IsIdempotent);
            return typeof(Test.IMyDerivedClass).GetIceTypeId()!;
        }

        public ValueTask shutdownAsync(Current current)
        {
            while (_opVoidThread != null)
            {
                _opVoidThread.Join();
                _opVoidThread = null;
            }

            current.Adapter.Communicator.Shutdown();
            return new ValueTask(Task.CompletedTask);
        }

        public ValueTask<bool> supportsCompressAsync(Current current) => MakeValueTask(true);

        public ValueTask opVoidAsync(Current current)
        {
            TestHelper.Assert(!current.IsIdempotent);

            while (_opVoidThread != null)
            {
                _opVoidThread.Join();
                _opVoidThread = null;
            }

            _opVoidThread = new Thread_opVoid();
            _opVoidThread.Start();
            return new ValueTask(_opVoidThread.Task);
        }

        public ValueTask<(bool, bool)> opBoolAsync(bool p1, bool p2, Current current) => MakeValueTask((p2, p1));

        public ValueTask<(ReadOnlyMemory<bool>, ReadOnlyMemory<bool>)> opBoolSAsync(bool[] p1, bool[] p2,
            Current current)
        {
            bool[] p3 = new bool[p1.Length + p2.Length];
            Array.Copy(p1, p3, p1.Length);
            Array.Copy(p2, 0, p3, p1.Length, p2.Length);

            bool[] r = new bool[p1.Length];
            for (int i = 0; i < p1.Length; i++)
            {
                r[i] = p1[p1.Length - (i + 1)];
            }

            return ToReturnValue(r, p3);
        }

        public ValueTask<(IEnumerable<bool[]>, IEnumerable<bool[]>)>
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

            return ToReturnValue(r, p3);
        }

        public ValueTask<(byte, byte)> opByteAsync(byte p1, byte p2, Current current) => MakeValueTask((p1, (byte)(p1 ^ p2)));

        public ValueTask<(IReadOnlyDictionary<byte, bool>, IReadOnlyDictionary<byte, bool>)>
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
            return ToReturnValue(r, p3);
        }

        public ValueTask<(ReadOnlyMemory<byte>, ReadOnlyMemory<byte>)> opByteSAsync(byte[] p1, byte[] p2,
            Current current)
        {
            byte[] p3 = new byte[p1.Length];
            for (int i = 0; i < p1.Length; i++)
            {
                p3[i] = p1[p1.Length - (i + 1)];
            }

            byte[] r = new byte[p1.Length + p2.Length];
            Array.Copy(p1, r, p1.Length);
            Array.Copy(p2, 0, r, p1.Length, p2.Length);

            return ToReturnValue(r, p3);
        }

        public ValueTask<(IEnumerable<byte[]>, IEnumerable<byte[]>)>
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

            return ToReturnValue(r, p3);
        }

        public ValueTask<(double, float, double)>
        opFloatDoubleAsync(float p1, double p2, Current current) => MakeValueTask((p2, p1, p2));

        public ValueTask<(ReadOnlyMemory<double>, ReadOnlyMemory<float>, ReadOnlyMemory<double>)> opFloatDoubleSAsync(
            float[] p1, double[] p2, Current current)
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

            return MakeValueTask(((ReadOnlyMemory<double>)r, (ReadOnlyMemory<float>)p3, (ReadOnlyMemory<double>)p4));
        }

        public ValueTask<(IEnumerable<double[]>, IEnumerable<float[]>, IEnumerable<double[]>)>
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

            return MakeValueTask((r as IEnumerable<double[]>, p3 as IEnumerable<float[]>, p4 as IEnumerable<double[]>));
        }

        public ValueTask<(IReadOnlyDictionary<long, float>, IReadOnlyDictionary<long, float>)>
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

            return ToReturnValue(r, p3);
        }

        public ValueTask<(Test.IMyClassPrx?, Test.IMyClassPrx?, Test.IMyClassPrx?)>
        opMyClassAsync(Test.IMyClassPrx? p1, Current current)
        {
            var p2 = p1;
            var p3 = current.Adapter.CreateProxy("noSuchIdentity", Test.IMyClassPrx.Factory);
            return new ValueTask<(Test.IMyClassPrx?, Test.IMyClassPrx?, Test.IMyClassPrx?)>((
                current.Adapter.CreateProxy(current.Identity, Test.IMyClassPrx.Factory), p2, p3));
        }

        public ValueTask<(Test.MyEnum, Test.MyEnum)>
        opMyEnumAsync(Test.MyEnum p1, Current current) => MakeValueTask((Test.MyEnum.enum3, p1));

        public ValueTask<(IReadOnlyDictionary<short, int>, IReadOnlyDictionary<short, int>)>
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
            return ToReturnValue(r, p3);
        }

        public ValueTask<(long, short, int, long)>
        opShortIntLongAsync(short p1, int p2, long p3, Current current) => MakeValueTask((p3, p1, p2, p3));

        public ValueTask<(ReadOnlyMemory<long>, ReadOnlyMemory<short>, ReadOnlyMemory<int>, ReadOnlyMemory<long>)>
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
            return MakeValueTask(((ReadOnlyMemory<long>)p3, (ReadOnlyMemory<short>)p4, (ReadOnlyMemory<int>)p5,
                (ReadOnlyMemory<long>)p6));
        }

        public ValueTask<(IEnumerable<long[]>, IEnumerable<short[]>, IEnumerable<int[]>, IEnumerable<long[]>)>
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
            return MakeValueTask((p3 as IEnumerable<long[]>, p4 as IEnumerable<short[]>, p5 as IEnumerable<int[]>,
                p6 as IEnumerable<long[]>));
        }

        public ValueTask<(string, string)>
        opStringAsync(string p1, string p2, Current current) => MakeValueTask(($"{p1} {p2}", $"{p2} {p1}"));

        public ValueTask<(IReadOnlyDictionary<string, Test.MyEnum>, IReadOnlyDictionary<string, Test.MyEnum>)>
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
            return ToReturnValue(r, p3);
        }

        public ValueTask<(IReadOnlyDictionary<Test.MyEnum, string>, IReadOnlyDictionary<Test.MyEnum, string>)>
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
            return ToReturnValue(r, p3);
        }

        public ValueTask<(IReadOnlyDictionary<Test.MyStruct, Test.MyEnum>, IReadOnlyDictionary<Test.MyStruct, Test.MyEnum>)>
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
            return ToReturnValue(r, p3);
        }

        public ValueTask<(IEnumerable<Dictionary<byte, bool>>, IEnumerable<Dictionary<byte, bool>>)>
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
            return ToReturnValue(r, p3);
        }

        public ValueTask<(IEnumerable<Dictionary<short, int>>, IEnumerable<Dictionary<short, int>>)>
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
            return ToReturnValue(r, p3);
        }

        public ValueTask<(IEnumerable<Dictionary<long, float>>, IEnumerable<Dictionary<long, float>>)>
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
            return ToReturnValue(r, p3);
        }

        public ValueTask<(IEnumerable<Dictionary<string, string>>, IEnumerable<Dictionary<string, string>>)>
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
            return ToReturnValue(r, p3);
        }

        public ValueTask<(IEnumerable<Dictionary<string, Test.MyEnum>>, IEnumerable<Dictionary<string, Test.MyEnum>>)>
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
            return ToReturnValue(r, p3);
        }

        public ValueTask<(IEnumerable<Dictionary<Test.MyEnum, string>>, IEnumerable<Dictionary<Test.MyEnum, string>>)>
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
            return ToReturnValue(r, p3);
        }

        public ValueTask<(IEnumerable<Dictionary<Test.MyStruct, Test.MyEnum>>, IEnumerable<Dictionary<Test.MyStruct, Test.MyEnum>>)>
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
            return ToReturnValue(r, p3);
        }

        public ValueTask<(IReadOnlyDictionary<byte, byte[]>, IReadOnlyDictionary<byte, byte[]>)>
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
            return ToReturnValue(r, p3);
        }

        public ValueTask<(IReadOnlyDictionary<bool, bool[]>, IReadOnlyDictionary<bool, bool[]>)>
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
            return ToReturnValue(r, p3);
        }

        public ValueTask<(IReadOnlyDictionary<short, short[]>, IReadOnlyDictionary<short, short[]>)>
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
            return ToReturnValue(r, p3);
        }

        public ValueTask<(IReadOnlyDictionary<int, int[]>, IReadOnlyDictionary<int, int[]>)>
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
            return ToReturnValue(r, p3);
        }

        public ValueTask<(IReadOnlyDictionary<long, long[]>, IReadOnlyDictionary<long, long[]>)>
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
            return ToReturnValue(r, p3);
        }

        public ValueTask<(IReadOnlyDictionary<string, float[]>, IReadOnlyDictionary<string, float[]>)>
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
            return ToReturnValue(r, p3);
        }

        public ValueTask<(IReadOnlyDictionary<string, double[]>, IReadOnlyDictionary<string, double[]>)>
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
            return ToReturnValue(r, p3);
        }

        public ValueTask<(IReadOnlyDictionary<string, string[]>, IReadOnlyDictionary<string, string[]>)>
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
            return ToReturnValue(r, p3);
        }

        public ValueTask<(IReadOnlyDictionary<Test.MyEnum, Test.MyEnum[]>, IReadOnlyDictionary<Test.MyEnum, Test.MyEnum[]>)>
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
            return ToReturnValue(r, p3);
        }

        public ValueTask<ReadOnlyMemory<int>> opIntSAsync(int[] s, Current current)
        {
            var r = new int[s.Length];
            for (int i = 0; i < s.Length; ++i)
            {
                r[i] = -s[i];
            }
            return MakeValueTask((ReadOnlyMemory<int>)r);
        }

        public ValueTask<IReadOnlyDictionary<string, string>> opContextAsync(Current current) =>
            MakeValueTask(current.Context as IReadOnlyDictionary<string, string>);

        public ValueTask
        opByteSOnewayAsync(byte[] s, Current current)
        {
            lock (this)
            {
                ++_opByteSOnewayCallCount;
            }
            return new ValueTask(Task.CompletedTask);
        }

        public ValueTask<int>
        opByteSOnewayCallCountAsync(Current current)
        {
            lock (this)
            {
                var count = _opByteSOnewayCallCount;
                _opByteSOnewayCallCount = 0;
                return MakeValueTask(count);
            }
        }

        public ValueTask
        opDoubleMarshalingAsync(double p1, double[] p2, Current current)
        {
            var d = 1278312346.0 / 13.0;
            TestHelper.Assert(p1 == d);
            for (int i = 0; i < p2.Length; ++i)
            {
                TestHelper.Assert(p2[i] == d);
            }
            return new ValueTask(Task.CompletedTask);
        }

        public ValueTask<(IEnumerable<string>, IEnumerable<string>)>
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
            return ToReturnValue(r, p3);
        }

        public ValueTask<(IEnumerable<string[]>, IEnumerable<string[]>)>
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
            return ToReturnValue(r, p3);
        }

        public ValueTask<(IEnumerable<string[][]>, IEnumerable<string[][]>)>
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
            return ToReturnValue(r, p3);
        }

        public ValueTask<(IReadOnlyDictionary<string, string>, IReadOnlyDictionary<string, string>)>
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
            return ToReturnValue(r, p3);
        }

        public ValueTask<(Test.Structure, Test.Structure)>
        opStructAsync(Test.Structure p1, Test.Structure p2, Current current)
        {
            var p3 = p1;
            p3.s.s = "a new string";
            return MakeValueTask((p2, p3));
        }

        public ValueTask
        opIdempotentAsync(Current current)
        {
            TestHelper.Assert(current.IsIdempotent);
            return new ValueTask(Task.CompletedTask);
        }

        public ValueTask opOnewayAsync(Current current)
        {
            // "return" exception when called two-way, otherwise succeeds.
            throw new Test.SomeException();
        }

        public ValueTask
        opDerivedAsync(Current current) => new ValueTask(Task.CompletedTask);

        public ValueTask<byte>
        opByte1Async(byte value, Current current) => MakeValueTask(value);

        public ValueTask<short>
        opShort1Async(short value, Current current) => MakeValueTask(value);

        public ValueTask<int>
        opInt1Async(int value, Current current) => MakeValueTask(value);

        public ValueTask<long>
        opLong1Async(long value, Current current) => MakeValueTask(value);

        public ValueTask<float>
        opFloat1Async(float value, Current current) => MakeValueTask(value);

        public ValueTask<double>
        opDouble1Async(double value, Current current) => MakeValueTask(value);

        public ValueTask<string>
        opString1Async(string value, Current current) => MakeValueTask(value);

        public ValueTask<IEnumerable<string>>
        opStringS1Async(string[] value, Current current) => MakeValueTask(value as IEnumerable<string>);

        public ValueTask<IReadOnlyDictionary<byte, bool>>
        opByteBoolD1Async(Dictionary<byte, bool> value, Current current) =>
            MakeValueTask(value as IReadOnlyDictionary<byte, bool>);

        public ValueTask<IEnumerable<string>>
        opStringS2Async(string[] value, Current current) => MakeValueTask(value as IEnumerable<string>);

        public ValueTask<IReadOnlyDictionary<byte, bool>>
        opByteBoolD2Async(Dictionary<byte, bool> value, Current current) =>
            MakeValueTask(value as IReadOnlyDictionary<byte, bool>);

        public ValueTask<Test.MyClass1?>
        opMyClass1Async(Test.MyClass1? value, Current current) => MakeValueTask(value);

        public ValueTask<Test.MyStruct1>
        opMyStruct1Async(Test.MyStruct1 value, Current current) => MakeValueTask(value);

        public ValueTask<IEnumerable<string>>
        opStringLiteralsAsync(Current current)
        {
            return MakeValueTask(new string[]
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
                } as IEnumerable<string>);
        }

        public ValueTask<IEnumerable<string>>
        opWStringLiteralsAsync(Current current)
        {
            return MakeValueTask(new string[]
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
                } as IEnumerable<string>);
        }

        public async ValueTask<Test.IMyClass.OpMStruct1MarshaledReturnValue>
        opMStruct1Async(Current current)
        {
            await Task.Delay(0);
            return new Test.IMyClass.OpMStruct1MarshaledReturnValue(
                new Test.Structure(null, Test.MyEnum.enum1, new Test.AnotherStruct("")), current);
        }

        public async ValueTask<Test.IMyClass.OpMStruct2MarshaledReturnValue>
        opMStruct2Async(Test.Structure p1, Current current)
        {
            await Task.Delay(0);
            return new Test.IMyClass.OpMStruct2MarshaledReturnValue(p1, p1, current);
        }

        public async ValueTask<Test.IMyClass.OpMSeq1MarshaledReturnValue>
        opMSeq1Async(Current current)
        {
            await Task.Delay(0);
            return new Test.IMyClass.OpMSeq1MarshaledReturnValue(new string[0], current);
        }

        public async ValueTask<Test.IMyClass.OpMSeq2MarshaledReturnValue>
        opMSeq2Async(string[] p1, Current current)
        {
            await Task.Delay(0);
            return new Test.IMyClass.OpMSeq2MarshaledReturnValue(p1, p1, current);
        }

        public async ValueTask<Test.IMyClass.OpMDict1MarshaledReturnValue>
        opMDict1Async(Current current)
        {
            await Task.Delay(0);
            return new Test.IMyClass.OpMDict1MarshaledReturnValue(new Dictionary<string, string>(), current);
        }

        public async ValueTask<Test.IMyClass.OpMDict2MarshaledReturnValue>
        opMDict2Async(Dictionary<string, string> p1, Current current)
        {
            await Task.Delay(0);
            return new Test.IMyClass.OpMDict2MarshaledReturnValue(p1, p1, current);
        }

        // Type-inference helper method
        private static ValueTask<T> MakeValueTask<T>(T result) => new ValueTask<T>(result);

        private static ValueTask<(ReadOnlyMemory<T>, ReadOnlyMemory<T>)> ToReturnValue<T>(T[] input1,
            T[] input2) where T : struct =>
                new ValueTask<(ReadOnlyMemory<T>, ReadOnlyMemory<T>)>((input1, input2));

        private static ValueTask<(IEnumerable<T>, IEnumerable<T>)> ToReturnValue<T>(IEnumerable<T> input1,
            IEnumerable<T> input2) => new ValueTask<(IEnumerable<T>, IEnumerable<T>)>((input1, input2));

        private static ValueTask<(IReadOnlyDictionary<TKey, TValue>, IReadOnlyDictionary<TKey, TValue>)>
        ToReturnValue<TKey, TValue>(IReadOnlyDictionary<TKey, TValue> input1,
            IReadOnlyDictionary<TKey, TValue> input2) where TKey : notnull =>
            new ValueTask<(IReadOnlyDictionary<TKey, TValue>, IReadOnlyDictionary<TKey, TValue>)>((input1, input2));

        private Thread_opVoid? _opVoidThread;
        private int _opByteSOnewayCallCount = 0;
    }
}
