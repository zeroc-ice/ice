// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Collections.Generic;
using System.Threading;

namespace ZeroC.Ice.Test.Tagged
{
    public class Initial : IInitial
    {
        public void Shutdown(Current current, CancellationToken cancel) =>
            current.Communicator.ShutdownAsync();

        public AnyClass? PingPong(AnyClass? obj, Current current, CancellationToken cancel) => obj;

        public void OpTaggedException(int? a, string? b, VarStruct? vs, Current current, CancellationToken cancel) =>
            throw new TaggedException(false, a, b, vs);

        public void OpDerivedException(int? a, string? b, VarStruct? vs, Current current, CancellationToken cancel) =>
            throw new DerivedException(false, a, b, vs, b, vs);

        public void OpRequiredException(int? a, string? b, VarStruct? vs, Current current, CancellationToken cancel) =>
            throw new RequiredException(false, a, b, vs, b ?? "test", vs ?? new VarStruct(""));

        public (byte?, byte?) OpByte(byte? p1, Current current, CancellationToken cancel) => (p1, p1);

        public (bool?, bool?) OpBool(bool? p1, Current current, CancellationToken cancel) => (p1, p1);

        public (short?, short?) OpShort(short? p1, Current current, CancellationToken cancel) => (p1, p1);

        public (int?, int?) OpInt(int? p1, Current current, CancellationToken cancel) => (p1, p1);

        public (long?, long?) OpLong(long? p1, Current current, CancellationToken cancel) => (p1, p1);

        public (float?, float?) OpFloat(float? p1, Current current, CancellationToken cancel) => (p1, p1);

        public (double?, double?) OpDouble(double? p1, Current current, CancellationToken cancel) => (p1, p1);

        public (string?, string?) OpString(string? p1, Current current, CancellationToken cancel) => (p1, p1);

        public (MyEnum?, MyEnum?) OpMyEnum(MyEnum? p1, Current current, CancellationToken cancel) => (p1, p1);

        public (SmallStruct?, SmallStruct?) OpSmallStruct(
            SmallStruct? p1,
            Current current,
            CancellationToken cancel) => (p1, p1);

        public (FixedStruct?, FixedStruct?) OpFixedStruct(
            FixedStruct? p1,
            Current current,
            CancellationToken cancel) => (p1, p1);

        public (VarStruct?, VarStruct?) OpVarStruct(VarStruct? p1, Current current, CancellationToken cancel) =>
            (p1, p1);

        public (ReadOnlyMemory<byte>, ReadOnlyMemory<byte>) OpByteSeq(
            byte[]? p1,
            Current current,
            CancellationToken cancel) => (p1, p1);
        public (IEnumerable<byte>?, IEnumerable<byte>?) OpByteList(
            List<byte>? p1,
            Current current,
            CancellationToken cancel) => (p1, p1);

        public (ReadOnlyMemory<bool>, ReadOnlyMemory<bool>) OpBoolSeq(
            bool[]? p1,
            Current current,
            CancellationToken cancel) => (p1, p1);
        public (IEnumerable<bool>?, IEnumerable<bool>?) OpBoolList(
            List<bool>? p1,
            Current current,
            CancellationToken cancel) => (p1, p1);

        public (ReadOnlyMemory<short>, ReadOnlyMemory<short>) OpShortSeq(
            short[]? p1,
            Current current,
            CancellationToken cancel) => (p1, p1);
        public (IEnumerable<short>?, IEnumerable<short>?) OpShortList(
            List<short>? p1,
            Current current,
            CancellationToken cancel) => (p1, p1);

        public (ReadOnlyMemory<int>, ReadOnlyMemory<int>) OpIntSeq(
            int[]? p1,
            Current current,
            CancellationToken cancel) => (p1, p1);
        public (IEnumerable<int>?, IEnumerable<int>?) OpIntList(
            List<int>? p1,
            Current current,
            CancellationToken cancel) => (p1, p1);

        public (ReadOnlyMemory<long>, ReadOnlyMemory<long>) OpLongSeq(
            long[]? p1,
            Current current,
            CancellationToken cancel) => (p1, p1);
        public (IEnumerable<long>?, IEnumerable<long>?) OpLongList(
            List<long>? p1,
            Current current,
            CancellationToken cancel) => (p1, p1);

        public (ReadOnlyMemory<float>, ReadOnlyMemory<float>) OpFloatSeq(
            float[]? p1,
            Current current,
            CancellationToken cancel) => (p1, p1);
        public (IEnumerable<float>?, IEnumerable<float>?) OpFloatList(
            List<float>? p1,
            Current current,
            CancellationToken cancel) => (p1, p1);

        public (ReadOnlyMemory<double>, ReadOnlyMemory<double>) OpDoubleSeq(
            double[]? p1,
            Current current,
            CancellationToken cancel) => (p1, p1);
        public (IEnumerable<double>?, IEnumerable<double>?) OpDoubleList(
            List<double>? p1,
            Current current,
            CancellationToken cancel) => (p1, p1);

        public (IEnumerable<string>?, IEnumerable<string>?) OpStringSeq(
            string[]? p1,
            Current current,
            CancellationToken cancel) => (p1, p1);
        public (IEnumerable<string>?, IEnumerable<string>?) OpStringList(
            List<string>? p1,
            Current current,
            CancellationToken cancel) => (p1, p1);

        public (IEnumerable<SmallStruct>?, IEnumerable<SmallStruct>?) OpSmallStructSeq(
            SmallStruct[]? p1,
            Current current,
            CancellationToken cancel) =>
            (p1, p1);

        public (IEnumerable<SmallStruct>?, IEnumerable<SmallStruct>?)
        OpSmallStructList(
            List<SmallStruct>? p1,
            Current current,
            CancellationToken cancel) => (p1, p1);

        public (IEnumerable<FixedStruct>?, IEnumerable<FixedStruct>?)
        OpFixedStructSeq(
            FixedStruct[]? p1,
            Current current,
            CancellationToken cancel) => (p1, p1);

        public (IEnumerable<FixedStruct>?, IEnumerable<FixedStruct>?)
        OpFixedStructList(
            LinkedList<FixedStruct>? p1,
            Current current,
            CancellationToken cancel) => (p1, p1);

        public (IEnumerable<VarStruct>?, IEnumerable<VarStruct>?) OpVarStructSeq(
            VarStruct[]? p1,
            Current current,
            CancellationToken cancel) => (p1, p1);

        public (IEnumerable<IEnumerable<int>>?, IEnumerable<IEnumerable<int>>?) OpIntSeqSeq(
            int[][]? p1,
            Current current,
            CancellationToken cancel) => (p1, p1);

        public (IReadOnlyDictionary<int, int>?, IReadOnlyDictionary<int, int>?) OpIntIntDict(
            Dictionary<int, int>? p1,
            Current current,
            CancellationToken cancel) => (p1, p1);

        public (IReadOnlyDictionary<string, int>?, IReadOnlyDictionary<string, int>?) OpStringIntDict(
            Dictionary<string, int>? p1,
            Current current,
            CancellationToken cancel) => (p1, p1);

        public void OpClassAndUnknownTagged(A? p, Current current, CancellationToken cancel)
        {
        }

        public void OpVoid(Current current, CancellationToken cancel)
        {
        }

        public IInitial.OpMStruct1MarshaledReturnValue OpMStruct1(
            Current current,
            CancellationToken cancel) => new(new SmallStruct(), current);

        public IInitial.OpMStruct2MarshaledReturnValue OpMStruct2(
            SmallStruct? p1,
            Current current,
            CancellationToken cancel) =>
            new(p1, p1, current);

        public IInitial.OpMSeq1MarshaledReturnValue OpMSeq1(Current current, CancellationToken cancel) =>
            new(Array.Empty<string>(), current);

        public IInitial.OpMSeq2MarshaledReturnValue OpMSeq2(string[]? p1, Current current, CancellationToken cancel) =>
            new(p1, p1, current);

        public IInitial.OpMDict1MarshaledReturnValue OpMDict1(Current current, CancellationToken cancel) =>
            new(new Dictionary<string, int>(), current);

        public IInitial.OpMDict2MarshaledReturnValue OpMDict2(
            Dictionary<string, int>? p1,
            Current current,
            CancellationToken cancel) =>
            new(p1, p1, current);

        public bool SupportsRequiredParams(Current current, CancellationToken cancel) => false;

        public bool SupportsJavaSerializable(Current current, CancellationToken cancel) => false;

        public bool SupportsCppStringView(Current current, CancellationToken cancel) => false;

        public bool SupportsNullTagged(Current current, CancellationToken cancel) => true;
    }
}
