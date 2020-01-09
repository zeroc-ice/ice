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
            public void shutdown(Current current)
            {
                current.Adapter.Communicator.shutdown();
            }

            public Ice.Value pingPong(Ice.Value obj, Current current)
            {
                return obj;
            }

            public void opOptionalException(int? a,
                                            string? b,
                                            Test.OneOptional? o,
                                            Current current)
            {
                throw new Test.OptionalException(false, a, b, o);
            }

            public void opDerivedException(int? a,
                                           string? b,
                                           Test.OneOptional? o,
                                           Current current)
            {
                throw new Test.DerivedException(false, a, b, o, b, o);
            }

            public void opRequiredException(int? a,
                                            string? b,
                                            Test.OneOptional? o,
                                            Current current)
            {
                Test.RequiredException e = new Test.RequiredException();
                e.a = a;
                e.b = b;
                e.o = o;
                if (b != null)
                {
                    e.ss = b;
                }

                if (o != null)
                {
                    e.o2 = o;
                }
                throw e;
            }

            public Test.Initial.OpByteReturnValue
            opByte(byte? p1, Current current) => new Test.Initial.OpByteReturnValue(p1, p1);

            public Test.Initial.OpBoolReturnValue
            opBool(bool? p1, Current current) => new Test.Initial.OpBoolReturnValue(p1, p1);

            public Test.Initial.OpShortReturnValue
            opShort(short? p1, Current current) => new Test.Initial.OpShortReturnValue(p1, p1);

            public Test.Initial.OpIntReturnValue
            opInt(int? p1, Current current) => new Test.Initial.OpIntReturnValue(p1, p1);

            public Test.Initial.OpLongReturnValue
            opLong(long? p1, Current current) => new Test.Initial.OpLongReturnValue(p1, p1);

            public Test.Initial.OpFloatReturnValue
            opFloat(float? p1, Current current) => new Test.Initial.OpFloatReturnValue(p1, p1);

            public Test.Initial.OpDoubleReturnValue
            opDouble(double? p1, Current current) => new Test.Initial.OpDoubleReturnValue(p1, p1);

            public Test.Initial.OpStringReturnValue
            opString(string? p1, Current current) => new Test.Initial.OpStringReturnValue(p1, p1);

            public Test.Initial.OpMyEnumReturnValue
            opMyEnum(Test.MyEnum? p1, Current current) => new Test.Initial.OpMyEnumReturnValue(p1, p1);

            public Test.Initial.OpSmallStructReturnValue
            opSmallStruct(Test.SmallStruct? p1, Current current) => new Test.Initial.OpSmallStructReturnValue(p1, p1);

            public Test.Initial.OpFixedStructReturnValue
            opFixedStruct(Test.FixedStruct? p1, Current current) => new Test.Initial.OpFixedStructReturnValue(p1, p1);

            public Test.Initial.OpVarStructReturnValue
            opVarStruct(Test.VarStruct? p1, Current current) => new Test.Initial.OpVarStructReturnValue(p1, p1);

            public Test.Initial.OpOneOptionalReturnValue
            opOneOptional(Test.OneOptional? p1, Current current) => new Test.Initial.OpOneOptionalReturnValue(p1, p1);

            public Test.Initial.OpOneOptionalProxyReturnValue
            opOneOptionalProxy(IObjectPrx? p1, Current current) => new Test.Initial.OpOneOptionalProxyReturnValue(p1, p1);

            public Test.Initial.OpByteSeqReturnValue
            opByteSeq(byte[]? p1, Current current) => new Test.Initial.OpByteSeqReturnValue(p1, p1);

            public Test.Initial.OpBoolSeqReturnValue
            opBoolSeq(bool[]? p1, Current current) => new Test.Initial.OpBoolSeqReturnValue(p1, p1);

            public Test.Initial.OpShortSeqReturnValue
            opShortSeq(short[]? p1, Current current) => new Test.Initial.OpShortSeqReturnValue(p1, p1);

            public Test.Initial.OpIntSeqReturnValue
            opIntSeq(int[]? p1, Current current) => new Test.Initial.OpIntSeqReturnValue(p1, p1);

            public Test.Initial.OpLongSeqReturnValue
            opLongSeq(long[]? p1, Current current) => new Test.Initial.OpLongSeqReturnValue(p1, p1);

            public Test.Initial.OpFloatSeqReturnValue
            opFloatSeq(float[]? p1, Current current) => new Test.Initial.OpFloatSeqReturnValue(p1, p1);

            public Test.Initial.OpDoubleSeqReturnValue
            opDoubleSeq(double[]? p1, Current current) => new Test.Initial.OpDoubleSeqReturnValue(p1, p1);

            public Test.Initial.OpStringSeqReturnValue
            opStringSeq(string[]? p1, Current current) => new Test.Initial.OpStringSeqReturnValue(p1, p1);

            public Test.Initial.OpSmallStructSeqReturnValue
            opSmallStructSeq(Test.SmallStruct[]? p1, Current current) => new Test.Initial.OpSmallStructSeqReturnValue(p1, p1);

            public Test.Initial.OpSmallStructListReturnValue
            opSmallStructList(List<Test.SmallStruct>? p1, Current current) => new Test.Initial.OpSmallStructListReturnValue(p1, p1);

            public Test.Initial.OpFixedStructSeqReturnValue
            opFixedStructSeq(Test.FixedStruct[]? p1, Current current) => new Test.Initial.OpFixedStructSeqReturnValue(p1, p1);

            public Test.Initial.OpFixedStructListReturnValue
            opFixedStructList(LinkedList<Test.FixedStruct>? p1, Current current) => new Test.Initial.OpFixedStructListReturnValue(p1, p1);

            public Test.Initial.OpVarStructSeqReturnValue
            opVarStructSeq(Test.VarStruct[]? p1, Current current) => new Test.Initial.OpVarStructSeqReturnValue(p1, p1);

            public Test.Initial.OpSerializableReturnValue
            opSerializable(Test.SerializableClass? p1, Current current) => new Test.Initial.OpSerializableReturnValue(p1, p1);

            public Test.Initial.OpIntIntDictReturnValue
            opIntIntDict(Dictionary<int, int>? p1, Current current) => new Test.Initial.OpIntIntDictReturnValue(p1, p1);

            public Test.Initial.OpStringIntDictReturnValue
            opStringIntDict(Dictionary<string, int>? p1, Current current) => new Test.Initial.OpStringIntDictReturnValue(p1, p1);

            public Test.Initial.OpIntOneOptionalDictReturnValue
            opIntOneOptionalDict(Dictionary<int, Test.OneOptional>? p1, Current current) =>
                new Test.Initial.OpIntOneOptionalDictReturnValue(p1, p1);

            public void opClassAndUnknownOptional(Test.A p, Current current)
            {
            }

            public void sendOptionalClass(bool req, Test.OneOptional? o, Current current)
            {
            }

            public Test.OneOptional? returnOptionalClass(bool req, Current current)
            {
                return new Test.OneOptional(53);
            }

            public Test.G opG(Test.G g, Current current)
            {
                return g;
            }

            public void opVoid(Current current)
            {
            }

            public Test.Initial.OpMStruct1MarshaledReturnValue
            opMStruct1(Current current)
            {
                return new Test.Initial.OpMStruct1MarshaledReturnValue(new Test.SmallStruct(), current);
            }

            public Test.Initial.OpMStruct2MarshaledReturnValue
            opMStruct2(Test.SmallStruct? p1, Current current)
            {
                return new Test.Initial.OpMStruct2MarshaledReturnValue(p1, p1, current);
            }

            public Test.Initial.OpMSeq1MarshaledReturnValue
            opMSeq1(Current current)
            {
                return new Test.Initial.OpMSeq1MarshaledReturnValue(new string[0], current);
            }

            public Test.Initial.OpMSeq2MarshaledReturnValue
            opMSeq2(string[]? p1, Current current)
            {
                return new Test.Initial.OpMSeq2MarshaledReturnValue(p1, p1, current);
            }

            public Test.Initial.OpMDict1MarshaledReturnValue
            opMDict1(Current current)
            {
                return new Test.Initial.OpMDict1MarshaledReturnValue(new Dictionary<string, int>(), current);
            }

            public Test.Initial.OpMDict2MarshaledReturnValue
            opMDict2(Dictionary<string, int>? p1, Current current)
            {
                return new Test.Initial.OpMDict2MarshaledReturnValue(p1, p1, current);
            }

            public Test.Initial.OpMG1MarshaledReturnValue
            opMG1(Current current)
            {
                return new Test.Initial.OpMG1MarshaledReturnValue(new Test.G(), current);
            }

            public Test.Initial.OpMG2MarshaledReturnValue
            opMG2(Test.G? p1, Current current)
            {
                return new Test.Initial.OpMG2MarshaledReturnValue(p1, p1, current);
            }

            public bool supportsRequiredParams(Current current)
            {
                return false;
            }

            public bool supportsJavaSerializable(Current current)
            {
                return false;
            }

            public bool supportsCsharpSerializable(Current current)
            {
                return true;
            }

            public bool supportsCppStringView(Current current)
            {
                return false;
            }

            public bool supportsNullOptional(Current current)
            {
                return true;
            }
        }
    }
}
