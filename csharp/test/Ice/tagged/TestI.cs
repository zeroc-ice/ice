//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;

namespace ZeroC.Ice.Test.Tagged
{
    public class Initial : IInitial
    {
        public void Shutdown(Current current) => current.Adapter.Communicator.ShutdownAsync();

        public AnyClass? PingPong(AnyClass? obj, Current current) => obj;

        public void OpTaggedException(int? a, string? b, OneTagged? o, Current current) =>
            throw new TaggedException(false, a, b, o);

        public void OpDerivedException(int? a, string? b, OneTagged? o, Current current) =>
            throw new DerivedException(false, a, b, o, b, o);

        public void OpRequiredException(int? a,
                                        string? b,
                                        OneTagged? o,
                                        Current current)
        {
            RequiredException e = new RequiredException();
            e.A = a;
            e.B = b;
            e.O = o;
            if (b != null)
            {
                e.Ss = b;
            }

            if (o != null)
            {
                e.O2 = o;
            }
            throw e;
        }

        public (byte?, byte?) OpByte(byte? p1, Current current) => (p1, p1);

        public (bool?, bool?) OpBool(bool? p1, Current current) => (p1, p1);

        public (short?, short?) OpShort(short? p1, Current current) => (p1, p1);

        public (int?, int?) OpInt(int? p1, Current current) => (p1, p1);

        public (long?, long?) OpLong(long? p1, Current current) => (p1, p1);

        public (float?, float?) OpFloat(float? p1, Current current) => (p1, p1);

        public (double?, double?) OpDouble(double? p1, Current current) => (p1, p1);

        public (string?, string?) OpString(string? p1, Current current) => (p1, p1);

        public (MyEnum?, MyEnum?) OpMyEnum(MyEnum? p1, Current current) => (p1, p1);

        public (SmallStruct?, SmallStruct?) OpSmallStruct(SmallStruct? p1, Current current) => (p1, p1);

        public (FixedStruct?, FixedStruct?) OpFixedStruct(FixedStruct? p1, Current current) => (p1, p1);

        public (VarStruct?, VarStruct?) OpVarStruct(VarStruct? p1, Current current) => (p1, p1);

        public (OneTagged?, OneTagged?) OpOneTagged(OneTagged? p1, Current current) => (p1, p1);

        public (ReadOnlyMemory<byte>, ReadOnlyMemory<byte>) OpByteSeq(byte[]? p1, Current current) => (p1, p1);
        public (IEnumerable<byte>?, IEnumerable<byte>?) OpByteList(List<byte>? p1, Current current) => (p1, p1);

        public (ReadOnlyMemory<bool>, ReadOnlyMemory<bool>) OpBoolSeq(bool[]? p1, Current current) => (p1, p1);
        public (IEnumerable<bool>?, IEnumerable<bool>?) OpBoolList(List<bool>? p1, Current current) => (p1, p1);

        public (ReadOnlyMemory<short>, ReadOnlyMemory<short>) OpShortSeq(short[]? p1, Current current) => (p1, p1);
        public (IEnumerable<short>?, IEnumerable<short>?) OpShortList(List<short>? p1, Current current) => (p1, p1);

        public (ReadOnlyMemory<int>, ReadOnlyMemory<int>) OpIntSeq(int[]? p1, Current current) => (p1, p1);
        public (IEnumerable<int>?, IEnumerable<int>?) OpIntList(List<int>? p1, Current current) => (p1, p1);

        public (ReadOnlyMemory<long>, ReadOnlyMemory<long>) OpLongSeq(long[]? p1, Current current) => (p1, p1);
        public (IEnumerable<long>?, IEnumerable<long>?) OpLongList(List<long>? p1, Current current) => (p1, p1);

        public (ReadOnlyMemory<float>, ReadOnlyMemory<float>) OpFloatSeq(float[]? p1, Current current) => (p1, p1);
        public (IEnumerable<float>?, IEnumerable<float>?) OpFloatList(List<float>? p1, Current current) => (p1, p1);

        public (ReadOnlyMemory<double>, ReadOnlyMemory<double>) OpDoubleSeq(double[]? p1, Current current) => (p1, p1);
        public (IEnumerable<double>?, IEnumerable<double>?) OpDoubleList(List<double>? p1, Current current) => (p1, p1);

        public (IEnumerable<string>?, IEnumerable<string>?) OpStringSeq(string[]? p1, Current current) => (p1, p1);
        public (IEnumerable<string>?, IEnumerable<string>?) OpStringList(List<string>? p1, Current current) => (p1, p1);

        public (IEnumerable<SmallStruct>?, IEnumerable<SmallStruct>?) OpSmallStructSeq(SmallStruct[]? p1, Current current) =>
            (p1, p1);

        public (IEnumerable<SmallStruct>?, IEnumerable<SmallStruct>?)
        OpSmallStructList(List<SmallStruct>? p1, Current current) => (p1, p1);

        public (IEnumerable<FixedStruct>?, IEnumerable<FixedStruct>?)
        OpFixedStructSeq(FixedStruct[]? p1, Current current) => (p1, p1);

        public (IEnumerable<FixedStruct>?, IEnumerable<FixedStruct>?)
        OpFixedStructList(LinkedList<FixedStruct>? p1, Current current) => (p1, p1);

        public (IEnumerable<VarStruct>?, IEnumerable<VarStruct>?) OpVarStructSeq(VarStruct[]? p1, Current current) => (p1, p1);

        public (SerializableClass?, SerializableClass?)
        OpSerializable(SerializableClass? p1, Current current) => (p1, p1);

        public (IReadOnlyDictionary<int, int>?, IReadOnlyDictionary<int, int>?)
        OpIntIntDict(Dictionary<int, int>? p1, Current current) => (p1, p1);

        public (IReadOnlyDictionary<string, int>?, IReadOnlyDictionary<string, int>?)
        OpStringIntDict(Dictionary<string, int>? p1, Current current) => (p1, p1);

        public (IReadOnlyDictionary<int, OneTagged?>?, IReadOnlyDictionary<int, OneTagged?>?)
        OpIntOneTaggedDict(Dictionary<int, OneTagged?>? p1, Current current) => (p1, p1);

        public void OpClassAndUnknownTagged(A? p, Current current)
        {
        }

        public void SendTaggedClass(bool req, OneTagged? o, Current current)
        {
        }

        public OneTagged? ReturnTaggedClass(bool req, Current current) => new OneTagged(53);

        public G? OpG(G? g, Current current) => g;

        public void OpVoid(Current current)
        {
        }

        public IInitial.OpMStruct1MarshaledReturnValue
        OpMStruct1(Current current) => new IInitial.OpMStruct1MarshaledReturnValue(new SmallStruct(), current);

        public IInitial.OpMStruct2MarshaledReturnValue
        OpMStruct2(SmallStruct? p1, Current current) =>
            new IInitial.OpMStruct2MarshaledReturnValue(p1, p1, current);

        public IInitial.OpMSeq1MarshaledReturnValue
        OpMSeq1(Current current) => new IInitial.OpMSeq1MarshaledReturnValue(Array.Empty<string>(), current);

        public IInitial.OpMSeq2MarshaledReturnValue
        OpMSeq2(string[]? p1, Current current) => new IInitial.OpMSeq2MarshaledReturnValue(p1, p1, current);

        public IInitial.OpMDict1MarshaledReturnValue
        OpMDict1(Current current) =>
            new IInitial.OpMDict1MarshaledReturnValue(new Dictionary<string, int>(), current);

        public IInitial.OpMDict2MarshaledReturnValue
        OpMDict2(Dictionary<string, int>? p1, Current current) =>
            new IInitial.OpMDict2MarshaledReturnValue(p1, p1, current);

        public IInitial.OpMG1MarshaledReturnValue
        OpMG1(Current current) => new IInitial.OpMG1MarshaledReturnValue(new G(), current);

        public IInitial.OpMG2MarshaledReturnValue
        OpMG2(G? p1, Current current) => new IInitial.OpMG2MarshaledReturnValue(p1, p1, current);

        public bool SupportsRequiredParams(Current current) => false;

        public bool SupportsJavaSerializable(Current current) => false;

        public bool SupportsCsharpSerializable(Current current) => true;

        public bool SupportsCppStringView(Current current) => false;

        public bool SupportsNullTagged(Current current) => true;
    }
}
