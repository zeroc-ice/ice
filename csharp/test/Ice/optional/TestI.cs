//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;

namespace Ice
{
    namespace optional
    {
        public class InitialI : Test.Initial
        {
            public void shutdown(Ice.Current current)
            {
                current.adapter.GetCommunicator().shutdown();
            }

            public Ice.Value pingPong(Ice.Value obj, Ice.Current current)
            {
                return obj;
            }

            public void opOptionalException(Ice.Optional<int> a,
                                                     Ice.Optional<string> b,
                                                     Ice.Optional<Test.OneOptional> o,
                                                     Ice.Current current)
            {
                throw new Test.OptionalException(false, a, b, o);
            }

            public void opDerivedException(Ice.Optional<int> a,
                                                    Ice.Optional<string> b,
                                                    Ice.Optional<Test.OneOptional> o,
                                                    Ice.Current current)
            {
                throw new Test.DerivedException(false, a, b, o, b, o);
            }

            public void opRequiredException(Ice.Optional<int> a,
                                                     Ice.Optional<string> b,
                                                     Ice.Optional<Test.OneOptional> o,
                                                     Ice.Current current)
            {
                Test.RequiredException e = new Test.RequiredException();
                e.a = a;
                e.b = b;
                e.o = o;
                if (b.HasValue)
                {
                    e.ss = b.Value;
                }
                if (o.HasValue)
                {
                    e.o2 = o.Value;
                }
                throw e;
            }

            public Ice.Optional<byte> opByte(Ice.Optional<byte> p1,
                                                              out Ice.Optional<byte> p3,
                                                              Ice.Current current)
            {
                p3 = p1;
                return p1;
            }

            public Ice.Optional<bool> opBool(Ice.Optional<bool> p1,
                                                              out Ice.Optional<bool> p3,
                                                              Ice.Current current)
            {
                p3 = p1;
                return p1;
            }

            public Ice.Optional<short> opShort(Ice.Optional<short> p1,
                                                                out Ice.Optional<short> p3,
                                                                Ice.Current current)
            {
                p3 = p1;
                return p1;
            }

            public Ice.Optional<int> opInt(Ice.Optional<int> p1,
                                                            out Ice.Optional<int> p3,
                                                            Ice.Current current)
            {
                p3 = p1;
                return p1;
            }

            public Ice.Optional<long> opLong(Ice.Optional<long> p1,
                                                              out Ice.Optional<long> p3,
                                                              Ice.Current current)
            {
                p3 = p1;
                return p1;
            }

            public Ice.Optional<float> opFloat(Ice.Optional<float> p1,
                                                                out Ice.Optional<float> p3,
                                                                Ice.Current current)
            {
                p3 = p1;
                return p1;
            }

            public Ice.Optional<double> opDouble(Ice.Optional<double> p1,
                                                                  out Ice.Optional<double> p3,
                                                                  Ice.Current current)
            {
                p3 = p1;
                return p1;
            }

            public Ice.Optional<string> opString(Ice.Optional<string> p1,
                                                                  out Ice.Optional<string> p3,
                                                                  Ice.Current current)
            {
                p3 = p1;
                return p1;
            }

            public Ice.Optional<Test.MyEnum> opMyEnum(Ice.Optional<Test.MyEnum> p1,
                                                                       out Ice.Optional<Test.MyEnum> p3,
                                                                       Ice.Current current)
            {
                p3 = p1;
                return p1;
            }

            public Ice.Optional<Test.SmallStruct> opSmallStruct(Ice.Optional<Test.SmallStruct> p1,
                                                                                 out Ice.Optional<Test.SmallStruct> p3,
                                                                                 Ice.Current current)
            {
                p3 = p1;
                return p1;
            }

            public Ice.Optional<Test.FixedStruct> opFixedStruct(Ice.Optional<Test.FixedStruct> p1,
                                                                                 out Ice.Optional<Test.FixedStruct> p3,
                                                                                 Ice.Current current)
            {
                p3 = p1;
                return p1;
            }

            public Ice.Optional<Test.VarStruct> opVarStruct(Ice.Optional<Test.VarStruct> p1,
                                                                             out Ice.Optional<Test.VarStruct> p3,
                                                                             Ice.Current current)
            {
                p3 = p1;
                return p1;
            }

            public Ice.Optional<Test.OneOptional> opOneOptional(Ice.Optional<Test.OneOptional> p1,
                                                                                 out Ice.Optional<Test.OneOptional> p3,
                                                                                 Ice.Current current)
            {
                p3 = p1;
                return p1;
            }

            public Ice.Optional<Ice.ObjectPrx>
            opOneOptionalProxy(Ice.Optional<Ice.ObjectPrx> p1,
                               out Ice.Optional<Ice.ObjectPrx> p3,
                               Ice.Current current)
            {
                p3 = p1;
                return p1;
            }

            public Ice.Optional<byte[]> opByteSeq(Ice.Optional<byte[]> p1,
                                                                   out Ice.Optional<byte[]> p3,
                                                                   Ice.Current current)
            {
                p3 = p1;
                return p1;
            }

            public Ice.Optional<bool[]> opBoolSeq(Ice.Optional<bool[]> p1,
                                                                   out Ice.Optional<bool[]> p3,
                                                                   Ice.Current current)
            {
                p3 = p1;
                return p1;
            }

            public Ice.Optional<short[]> opShortSeq(Ice.Optional<short[]> p1,
                                                                     out Ice.Optional<short[]> p3,
                                                                     Ice.Current current)
            {
                p3 = p1;
                return p1;
            }

            public Ice.Optional<int[]> opIntSeq(Ice.Optional<int[]> p1,
                                                                 out Ice.Optional<int[]> p3,
                                                                 Ice.Current current)
            {
                p3 = p1;
                return p1;
            }

            public Ice.Optional<long[]> opLongSeq(Ice.Optional<long[]> p1,
                                                                   out Ice.Optional<long[]> p3,
                                                                   Ice.Current current)
            {
                p3 = p1;
                return p1;
            }

            public Ice.Optional<float[]> opFloatSeq(Ice.Optional<float[]> p1,
                                                                     out Ice.Optional<float[]> p3,
                                                                     Ice.Current current)
            {
                p3 = p1;
                return p1;
            }

            public Ice.Optional<double[]> opDoubleSeq(Ice.Optional<double[]> p1,
                                                                       out Ice.Optional<double[]> p3,
                                                                       Ice.Current current)
            {
                p3 = p1;
                return p1;
            }

            public Ice.Optional<String[]> opStringSeq(Ice.Optional<String[]> p1,
                                                                       out Ice.Optional<String[]> p3,
                                                                       Ice.Current current)
            {
                p3 = p1;
                return p1;
            }

            public Ice.Optional<Test.SmallStruct[]>
            opSmallStructSeq(Ice.Optional<Test.SmallStruct[]> p1,
                             out Ice.Optional<Test.SmallStruct[]> p3,
                             Ice.Current current)
            {
                p3 = p1;
                return p1;
            }

            public Ice.Optional<List<Test.SmallStruct>>
            opSmallStructList(Ice.Optional<List<Test.SmallStruct>> p1,
                              out Ice.Optional<List<Test.SmallStruct>> p3,
                              Ice.Current current)
            {
                p3 = p1;
                return p1;
            }

            public Ice.Optional<Test.FixedStruct[]>
            opFixedStructSeq(Ice.Optional<Test.FixedStruct[]> p1,
                             out Ice.Optional<Test.FixedStruct[]> p3,
                             Ice.Current current)
            {
                p3 = p1;
                return p1;
            }

            public Ice.Optional<LinkedList<Test.FixedStruct>>
            opFixedStructList(Ice.Optional<LinkedList<Test.FixedStruct>> p1,
                              out Ice.Optional<LinkedList<Test.FixedStruct>> p3,
                              Ice.Current current)
            {
                p3 = p1;
                return p1;
            }

            public Ice.Optional<Test.VarStruct[]>
            opVarStructSeq(Ice.Optional<Test.VarStruct[]> p1,
                           out Ice.Optional<Test.VarStruct[]> p3,
                           Ice.Current current)
            {
                p3 = p1;
                return p1;
            }

            public Ice.Optional<Test.SerializableClass>
            opSerializable(Ice.Optional<Test.SerializableClass> p1,
                           out Ice.Optional<Test.SerializableClass> p3,
                           Ice.Current current)
            {
                p3 = p1;
                return p1;
            }

            public Ice.Optional<Dictionary<int, int>>
            opIntIntDict(Ice.Optional<Dictionary<int, int>> p1,
                         out Ice.Optional<Dictionary<int, int>> p3,
                         Ice.Current current)
            {
                p3 = p1;
                return p1;
            }

            public Ice.Optional<Dictionary<string, int>>
            opStringIntDict(Ice.Optional<Dictionary<string, int>> p1,
                            out Ice.Optional<Dictionary<string, int>> p3,
                            Ice.Current current)
            {
                p3 = p1;
                return p1;
            }

            public Ice.Optional<Dictionary<int, Test.OneOptional>>
            opIntOneOptionalDict(Ice.Optional<Dictionary<int, Test.OneOptional>> p1,
                                 out Ice.Optional<Dictionary<int, Test.OneOptional>> p3,
                                 Ice.Current current)
            {
                p3 = p1;
                return p1;
            }

            public void opClassAndUnknownOptional(Test.A p, Ice.Current current)
            {
            }

            public void sendOptionalClass(bool req,
                                                   Ice.Optional<Test.OneOptional> o,
                                                   Ice.Current current)
            {
            }

            public void returnOptionalClass(bool req,
                                                     out Ice.Optional<Test.OneOptional> o,
                                                     Ice.Current current)
            {
                o = new Test.OneOptional(53);
            }

            public Test.G opG(Test.G g, Ice.Current current)
            {
                return g;
            }

            public void opVoid(Ice.Current current)
            {
            }

            public Test.Initial_OpMStruct1MarshaledResult
            opMStruct1(Ice.Current current)
            {
                return new Test.Initial_OpMStruct1MarshaledResult(new Test.SmallStruct(), current);
            }

            public Test.Initial_OpMStruct2MarshaledResult
            opMStruct2(Ice.Optional<Test.SmallStruct> p1, Ice.Current current)
            {
                return new Test.Initial_OpMStruct2MarshaledResult(p1, p1, current);
            }

            public Test.Initial_OpMSeq1MarshaledResult
            opMSeq1(Ice.Current current)
            {
                return new Test.Initial_OpMSeq1MarshaledResult(new string[0], current);
            }

            public Test.Initial_OpMSeq2MarshaledResult
            opMSeq2(Ice.Optional<string[]> p1, Ice.Current current)
            {
                return new Test.Initial_OpMSeq2MarshaledResult(p1, p1, current);
            }

            public Test.Initial_OpMDict1MarshaledResult
            opMDict1(Ice.Current current)
            {
                return new Test.Initial_OpMDict1MarshaledResult(new Dictionary<string, int>(), current);
            }

            public Test.Initial_OpMDict2MarshaledResult
            opMDict2(Ice.Optional<Dictionary<string, int>> p1, Ice.Current current)
            {
                return new Test.Initial_OpMDict2MarshaledResult(p1, p1, current);
            }

            public Test.Initial_OpMG1MarshaledResult
            opMG1(Ice.Current current)
            {
                return new Test.Initial_OpMG1MarshaledResult(new Test.G(), current);
            }

            public Test.Initial_OpMG2MarshaledResult
            opMG2(Ice.Optional<Test.G> p1, Ice.Current current)
            {
                return new Test.Initial_OpMG2MarshaledResult(p1, p1, current);
            }

            public bool supportsRequiredParams(Ice.Current current)
            {
                return false;
            }

            public bool supportsJavaSerializable(Ice.Current current)
            {
                return false;
            }

            public bool supportsCsharpSerializable(Ice.Current current)
            {
                return true;
            }

            public bool supportsCppStringView(Ice.Current current)
            {
                return false;
            }

            public bool supportsNullOptional(Ice.Current current)
            {
                return true;
            }
        }
    }
}
