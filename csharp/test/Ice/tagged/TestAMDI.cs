//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using System.Threading.Tasks;

namespace ZeroC.Ice.Test.Tagged
{
    public class InitialAsync : IInitialAsync
    {
        public ValueTask ShutdownAsync(Current current)
        {
            current.Adapter.Communicator.ShutdownAsync();
            return new ValueTask(Task.CompletedTask);
        }

        public ValueTask<AnyClass?> PingPongAsync(AnyClass? obj, Current current) => MakeValueTask(obj);

        public ValueTask OpTaggedExceptionAsync(int? a, string? b, VarStruct? vs, Current c) =>
            throw new TaggedException(false, a, b, vs);

        public ValueTask OpDerivedExceptionAsync(int? a, string? b, VarStruct? vs, Current c) =>
            throw new DerivedException(false, a, b, vs, b, vs);

        public ValueTask OpRequiredExceptionAsync(int? a, string? b, VarStruct? vs, Current c)
        {
            throw new RequiredException(false, a, b, vs, b ?? "test", vs ?? new VarStruct(""));
        }

        public ValueTask<(byte?, byte?)> OpByteAsync(byte? p1, Current current) => MakeValueTask((p1, p1));

        public ValueTask<(bool?, bool?)> OpBoolAsync(bool? p1, Current current) => MakeValueTask((p1, p1));

        public ValueTask<(short?, short?)> OpShortAsync(short? p1, Current current) => MakeValueTask((p1, p1));

        public ValueTask<(int?, int?)> OpIntAsync(int? p1, Current current) => MakeValueTask((p1, p1));

        public ValueTask<(long?, long?)> OpLongAsync(long? p1, Current current) => MakeValueTask((p1, p1));

        public ValueTask<(float?, float?)> OpFloatAsync(float? p1, Current current) => MakeValueTask((p1, p1));

        public ValueTask<(double?, double?)> OpDoubleAsync(double? p1, Current current) => MakeValueTask((p1, p1));

        public ValueTask<(string?, string?)> OpStringAsync(string? p1, Current current) => MakeValueTask((p1, p1));

        public ValueTask<(MyEnum?, MyEnum?)> OpMyEnumAsync(MyEnum? p1, Current current) =>
            MakeValueTask((p1, p1));

        public ValueTask<(SmallStruct?, SmallStruct?)> OpSmallStructAsync(SmallStruct? p1, Current current) =>
            MakeValueTask((p1, p1));

        public ValueTask<(FixedStruct?, FixedStruct?)> OpFixedStructAsync(FixedStruct? p1, Current current) =>
            MakeValueTask((p1, p1));

        public ValueTask<(VarStruct?, VarStruct?)>
        OpVarStructAsync(VarStruct? p1, Current current) => MakeValueTask((p1, p1));

        public ValueTask<(ReadOnlyMemory<byte>, ReadOnlyMemory<byte>)> OpByteSeqAsync(byte[]? p1, Current current) =>
            ToReturnValue(p1);
        public ValueTask<(IEnumerable<byte>?, IEnumerable<byte>?)> OpByteListAsync(List<byte>? p1, Current current) =>
            ToReturnValue(p1);

        public ValueTask<(ReadOnlyMemory<bool>, ReadOnlyMemory<bool>)> OpBoolSeqAsync(bool[]? p1, Current current) =>
            ToReturnValue(p1);

        public ValueTask<(IEnumerable<bool>?, IEnumerable<bool>?)> OpBoolListAsync(List<bool>? p1, Current current) =>
            ToReturnValue(p1);

        public ValueTask<(ReadOnlyMemory<short>, ReadOnlyMemory<short>)> OpShortSeqAsync(short[]? p1,
            Current current) => ToReturnValue(p1);

        public ValueTask<(IEnumerable<short>?, IEnumerable<short>?)> OpShortListAsync(List<short>? p1,
            Current current) => ToReturnValue(p1);

        public ValueTask<(ReadOnlyMemory<int>, ReadOnlyMemory<int>)> OpIntSeqAsync(int[]? p1, Current current) =>
            ToReturnValue(p1);
        public ValueTask<(IEnumerable<int>?, IEnumerable<int>?)> OpIntListAsync(List<int>? p1, Current current) =>
            ToReturnValue(p1);

        public ValueTask<(ReadOnlyMemory<long>, ReadOnlyMemory<long>)> OpLongSeqAsync(long[]? p1, Current current) =>
            ToReturnValue(p1);
        public ValueTask<(IEnumerable<long>?, IEnumerable<long>?)> OpLongListAsync(List<long>? p1, Current current) =>
            ToReturnValue(p1);

        public ValueTask<(ReadOnlyMemory<float>, ReadOnlyMemory<float>)> OpFloatSeqAsync(float[]? p1, Current current) =>
            ToReturnValue(p1);
        public ValueTask<(IEnumerable<float>?, IEnumerable<float>?)> OpFloatListAsync(List<float>? p1, Current current) =>
            ToReturnValue(p1);

        public ValueTask<(ReadOnlyMemory<double>, ReadOnlyMemory<double>)> OpDoubleSeqAsync(double[]? p1, Current current) =>
            ToReturnValue(p1);
        public ValueTask<(IEnumerable<double>?, IEnumerable<double>?)> OpDoubleListAsync(List<double>? p1, Current current) =>
            ToReturnValue(p1);

        public ValueTask<(IEnumerable<string>?, IEnumerable<string>?)> OpStringSeqAsync(string[]? p1, Current current) =>
            ToReturnValue(p1);
        public ValueTask<(IEnumerable<string>?, IEnumerable<string>?)> OpStringListAsync(List<string>? p1, Current current) =>
            ToReturnValue(p1);

        public ValueTask<(IEnumerable<SmallStruct>?, IEnumerable<SmallStruct>?)> OpSmallStructSeqAsync(
            SmallStruct[]? p1, Current current) => ToReturnValue(p1 as IEnumerable<SmallStruct>);

        public ValueTask<(IEnumerable<SmallStruct>?, IEnumerable<SmallStruct>?)>
        OpSmallStructListAsync(List<SmallStruct>? p1, Current current) => ToReturnValue(p1);

        public ValueTask<(IEnumerable<FixedStruct>?, IEnumerable<FixedStruct>?)> OpFixedStructSeqAsync(
            FixedStruct[]? p1, Current current) => ToReturnValue(p1 as IEnumerable<FixedStruct>);

        public ValueTask<(IEnumerable<FixedStruct>?, IEnumerable<FixedStruct>?)>
        OpFixedStructListAsync(LinkedList<FixedStruct>? p1, Current current) => ToReturnValue(p1);

        public ValueTask<(IEnumerable<VarStruct>?, IEnumerable<VarStruct>?)> OpVarStructSeqAsync(
            VarStruct[]? p1, Current current) => ToReturnValue(p1 as IEnumerable<VarStruct>);

        public ValueTask<(ZeroC.Ice.Test.Tagged.SerializableClass?, ZeroC.Ice.Test.Tagged.SerializableClass?)>
        OpSerializableAsync(ZeroC.Ice.Test.Tagged.SerializableClass? p1, Current current) => MakeValueTask((p1, p1));

        public ValueTask<(IReadOnlyDictionary<int, int>?, IReadOnlyDictionary<int, int>?)>
        OpIntIntDictAsync(Dictionary<int, int>? p1, Current current) => ToReturnValue(p1);

        public ValueTask<(IReadOnlyDictionary<string, int>?, IReadOnlyDictionary<string, int>?)>
        OpStringIntDictAsync(Dictionary<string, int>? p1, Current current) => ToReturnValue(p1);

        public ValueTask OpClassAndUnknownTaggedAsync(A? p, Current current) => new ValueTask(Task.CompletedTask);

        public ValueTask OpVoidAsync(Current current) => new ValueTask(Task.CompletedTask);

        public async ValueTask<IInitial.OpMStruct1MarshaledReturnValue>
        OpMStruct1Async(Current current)
        {
            await Task.Delay(0);
            return new IInitial.OpMStruct1MarshaledReturnValue(new SmallStruct(), current);
        }

        public async ValueTask<IInitial.OpMStruct2MarshaledReturnValue>
        OpMStruct2Async(SmallStruct? p1, Current current)
        {
            await Task.Delay(0);
            return new IInitial.OpMStruct2MarshaledReturnValue(p1, p1, current);
        }

        public async ValueTask<IInitial.OpMSeq1MarshaledReturnValue>
        OpMSeq1Async(Current current)
        {
            await Task.Delay(0);
            return new IInitial.OpMSeq1MarshaledReturnValue(Array.Empty<string>(), current);
        }

        public async ValueTask<IInitial.OpMSeq2MarshaledReturnValue>
        OpMSeq2Async(string[]? p1, Current current)
        {
            await Task.Delay(0);
            return new IInitial.OpMSeq2MarshaledReturnValue(p1, p1, current);
        }

        public async ValueTask<IInitial.OpMDict1MarshaledReturnValue>
        OpMDict1Async(Current current)
        {
            await Task.Delay(0);
            return new IInitial.OpMDict1MarshaledReturnValue(new Dictionary<string, int>(), current);
        }

        public async ValueTask<IInitial.OpMDict2MarshaledReturnValue>
        OpMDict2Async(Dictionary<string, int>? p1, Current current)
        {
            await Task.Delay(0);
            return new IInitial.OpMDict2MarshaledReturnValue(p1, p1, current);
        }

        public ValueTask<bool>
        SupportsRequiredParamsAsync(Current current) => MakeValueTask(false);

        public ValueTask<bool>
        SupportsJavaSerializableAsync(Current current) => MakeValueTask(false);

        public ValueTask<bool>
        SupportsCsharpSerializableAsync(Current current) => MakeValueTask(true);

        public ValueTask<bool>
        SupportsCppStringViewAsync(Current current) => MakeValueTask(false);

        public ValueTask<bool>
        SupportsNullTaggedAsync(Current current) => MakeValueTask(true);

        private static ValueTask<T> MakeValueTask<T>(T result) => new ValueTask<T>(result);

        private static ValueTask<(ReadOnlyMemory<T>, ReadOnlyMemory<T>)> ToReturnValue<T>(T[]? input)
            where T : struct => new ValueTask<(ReadOnlyMemory<T>, ReadOnlyMemory<T>)>((input, input));

        private static ValueTask<(IEnumerable<T>?, IEnumerable<T>?)> ToReturnValue<T>(IEnumerable<T>? input) =>
            new ValueTask<(IEnumerable<T>?, IEnumerable<T>?)>((input, input));

        private static ValueTask<(IReadOnlyDictionary<TKey, TValue>?, IReadOnlyDictionary<TKey, TValue>?)>
        ToReturnValue<TKey, TValue>(IReadOnlyDictionary<TKey, TValue>? input) where TKey : notnull =>
            new ValueTask<(IReadOnlyDictionary<TKey, TValue>?, IReadOnlyDictionary<TKey, TValue>?)>((input, input));
    }
}
