//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;

namespace ZeroC.Ice.tagged
{
    public class Initial : Test.IInitial
    {
        public void shutdown(Current current) => current.Adapter.Communicator.Shutdown();

        public AnyClass? pingPong(AnyClass? obj, Current current) => obj;

        public void opTaggedException(int? a, string? b, Test.OneTagged? o, Current current) =>
            throw new Test.TaggedException(false, a, b, o);

        public void opDerivedException(int? a, string? b, Test.OneTagged? o, Current current) =>
            throw new Test.DerivedException(false, a, b, o, b, o);

        public void opRequiredException(int? a,
                                        string? b,
                                        Test.OneTagged? o,
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

        public (byte?, byte?) opByte(byte? p1, Current current) => (p1, p1);

        public (bool?, bool?) opBool(bool? p1, Current current) => (p1, p1);

        public (short?, short?) opShort(short? p1, Current current) => (p1, p1);

        public (int?, int?) opInt(int? p1, Current current) => (p1, p1);

        public (long?, long?) opLong(long? p1, Current current) => (p1, p1);

        public (float?, float?) opFloat(float? p1, Current current) => (p1, p1);

        public (double?, double?) opDouble(double? p1, Current current) => (p1, p1);

        public (string?, string?) opString(string? p1, Current current) => (p1, p1);

        public (Test.MyEnum?, Test.MyEnum?) opMyEnum(Test.MyEnum? p1, Current current) => (p1, p1);

        public (Test.SmallStruct?, Test.SmallStruct?) opSmallStruct(Test.SmallStruct? p1, Current current) => (p1, p1);

        public (Test.FixedStruct?, Test.FixedStruct?) opFixedStruct(Test.FixedStruct? p1, Current current) => (p1, p1);

        public (Test.VarStruct?, Test.VarStruct?) opVarStruct(Test.VarStruct? p1, Current current) => (p1, p1);

        public (Test.OneTagged?, Test.OneTagged?) opOneTagged(Test.OneTagged? p1, Current current) => (p1, p1);

        public (ReadOnlyMemory<byte>, ReadOnlyMemory<byte>) opByteSeq(byte[]? p1, Current current) => (p1, p1);
        public (IEnumerable<byte>?, IEnumerable<byte>?) opByteList(List<byte>? p1, Current current) => (p1, p1);

        public (ReadOnlyMemory<bool>, ReadOnlyMemory<bool>) opBoolSeq(bool[]? p1, Current current) => (p1, p1);
        public (IEnumerable<bool>?, IEnumerable<bool>?) opBoolList(List<bool>? p1, Current current) => (p1, p1);

        public (ReadOnlyMemory<short>, ReadOnlyMemory<short>) opShortSeq(short[]? p1, Current current) => (p1, p1);
        public (IEnumerable<short>?, IEnumerable<short>?) opShortList(List<short>? p1, Current current) => (p1, p1);

        public (ReadOnlyMemory<int>, ReadOnlyMemory<int>) opIntSeq(int[]? p1, Current current) => (p1, p1);
        public (IEnumerable<int>?, IEnumerable<int>?) opIntList(List<int>? p1, Current current) => (p1, p1);

        public (ReadOnlyMemory<long>, ReadOnlyMemory<long>) opLongSeq(long[]? p1, Current current) => (p1, p1);
        public (IEnumerable<long>?, IEnumerable<long>?) opLongList(List<long>? p1, Current current) => (p1, p1);

        public (ReadOnlyMemory<float>, ReadOnlyMemory<float>) opFloatSeq(float[]? p1, Current current) => (p1, p1);
        public (IEnumerable<float>?, IEnumerable<float>?) opFloatList(List<float>? p1, Current current) => (p1, p1);

        public (ReadOnlyMemory<double>, ReadOnlyMemory<double>) opDoubleSeq(double[]? p1, Current current) => (p1, p1);
        public (IEnumerable<double>?, IEnumerable<double>?) opDoubleList(List<double>? p1, Current current) => (p1, p1);

        public (IEnumerable<string>?, IEnumerable<string>?) opStringSeq(string[]? p1, Current current) => (p1, p1);
        public (IEnumerable<string>?, IEnumerable<string>?) opStringList(List<string>? p1, Current current) => (p1, p1);

        public (IEnumerable<Test.SmallStruct>?, IEnumerable<Test.SmallStruct>?) opSmallStructSeq(Test.SmallStruct[]? p1, Current current) =>
            (p1, p1);

        public (IEnumerable<Test.SmallStruct>?, IEnumerable<Test.SmallStruct>?)
        opSmallStructList(List<Test.SmallStruct>? p1, Current current) => (p1, p1);

        public (IEnumerable<Test.FixedStruct>?, IEnumerable<Test.FixedStruct>?)
        opFixedStructSeq(Test.FixedStruct[]? p1, Current current) => (p1, p1);

        public (IEnumerable<Test.FixedStruct>?, IEnumerable<Test.FixedStruct>?)
        opFixedStructList(LinkedList<Test.FixedStruct>? p1, Current current) => (p1, p1);

        public (IEnumerable<Test.VarStruct>?, IEnumerable<Test.VarStruct>?) opVarStructSeq(Test.VarStruct[]? p1, Current current) => (p1, p1);

        public (Test.SerializableClass?, Test.SerializableClass?)
        opSerializable(Test.SerializableClass? p1, Current current) => (p1, p1);

        public (IReadOnlyDictionary<int, int>?, IReadOnlyDictionary<int, int>?)
        opIntIntDict(Dictionary<int, int>? p1, Current current) => (p1, p1);

        public (IReadOnlyDictionary<string, int>?, IReadOnlyDictionary<string, int>?)
        opStringIntDict(Dictionary<string, int>? p1, Current current) => (p1, p1);

        public (IReadOnlyDictionary<int, Test.OneTagged?>?, IReadOnlyDictionary<int, Test.OneTagged?>?)
        opIntOneTaggedDict(Dictionary<int, Test.OneTagged?>? p1, Current current) => (p1, p1);

        public void opClassAndUnknownTagged(Test.A? p, Current current)
        {
        }

        public void sendTaggedClass(bool req, Test.OneTagged? o, Current current)
        {
        }

        public Test.OneTagged? returnTaggedClass(bool req, Current current) => new Test.OneTagged(53);

        public Test.G? opG(Test.G? g, Current current) => g;

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

        public bool supportsNullTagged(Current current) => true;
    }
}
