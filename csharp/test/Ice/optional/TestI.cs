// Copyright (c) ZeroC, Inc.

namespace Ice.optional
{
        public class InitialI : Test.InitialDisp_
        {
            public override void shutdown(Ice.Current current)
            {
                current.adapter.getCommunicator().shutdown();
            }

            public override Ice.Value pingPong(Ice.Value obj, Ice.Current current)
            {
                return obj;
            }

            public override void opOptionalException(int? a,
                                                     string b,
                                                     Ice.Current current)
            {
                throw new Test.OptionalException(false, a, b);
            }

            public override void opDerivedException(int? a,
                                                    string b,
                                                    Ice.Current current)
            {
                throw new Test.DerivedException(false, a, b, "d1", b, "d2");
            }

            public override void opRequiredException(int? a,
                                                     string b,
                                                     Ice.Current current)
            {
                Test.RequiredException e = new Test.RequiredException();
                e.a = a;
                e.b = b;
                if (b is not null)
                {
                    e.ss = b;
                }
                throw e;
            }

            public override byte? opByte(byte? p1,
                                                              out byte? p3,
                                                              Ice.Current current)
            {
                p3 = p1;
                return p1;
            }

            public override bool? opBool(bool? p1,
                                                              out bool? p3,
                                                              Ice.Current current)
            {
                p3 = p1;
                return p1;
            }

            public override short? opShort(short? p1,
                                                                out short? p3,
                                                                Ice.Current current)
            {
                p3 = p1;
                return p1;
            }

            public override int? opInt(int? p1,
                                                            out int? p3,
                                                            Ice.Current current)
            {
                p3 = p1;
                return p1;
            }

            public override long? opLong(long? p1,
                                                              out long? p3,
                                                              Ice.Current current)
            {
                p3 = p1;
                return p1;
            }

            public override float? opFloat(float? p1,
                                                                out float? p3,
                                                                Ice.Current current)
            {
                p3 = p1;
                return p1;
            }

            public override double? opDouble(double? p1,
                                                                  out double? p3,
                                                                  Ice.Current current)
            {
                p3 = p1;
                return p1;
            }

            public override string opString(string p1,
                                                                  out string p3,
                                                                  Ice.Current current)
            {
                p3 = p1;
                return p1;
            }

            public override Test.MyEnum? opMyEnum(Test.MyEnum? p1,
                                                                       out Test.MyEnum? p3,
                                                                       Ice.Current current)
            {
                p3 = p1;
                return p1;
            }

            public override Test.SmallStruct? opSmallStruct(Test.SmallStruct? p1,
                                                                                 out Test.SmallStruct? p3,
                                                                                 Ice.Current current)
            {
                p3 = p1;
                return p1;
            }

            public override Test.FixedStruct? opFixedStruct(Test.FixedStruct? p1,
                                                                                 out Test.FixedStruct? p3,
                                                                                 Ice.Current current)
            {
                p3 = p1;
                return p1;
            }

            public override Test.VarStruct opVarStruct(Test.VarStruct p1,
                                                                             out Test.VarStruct p3,
                                                                             Ice.Current current)
            {
                p3 = p1;
                return p1;
            }

            public override Test.MyInterfacePrx
            opMyInterfaceProxy(Test.MyInterfacePrx p1,
                               out Test.MyInterfacePrx p3,
                               Ice.Current current)
            {
                p3 = p1;
                return p1;
            }

            public override Test.OneOptional opOneOptional(Test.OneOptional p1,
                                                           out Test.OneOptional p3,
                                                           Ice.Current current)
            {
                p3 = p1;
                return p1;
            }

            public override byte[] opByteSeq(byte[] p1,
                                                                   out byte[] p3,
                                                                   Ice.Current current)
            {
                p3 = p1;
                return p1;
            }

            public override bool[] opBoolSeq(bool[] p1,
                                                                   out bool[] p3,
                                                                   Ice.Current current)
            {
                p3 = p1;
                return p1;
            }

            public override List<short> opShortSeq(List<short> p1, out List<short> p3, Ice.Current current)
            {
                p3 = p1;
                return p1;
            }

            public override int[] opIntSeq(int[] p1,
                                                                 out int[] p3,
                                                                 Ice.Current current)
            {
                p3 = p1;
                return p1;
            }

            public override long[] opLongSeq(long[] p1,
                                                                   out long[] p3,
                                                                   Ice.Current current)
            {
                p3 = p1;
                return p1;
            }

            public override float[] opFloatSeq(float[] p1,
                                                                     out float[] p3,
                                                                     Ice.Current current)
            {
                p3 = p1;
                return p1;
            }

            public override double[] opDoubleSeq(double[] p1,
                                                                       out double[] p3,
                                                                       Ice.Current current)
            {
                p3 = p1;
                return p1;
            }

            public override string[] opStringSeq(string[] p1,
                                                                       out string[] p3,
                                                                       Ice.Current current)
            {
                p3 = p1;
                return p1;
            }

            public override Test.SmallStruct[]
            opSmallStructSeq(Test.SmallStruct[] p1,
                             out Test.SmallStruct[] p3,
                             Ice.Current current)
            {
                p3 = p1;
                return p1;
            }

            public override List<Test.SmallStruct>
            opSmallStructList(List<Test.SmallStruct> p1,
                              out List<Test.SmallStruct> p3,
                              Ice.Current current)
            {
                p3 = p1;
                return p1;
            }

            public override Test.FixedStruct[]
            opFixedStructSeq(Test.FixedStruct[] p1,
                             out Test.FixedStruct[] p3,
                             Ice.Current current)
            {
                p3 = p1;
                return p1;
            }

            public override LinkedList<Test.FixedStruct>
            opFixedStructList(LinkedList<Test.FixedStruct> p1,
                              out LinkedList<Test.FixedStruct> p3,
                              Ice.Current current)
            {
                p3 = p1;
                return p1;
            }

            public override Test.VarStruct[]
            opVarStructSeq(Test.VarStruct[] p1,
                           out Test.VarStruct[] p3,
                           Ice.Current current)
            {
                p3 = p1;
                return p1;
            }

            public override Dictionary<int, int>
            opIntIntDict(Dictionary<int, int> p1,
                         out Dictionary<int, int> p3,
                         Ice.Current current)
            {
                p3 = p1;
                return p1;
            }

            public override Dictionary<string, int>
            opStringIntDict(Dictionary<string, int> p1,
                            out Dictionary<string, int> p3,
                            Ice.Current current)
            {
                p3 = p1;
                return p1;
            }

            public override void opClassAndUnknownOptional(Test.A p, Ice.Current current)
            {
            }

            public override Test.G opG(Test.G g, Ice.Current current)
            {
                return g;
            }

            public override void opVoid(Ice.Current current)
            {
            }

            public override Test.Initial_OpMStruct1MarshaledResult
            opMStruct1(Ice.Current current)
            {
                return new Test.Initial_OpMStruct1MarshaledResult(new Test.SmallStruct(), current);
            }

            public override Test.Initial_OpMStruct2MarshaledResult
            opMStruct2(Test.SmallStruct? p1, Ice.Current current)
            {
                return new Test.Initial_OpMStruct2MarshaledResult(p1, p1, current);
            }

            public override Test.Initial_OpMSeq1MarshaledResult
            opMSeq1(Ice.Current current)
            {
                return new Test.Initial_OpMSeq1MarshaledResult(new string[0], current);
            }

            public override Test.Initial_OpMSeq2MarshaledResult
            opMSeq2(string[] p1, Ice.Current current)
            {
                return new Test.Initial_OpMSeq2MarshaledResult(p1, p1, current);
            }

            public override Test.Initial_OpMDict1MarshaledResult
            opMDict1(Ice.Current current)
            {
                return new Test.Initial_OpMDict1MarshaledResult(new Dictionary<string, int>(), current);
            }

            public override Test.Initial_OpMDict2MarshaledResult
            opMDict2(Dictionary<string, int> p1, Ice.Current current)
            {
                return new Test.Initial_OpMDict2MarshaledResult(p1, p1, current);
            }

            public override bool supportsJavaSerializable(Ice.Current current)
            {
                return false;
            }
        }
    }

