//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Linq;
using System.Collections.Generic;
using Test;

namespace ZeroC.Ice.Test.Operations
{
    public sealed class MyDerivedClass : IMyDerivedClass, IObject
    {
        // Override the Object "pseudo" operations to verify the operation mode.
        public bool IceIsA(string id, Current current)
        {
            TestHelper.Assert(current.IsIdempotent);
            return typeof(IMyDerivedClass).GetAllIceTypeIds().Contains(id);
        }

        public void IcePing(Current current) => TestHelper.Assert(current.IsIdempotent);

        public IEnumerable<string> IceIds(Current current)
        {
            TestHelper.Assert(current.IsIdempotent);
            return typeof(IMyDerivedClass).GetAllIceTypeIds();
        }

        public string IceId(Current current)
        {
            TestHelper.Assert(current.IsIdempotent);
            return typeof(IMyDerivedClass).GetIceTypeId()!;
        }

        public void shutdown(Current current) => current.Adapter.Communicator.Shutdown();

        // TODO check if compress is supported.
        public bool supportsCompress(Current current) => false;

        public void opVoid(Current current) => TestHelper.Assert(!current.IsIdempotent);

        public (bool, bool) opBool(bool p1, bool p2, Current current) => (p2, p1);

        public (ReadOnlyMemory<bool>, ReadOnlyMemory<bool>) opBoolS(bool[] p1, bool[] p2, Current current)
        {
            var p3 = new bool[p1.Length + p2.Length];
            Array.Copy(p1, p3, p1.Length);
            Array.Copy(p2, 0, p3, p1.Length, p2.Length);

            bool[] r = new bool[p1.Length];
            for (int i = 0; i < p1.Length; i++)
            {
                r[i] = p1[p1.Length - (i + 1)];
            }
            return (r, p3);
        }

        public (IEnumerable<bool[]>, IEnumerable<bool[]>) opBoolSS(bool[][] p1, bool[][] p2, Current current)
        {
            var p3 = new bool[p1.Length + p2.Length][];
            Array.Copy(p1, p3, p1.Length);
            Array.Copy(p2, 0, p3, p1.Length, p2.Length);

            bool[][] r = new bool[p1.Length][];
            for (int i = 0; i < p1.Length; i++)
            {
                r[i] = p1[p1.Length - (i + 1)];
            }
            return (r, p3);
        }

        public (byte, byte) opByte(byte p1, byte p2, Current current) => (p1, (byte)(p1 ^ p2));

        public (IReadOnlyDictionary<byte, bool>, IReadOnlyDictionary<byte, bool>) opByteBoolD(
            Dictionary<byte, bool> p1, Dictionary<byte, bool> p2, Current current)
        {
            var r = new Dictionary<byte, bool>();
            foreach (KeyValuePair<byte, bool> e in p1)
            {
                r[e.Key] = e.Value;
            }
            foreach (KeyValuePair<byte, bool> e in p2)
            {
                r[e.Key] = e.Value;
            }
            return (r, p1);
        }

        public (ReadOnlyMemory<byte>, ReadOnlyMemory<byte>) opByteS(byte[] p1, byte[] p2, Current current)
        {
            var p3 = new byte[p1.Length];
            for (int i = 0; i < p1.Length; i++)
            {
                p3[i] = p1[p1.Length - (i + 1)];
            }

            byte[] r = new byte[p1.Length + p2.Length];
            Array.Copy(p1, r, p1.Length);
            Array.Copy(p2, 0, r, p1.Length, p2.Length);
            return (r, p3);
        }

        public (IEnumerable<byte[]>, IEnumerable<byte[]>) opByteSS(byte[][] p1, byte[][] p2, Current current)
        {
            var p3 = new byte[p1.Length][];
            for (int i = 0; i < p1.Length; i++)
            {
                p3[i] = p1[p1.Length - (i + 1)];
            }

            byte[][] r = new byte[p1.Length + p2.Length][];
            Array.Copy(p1, r, p1.Length);
            Array.Copy(p2, 0, r, p1.Length, p2.Length);
            return (r, p3);
        }

        public (double, float, double) opFloatDouble(float p1, double p2, Current current) => (p2, p1, p2);

        public (ReadOnlyMemory<double>, ReadOnlyMemory<float>, ReadOnlyMemory<double>) opFloatDoubleS(float[] p1,
            double[] p2, Current current)
        {
            var p4 = new double[p2.Length];
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
            return (r, p1, p4);
        }

        public (IEnumerable<double[]>, IEnumerable<float[]>, IEnumerable<double[]>) opFloatDoubleSS(float[][] p1,
            double[][] p2, Current current)
        {
            var p4 = new double[p2.Length][];
            for (int i = 0; i < p2.Length; i++)
            {
                p4[i] = p2[p2.Length - (i + 1)];
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
            return (r, p1, p4);
        }

        public (IReadOnlyDictionary<long, float>, IReadOnlyDictionary<long, float>) opLongFloatD(
            Dictionary<long, float> p1, Dictionary<long, float> p2, Current current)
        {
            var r = new Dictionary<long, float>();
            foreach (KeyValuePair<long, float> e in p1)
            {
                r[e.Key] = e.Value;
            }
            foreach (KeyValuePair<long, float> e in p2)
            {
                r[e.Key] = e.Value;
            }
            return (r, p1);
        }

        public (IReadOnlyDictionary<ulong, float>, IReadOnlyDictionary<ulong, float>) opULongFloatD(
            Dictionary<ulong, float> p1, Dictionary<ulong, float> p2, Current current)
        {
            var r = new Dictionary<ulong, float>();
            foreach (KeyValuePair<ulong, float> e in p1)
            {
                r[e.Key] = e.Value;
            }
            foreach (KeyValuePair<ulong, float> e in p2)
            {
                r[e.Key] = e.Value;
            }
            return (r, p1);
        }

        public (IMyClassPrx?, IMyClassPrx?, IMyClassPrx?) opMyClass(IMyClassPrx? p1,
            Current current) =>
            (current.Adapter.CreateProxy(current.Identity, IMyClassPrx.Factory),
            p1,
            current.Adapter.CreateProxy("noSuchIdentity", IMyClassPrx.Factory));

        public (MyEnum, MyEnum) opMyEnum(MyEnum p1, Current current) => (MyEnum.enum3, p1);

        public (IReadOnlyDictionary<short, int>, IReadOnlyDictionary<short, int>) opShortIntD(Dictionary<short, int> p1,
            Dictionary<short, int> p2, Current current)
        {
            var r = new Dictionary<short, int>();
            foreach (KeyValuePair<short, int> e in p1)
            {
                r[e.Key] = e.Value;
            }
            foreach (KeyValuePair<short, int> e in p2)
            {
                r[e.Key] = e.Value;
            }
            return (r, p1);
        }

        public (IReadOnlyDictionary<ushort, uint>, IReadOnlyDictionary<ushort, uint>) opUShortUIntD(
            Dictionary<ushort, uint> p1,
            Dictionary<ushort, uint> p2,
            Current current)
        {
            var r = new Dictionary<ushort, uint>();
            foreach (KeyValuePair<ushort, uint> e in p1)
            {
                r[e.Key] = e.Value;
            }
            foreach (KeyValuePair<ushort, uint> e in p2)
            {
                r[e.Key] = e.Value;
            }
            return (r, p1);
        }

        public (long, short, int, long) opShortIntLong(short p1, int p2, long p3, Current current) => (p3, p1, p2, p3);

        public (ulong, ushort, uint, ulong) opUShortUIntULong(ushort p1, uint p2, ulong p3, Current current) =>
            (p3, p1, p2, p3);

        public int opVarInt(int v, Current current) => v;
        public uint opVarUInt(uint v, Current current) => v;
        public long opVarLong(long v, Current current) => v;
        public ulong opVarULong(ulong v, Current current) => v;

        public (ReadOnlyMemory<long>, ReadOnlyMemory<short>, ReadOnlyMemory<int>, ReadOnlyMemory<long>)
        opShortIntLongS(short[] p1, int[] p2, long[] p3, Current current)
        {
            var p5 = new int[p2.Length];
            for (int i = 0; i < p2.Length; i++)
            {
                p5[i] = p2[p2.Length - (i + 1)];
            }

            var p6 = new long[p3.Length + p3.Length];
            Array.Copy(p3, p6, p3.Length);
            Array.Copy(p3, 0, p6, p3.Length, p3.Length);

            return (p3, p1, p5, p6);
        }

        public (ReadOnlyMemory<ulong>, ReadOnlyMemory<ushort>, ReadOnlyMemory<uint>, ReadOnlyMemory<ulong>)
        opUShortUIntULongS(ushort[] p1, uint[] p2, ulong[] p3, Current current)
        {
            var p5 = new uint[p2.Length];
            for (int i = 0; i < p2.Length; i++)
            {
                p5[i] = p2[p2.Length - (i + 1)];
            }

            var p6 = new ulong[p3.Length + p3.Length];
            Array.Copy(p3, p6, p3.Length);
            Array.Copy(p3, 0, p6, p3.Length, p3.Length);

            return (p3, p1, p5, p6);
        }

        public (IEnumerable<long>, IEnumerable<int>, IEnumerable<long>)
        opVarIntVarLongS(int[] p1, long[] p2, Current current)
        {
            var p4 = new int[p1.Length];
            for (int i = 0; i < p1.Length; i++)
            {
                p4[i] = p1[p1.Length - (i + 1)];
            }

            var p5 = new long[p2.Length + p2.Length];
            Array.Copy(p2, p5, p2.Length);
            Array.Copy(p2, 0, p5, p2.Length, p2.Length);

            return (p2, p4, p5);
        }

        public (IEnumerable<ulong>, IEnumerable<uint>, IEnumerable<ulong>)
        opVarUIntVarULongS(uint[] p1, ulong[] p2, Current current)
        {
            var p4 = new uint[p1.Length];
            for (int i = 0; i < p1.Length; i++)
            {
                p4[i] = p1[p1.Length - (i + 1)];
            }

            var p5 = new ulong[p2.Length + p2.Length];
            Array.Copy(p2, p5, p2.Length);
            Array.Copy(p2, 0, p5, p2.Length, p2.Length);

            return (p2, p4, p5);
        }

        public (IEnumerable<long[]>, IEnumerable<short[]>, IEnumerable<int[]>, IEnumerable<long[]>) opShortIntLongSS(
            short[][] p1, int[][] p2, long[][] p3, Current current)
        {
            var p5 = new int[p2.Length][];
            for (int i = 0; i < p2.Length; i++)
            {
                p5[i] = p2[p2.Length - (i + 1)];
            }

            var p6 = new long[p3.Length + p3.Length][];
            Array.Copy(p3, p6, p3.Length);
            Array.Copy(p3, 0, p6, p3.Length, p3.Length);

            return (p3, p1, p5, p6);
        }

        public (IEnumerable<ulong[]>, IEnumerable<ushort[]>, IEnumerable<uint[]>, IEnumerable<ulong[]>)
        opUShortUIntULongSS(ushort[][] p1, uint[][] p2, ulong[][] p3, Current current)
        {
            var p5 = new uint[p2.Length][];
            for (int i = 0; i < p2.Length; i++)
            {
                p5[i] = p2[p2.Length - (i + 1)];
            }

            var p6 = new ulong[p3.Length + p3.Length][];
            Array.Copy(p3, p6, p3.Length);
            Array.Copy(p3, 0, p6, p3.Length, p3.Length);

            return (p3, p1, p5, p6);
        }

        public (string, string) opString(string p1, string p2, Current current) => (p1 + " " + p2, p2 + " " + p1);

        public (IReadOnlyDictionary<string, MyEnum>, IReadOnlyDictionary<string, MyEnum>) opStringMyEnumD(
            Dictionary<string, MyEnum> p1, Dictionary<string, MyEnum> p2, Current current)
        {
            var r = new Dictionary<string, MyEnum>();
            foreach (KeyValuePair<string, MyEnum> e in p1)
            {
                r[e.Key] = e.Value;
            }
            foreach (KeyValuePair<string, MyEnum> e in p2)
            {
                r[e.Key] = e.Value;
            }
            return (r, p1);
        }

        public (IReadOnlyDictionary<MyEnum, string>, IReadOnlyDictionary<MyEnum, string>) opMyEnumStringD(
            Dictionary<MyEnum, string> p1, Dictionary<MyEnum, string> p2, Current current)
        {
            var r = new Dictionary<MyEnum, string>();
            foreach (var e in p1)
            {
                r[e.Key] = e.Value;
            }
            foreach (var e in p2)
            {
                r[e.Key] = e.Value;
            }
            return (r, p1);
        }

        public (IReadOnlyDictionary<MyStruct, MyEnum>, IReadOnlyDictionary<MyStruct, MyEnum>)
        opMyStructMyEnumD(Dictionary<MyStruct, MyEnum> p1, Dictionary<MyStruct, MyEnum> p2,
            Current current)
        {
            var r = new Dictionary<MyStruct, MyEnum>();
            foreach (var e in p1)
            {
                r[e.Key] = e.Value;
            }
            foreach (var e in p2)
            {
                r[e.Key] = e.Value;
            }
            return (r, p1);
        }

        public (IEnumerable<Dictionary<byte, bool>>, IEnumerable<Dictionary<byte, bool>>) opByteBoolDS(
            Dictionary<byte, bool>[] p1, Dictionary<byte, bool>[] p2, Current current)
        {
            var p3 = new Dictionary<byte, bool>[p1.Length + p2.Length];
            Array.Copy(p2, p3, p2.Length);
            Array.Copy(p1, 0, p3, p2.Length, p1.Length);

            Dictionary<byte, bool>[] r = new Dictionary<byte, bool>[p1.Length];
            for (int i = 0; i < p1.Length; i++)
            {
                r[i] = p1[p1.Length - (i + 1)];
            }
            return (r, p3);
        }

        public (IEnumerable<Dictionary<short, int>>, IEnumerable<Dictionary<short, int>>) opShortIntDS(
            Dictionary<short, int>[] p1, Dictionary<short, int>[] p2, Current current)
        {
            var p3 = new Dictionary<short, int>[p1.Length + p2.Length];
            Array.Copy(p2, p3, p2.Length);
            Array.Copy(p1, 0, p3, p2.Length, p1.Length);

            var r = new Dictionary<short, int>[p1.Length];
            for (int i = 0; i < p1.Length; i++)
            {
                r[i] = p1[p1.Length - (i + 1)];
            }
            return (r, p3);
        }

        public (IEnumerable<Dictionary<ushort, uint>>, IEnumerable<Dictionary<ushort, uint>>) opUShortUIntDS(
            Dictionary<ushort, uint>[] p1, Dictionary<ushort, uint>[] p2, Current current)
        {
            var p3 = new Dictionary<ushort, uint>[p1.Length + p2.Length];
            Array.Copy(p2, p3, p2.Length);
            Array.Copy(p1, 0, p3, p2.Length, p1.Length);

            var r = new Dictionary<ushort, uint>[p1.Length];
            for (int i = 0; i < p1.Length; i++)
            {
                r[i] = p1[p1.Length - (i + 1)];
            }
            return (r, p3);
        }

        public (IEnumerable<Dictionary<long, float>>, IEnumerable<Dictionary<long, float>>) opLongFloatDS(
            Dictionary<long, float>[] p1, Dictionary<long, float>[] p2, Current current)
        {
            var p3 = new Dictionary<long, float>[p1.Length + p2.Length];
            Array.Copy(p2, p3, p2.Length);
            Array.Copy(p1, 0, p3, p2.Length, p1.Length);

            var r = new Dictionary<long, float>[p1.Length];
            for (int i = 0; i < p1.Length; i++)
            {
                r[i] = p1[p1.Length - (i + 1)];
            }
            return (r, p3);
        }

        public (IEnumerable<Dictionary<ulong, float>>, IEnumerable<Dictionary<ulong, float>>) opULongFloatDS(
            Dictionary<ulong, float>[] p1, Dictionary<ulong, float>[] p2, Current current)
        {
            var p3 = new Dictionary<ulong, float>[p1.Length + p2.Length];
            Array.Copy(p2, p3, p2.Length);
            Array.Copy(p1, 0, p3, p2.Length, p1.Length);

            var r = new Dictionary<ulong, float>[p1.Length];
            for (int i = 0; i < p1.Length; i++)
            {
                r[i] = p1[p1.Length - (i + 1)];
            }
            return (r, p3);
        }

        public (IEnumerable<Dictionary<string, string>>, IEnumerable<Dictionary<string, string>>) opStringStringDS(
            Dictionary<string, string>[] p1, Dictionary<string, string>[] p2, Current current)
        {
            var p3 = new Dictionary<string, string>[p1.Length + p2.Length];
            Array.Copy(p2, p3, p2.Length);
            Array.Copy(p1, 0, p3, p2.Length, p1.Length);

            var r = new Dictionary<string, string>[p1.Length];
            for (int i = 0; i < p1.Length; i++)
            {
                r[i] = p1[p1.Length - (i + 1)];
            }
            return (r, p3);
        }

        public (IEnumerable<Dictionary<string, MyEnum>>, IEnumerable<Dictionary<string, MyEnum>>)
        opStringMyEnumDS(Dictionary<string, MyEnum>[] p1, Dictionary<string, MyEnum>[] p2, Current current)
        {
            var p3 = new Dictionary<string, MyEnum>[p1.Length + p2.Length];
            Array.Copy(p2, p3, p2.Length);
            Array.Copy(p1, 0, p3, p2.Length, p1.Length);

            var r = new Dictionary<string, MyEnum>[p1.Length];
            for (int i = 0; i < p1.Length; i++)
            {
                r[i] = p1[p1.Length - (i + 1)];
            }
            return (r, p3);
        }

        public (IEnumerable<Dictionary<MyEnum, string>>, IEnumerable<Dictionary<MyEnum, string>>)
        opMyEnumStringDS(Dictionary<MyEnum, string>[] p1,
                         Dictionary<MyEnum, string>[] p2,
                            Current current)
        {
            var p3 = new Dictionary<MyEnum, string>[p1.Length + p2.Length];
            Array.Copy(p2, p3, p2.Length);
            Array.Copy(p1, 0, p3, p2.Length, p1.Length);

            var r = new Dictionary<MyEnum, string>[p1.Length];
            for (int i = 0; i < p1.Length; i++)
            {
                r[i] = p1[p1.Length - (i + 1)];
            }
            return (r, p3);
        }

        public (IEnumerable<Dictionary<MyStruct, MyEnum>>,
                IEnumerable<Dictionary<MyStruct, MyEnum>>)
        opMyStructMyEnumDS(Dictionary<MyStruct, MyEnum>[] p1,
            Dictionary<MyStruct, MyEnum>[] p2,
            Current current)
        {
            var p3 = new Dictionary<MyStruct, MyEnum>[p1.Length + p2.Length];
            Array.Copy(p2, p3, p2.Length);
            Array.Copy(p1, 0, p3, p2.Length, p1.Length);

            var r = new Dictionary<MyStruct, MyEnum>[p1.Length];
            for (int i = 0; i < p1.Length; i++)
            {
                r[i] = p1[p1.Length - (i + 1)];
            }
            return (r, p3);
        }

        public (IReadOnlyDictionary<byte, byte[]>, IReadOnlyDictionary<byte, byte[]>) opByteByteSD(
            Dictionary<byte, byte[]> p1,
            Dictionary<byte, byte[]> p2,
            Current current)
        {
            var r = new Dictionary<byte, byte[]>();
            foreach (var e in p1)
            {
                r[e.Key] = e.Value;
            }
            foreach (var e in p2)
            {
                r[e.Key] = e.Value;
            }
            return (r, p2);
        }

        public (IReadOnlyDictionary<bool, bool[]>, IReadOnlyDictionary<bool, bool[]>) opBoolBoolSD(
            Dictionary<bool, bool[]> p1, Dictionary<bool, bool[]> p2, Current current)
        {
            var r = new Dictionary<bool, bool[]>();
            foreach (KeyValuePair<bool, bool[]> e in p1)
            {
                r[e.Key] = e.Value;
            }
            foreach (KeyValuePair<bool, bool[]> e in p2)
            {
                r[e.Key] = e.Value;
            }
            return (r, p2);
        }

        public (IReadOnlyDictionary<short, short[]>, IReadOnlyDictionary<short, short[]>) opShortShortSD(
            Dictionary<short, short[]> p1, Dictionary<short, short[]> p2, Current current)
        {
            var r = new Dictionary<short, short[]>();
            foreach (KeyValuePair<short, short[]> e in p1)
            {
                r[e.Key] = e.Value;
            }
            foreach (KeyValuePair<short, short[]> e in p2)
            {
                r[e.Key] = e.Value;
            }
            return (r, p2);
        }

        public (IReadOnlyDictionary<ushort, ushort[]>, IReadOnlyDictionary<ushort, ushort[]>) opUShortUShortSD(
            Dictionary<ushort, ushort[]> p1, Dictionary<ushort, ushort[]> p2, Current current)
        {
            var r = new Dictionary<ushort, ushort[]>();
            foreach (KeyValuePair<ushort, ushort[]> e in p1)
            {
                r[e.Key] = e.Value;
            }
            foreach (KeyValuePair<ushort, ushort[]> e in p2)
            {
                r[e.Key] = e.Value;
            }
            return (r, p2);
        }

        public (IReadOnlyDictionary<int, int[]>, IReadOnlyDictionary<int, int[]>) opIntIntSD(Dictionary<int, int[]> p1,
            Dictionary<int, int[]> p2,
            Current current)
        {
            var r = new Dictionary<int, int[]>();
            foreach (KeyValuePair<int, int[]> e in p1)
            {
                r[e.Key] = e.Value;
            }
            foreach (KeyValuePair<int, int[]> e in p2)
            {
                r[e.Key] = e.Value;
            }
            return (r, p2);
        }

        public (IReadOnlyDictionary<uint, uint[]>, IReadOnlyDictionary<uint, uint[]>) opUIntUIntSD(
            Dictionary<uint, uint[]> p1,
            Dictionary<uint, uint[]> p2,
            Current current)
        {
            var r = new Dictionary<uint, uint[]>();
            foreach (KeyValuePair<uint, uint[]> e in p1)
            {
                r[e.Key] = e.Value;
            }
            foreach (KeyValuePair<uint, uint[]> e in p2)
            {
                r[e.Key] = e.Value;
            }
            return (r, p2);
        }

        public (IReadOnlyDictionary<long, long[]>, IReadOnlyDictionary<long, long[]>) opLongLongSD(
            Dictionary<long, long[]> p1, Dictionary<long, long[]> p2, Current current)
        {
            var r = new Dictionary<long, long[]>();
            foreach (KeyValuePair<long, long[]> e in p1)
            {
                r[e.Key] = e.Value;
            }
            foreach (KeyValuePair<long, long[]> e in p2)
            {
                r[e.Key] = e.Value;
            }
            return (r, p2);
        }

        public (IReadOnlyDictionary<ulong, ulong[]>, IReadOnlyDictionary<ulong, ulong[]>) opULongULongSD(
            Dictionary<ulong, ulong[]> p1, Dictionary<ulong, ulong[]> p2, Current current)
        {
            var r = new Dictionary<ulong, ulong[]>();
            foreach (KeyValuePair<ulong, ulong[]> e in p1)
            {
                r[e.Key] = e.Value;
            }
            foreach (KeyValuePair<ulong, ulong[]> e in p2)
            {
                r[e.Key] = e.Value;
            }
            return (r, p2);
        }

        public (IReadOnlyDictionary<string, float[]>, IReadOnlyDictionary<string, float[]>) opStringFloatSD(
            Dictionary<string, float[]> p1, Dictionary<string, float[]> p2, Current current)
        {
            var r = new Dictionary<string, float[]>();
            foreach (KeyValuePair<string, float[]> e in p1)
            {
                r[e.Key] = e.Value;
            }
            foreach (KeyValuePair<string, float[]> e in p2)
            {
                r[e.Key] = e.Value;
            }
            return (r, p2);
        }

        public (IReadOnlyDictionary<string, double[]>, IReadOnlyDictionary<string, double[]>) opStringDoubleSD(
            Dictionary<string, double[]> p1, Dictionary<string, double[]> p2, Current current)
        {
            var r = new Dictionary<string, double[]>();
            foreach (KeyValuePair<string, double[]> e in p1)
            {
                r[e.Key] = e.Value;
            }
            foreach (KeyValuePair<string, double[]> e in p2)
            {
                r[e.Key] = e.Value;
            }
            return (r, p2);
        }

        public (IReadOnlyDictionary<string, string[]>, IReadOnlyDictionary<string, string[]>) opStringStringSD(
            Dictionary<string, string[]> p1, Dictionary<string, string[]> p2, Current current)
        {
            var r = new Dictionary<string, string[]>();
            foreach (KeyValuePair<string, string[]> e in p1)
            {
                r[e.Key] = e.Value;
            }
            foreach (KeyValuePair<string, string[]> e in p2)
            {
                r[e.Key] = e.Value;
            }
            return (r, p2);
        }

        public (IReadOnlyDictionary<MyEnum, MyEnum[]>, IReadOnlyDictionary<MyEnum, MyEnum[]>)
        opMyEnumMyEnumSD(
            Dictionary<MyEnum, MyEnum[]> p1,
            Dictionary<MyEnum, MyEnum[]> p2,
            Current ice)
        {
            var r = new Dictionary<MyEnum, MyEnum[]>();
            foreach (var e in p1)
            {
                r[e.Key] = e.Value;
            }
            foreach (var e in p2)
            {
                r[e.Key] = e.Value;
            }
            return (r, p2);
        }

        public ReadOnlyMemory<int> opIntS(int[] s, Current current)
        {
            int[] r = new int[s.Length];
            for (int i = 0; i < s.Length; ++i)
            {
                r[i] = -s[i];
            }
            return r;
        }

        public void opByteSOneway(byte[] s, Current current)
        {
            lock (this)
            {
                ++_opByteSOnewayCallCount;
            }
        }

        public int opByteSOnewayCallCount(Current current)
        {
            lock (this)
            {
                int count = _opByteSOnewayCallCount;
                _opByteSOnewayCallCount = 0;
                return count;
            }
        }

        public IReadOnlyDictionary<string, string> opContext(Current current) =>
            current.Context == null ? new Dictionary<string, string>() : new Dictionary<string, string>(current.Context);

        public void opDoubleMarshaling(double p1, double[] p2, Current current)
        {
            double d = 1278312346.0 / 13.0;
            TestHelper.Assert(p1 == d);
            for (int i = 0; i < p2.Length; ++i)
            {
                TestHelper.Assert(p2[i] == d);
            }
        }

        public (IEnumerable<string>, IEnumerable<string>) opStringS(string[] p1, string[] p2, Current current)
        {
            var p3 = new string[p1.Length + p2.Length];
            Array.Copy(p1, p3, p1.Length);
            Array.Copy(p2, 0, p3, p1.Length, p2.Length);

            string[] r = new string[p1.Length];
            for (int i = 0; i < p1.Length; i++)
            {
                r[i] = p1[p1.Length - (i + 1)];
            }
            return (r, p3);
        }

        public (IEnumerable<string[]>, IEnumerable<string[]>) opStringSS(string[][] p1, string[][] p2, Current current)
        {
            var p3 = new string[p1.Length + p2.Length][];
            Array.Copy(p1, p3, p1.Length);
            Array.Copy(p2, 0, p3, p1.Length, p2.Length);

            string[][] r = new string[p2.Length][];
            for (int i = 0; i < p2.Length; i++)
            {
                r[i] = p2[p2.Length - (i + 1)];
            }
            return (r, p3);
        }

        public (IEnumerable<string[][]>, IEnumerable<string[][]>) opStringSSS(string[][][] p1, string[][][] p2,
            Current current)
        {
            var p3 = new string[p1.Length + p2.Length][][];
            Array.Copy(p1, p3, p1.Length);
            Array.Copy(p2, 0, p3, p1.Length, p2.Length);

            string[][][] r = new string[p2.Length][][];
            for (int i = 0; i < p2.Length; i++)
            {
                r[i] = p2[p2.Length - (i + 1)];
            }
            return (r, p3);
        }

        public (IReadOnlyDictionary<string, string>, IReadOnlyDictionary<string, string>) opStringStringD(
            Dictionary<string, string> p1,
            Dictionary<string, string> p2,
            Current current)
        {
            var p3 = p1;
            Dictionary<string, string> r = new Dictionary<string, string>();
            foreach (KeyValuePair<string, string> e in p1)
            {
                r[e.Key] = e.Value;
            }
            foreach (KeyValuePair<string, string> e in p2)
            {
                r[e.Key] = e.Value;
            }
            return (r, p3);
        }

        public (Structure, Structure) opStruct(Structure p1, Structure p2, Current current)
        {
            var p3 = p1;
            p3.s.s = "a new string";
            return (p2, p3);
        }

        public void opIdempotent(Current current) => TestHelper.Assert(current.IsIdempotent);

        // "return" exception when called two-way, otherwise succeeds.
        public void opOneway(Current current) => throw new SomeException();

        // "return" exception when called two-way, otherwise succeeds.
        public void opOnewayMetadata(Current current) => throw new SomeException();

        public void opDerived(Current current)
        {
        }

        public byte opByte1(byte opByte1, Current current) => opByte1;

        public short opShort1(short opShort1, Current current) => opShort1;
        public ushort opUShort1(ushort opUShort1, Current current) => opUShort1;

        public int opInt1(int opInt1, Current current) => opInt1;
        public uint opUInt1(uint opUInt1, Current current) => opUInt1;

        public long opLong1(long opLong1, Current current) => opLong1;
        public ulong opULong1(ulong opULong1, Current current) => opULong1;

        public float opFloat1(float opFloat1, Current current) => opFloat1;

        public double opDouble1(double opDouble1, Current current) => opDouble1;

        public string opString1(string opString1, Current current) => opString1;

        public IEnumerable<string> opStringS1(string[] opStringS1, Current current) => opStringS1;

        public IReadOnlyDictionary<byte, bool> opByteBoolD1(Dictionary<byte, bool> opByteBoolD1, Current current) =>
            opByteBoolD1;

        public IEnumerable<string> opStringS2(string[] opStringS2, Current current) => opStringS2;

        public IReadOnlyDictionary<byte, bool> opByteBoolD2(Dictionary<byte, bool> opByteBoolD2, Current current) =>
            opByteBoolD2;

        public MyClass1? opMyClass1(MyClass1? c, Current current) => c;

        public MyStruct1 opMyStruct1(MyStruct1 s, Current current) => s;

        public IEnumerable<string> opStringLiterals(Current current)
        {
            return new string[]
                {
                        Constants.s0,
                        Constants.s1,
                        Constants.s2,
                        Constants.s3,
                        Constants.s4,
                        Constants.s5,
                        Constants.s6,
                        Constants.s7,
                        Constants.s8,
                        Constants.s9,
                        Constants.s10,

                        Constants.sw0,
                        Constants.sw1,
                        Constants.sw2,
                        Constants.sw3,
                        Constants.sw4,
                        Constants.sw5,
                        Constants.sw6,
                        Constants.sw7,
                        Constants.sw8,
                        Constants.sw9,
                        Constants.sw10,

                        Constants.ss0,
                        Constants.ss1,
                        Constants.ss2,
                        Constants.ss3,
                        Constants.ss4,
                        Constants.ss5,

                        Constants.su0,
                        Constants.su1,
                        Constants.su2
                };
        }

        public IEnumerable<string> opWStringLiterals(Current current) => opStringLiterals(current);

        public IMyClass.OpMStruct1MarshaledReturnValue opMStruct1(Current current) =>
            new IMyClass.OpMStruct1MarshaledReturnValue(
                new Structure(null, MyEnum.enum1, new AnotherStruct("")), current);

        public IMyClass.OpMStruct2MarshaledReturnValue opMStruct2(Structure p1, Current current) =>
            new IMyClass.OpMStruct2MarshaledReturnValue(p1, p1, current);

        public IMyClass.OpMSeq1MarshaledReturnValue opMSeq1(Current current) =>
            new IMyClass.OpMSeq1MarshaledReturnValue(new string[0], current);

        public IMyClass.OpMSeq2MarshaledReturnValue opMSeq2(string[] p1, Current current) =>
            new IMyClass.OpMSeq2MarshaledReturnValue(p1, p1, current);

        public IMyClass.OpMDict1MarshaledReturnValue opMDict1(Current current) =>
            new IMyClass.OpMDict1MarshaledReturnValue(new Dictionary<string, string>(), current);

        public IMyClass.OpMDict2MarshaledReturnValue opMDict2(Dictionary<string, string> p1, Current current) =>
            new IMyClass.OpMDict2MarshaledReturnValue(p1, p1, current);

        private int _opByteSOnewayCallCount = 0;
    }
}
