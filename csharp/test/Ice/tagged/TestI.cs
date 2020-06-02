//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;

namespace ZeroC.Ice.Test.Tagged
{
    public class Initial : IInitial
    {
        public void shutdown(Current current) => current.Adapter.Communicator.Shutdown();

        public AnyClass? pingPong(AnyClass? obj, Current current) => obj;

        public void opTaggedException(int? a, string? b, OneTagged? o, Current current) =>
            throw new TaggedException(false, a, b, o);

        public void opDerivedException(int? a, string? b, OneTagged? o, Current current) =>
            throw new DerivedException(false, a, b, o, b, o);

        public void opRequiredException(int? a,
                                        string? b,
                                        OneTagged? o,
                                        Current current)
        {
            RequiredException e = new RequiredException();
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

        public (MyEnum?, MyEnum?) opMyEnum(MyEnum? p1, Current current) => (p1, p1);

        public (SmallStruct?, SmallStruct?) opSmallStruct(SmallStruct? p1, Current current) => (p1, p1);

        public (FixedStruct?, FixedStruct?) opFixedStruct(FixedStruct? p1, Current current) => (p1, p1);

        public (VarStruct?, VarStruct?) opVarStruct(VarStruct? p1, Current current) => (p1, p1);

        public (OneTagged?, OneTagged?) opOneTagged(OneTagged? p1, Current current) => (p1, p1);

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

        public (IEnumerable<SmallStruct>?, IEnumerable<SmallStruct>?) opSmallStructSeq(SmallStruct[]? p1, Current current) =>
            (p1, p1);

        public (IEnumerable<SmallStruct>?, IEnumerable<SmallStruct>?)
        opSmallStructList(List<SmallStruct>? p1, Current current) => (p1, p1);

        public (IEnumerable<FixedStruct>?, IEnumerable<FixedStruct>?)
        opFixedStructSeq(FixedStruct[]? p1, Current current) => (p1, p1);

        public (IEnumerable<FixedStruct>?, IEnumerable<FixedStruct>?)
        opFixedStructList(LinkedList<FixedStruct>? p1, Current current) => (p1, p1);

        public (IEnumerable<VarStruct>?, IEnumerable<VarStruct>?) opVarStructSeq(VarStruct[]? p1, Current current) => (p1, p1);

        public (SerializableClass?, SerializableClass?)
        opSerializable(SerializableClass? p1, Current current) => (p1, p1);

        public (IReadOnlyDictionary<int, int>?, IReadOnlyDictionary<int, int>?)
        opIntIntDict(Dictionary<int, int>? p1, Current current) => (p1, p1);

        public (IReadOnlyDictionary<string, int>?, IReadOnlyDictionary<string, int>?)
        opStringIntDict(Dictionary<string, int>? p1, Current current) => (p1, p1);

        public (IReadOnlyDictionary<int, OneTagged?>?, IReadOnlyDictionary<int, OneTagged?>?)
        opIntOneTaggedDict(Dictionary<int, OneTagged?>? p1, Current current) => (p1, p1);

        public void opClassAndUnknownTagged(A? p, Current current)
        {
        }

        public void sendTaggedClass(bool req, OneTagged? o, Current current)
        {
        }

        public OneTagged? returnTaggedClass(bool req, Current current) => new OneTagged(53);

        public G? opG(G? g, Current current) => g;

        public void opVoid(Current current)
        {
        }

        public IInitial.OpMStruct1MarshaledReturnValue
        opMStruct1(Current current) => new IInitial.OpMStruct1MarshaledReturnValue(new SmallStruct(), current);

        public IInitial.OpMStruct2MarshaledReturnValue
        opMStruct2(SmallStruct? p1, Current current) =>
            new IInitial.OpMStruct2MarshaledReturnValue(p1, p1, current);

        public IInitial.OpMSeq1MarshaledReturnValue
        opMSeq1(Current current) => new IInitial.OpMSeq1MarshaledReturnValue(new string[0], current);

        public IInitial.OpMSeq2MarshaledReturnValue
        opMSeq2(string[]? p1, Current current) => new IInitial.OpMSeq2MarshaledReturnValue(p1, p1, current);

        public IInitial.OpMDict1MarshaledReturnValue
        opMDict1(Current current) =>
            new IInitial.OpMDict1MarshaledReturnValue(new Dictionary<string, int>(), current);

        public IInitial.OpMDict2MarshaledReturnValue
        opMDict2(Dictionary<string, int>? p1, Current current) =>
            new IInitial.OpMDict2MarshaledReturnValue(p1, p1, current);

        public IInitial.OpMG1MarshaledReturnValue
        opMG1(Current current) => new IInitial.OpMG1MarshaledReturnValue(new G(), current);

        public IInitial.OpMG2MarshaledReturnValue
        opMG2(G? p1, Current current) => new IInitial.OpMG2MarshaledReturnValue(p1, p1, current);

        public bool supportsRequiredParams(Current current) => false;

        public bool supportsJavaSerializable(Current current) => false;

        public bool supportsCsharpSerializable(Current current) => true;

        public bool supportsCppStringView(Current current) => false;

        public bool supportsNullTagged(Current current) => true;
    }
}
