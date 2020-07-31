//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Linq;
using Test;

namespace ZeroC.Ice.Test.Compress
{
    public class AllTests
    {
        public static ITestIntfPrx Run(TestHelper helper)
        {
            Communicator? communicator = helper.Communicator();
            TestHelper.Assert(communicator != null);
            System.IO.TextWriter output = helper.GetWriter();

            var p = ITestIntfPrx.Parse(helper.GetTestProxy("test", 0), communicator);

            output.Write("testing operations using compression... ");
            output.Flush();
            int[] lengths = new int[] { 8, 256, 1024, 2048 };

            foreach (int length in lengths)
            {
                byte[] bsi1 = Enumerable.Range(0, length).Select(i => (byte)i).ToArray();
                (byte[] rso, byte[] bso) = p.OpByteS(bsi1, bsi1);
                for (int i = 0; i < length; ++i)
                {
                    TestHelper.Assert(rso[i] == bsi1[i]);
                    TestHelper.Assert(rso[i + length] == bsi1[i]);
                    TestHelper.Assert(bso[i] == bsi1[^(i + 1)]);
                }
            }

            foreach (int length in lengths)
            {
                bool[] bsi1 = Enumerable.Range(0, length).Select(i => (i % 2) == 0).ToArray();
                (bool[] rso, bool[] bso) = p.OpBoolS(bsi1, bsi1);
                for (int i = 0; i < length; ++i)
                {
                    TestHelper.Assert(bso[i] == bsi1[i]);
                    TestHelper.Assert(bso[i + length] == bsi1[i]);
                    TestHelper.Assert(rso[i] == bsi1[^(i + 1)]);
                }
            }

            foreach (int length in lengths)
            {
                short[] ssi = Enumerable.Range(0, length).Select(i => (short)i).ToArray();
                int[] isi = Enumerable.Range(0, length).ToArray();
                long[] lsi = Enumerable.Range(0, length).Select(i => (long)i).ToArray();

                (long[] rso, short[] sso, int[] iso, long[] lso) = p.OpShortIntLongS(ssi, isi, lsi);

                TestHelper.Assert(rso.SequenceEqual(lsi));
                TestHelper.Assert(sso.SequenceEqual(ssi));
                for (int i = 0; i < length; ++i)
                {
                    TestHelper.Assert(iso[i] == isi[^(i + 1)]);
                }

                for (int i = 0; i < lso.Length; ++i)
                {
                    TestHelper.Assert(lso[i] == lsi[i % length]);
                }
            }

            foreach (int length in lengths)
            {
                ushort[] ssi = Enumerable.Range(0, length).Select(i => (ushort)i).ToArray();
                uint[] isi = Enumerable.Range(0, length).Select(i => (uint)i).ToArray();
                ulong[] lsi = Enumerable.Range(0, length).Select(i => (ulong)i).ToArray();

                (ulong[] rso, ushort[] sso, uint[] iso, ulong[] lso) = p.OpUShortUIntULongS(ssi, isi, lsi);

                TestHelper.Assert(rso.SequenceEqual(lsi));
                TestHelper.Assert(sso.SequenceEqual(ssi));
                for (int i = 0; i < length; ++i)
                {
                    TestHelper.Assert(iso[i] == isi[^(i + 1)]);
                }

                for (int i = 0; i < lso.Length; ++i)
                {
                    TestHelper.Assert(lso[i] == lsi[i % length]);
                }
            }

            foreach (int length in lengths)
            {
                int[] isi = Enumerable.Range(0, length).ToArray();
                long[] lsi = Enumerable.Range(0, length).Select(i => (long)i).ToArray();

                (long[] rso, int[] iso, long[] lso) = p.OpVarIntVarLongS(isi, lsi);

                TestHelper.Assert(rso.SequenceEqual(lsi));
                for (int i = 0; i < length; ++i)
                {
                    TestHelper.Assert(iso[i] == isi[^(i + 1)]);
                }

                for (int i = 0; i < lso.Length; ++i)
                {
                    TestHelper.Assert(lso[i] == lsi[i % length]);
                }
            }

            foreach (int length in lengths)
            {
                uint[] isi = Enumerable.Range(0, length).Select(i => (uint)i).ToArray();
                ulong[] lsi = Enumerable.Range(0, length).Select(i => (ulong)i).ToArray();

                (ulong[] rso, uint[] iso, ulong[] lso) = p.OpVarUIntVarULongS(isi, lsi);

                TestHelper.Assert(rso.SequenceEqual(lsi));
                for (int i = 0; i < length; ++i)
                {
                    TestHelper.Assert(iso[i] == isi[^(i + 1)]);
                }

                for (int i = 0; i < lso.Length; ++i)
                {
                    TestHelper.Assert(lso[i] == lsi[i % length]);
                }
            }

            foreach (int length in lengths)
            {
                float[] fsi = Enumerable.Range(0, length).Select(i => (float)i).ToArray();
                double[] dsi = Enumerable.Range(0, length).Select(i => (double)i).ToArray();

                (double[] rso, float[] fso, double[] dso) = p.OpFloatDoubleS(fsi, dsi);

                TestHelper.Assert(fso.SequenceEqual(fsi));
                for (int i = 0; i < fsi.Length; ++i)
                {
                    TestHelper.Assert(rso[i] == fsi[i]);
                }

                for (int i = 0; i < length; ++i)
                {
                    TestHelper.Assert(rso[i + length] == dsi[i]);
                }

                for (int i = 0; i < length; ++i)
                {
                    TestHelper.Assert(dso[i] == dsi[^(i + 1)]);
                }
            }

            foreach (int length in lengths)
            {
                string[] ssi1 = Enumerable.Range(0, length).Select(i => $"string-{i}").ToArray();
                (string[] rso, string[] sso) = p.OpStringS(ssi1, ssi1);

                for (int i = 0; i < rso.Length; i++)
                {
                    TestHelper.Assert(rso[i] == ssi1[^(i + 1)]);
                }

                for (int i = 0; i < length; ++i)
                {
                    TestHelper.Assert(sso[i] == ssi1[i]);
                    TestHelper.Assert(sso[i + length] == ssi1[i]);
                }
            }

            foreach (int length in lengths)
            {
                MyStruct[] ssi1 = Enumerable.Range(0, length).Select(i => new MyStruct(i, i)).ToArray();
                (MyStruct[] rso, MyStruct[] sso) = p.OpMyStructS(ssi1, ssi1);

                for (int i = 0; i < rso.Length; i++)
                {
                    TestHelper.Assert(rso[i] == ssi1[^(i + 1)]);
                }

                for (int i = 0; i < length; ++i)
                {
                    TestHelper.Assert(sso[i] == ssi1[i]);
                    TestHelper.Assert(sso[i + length] == ssi1[i]);
                }
            }

            foreach (int length in lengths)
            {
                MyValue[] ssi1 = Enumerable.Range(0, length).Select(i => new MyValue(new MyStruct(i, i))).ToArray();
                (MyValue?[] rso, MyValue?[] sso) = p.OpMyValueS(ssi1, ssi1);

                for (int i = 0; i < rso.Length; i++)
                {
                    TestHelper.Assert(rso[i]!.S == ssi1[^(i + 1)]!.S);
                }

                for (int i = 0; i < length; ++i)
                {
                    TestHelper.Assert(sso[i]!.S == ssi1[i]!.S);
                    TestHelper.Assert(sso[i + length]!.S == ssi1[i]!.S);
                }
            }

            foreach (int length in lengths)
            {
                bool ice1 = communicator.DefaultProtocol == Protocol.Ice1;
                IMyInterfacePrx[] ssi1 = Enumerable.Range(0, length).Select(
                    i => IMyInterfacePrx.Parse(ice1 ? "test-{i}:tcp -h localhost" : $"ice+tcp://localhost/test-{i}",
                                               communicator)).ToArray();
                (IMyInterfacePrx?[] rso, IMyInterfacePrx?[] sso) = p.OpMyInterfaceS(ssi1, ssi1);

                for (int i = 0; i < rso.Length; i++)
                {
                    TestHelper.Assert(rso[i]!.Equals(ssi1[^(i + 1)]));
                }

                for (int i = 0; i < length; ++i)
                {
                    TestHelper.Assert(sso[i]!.Equals(ssi1[i]));
                    TestHelper.Assert(sso[i + length]!.Equals(ssi1[i]));
                }
            }

            foreach (int length in lengths)
            {
                MyEnum[] ssi1 = Enumerable.Range(0, length).Select(i => (MyEnum)(i % 3)).ToArray();
                (MyEnum[] rso, MyEnum[] sso) = p.OpMyEnumS(ssi1, ssi1);

                for (int i = 0; i < rso.Length; i++)
                {
                    TestHelper.Assert(rso[i] == ssi1[^(i + 1)]);
                }

                for (int i = 0; i < length; ++i)
                {
                    TestHelper.Assert(sso[i] == ssi1[i]);
                    TestHelper.Assert(sso[i + length] == ssi1[i]);
                }
            }
            output.WriteLine("ok");

            return p;
        }
    }
}
