// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Collections.Generic;
using System.Threading;
using System.Threading.Tasks;

namespace ZeroC.Ice.Test.Tagged
{
    public class AsyncInitial : IAsyncInitial
    {
        public ValueTask ShutdownAsync(Current current, CancellationToken cancel)
        {
            current.Communicator.ShutdownAsync();
            return default;
        }

        public ValueTask<AnyClass?> PingPongAsync(AnyClass? obj, Current current, CancellationToken cancel) =>
            new(obj);

        public ValueTask OpTaggedExceptionAsync(
            int? a,
            string? b,
            VarStruct? vs,
            Current current,
            CancellationToken cancel) =>
            throw new TaggedException(false, a, b, vs);

        public ValueTask OpDerivedExceptionAsync(
            int? a,
            string? b,
            VarStruct? vs,
            Current current,
            CancellationToken cancel) =>
            throw new DerivedException(false, a, b, vs, b, vs);

        public ValueTask OpRequiredExceptionAsync(
            int? a,
            string? b,
            VarStruct? vs,
            Current current,
            CancellationToken cancel) =>
            throw new RequiredException(false, a, b, vs, b ?? "test", vs ?? new VarStruct(""));

        public ValueTask<(byte?, byte?)> OpByteAsync(
            byte? p1,
            Current current,
            CancellationToken cancel) => new((p1, p1));

        public ValueTask<(bool?, bool?)> OpBoolAsync(
            bool? p1,
            Current current,
            CancellationToken cancel) => new((p1, p1));

        public ValueTask<(short?, short?)> OpShortAsync(
            short? p1,
            Current current,
            CancellationToken cancel) => new((p1, p1));

        public ValueTask<(int?, int?)> OpIntAsync(
            int? p1,
            Current current,
            CancellationToken cancel) => new((p1, p1));

        public ValueTask<(long?, long?)> OpLongAsync(
            long? p1,
            Current current,
            CancellationToken cancel) => new((p1, p1));

        public ValueTask<(float?, float?)> OpFloatAsync(
            float? p1,
            Current current,
            CancellationToken cancel) => new((p1, p1));

        public ValueTask<(double?, double?)> OpDoubleAsync(
            double? p1,
            Current current,
            CancellationToken cancel) => new((p1, p1));

        public ValueTask<(string?, string?)> OpStringAsync(
            string? p1,
            Current current,
            CancellationToken cancel) => new((p1, p1));

        public ValueTask<(MyEnum?, MyEnum?)> OpMyEnumAsync(
            MyEnum? p1,
            Current current,
            CancellationToken cancel) => new((p1, p1));

        public ValueTask<(SmallStruct?, SmallStruct?)> OpSmallStructAsync(
            SmallStruct? p1,
            Current current,
            CancellationToken cancel) =>
            new((p1, p1));

        public ValueTask<(FixedStruct?, FixedStruct?)> OpFixedStructAsync(
            FixedStruct? p1,
            Current current,
            CancellationToken cancel) =>
            new((p1, p1));

        public ValueTask<(VarStruct?, VarStruct?)> OpVarStructAsync(
            VarStruct? p1,
            Current current,
            CancellationToken cancel) =>
            new((p1, p1));

        public ValueTask<(IEnumerable<IEnumerable<int>>?, IEnumerable<IEnumerable<int>>?)> OpIntSeqSeqAsync(
            int[][]? p1,
            Current current,
            CancellationToken cancel) => new((p1, p1));

        public ValueTask<(ReadOnlyMemory<byte>, ReadOnlyMemory<byte>)> OpByteSeqAsync(
            byte[]? p1,
            Current current,
            CancellationToken cancel) =>
            new((p1, p1));
        public ValueTask<(IEnumerable<byte>?, IEnumerable<byte>?)> OpByteListAsync(
            List<byte>? p1,
            Current current,
            CancellationToken cancel) =>
            new((p1, p1));

        public ValueTask<(ReadOnlyMemory<bool>, ReadOnlyMemory<bool>)> OpBoolSeqAsync(
            bool[]? p1,
            Current current,
            CancellationToken cancel) =>
            new((p1, p1));

        public ValueTask<(IEnumerable<bool>?, IEnumerable<bool>?)> OpBoolListAsync(
            List<bool>? p1,
            Current current,
            CancellationToken cancel) =>
            new((p1, p1));

        public ValueTask<(ReadOnlyMemory<short>, ReadOnlyMemory<short>)> OpShortSeqAsync(
            short[]? p1,
            Current current,
            CancellationToken cancel) => new((p1, p1));

        public ValueTask<(IEnumerable<short>?, IEnumerable<short>?)> OpShortListAsync(
            List<short>? p1,
            Current current,
            CancellationToken cancel) => new((p1, p1));

        public ValueTask<(ReadOnlyMemory<int>, ReadOnlyMemory<int>)> OpIntSeqAsync(
            int[]? p1,
            Current current,
            CancellationToken cancel) =>
            new((p1, p1));
        public ValueTask<(IEnumerable<int>?, IEnumerable<int>?)> OpIntListAsync(
            List<int>? p1,
            Current current,
            CancellationToken cancel) =>
            new((p1, p1));

        public ValueTask<(ReadOnlyMemory<long>, ReadOnlyMemory<long>)> OpLongSeqAsync(
            long[]? p1,
            Current current,
            CancellationToken cancel) =>
            new((p1, p1));
        public ValueTask<(IEnumerable<long>?, IEnumerable<long>?)> OpLongListAsync(
            List<long>? p1,
            Current current,
            CancellationToken cancel) =>
            new((p1, p1));

        public ValueTask<(ReadOnlyMemory<float>, ReadOnlyMemory<float>)> OpFloatSeqAsync(
            float[]? p1,
            Current current,
            CancellationToken cancel) => new((p1, p1));
        public ValueTask<(IEnumerable<float>?, IEnumerable<float>?)> OpFloatListAsync(
            List<float>? p1,
            Current current,
            CancellationToken cancel) => new((p1, p1));

        public ValueTask<(ReadOnlyMemory<double>, ReadOnlyMemory<double>)> OpDoubleSeqAsync(
            double[]? p1,
            Current current,
            CancellationToken cancel) => new((p1, p1));

        public ValueTask<(IEnumerable<double>?, IEnumerable<double>?)> OpDoubleListAsync(
            List<double>? p1,
            Current current,
            CancellationToken cancel) => new((p1, p1));

        public ValueTask<(IEnumerable<string>?, IEnumerable<string>?)> OpStringSeqAsync(
            string[]? p1,
            Current current,
            CancellationToken cancel) => new((p1, p1));
        public ValueTask<(IEnumerable<string>?, IEnumerable<string>?)> OpStringListAsync(
            List<string>? p1,
            Current current,
            CancellationToken cancel) => new((p1, p1));

        public ValueTask<(IEnumerable<SmallStruct>?, IEnumerable<SmallStruct>?)> OpSmallStructSeqAsync(
            SmallStruct[]? p1,
            Current current,
            CancellationToken cancel) => new((p1, p1));

        public ValueTask<(IEnumerable<SmallStruct>?, IEnumerable<SmallStruct>?)> OpSmallStructListAsync(
            List<SmallStruct>? p1,
            Current current,
            CancellationToken cancel) => new((p1, p1));

        public ValueTask<(IEnumerable<FixedStruct>?, IEnumerable<FixedStruct>?)> OpFixedStructSeqAsync(
            FixedStruct[]? p1,
            Current current,
            CancellationToken cancel) => new((p1, p1));

        public ValueTask<(IEnumerable<FixedStruct>?, IEnumerable<FixedStruct>?)> OpFixedStructListAsync(
            LinkedList<FixedStruct>? p1,
            Current current, CancellationToken cancel) => new((p1, p1));

        public ValueTask<(IEnumerable<VarStruct>?, IEnumerable<VarStruct>?)> OpVarStructSeqAsync(
            VarStruct[]? p1,
            Current current,
            CancellationToken cancel) => new((p1, p1));

        public ValueTask<(IReadOnlyDictionary<int, int>?, IReadOnlyDictionary<int, int>?)> OpIntIntDictAsync(
            Dictionary<int, int>? p1,
            Current current,
            CancellationToken cancel) => new((p1, p1));

        public ValueTask<(IReadOnlyDictionary<string, int>?, IReadOnlyDictionary<string, int>?)> OpStringIntDictAsync(
            Dictionary<string, int>? p1,
            Current current,
            CancellationToken cancel) => new((p1, p1));

        public ValueTask OpClassAndUnknownTaggedAsync(A? p, Current current, CancellationToken cancel) => default;

        public ValueTask OpVoidAsync(Current current, CancellationToken cancel) => default;

        public async ValueTask<IInitial.OpMStruct1MarshaledReturnValue> OpMStruct1Async(
            Current current,
            CancellationToken cancel)
        {
            await Task.Delay(0, cancel);
            return new IInitial.OpMStruct1MarshaledReturnValue(new SmallStruct(), current);
        }

        public async ValueTask<IInitial.OpMStruct2MarshaledReturnValue> OpMStruct2Async(
            SmallStruct? p1,
            Current current,
            CancellationToken cancel)
        {
            await Task.Delay(0, cancel);
            return new IInitial.OpMStruct2MarshaledReturnValue(p1, p1, current);
        }

        public async ValueTask<IInitial.OpMSeq1MarshaledReturnValue> OpMSeq1Async(
            Current current,
            CancellationToken cancel)
        {
            await Task.Delay(0, cancel);
            return new IInitial.OpMSeq1MarshaledReturnValue(Array.Empty<string>(), current);
        }

        public async ValueTask<IInitial.OpMSeq2MarshaledReturnValue> OpMSeq2Async(
            string[]? p1,
            Current current,
            CancellationToken cancel)
        {
            await Task.Delay(0, cancel);
            return new IInitial.OpMSeq2MarshaledReturnValue(p1, p1, current);
        }

        public async ValueTask<IInitial.OpMDict1MarshaledReturnValue> OpMDict1Async(
            Current current,
            CancellationToken cancel)
        {
            await Task.Delay(0, cancel);
            return new IInitial.OpMDict1MarshaledReturnValue(new Dictionary<string, int>(), current);
        }

        public async ValueTask<IInitial.OpMDict2MarshaledReturnValue> OpMDict2Async(
            Dictionary<string, int>? p1,
            Current current,
            CancellationToken cancel)
        {
            await Task.Delay(0, cancel);
            return new IInitial.OpMDict2MarshaledReturnValue(p1, p1, current);
        }

        public ValueTask<bool> SupportsRequiredParamsAsync(Current current, CancellationToken cancel) => new(false);

        public ValueTask<bool> SupportsJavaSerializableAsync(Current current, CancellationToken cancel) => new(false);

        public ValueTask<bool> SupportsCppStringViewAsync(Current current, CancellationToken cancel) => new(false);

        public ValueTask<bool> SupportsNullTaggedAsync(Current current, CancellationToken cancel) => new(true);
    }
}
