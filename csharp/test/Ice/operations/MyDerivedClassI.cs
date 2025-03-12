// Copyright (c) ZeroC, Inc.

namespace Ice.operations
{
    public sealed class MyDerivedClassI : Test.MyDerivedClassDisp_
    {
        private static void test(bool b) => global::Test.TestHelper.test(b);

        public override void shutdown(Ice.Current current)
        {
            current.adapter.getCommunicator().shutdown();
        }

        public override bool supportsCompress(Ice.Current current)
        {
            return Ice.Internal.BZip2.isLoaded(current.adapter.getCommunicator().getLogger());
        }

        public override void opVoid(Ice.Current current)
        {
            test(current.mode == Ice.OperationMode.Normal);
        }

        public override bool opBool(bool p1, bool p2, out bool p3, Ice.Current current)
        {
            p3 = p1;
            return p2;
        }

        public override bool[] opBoolS(bool[] p1, bool[] p2, out bool[] p3, Ice.Current current)
        {
            p3 = new bool[p1.Length + p2.Length];
            Array.Copy(p1, p3, p1.Length);
            Array.Copy(p2, 0, p3, p1.Length, p2.Length);

            bool[] r = new bool[p1.Length];
            for (int i = 0; i < p1.Length; i++)
            {
                r[i] = p1[p1.Length - (i + 1)];
            }
            return r;
        }

        public override bool[][] opBoolSS(bool[][] p1, bool[][] p2, out bool[][] p3, Ice.Current current)
        {
            p3 = new bool[p1.Length + p2.Length][];
            Array.Copy(p1, p3, p1.Length);
            Array.Copy(p2, 0, p3, p1.Length, p2.Length);

            bool[][] r = new bool[p1.Length][];
            for (int i = 0; i < p1.Length; i++)
            {
                r[i] = p1[p1.Length - (i + 1)];
            }
            return r;
        }

        public override byte opByte(byte p1, byte p2, out byte p3, Ice.Current current)
        {
            p3 = (byte)(p1 ^ p2);
            return p1;
        }

        public override Dictionary<byte, bool> opByteBoolD(Dictionary<byte, bool> p1, Dictionary<byte, bool> p2,
                                                           out Dictionary<byte, bool> p3,
                                                           Ice.Current current)
        {
            p3 = p1;
            Dictionary<byte, bool> r = new Dictionary<byte, bool>();
            foreach (KeyValuePair<byte, bool> e in p1)
            {
                r[e.Key] = e.Value;
            }
            foreach (KeyValuePair<byte, bool> e in p2)
            {
                r[e.Key] = e.Value;
            }
            return r;
        }

        public override byte[] opByteS(byte[] p1, byte[] p2, out byte[] p3, Ice.Current current)
        {
            p3 = new byte[p1.Length];
            for (int i = 0; i < p1.Length; i++)
            {
                p3[i] = p1[p1.Length - (i + 1)];
            }

            byte[] r = new byte[p1.Length + p2.Length];
            Array.Copy(p1, r, p1.Length);
            Array.Copy(p2, 0, r, p1.Length, p2.Length);
            return r;
        }

        public override byte[][] opByteSS(byte[][] p1, byte[][] p2, out byte[][] p3, Ice.Current current)
        {
            p3 = new byte[p1.Length][];
            for (int i = 0; i < p1.Length; i++)
            {
                p3[i] = p1[p1.Length - (i + 1)];
            }

            byte[][] r = new byte[p1.Length + p2.Length][];
            Array.Copy(p1, r, p1.Length);
            Array.Copy(p2, 0, r, p1.Length, p2.Length);
            return r;
        }

        public override double opFloatDouble(float p1, double p2, out float p3, out double p4, Ice.Current current)
        {
            p3 = p1;
            p4 = p2;
            return p2;
        }

        public override double[] opFloatDoubleS(float[] p1, double[] p2,
                                                out float[] p3, out double[] p4,
                                                Ice.Current current)
        {
            p3 = p1;

            p4 = new double[p2.Length];
            for (int i = 0; i < p2.Length; i++)
            {
                p4[i] = p2[p2.Length - (i + 1)];
            }

            double[] r = new double[p2.Length + p1.Length];
            Array.Copy(p2, r, p2.Length);
            for (int i = 0; i < p1.Length; i++)
            {
                r[p2.Length + i] = (double)p1[i];
            }
            return r;
        }

        public override double[][] opFloatDoubleSS(float[][] p1, double[][] p2,
                                                   out float[][] p3, out double[][] p4,
                                                   Ice.Current current)
        {
            p3 = p1;

            p4 = new double[p2.Length][];
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
                    r[p2.Length + i][j] = (double)p2[i][j];
                }
            }
            return r;
        }

        public override Dictionary<long, float> opLongFloatD(Dictionary<long, float> p1, Dictionary<long, float> p2,
                                                             out Dictionary<long, float> p3,
                                                             Ice.Current current)
        {
            p3 = p1;
            Dictionary<long, float> r = new Dictionary<long, float>();
            foreach (KeyValuePair<long, float> e in p1)
            {
                r[e.Key] = e.Value;
            }
            foreach (KeyValuePair<long, float> e in p2)
            {
                r[e.Key] = e.Value;
            }
            return r;
        }

        public override Test.MyClassPrx opMyClass(Test.MyClassPrx p1, out Test.MyClassPrx p2, out Test.MyClassPrx p3,
                                                  Ice.Current current)
        {
            p2 = p1;
            p3 = Test.MyClassPrxHelper.uncheckedCast(current.adapter.createProxy(
                                                    Ice.Util.stringToIdentity("noSuchIdentity")));
            return Test.MyClassPrxHelper.uncheckedCast(current.adapter.createProxy(current.id));
        }

        public override Test.MyEnum opMyEnum(Test.MyEnum p1, out Test.MyEnum p2, Ice.Current current)
        {
            p2 = p1;
            return Test.MyEnum.enum3;
        }

        public override Dictionary<short, int> opShortIntD(Dictionary<short, int> p1, Dictionary<short, int> p2,
                                                           out Dictionary<short, int> p3, Ice.Current current)
        {
            p3 = p1;
            Dictionary<short, int> r = new Dictionary<short, int>();
            foreach (KeyValuePair<short, int> e in p1)
            {
                r[e.Key] = e.Value;
            }
            foreach (KeyValuePair<short, int> e in p2)
            {
                r[e.Key] = e.Value;
            }
            return r;
        }

        public override long opShortIntLong(short p1, int p2, long p3, out short p4, out int p5, out long p6,
                                            Ice.Current current)
        {
            p4 = p1;
            p5 = p2;
            p6 = p3;
            return p3;
        }

        public override long[] opShortIntLongS(short[] p1, int[] p2, long[] p3,
                                               out short[] p4, out int[] p5, out long[] p6,
                                               Ice.Current current)
        {
            p4 = p1;

            p5 = new int[p2.Length];
            for (int i = 0; i < p2.Length; i++)
            {
                p5[i] = p2[p2.Length - (i + 1)];
            }

            p6 = new long[p3.Length + p3.Length];
            Array.Copy(p3, p6, p3.Length);
            Array.Copy(p3, 0, p6, p3.Length, p3.Length);

            return p3;
        }

        public override long[][] opShortIntLongSS(short[][] p1, int[][] p2, long[][] p3,
                                                  out short[][] p4, out int[][] p5, out long[][] p6,
                                                  Ice.Current current)
        {
            p4 = p1;

            p5 = new int[p2.Length][];
            for (int i = 0; i < p2.Length; i++)
            {
                p5[i] = p2[p2.Length - (i + 1)];
            }

            p6 = new long[p3.Length + p3.Length][];
            Array.Copy(p3, p6, p3.Length);
            Array.Copy(p3, 0, p6, p3.Length, p3.Length);

            return p3;
        }

        public override string opString(string p1, string p2, out string p3, Ice.Current current)
        {
            p3 = p2 + " " + p1;
            return p1 + " " + p2;
        }

        public override Dictionary<string, Test.MyEnum> opStringMyEnumD(Dictionary<string, Test.MyEnum> p1,
                                                                        Dictionary<string, Test.MyEnum> p2,
                                                                        out Dictionary<string, Test.MyEnum> p3,
                                                                        Ice.Current current)
        {
            p3 = p1;
            var r = new Dictionary<string, Test.MyEnum>();
            foreach (KeyValuePair<string, Test.MyEnum> e in p1)
            {
                r[e.Key] = e.Value;
            }
            foreach (KeyValuePair<string, Test.MyEnum> e in p2)
            {
                r[e.Key] = e.Value;
            }
            return r;
        }

        public override Dictionary<Test.MyEnum, string> opMyEnumStringD(Dictionary<Test.MyEnum, string> p1,
                                                                        Dictionary<Test.MyEnum, string> p2,
                                                                        out Dictionary<Test.MyEnum, string> p3,
                                                                        Ice.Current current)
        {
            p3 = p1;
            var r = new Dictionary<Test.MyEnum, string>();
            foreach (var e in p1)
            {
                r[e.Key] = e.Value;
            }
            foreach (var e in p2)
            {
                r[e.Key] = e.Value;
            }
            return r;
        }

        public override Dictionary<Test.MyStruct, Test.MyEnum> opMyStructMyEnumD(
                                                    Dictionary<Test.MyStruct, Test.MyEnum> p1,
                                                    Dictionary<Test.MyStruct, Test.MyEnum> p2,
                                                    out Dictionary<Test.MyStruct, Test.MyEnum> p3,
                                                    Ice.Current current)
        {
            p3 = p1;
            var r = new Dictionary<Test.MyStruct, Test.MyEnum>();
            foreach (var e in p1)
            {
                r[e.Key] = e.Value;
            }
            foreach (var e in p2)
            {
                r[e.Key] = e.Value;
            }
            return r;
        }

        public override Dictionary<byte, bool>[] opByteBoolDS(Dictionary<byte, bool>[] p1,
                                                             Dictionary<byte, bool>[] p2,
                                                             out Dictionary<byte, bool>[] p3,
                                                             Ice.Current current)
        {
            p3 = new Dictionary<byte, bool>[p1.Length + p2.Length];
            Array.Copy(p2, p3, p2.Length);
            Array.Copy(p1, 0, p3, p2.Length, p1.Length);

            Dictionary<byte, bool>[] r = new Dictionary<byte, bool>[p1.Length];
            for (int i = 0; i < p1.Length; i++)
            {
                r[i] = p1[p1.Length - (i + 1)];
            }
            return r;
        }

        public override Dictionary<short, int>[] opShortIntDS(Dictionary<short, int>[] p1,
                                                              Dictionary<short, int>[] p2,
                                                              out Dictionary<short, int>[] p3,
                                                              Ice.Current current)
        {
            p3 = new Dictionary<short, int>[p1.Length + p2.Length];
            Array.Copy(p2, p3, p2.Length);
            Array.Copy(p1, 0, p3, p2.Length, p1.Length);

            Dictionary<short, int>[] r = new Dictionary<short, int>[p1.Length];
            for (int i = 0; i < p1.Length; i++)
            {
                r[i] = p1[p1.Length - (i + 1)];
            }
            return r;
        }

        public override Dictionary<long, float>[] opLongFloatDS(Dictionary<long, float>[] p1,
                                                                Dictionary<long, float>[] p2,
                                                                out Dictionary<long, float>[] p3,
                                                                Ice.Current current)
        {
            p3 = new Dictionary<long, float>[p1.Length + p2.Length];
            Array.Copy(p2, p3, p2.Length);
            Array.Copy(p1, 0, p3, p2.Length, p1.Length);

            Dictionary<long, float>[] r = new Dictionary<long, float>[p1.Length];
            for (int i = 0; i < p1.Length; i++)
            {
                r[i] = p1[p1.Length - (i + 1)];
            }
            return r;
        }

        public override Dictionary<string, string>[] opStringStringDS(Dictionary<string, string>[] p1,
                                                                      Dictionary<string, string>[] p2,
                                                                      out Dictionary<string, string>[] p3,
                                                                      Ice.Current current)
        {
            p3 = new Dictionary<string, string>[p1.Length + p2.Length];
            Array.Copy(p2, p3, p2.Length);
            Array.Copy(p1, 0, p3, p2.Length, p1.Length);

            Dictionary<string, string>[] r = new Dictionary<string, string>[p1.Length];
            for (int i = 0; i < p1.Length; i++)
            {
                r[i] = p1[p1.Length - (i + 1)];
            }
            return r;
        }

        public override Dictionary<string, Test.MyEnum>[] opStringMyEnumDS(Dictionary<string, Test.MyEnum>[] p1,
                                                                           Dictionary<string, Test.MyEnum>[] p2,
                                                                           out Dictionary<string, Test.MyEnum>[] p3,
                                                                           Ice.Current current)
        {
            p3 = new Dictionary<string, Test.MyEnum>[p1.Length + p2.Length];
            Array.Copy(p2, p3, p2.Length);
            Array.Copy(p1, 0, p3, p2.Length, p1.Length);

            var r = new Dictionary<string, Test.MyEnum>[p1.Length];
            for (int i = 0; i < p1.Length; i++)
            {
                r[i] = p1[p1.Length - (i + 1)];
            }
            return r;
        }

        public override Dictionary<Test.MyEnum, string>[] opMyEnumStringDS(Dictionary<Test.MyEnum, string>[] p1,
                                                                           Dictionary<Test.MyEnum, string>[] p2,
                                                                           out Dictionary<Test.MyEnum, string>[] p3,
                                                                           Ice.Current current)
        {
            p3 = new Dictionary<Test.MyEnum, string>[p1.Length + p2.Length];
            Array.Copy(p2, p3, p2.Length);
            Array.Copy(p1, 0, p3, p2.Length, p1.Length);

            var r = new Dictionary<Test.MyEnum, string>[p1.Length];
            for (int i = 0; i < p1.Length; i++)
            {
                r[i] = p1[p1.Length - (i + 1)];
            }
            return r;
        }

        public override Dictionary<Test.MyStruct, Test.MyEnum>[] opMyStructMyEnumDS(
            Dictionary<Test.MyStruct, Test.MyEnum>[] p1,
            Dictionary<Test.MyStruct, Test.MyEnum>[] p2,
            out Dictionary<Test.MyStruct, Test.MyEnum>[] p3,
            Ice.Current current)
        {
            p3 = new Dictionary<Test.MyStruct, Test.MyEnum>[p1.Length + p2.Length];
            Array.Copy(p2, p3, p2.Length);
            Array.Copy(p1, 0, p3, p2.Length, p1.Length);

            var r = new Dictionary<Test.MyStruct, Test.MyEnum>[p1.Length];
            for (int i = 0; i < p1.Length; i++)
            {
                r[i] = p1[p1.Length - (i + 1)];
            }
            return r;
        }

        public override Dictionary<byte, byte[]> opByteByteSD(Dictionary<byte, byte[]> p1,
                                                              Dictionary<byte, byte[]> p2,
                                                              out Dictionary<byte, byte[]> p3,
                                                              Ice.Current current)
        {
            p3 = p2;
            Dictionary<byte, byte[]> r = new Dictionary<byte, byte[]>();
            foreach (var e in p1)
            {
                r[e.Key] = e.Value;
            }
            foreach (var e in p2)
            {
                r[e.Key] = e.Value;
            }
            return r;
        }

        public override Dictionary<bool, bool[]> opBoolBoolSD(Dictionary<bool, bool[]> p1,
                                                              Dictionary<bool, bool[]> p2,
                                                              out Dictionary<bool, bool[]> p3,
                                                              Ice.Current current)
        {
            p3 = p2;
            Dictionary<bool, bool[]> r = new Dictionary<bool, bool[]>();
            foreach (KeyValuePair<bool, bool[]> e in p1)
            {
                r[e.Key] = e.Value;
            }
            foreach (KeyValuePair<bool, bool[]> e in p2)
            {
                r[e.Key] = e.Value;
            }
            return r;
        }

        public override Dictionary<short, short[]> opShortShortSD(Dictionary<short, short[]> p1,
                                                                  Dictionary<short, short[]> p2,
                                                                  out Dictionary<short, short[]> p3,
                                                                  Ice.Current current)
        {
            p3 = p2;
            Dictionary<short, short[]> r = new Dictionary<short, short[]>();
            foreach (KeyValuePair<short, short[]> e in p1)
            {
                r[e.Key] = e.Value;
            }
            foreach (KeyValuePair<short, short[]> e in p2)
            {
                r[e.Key] = e.Value;
            }
            return r;
        }

        public override Dictionary<int, int[]> opIntIntSD(Dictionary<int, int[]> p1,
                                                          Dictionary<int, int[]> p2,
                                                          out Dictionary<int, int[]> p3,
                                                          Ice.Current current)
        {
            p3 = p2;
            Dictionary<int, int[]> r = new Dictionary<int, int[]>();
            foreach (KeyValuePair<int, int[]> e in p1)
            {
                r[e.Key] = e.Value;
            }
            foreach (KeyValuePair<int, int[]> e in p2)
            {
                r[e.Key] = e.Value;
            }
            return r;
        }

        public override Dictionary<long, long[]> opLongLongSD(Dictionary<long, long[]> p1,
                                                              Dictionary<long, long[]> p2,
                                                              out Dictionary<long, long[]> p3,
                                                              Ice.Current current)
        {
            p3 = p2;
            Dictionary<long, long[]> r = new Dictionary<long, long[]>();
            foreach (KeyValuePair<long, long[]> e in p1)
            {
                r[e.Key] = e.Value;
            }
            foreach (KeyValuePair<long, long[]> e in p2)
            {
                r[e.Key] = e.Value;
            }
            return r;
        }

        public override Dictionary<string, float[]> opStringFloatSD(Dictionary<string, float[]> p1,
                                                                    Dictionary<string, float[]> p2,
                                                                    out Dictionary<string, float[]> p3,
                                                                    Ice.Current current)
        {
            p3 = p2;
            Dictionary<string, float[]> r = new Dictionary<string, float[]>();
            foreach (KeyValuePair<string, float[]> e in p1)
            {
                r[e.Key] = e.Value;
            }
            foreach (KeyValuePair<string, float[]> e in p2)
            {
                r[e.Key] = e.Value;
            }
            return r;
        }

        public override Dictionary<string, double[]> opStringDoubleSD(Dictionary<string, double[]> p1,
                                                                      Dictionary<string, double[]> p2,
                                                                      out Dictionary<string, double[]> p3,
                                                                      Ice.Current current)
        {
            p3 = p2;
            Dictionary<string, double[]> r = new Dictionary<string, double[]>();
            foreach (KeyValuePair<string, double[]> e in p1)
            {
                r[e.Key] = e.Value;
            }
            foreach (KeyValuePair<string, double[]> e in p2)
            {
                r[e.Key] = e.Value;
            }
            return r;
        }

        public override Dictionary<string, string[]> opStringStringSD(Dictionary<string, string[]> p1,
                                                                      Dictionary<string, string[]> p2,
                                                                      out Dictionary<string, string[]> p3,
                                                                      Ice.Current current)
        {
            p3 = p2;
            Dictionary<string, string[]> r = new Dictionary<string, string[]>();
            foreach (KeyValuePair<string, string[]> e in p1)
            {
                r[e.Key] = e.Value;
            }
            foreach (KeyValuePair<string, string[]> e in p2)
            {
                r[e.Key] = e.Value;
            }
            return r;
        }

        public override Dictionary<Test.MyEnum, Test.MyEnum[]> opMyEnumMyEnumSD(
            Dictionary<Test.MyEnum, Test.MyEnum[]> p1,
            Dictionary<Test.MyEnum, Test.MyEnum[]> p2,
            out Dictionary<Test.MyEnum, Test.MyEnum[]> p3,
            Ice.Current ice)
        {
            p3 = p2;
            var r = new Dictionary<Test.MyEnum, Test.MyEnum[]>();
            foreach (var e in p1)
            {
                r[e.Key] = e.Value;
            }
            foreach (var e in p2)
            {
                r[e.Key] = e.Value;
            }
            return r;
        }

        public override int[] opIntS(int[] s, Ice.Current current)
        {
            int[] r = new int[s.Length];
            for (int i = 0; i < s.Length; ++i)
            {
                r[i] = -s[i];
            }
            return r;
        }

        public override void opByteSOneway(byte[] s, Ice.Current current)
        {
            lock (this)
            {
                ++_opByteSOnewayCallCount;
            }
        }

        public override int opByteSOnewayCallCount(Ice.Current current)
        {
            lock (this)
            {
                int count = _opByteSOnewayCallCount;
                _opByteSOnewayCallCount = 0;
                return count;
            }
        }

        public override Dictionary<string, string> opContext(Ice.Current current)
        {
            return current.ctx == null ? new Dictionary<string, string>() : new Dictionary<string, string>(current.ctx);
        }

        public override void opDoubleMarshaling(double p1, double[] p2, Ice.Current current)
        {
            double d = 1278312346.0 / 13.0;
            test(p1 == d);
            for (int i = 0; i < p2.Length; ++i)
            {
                test(p2[i] == d);
            }
        }

        public override string[] opStringS(string[] p1, string[] p2, out string[] p3, Ice.Current current)
        {
            p3 = new string[p1.Length + p2.Length];
            Array.Copy(p1, p3, p1.Length);
            Array.Copy(p2, 0, p3, p1.Length, p2.Length);

            string[] r = new string[p1.Length];
            for (int i = 0; i < p1.Length; i++)
            {
                r[i] = p1[p1.Length - (i + 1)];
            }
            return r;
        }

        public override string[][] opStringSS(string[][] p1, string[][] p2, out string[][] p3, Ice.Current current)
        {
            p3 = new string[p1.Length + p2.Length][];
            Array.Copy(p1, p3, p1.Length);
            Array.Copy(p2, 0, p3, p1.Length, p2.Length);

            string[][] r = new string[p2.Length][];
            for (int i = 0; i < p2.Length; i++)
            {
                r[i] = p2[p2.Length - (i + 1)];
            }
            return r;
        }

        public override string[][][] opStringSSS(string[][][] p1, string[][][] p2, out string[][][] p3, Ice.Current current)
        {
            p3 = new string[p1.Length + p2.Length][][];
            Array.Copy(p1, p3, p1.Length);
            Array.Copy(p2, 0, p3, p1.Length, p2.Length);

            string[][][] r = new string[p2.Length][][];
            for (int i = 0; i < p2.Length; i++)
            {
                r[i] = p2[p2.Length - (i + 1)];
            }
            return r;
        }

        public override Dictionary<string, string> opStringStringD(Dictionary<string, string> p1,
                                                                   Dictionary<string, string> p2,
                                                                   out Dictionary<string, string> p3,
                                                                   Ice.Current current)
        {
            p3 = p1;
            Dictionary<string, string> r = new Dictionary<string, string>();
            foreach (KeyValuePair<string, string> e in p1)
            {
                r[e.Key] = e.Value;
            }
            foreach (KeyValuePair<string, string> e in p2)
            {
                r[e.Key] = e.Value;
            }
            return r;
        }

        public override Test.Structure opStruct(Test.Structure p1, Test.Structure p2, out Test.Structure p3, Ice.Current current)
        {
            p3 = p1;
            p3.s.s = "a new string";
            return p2;
        }

        public override void opIdempotent(Ice.Current current)
        {
            test(current.mode == Ice.OperationMode.Idempotent);
        }

        public override void opDerived(Ice.Current current)
        {
        }

        public override byte opByte1(byte opByte1, Ice.Current current)
        {
            return opByte1;
        }

        public override short opShort1(short opShort1, Ice.Current current)
        {
            return opShort1;
        }

        public override int opInt1(int opInt1, Ice.Current current)
        {
            return opInt1;
        }

        public override long opLong1(long opLong1, Ice.Current current)
        {
            return opLong1;
        }

        public override float opFloat1(float opFloat1, Ice.Current current)
        {
            return opFloat1;
        }

        public override double opDouble1(double opDouble1, Ice.Current current)
        {
            return opDouble1;
        }

        public override string opString1(string opString1, Ice.Current current)
        {
            return opString1;
        }

        public override string[] opStringS1(string[] opStringS1, Ice.Current current)
        {
            return opStringS1;
        }

        public override Dictionary<byte, bool> opByteBoolD1(Dictionary<byte, bool> opByteBoolD1, Ice.Current current)
        {
            return opByteBoolD1;
        }

        public override string[] opStringS2(string[] opStringS2, Ice.Current current)
        {
            return opStringS2;
        }

        public override Dictionary<byte, bool> opByteBoolD2(Dictionary<byte, bool> opByteBoolD2, Ice.Current current)
        {
            return opByteBoolD2;
        }

        public override Test.MyClass1 opMyClass1(Test.MyClass1 c, Ice.Current current)
        {
            return c;
        }

        public override Test.MyStruct1 opMyStruct1(Test.MyStruct1 s, Ice.Current current)
        {
            return s;
        }

        public override string[] opStringLiterals(Ice.Current current)
        {
            return new string[]
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
                };
        }

        public override string[] opWStringLiterals(Ice.Current current)
        {
            return opStringLiterals(current);
        }

        public override Test.MyClass_OpMStruct1MarshaledResult opMStruct1(Ice.Current current)
        {
            return new Test.MyClass_OpMStruct1MarshaledResult(
                new Test.Structure(new Test.AnotherStruct()), current);
        }

        public override Test.MyClass_OpMStruct2MarshaledResult opMStruct2(Test.Structure p1, Ice.Current current)
        {
            return new Test.MyClass_OpMStruct2MarshaledResult(p1, p1, current);
        }

        public override Test.MyClass_OpMSeq1MarshaledResult opMSeq1(Ice.Current current)
        {
            return new Test.MyClass_OpMSeq1MarshaledResult(new string[0], current);
        }

        public override Test.MyClass_OpMSeq2MarshaledResult opMSeq2(string[] p1, Ice.Current current)
        {
            return new Test.MyClass_OpMSeq2MarshaledResult(p1, p1, current);
        }

        public override Test.MyClass_OpMDict1MarshaledResult opMDict1(Ice.Current current)
        {
            return new Test.MyClass_OpMDict1MarshaledResult(new Dictionary<string, string>(), current);
        }

        public override Test.MyClass_OpMDict2MarshaledResult opMDict2(Dictionary<string, string> p1, Ice.Current current)
        {
            return new Test.MyClass_OpMDict2MarshaledResult(p1, p1, current);
        }

        private int _opByteSOnewayCallCount = 0;
    }
}
