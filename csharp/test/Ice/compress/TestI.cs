//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;

namespace ZeroC.Ice.Test.Compress
{
    public sealed class TestIntf : ITestIntf, IObject
    {

        public (ReadOnlyMemory<bool>, ReadOnlyMemory<bool>) OpBoolS(bool[] p1, bool[] p2, Current current)
        {
            bool[] p3 = new bool[p1.Length + p2.Length];
            Array.Copy(p1, p3, p1.Length);
            Array.Copy(p2, 0, p3, p1.Length, p2.Length);

            bool[] r = new bool[p1.Length];
            for (int i = 0; i < p1.Length; i++)
            {
                r[i] = p1[^(i + 1)];
            }
            return (r, p3);
        }

        public (ReadOnlyMemory<byte>, ReadOnlyMemory<byte>) OpByteS(byte[] p1, byte[] p2, Current current)
        {
            byte[] p3 = new byte[p1.Length];
            for (int i = 0; i < p1.Length; i++)
            {
                p3[i] = p1[^(i + 1)];
            }

            byte[] r = new byte[p1.Length + p2.Length];
            Array.Copy(p1, r, p1.Length);
            Array.Copy(p2, 0, r, p1.Length, p2.Length);
            return (r, p3);
        }

        public (ReadOnlyMemory<double>, ReadOnlyMemory<float>, ReadOnlyMemory<double>) OpFloatDoubleS(
            float[] p1,
            double[] p2,
            Current current)
        {
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
            return (r, p1, p4);
        }

        public (ReadOnlyMemory<long>,
                ReadOnlyMemory<short>,
                ReadOnlyMemory<int>,
                ReadOnlyMemory<long>) OpShortIntLongS(short[] p1, int[] p2, long[] p3, Current current)
        {
            int[] p5 = new int[p2.Length];
            for (int i = 0; i < p2.Length; i++)
            {
                p5[i] = p2[^(i + 1)];
            }

            long[] p6 = new long[p3.Length + p3.Length];
            Array.Copy(p3, p6, p3.Length);
            Array.Copy(p3, 0, p6, p3.Length, p3.Length);

            return (p3, p1, p5, p6);
        }

        public (ReadOnlyMemory<ulong>,
                ReadOnlyMemory<ushort>,
                ReadOnlyMemory<uint>,
                ReadOnlyMemory<ulong>) OpUShortUIntULongS(ushort[] p1, uint[] p2, ulong[] p3, Current current)
        {
            uint[] p5 = new uint[p2.Length];
            for (int i = 0; i < p2.Length; i++)
            {
                p5[i] = p2[^(i + 1)];
            }

            ulong[] p6 = new ulong[p3.Length + p3.Length];
            Array.Copy(p3, p6, p3.Length);
            Array.Copy(p3, 0, p6, p3.Length, p3.Length);

            return (p3, p1, p5, p6);
        }

        public (IEnumerable<long>, IEnumerable<int>, IEnumerable<long>) OpVarIntVarLongS(
            int[] p1,
            long[] p2,
            Current current)
        {
            int[] p4 = new int[p1.Length];
            for (int i = 0; i < p1.Length; i++)
            {
                p4[i] = p1[^(i + 1)];
            }

            long[] p5 = new long[p2.Length + p2.Length];
            Array.Copy(p2, p5, p2.Length);
            Array.Copy(p2, 0, p5, p2.Length, p2.Length);

            return (p2, p4, p5);
        }

        public (IEnumerable<ulong>, IEnumerable<uint>, IEnumerable<ulong>) OpVarUIntVarULongS(
            uint[] p1,
            ulong[] p2,
            Current current)
        {
            uint[] p4 = new uint[p1.Length];
            for (int i = 0; i < p1.Length; i++)
            {
                p4[i] = p1[^(i + 1)];
            }

            ulong[] p5 = new ulong[p2.Length + p2.Length];
            Array.Copy(p2, p5, p2.Length);
            Array.Copy(p2, 0, p5, p2.Length, p2.Length);

            return (p2, p4, p5);
        }

        public (IEnumerable<string>, IEnumerable<string>) OpStringS(
            string[] p1,
            string[] p2,
            Current current)
        {
            string[] p3 = new string[p1.Length + p2.Length];
            Array.Copy(p1, p3, p1.Length);
            Array.Copy(p2, 0, p3, p1.Length, p2.Length);

            string[] r = new string[p1.Length];
            for (int i = 0; i < p1.Length; i++)
            {
                r[i] = p1[^(i + 1)];
            }
            return (r, p3);
        }

        public (IEnumerable<MyStruct>, IEnumerable<MyStruct>) OpMyStructS(
            MyStruct[] p1,
            MyStruct[] p2,
            Current current)
        {
            var p3 = new MyStruct[p1.Length + p2.Length];
            Array.Copy(p1, p3, p1.Length);
            Array.Copy(p2, 0, p3, p1.Length, p2.Length);

            var r = new MyStruct[p1.Length];
            for (int i = 0; i < p1.Length; i++)
            {
                r[i] = p1[^(i + 1)];
            }
            return (r, p3);
        }

        public (IEnumerable<MyValue>, IEnumerable<MyValue>) OpMyValueS(
            MyValue[] p1,
            MyValue[] p2,
            Current current)
        {
            var p3 = new MyValue[p1.Length + p2.Length];
            Array.Copy(p1, p3, p1.Length);
            Array.Copy(p2, 0, p3, p1.Length, p2.Length);

            var r = new MyValue[p1.Length];
            for (int i = 0; i < p1.Length; i++)
            {
                r[i] = p1[^(i + 1)];
            }
            return (r, p3);
        }

        public (IEnumerable<IMyInterfacePrx?>, IEnumerable<IMyInterfacePrx?>) OpMyInterfaceS(
            IMyInterfacePrx?[] p1,
            IMyInterfacePrx?[] p2,
            Current current)
        {
            var p3 = new IMyInterfacePrx?[p1.Length + p2.Length];
            Array.Copy(p1, p3, p1.Length);
            Array.Copy(p2, 0, p3, p1.Length, p2.Length);

            var r = new IMyInterfacePrx?[p1.Length];
            for (int i = 0; i < p1.Length; i++)
            {
                r[i] = p1[^(i + 1)];
            }
            return (r, p3);
        }

        public (IEnumerable<MyEnum>, IEnumerable<MyEnum>) OpMyEnumS(MyEnum[] p1, MyEnum[] p2, Current current)
        {
            var p3 = new MyEnum[p1.Length + p2.Length];
            Array.Copy(p1, p3, p1.Length);
            Array.Copy(p2, 0, p3, p1.Length, p2.Length);

            var r = new MyEnum[p1.Length];
            for (int i = 0; i < p1.Length; i++)
            {
                r[i] = p1[^(i + 1)];
            }
            return (r, p3);
        }

        public void Shutdown(Current current) => current.Adapter.Communicator.ShutdownAsync();
    }
}
