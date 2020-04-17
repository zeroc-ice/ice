//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using System.Threading.Tasks;

namespace Ice.optional.AMD
{
    public class Initial : Test.IInitial
    {
        public ValueTask shutdownAsync(Current current)
        {
            current.Adapter.Communicator.Shutdown();
            return new ValueTask(Task.CompletedTask);
        }

        public ValueTask<AnyClass?>
        pingPongAsync(AnyClass? obj, Current current) => MakeValueTask(obj);

        public ValueTask
        opOptionalExceptionAsync(int? a, string? b, Test.OneOptional? o, Current c) =>
            throw new Test.OptionalException(false, a, b, o);

        public ValueTask
        opDerivedExceptionAsync(int? a, string? b, Test.OneOptional? o, Current c) =>
            throw new Test.DerivedException(false, a, b, o, b, o);

        public ValueTask
        opRequiredExceptionAsync(int? a,
                                    string? b,
                                    Test.OneOptional? o,
                                    Current c)
        {
            var e = new Test.RequiredException();
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

        public ValueTask<(byte?, byte?)> opByteAsync(byte? p1, Current current) => MakeValueTask((p1, p1));

        public ValueTask<(bool?, bool?)> opBoolAsync(bool? p1, Current current) => MakeValueTask((p1, p1));

        public ValueTask<(short?, short?)> opShortAsync(short? p1, Current current) => MakeValueTask((p1, p1));

        public ValueTask<(int?, int?)> opIntAsync(int? p1, Current current) => MakeValueTask((p1, p1));

        public ValueTask<(long?, long?)> opLongAsync(long? p1, Current current) => MakeValueTask((p1, p1));

        public ValueTask<(float?, float?)> opFloatAsync(float? p1, Current current) => MakeValueTask((p1, p1));

        public ValueTask<(double?, double?)> opDoubleAsync(double? p1, Current current) => MakeValueTask((p1, p1));

        public ValueTask<(string?, string?)> opStringAsync(string? p1, Current current) => MakeValueTask((p1, p1));

        public ValueTask<(Test.MyEnum?, Test.MyEnum?)> opMyEnumAsync(Test.MyEnum? p1, Current current) =>
            MakeValueTask((p1, p1));

        public ValueTask<(Test.SmallStruct?, Test.SmallStruct?)> opSmallStructAsync(Test.SmallStruct? p1, Current current) =>
            MakeValueTask((p1, p1));

        public ValueTask<(Test.FixedStruct?, Test.FixedStruct?)> opFixedStructAsync(Test.FixedStruct? p1, Current current) =>
            MakeValueTask((p1, p1));

        public ValueTask<(Test.VarStruct?, Test.VarStruct?)>
        opVarStructAsync(Test.VarStruct? p1, Current current) => MakeValueTask((p1, p1));

        public ValueTask<(Test.OneOptional?, Test.OneOptional?)> opOneOptionalAsync(Test.OneOptional? p1, Current current) =>
            MakeValueTask((p1, p1));

        public ValueTask<(IObjectPrx?, IObjectPrx?)> opOneOptionalProxyAsync(IObjectPrx? p1, Current current) =>
            MakeValueTask((p1, p1));

        public ValueTask<(ReadOnlyMemory<byte>, ReadOnlyMemory<byte>)> opByteSeqAsync(byte[]? p1, Current current) =>
            ToReturnValue(p1);
        public ValueTask<(IEnumerable<byte>?, IEnumerable<byte>?)> opByteListAsync(List<byte>? p1, Current current) =>
            ToReturnValue(p1);

        public ValueTask<(ReadOnlyMemory<bool>, ReadOnlyMemory<bool>)> opBoolSeqAsync(bool[]? p1, Current current) =>
            ToReturnValue(p1);

        public ValueTask<(IEnumerable<bool>?, IEnumerable<bool>?)> opBoolListAsync(List<bool>? p1, Current current) =>
            ToReturnValue(p1);

        public ValueTask<(ReadOnlyMemory<short>, ReadOnlyMemory<short>)> opShortSeqAsync(short[]? p1,
            Current current) => ToReturnValue(p1);

        public ValueTask<(IEnumerable<short>?, IEnumerable<short>?)> opShortListAsync(List<short>? p1,
            Current current) =>  ToReturnValue(p1);

        public ValueTask<(ReadOnlyMemory<int>, ReadOnlyMemory<int>)> opIntSeqAsync(int[]? p1, Current current) =>
            ToReturnValue(p1);
        public ValueTask<(IEnumerable<int>?, IEnumerable<int>?)> opIntListAsync(List<int>? p1, Current current) =>
            ToReturnValue(p1);

        public ValueTask<(ReadOnlyMemory<long>, ReadOnlyMemory<long>)> opLongSeqAsync(long[]? p1, Current current) =>
            ToReturnValue(p1);
        public ValueTask<(IEnumerable<long>?, IEnumerable<long>?)> opLongListAsync(List<long>? p1, Current current) =>
            ToReturnValue(p1);

        public ValueTask<(ReadOnlyMemory<float>, ReadOnlyMemory<float>)> opFloatSeqAsync(float[]? p1, Current current) =>
            ToReturnValue(p1);
        public ValueTask<(IEnumerable<float>?, IEnumerable<float>?)> opFloatListAsync(List<float>? p1, Current current) =>
            ToReturnValue(p1);

        public ValueTask<(ReadOnlyMemory<double>, ReadOnlyMemory<double>)> opDoubleSeqAsync(double[]? p1, Current current) =>
            ToReturnValue(p1);
        public ValueTask<(IEnumerable<double>?, IEnumerable<double>?)> opDoubleListAsync(List<double>? p1, Current current) =>
            ToReturnValue(p1);

        public ValueTask<(IEnumerable<string>?, IEnumerable<string>?)> opStringSeqAsync(string[]? p1, Current current) =>
            ToReturnValue(p1);
        public ValueTask<(IEnumerable<string>?, IEnumerable<string>?)> opStringListAsync(List<string>? p1, Current current) =>
            ToReturnValue(p1);

        public ValueTask<(IEnumerable<Test.SmallStruct>?, IEnumerable<Test.SmallStruct>?)> opSmallStructSeqAsync(
            Test.SmallStruct[]? p1, Current current) => ToReturnValue(p1 as IEnumerable<Test.SmallStruct>);

        public ValueTask<(IEnumerable<Test.SmallStruct>?, IEnumerable<Test.SmallStruct>?)>
        opSmallStructListAsync(List<Test.SmallStruct>? p1, Current current) => ToReturnValue(p1);

        public ValueTask<(IEnumerable<Test.FixedStruct>?, IEnumerable<Test.FixedStruct>?)> opFixedStructSeqAsync(
            Test.FixedStruct[]? p1, Current current) => ToReturnValue(p1 as IEnumerable<Test.FixedStruct>);

        public ValueTask<(IEnumerable<Test.FixedStruct>?, IEnumerable<Test.FixedStruct>?)>
        opFixedStructListAsync(LinkedList<Test.FixedStruct>? p1, Current current) => ToReturnValue(p1);

        public ValueTask<(IEnumerable<Test.VarStruct>?, IEnumerable<Test.VarStruct>?)> opVarStructSeqAsync(
            Test.VarStruct[]? p1, Current current) => ToReturnValue(p1 as IEnumerable<Test.VarStruct>);

        public ValueTask<(optional.Test.SerializableClass?, optional.Test.SerializableClass?)>
        opSerializableAsync(optional.Test.SerializableClass? p1, Current current) => MakeValueTask((p1, p1));

        public ValueTask<(IReadOnlyDictionary<int, int>?, IReadOnlyDictionary<int, int>?)>
        opIntIntDictAsync(Dictionary<int, int>? p1, Current current) => ToReturnValue(p1);

        public ValueTask<(IReadOnlyDictionary<string, int>?, IReadOnlyDictionary<string, int>?)>
        opStringIntDictAsync(Dictionary<string, int>? p1, Current current) => ToReturnValue(p1);

        public ValueTask<(IReadOnlyDictionary<int, Test.OneOptional?>?, IReadOnlyDictionary<int, Test.OneOptional?>?)>
        opIntOneOptionalDictAsync(Dictionary<int, Test.OneOptional?>? p1, Current current) => ToReturnValue(p1);

        public ValueTask
        opClassAndUnknownOptionalAsync(Test.A? p, Current current) => new ValueTask(Task.CompletedTask);

        public ValueTask
        sendOptionalClassAsync(bool req, Test.OneOptional? o, Current current) => new ValueTask(Task.CompletedTask);

        public ValueTask<Test.OneOptional?>
        returnOptionalClassAsync(bool req, Current current) =>
            new ValueTask<Test.OneOptional?>(new Test.OneOptional(53));

        public ValueTask<Test.G?> opGAsync(Test.G? g, Current current) => MakeValueTask(g);

        public ValueTask opVoidAsync(Current current) => new ValueTask(Task.CompletedTask);

        public async ValueTask<Test.IInitial.OpMStruct1MarshaledReturnValue>
        opMStruct1Async(Current current)
        {
            await Task.Delay(0);
            return new Test.IInitial.OpMStruct1MarshaledReturnValue(new Test.SmallStruct(), current);
        }

        public async ValueTask<Test.IInitial.OpMStruct2MarshaledReturnValue>
        opMStruct2Async(Test.SmallStruct? p1, Current current)
        {
            await Task.Delay(0);
            return new Test.IInitial.OpMStruct2MarshaledReturnValue(p1, p1, current);
        }

        public async ValueTask<Test.IInitial.OpMSeq1MarshaledReturnValue>
        opMSeq1Async(Current current)
        {
            await Task.Delay(0);
            return new Test.IInitial.OpMSeq1MarshaledReturnValue(new string[0], current);
        }

        public async ValueTask<Test.IInitial.OpMSeq2MarshaledReturnValue>
        opMSeq2Async(string[]? p1, Current current)
        {
            await Task.Delay(0);
            return new Test.IInitial.OpMSeq2MarshaledReturnValue(p1, p1, current);
        }

        public async ValueTask<Test.IInitial.OpMDict1MarshaledReturnValue>
        opMDict1Async(Current current)
        {
            await Task.Delay(0);
            return new Test.IInitial.OpMDict1MarshaledReturnValue(new Dictionary<string, int>(), current);
        }

        public async ValueTask<Test.IInitial.OpMDict2MarshaledReturnValue>
        opMDict2Async(Dictionary<string, int>? p1, Current current)
        {
            await Task.Delay(0);
            return new Test.IInitial.OpMDict2MarshaledReturnValue(p1, p1, current);
        }

        public async ValueTask<Test.IInitial.OpMG1MarshaledReturnValue>
        opMG1Async(Current current)
        {
            await Task.Delay(0);
            return new Test.IInitial.OpMG1MarshaledReturnValue(new Test.G(), current);
        }

        public async ValueTask<Test.IInitial.OpMG2MarshaledReturnValue>
        opMG2Async(Test.G? p1, Current current)
        {
            await Task.Delay(0);
            return new Test.IInitial.OpMG2MarshaledReturnValue(p1, p1, current);
        }

        public ValueTask<bool>
        supportsRequiredParamsAsync(Current current) => MakeValueTask(false);

        public ValueTask<bool>
        supportsJavaSerializableAsync(Current current) => MakeValueTask(false);

        public ValueTask<bool>
        supportsCsharpSerializableAsync(Current current) => MakeValueTask(true);

        public ValueTask<bool>
        supportsCppStringViewAsync(Current current) => MakeValueTask(false);

        public ValueTask<bool>
        supportsNullOptionalAsync(Current current) => MakeValueTask(true);

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
