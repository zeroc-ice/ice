//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Collections.Generic;

namespace Ice.optional
{
    public class Initial : Test.IInitial
    {
        public void shutdown(Current current) => current.Adapter.Communicator.shutdown();

        public Value pingPong(Value obj, Current current) => obj;

        public void opOptionalException(int? a, string? b, Test.OneOptional? o, Current current) =>
            throw new Test.OptionalException(false, a, b, o);

        public void opDerivedException(int? a, string? b, Test.OneOptional? o, Current current) =>
            throw new Test.DerivedException(false, a, b, o, b, o);

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

        public Test.IInitial.OpByteReturnValue
        opByte(byte? p1, Current current) => new Test.IInitial.OpByteReturnValue(p1, p1);

        public Test.IInitial.OpBoolReturnValue
        opBool(bool? p1, Current current) => new Test.IInitial.OpBoolReturnValue(p1, p1);

        public Test.IInitial.OpShortReturnValue
        opShort(short? p1, Current current) => new Test.IInitial.OpShortReturnValue(p1, p1);

        public Test.IInitial.OpIntReturnValue
        opInt(int? p1, Current current) => new Test.IInitial.OpIntReturnValue(p1, p1);

        public Test.IInitial.OpLongReturnValue
        opLong(long? p1, Current current) => new Test.IInitial.OpLongReturnValue(p1, p1);

        public Test.IInitial.OpFloatReturnValue
        opFloat(float? p1, Current current) => new Test.IInitial.OpFloatReturnValue(p1, p1);

        public Test.IInitial.OpDoubleReturnValue
        opDouble(double? p1, Current current) => new Test.IInitial.OpDoubleReturnValue(p1, p1);

        public Test.IInitial.OpStringReturnValue
        opString(string? p1, Current current) => new Test.IInitial.OpStringReturnValue(p1, p1);

        public Test.IInitial.OpMyEnumReturnValue
        opMyEnum(Test.MyEnum? p1, Current current) => new Test.IInitial.OpMyEnumReturnValue(p1, p1);

        public Test.IInitial.OpSmallStructReturnValue
        opSmallStruct(Test.SmallStruct? p1, Current current) => new Test.IInitial.OpSmallStructReturnValue(p1, p1);

        public Test.IInitial.OpFixedStructReturnValue
        opFixedStruct(Test.FixedStruct? p1, Current current) => new Test.IInitial.OpFixedStructReturnValue(p1, p1);

        public Test.IInitial.OpVarStructReturnValue
        opVarStruct(Test.VarStruct? p1, Current current) => new Test.IInitial.OpVarStructReturnValue(p1, p1);

        public Test.IInitial.OpOneOptionalReturnValue
        opOneOptional(Test.OneOptional? p1, Current current) => new Test.IInitial.OpOneOptionalReturnValue(p1, p1);

        public Test.IInitial.OpOneOptionalProxyReturnValue
        opOneOptionalProxy(IObjectPrx? p1, Current current) => new Test.IInitial.OpOneOptionalProxyReturnValue(p1, p1);

        public Test.IInitial.OpByteSeqReturnValue
        opByteSeq(byte[]? p1, Current current) => new Test.IInitial.OpByteSeqReturnValue(p1, p1);

        public Test.IInitial.OpBoolSeqReturnValue
        opBoolSeq(bool[]? p1, Current current) => new Test.IInitial.OpBoolSeqReturnValue(p1, p1);

        public Test.IInitial.OpShortSeqReturnValue
        opShortSeq(short[]? p1, Current current) => new Test.IInitial.OpShortSeqReturnValue(p1, p1);

        public Test.IInitial.OpIntSeqReturnValue
        opIntSeq(int[]? p1, Current current) => new Test.IInitial.OpIntSeqReturnValue(p1, p1);

        public Test.IInitial.OpLongSeqReturnValue
        opLongSeq(long[]? p1, Current current) => new Test.IInitial.OpLongSeqReturnValue(p1, p1);

        public Test.IInitial.OpFloatSeqReturnValue
        opFloatSeq(float[]? p1, Current current) => new Test.IInitial.OpFloatSeqReturnValue(p1, p1);

        public Test.IInitial.OpDoubleSeqReturnValue
        opDoubleSeq(double[]? p1, Current current) => new Test.IInitial.OpDoubleSeqReturnValue(p1, p1);

        public Test.IInitial.OpStringSeqReturnValue
        opStringSeq(string[]? p1, Current current) => new Test.IInitial.OpStringSeqReturnValue(p1, p1);

        public Test.IInitial.OpSmallStructSeqReturnValue
        opSmallStructSeq(Test.SmallStruct[]? p1, Current current) => new Test.IInitial.OpSmallStructSeqReturnValue(p1, p1);

        public Test.IInitial.OpSmallStructListReturnValue
        opSmallStructList(List<Test.SmallStruct>? p1, Current current) => new Test.IInitial.OpSmallStructListReturnValue(p1, p1);

        public Test.IInitial.OpFixedStructSeqReturnValue
        opFixedStructSeq(Test.FixedStruct[]? p1, Current current) => new Test.IInitial.OpFixedStructSeqReturnValue(p1, p1);

        public Test.IInitial.OpFixedStructListReturnValue
        opFixedStructList(LinkedList<Test.FixedStruct>? p1, Current current) => new Test.IInitial.OpFixedStructListReturnValue(p1, p1);

        public Test.IInitial.OpVarStructSeqReturnValue
        opVarStructSeq(Test.VarStruct[]? p1, Current current) => new Test.IInitial.OpVarStructSeqReturnValue(p1, p1);

        public Test.IInitial.OpSerializableReturnValue
        opSerializable(Test.SerializableClass? p1, Current current) => new Test.IInitial.OpSerializableReturnValue(p1, p1);

        public Test.IInitial.OpIntIntDictReturnValue
        opIntIntDict(Dictionary<int, int>? p1, Current current) => new Test.IInitial.OpIntIntDictReturnValue(p1, p1);

        public Test.IInitial.OpStringIntDictReturnValue
        opStringIntDict(Dictionary<string, int>? p1, Current current) => new Test.IInitial.OpStringIntDictReturnValue(p1, p1);

        public Test.IInitial.OpIntOneOptionalDictReturnValue
        opIntOneOptionalDict(Dictionary<int, Test.OneOptional>? p1, Current current) =>
            new Test.IInitial.OpIntOneOptionalDictReturnValue(p1, p1);

        public void opClassAndUnknownOptional(Test.A p, Current current)
        {
        }

        public void sendOptionalClass(bool req, Test.OneOptional? o, Current current)
        {
        }

        public Test.OneOptional? returnOptionalClass(bool req, Current current) => new Test.OneOptional(53);

        public Test.G opG(Test.G g, Current current) => g;

        public void opVoid(Current current)
        {
        }

        public Test.IInitial.OpMStruct1MarshaledReturnValue
        opMStruct1(Current current) => new Test.IInitial.OpMStruct1MarshaledReturnValue(new Test.SmallStruct(), current);

        public Test.IInitial.OpMStruct2MarshaledReturnValue
        opMStruct2(Test.SmallStruct? p1, Current current) =>
            new Test.IInitial.OpMStruct2MarshaledReturnValue(p1, p1, current);

        public Test.IInitial.OpMSeq1MarshaledReturnValue
        opMSeq1(Current current) => new Test.IInitial.OpMSeq1MarshaledReturnValue(new string[0], current);

        public Test.IInitial.OpMSeq2MarshaledReturnValue
        opMSeq2(string[]? p1, Current current) => new Test.IInitial.OpMSeq2MarshaledReturnValue(p1, p1, current);

        public Test.IInitial.OpMDict1MarshaledReturnValue
        opMDict1(Current current) =>
            new Test.IInitial.OpMDict1MarshaledReturnValue(new Dictionary<string, int>(), current);

        public Test.IInitial.OpMDict2MarshaledReturnValue
        opMDict2(Dictionary<string, int>? p1, Current current) =>
            new Test.IInitial.OpMDict2MarshaledReturnValue(p1, p1, current);

        public Test.IInitial.OpMG1MarshaledReturnValue
        opMG1(Current current) => new Test.IInitial.OpMG1MarshaledReturnValue(new Test.G(), current);

        public Test.IInitial.OpMG2MarshaledReturnValue
        opMG2(Test.G? p1, Current current) => new Test.IInitial.OpMG2MarshaledReturnValue(p1, p1, current);

        public bool supportsRequiredParams(Current current) => false;

        public bool supportsJavaSerializable(Current current) => false;

        public bool supportsCsharpSerializable(Current current) => true;

        public bool supportsCppStringView(Current current) => false;

        public bool supportsNullOptional(Current current) => true;
    }
}
