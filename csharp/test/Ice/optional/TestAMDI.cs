//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

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
        pingPongAsync(AnyClass? obj, Current current) => FromResult(obj);

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

        public ValueTask<(byte?, byte?)> opByteAsync(byte? p1, Current current) => FromResult((p1, p1));

        public ValueTask<(bool?, bool?)> opBoolAsync(bool? p1, Current current) => FromResult((p1, p1));

        public ValueTask<(short?, short?)> opShortAsync(short? p1, Current current) => FromResult((p1, p1));

        public ValueTask<(int?, int?)> opIntAsync(int? p1, Current current) => FromResult((p1, p1));

        public ValueTask<(long?, long?)> opLongAsync(long? p1, Current current) => FromResult((p1, p1));

        public ValueTask<(float?, float?)> opFloatAsync(float? p1, Current current) => FromResult((p1, p1));

        public ValueTask<(double?, double?)> opDoubleAsync(double? p1, Current current) => FromResult((p1, p1));

        public ValueTask<(string?, string?)> opStringAsync(string? p1, Current current) => FromResult((p1, p1));

        public ValueTask<(Test.MyEnum?, Test.MyEnum?)> opMyEnumAsync(Test.MyEnum? p1, Current current) =>
            FromResult((p1, p1));

        public ValueTask<(Test.SmallStruct?, Test.SmallStruct?)> opSmallStructAsync(Test.SmallStruct? p1, Current current) =>
            FromResult((p1, p1));

        public ValueTask<(Test.FixedStruct?, Test.FixedStruct?)> opFixedStructAsync(Test.FixedStruct? p1, Current current) =>
            FromResult((p1, p1));

        public ValueTask<(Test.VarStruct?, Test.VarStruct?)>
        opVarStructAsync(Test.VarStruct? p1, Current current) => FromResult((p1, p1));

        public ValueTask<(Test.OneOptional?, Test.OneOptional?)> opOneOptionalAsync(Test.OneOptional? p1, Current current) =>
            FromResult((p1, p1));

        public ValueTask<(IObjectPrx?, IObjectPrx?)> opOneOptionalProxyAsync(IObjectPrx? p1, Current current) =>
            FromResult((p1, p1));

        public ValueTask<(byte[]?, byte[]?)> opByteSeqAsync(byte[]? p1, Current current) => FromResult((p1, p1));
        public ValueTask<(List<byte>?, List<byte>?)> opByteListAsync(List<byte>? p1, Current current) =>
            FromResult((p1, p1));

        public ValueTask<(bool[]?, bool[]?)> opBoolSeqAsync(bool[]? p1, Current current) => FromResult((p1, p1));
        public ValueTask<(List<bool>?, List<bool>?)> opBoolListAsync(List<bool>? p1, Current current) =>
            FromResult((p1, p1));

        public ValueTask<(short[]?, short[]?)> opShortSeqAsync(short[]? p1, Current current) =>
            FromResult((p1, p1));
        public ValueTask<(List<short>?, List<short>?)> opShortListAsync(List<short>? p1, Current current) =>
            FromResult((p1, p1));

        public ValueTask<(int[]?, int[]?)> opIntSeqAsync(int[]? p1, Current current) =>
            FromResult((p1, p1));
        public ValueTask<(List<int>?, List<int>?)> opIntListAsync(List<int>? p1, Current current) =>
            FromResult((p1, p1));

        public ValueTask<(long[]?, long[]?)> opLongSeqAsync(long[]? p1, Current current) =>
            FromResult((p1, p1));
        public ValueTask<(List<long>?, List<long>?)> opLongListAsync(List<long>? p1, Current current) =>
            FromResult((p1, p1));

        public ValueTask<(float[]?, float[]?)> opFloatSeqAsync(float[]? p1, Current current) =>
            FromResult((p1, p1));
        public ValueTask<(List<float>?, List<float>?)> opFloatListAsync(List<float>? p1, Current current) =>
            FromResult((p1, p1));

        public ValueTask<(double[]?, double[]?)> opDoubleSeqAsync(double[]? p1, Current current) =>
            FromResult((p1, p1));
        public ValueTask<(List<double>?, List<double>?)> opDoubleListAsync(List<double>? p1, Current current) =>
            FromResult((p1, p1));

        public ValueTask<(string[]?, string[]?)> opStringSeqAsync(string[]? p1, Current current) =>
            FromResult((p1, p1));
        public ValueTask<(List<string>?, List<string>?)> opStringListAsync(List<string>? p1, Current current) =>
            FromResult((p1, p1));

        public ValueTask<(Test.SmallStruct[]?, Test.SmallStruct[]?)>
        opSmallStructSeqAsync(Test.SmallStruct[]? p1, Current current) => FromResult((p1, p1));

        public ValueTask<(List<Test.SmallStruct>?, List<Test.SmallStruct>?)>
        opSmallStructListAsync(List<Test.SmallStruct>? p1, Current current) => FromResult((p1, p1));

        public ValueTask<(Test.FixedStruct[]?, Test.FixedStruct[]?)>
        opFixedStructSeqAsync(Test.FixedStruct[]? p1, Current current) => FromResult((p1, p1));

        public ValueTask<(LinkedList<Test.FixedStruct>?, LinkedList<Test.FixedStruct>?)>
        opFixedStructListAsync(LinkedList<Test.FixedStruct>? p1, Current current) => FromResult((p1, p1));

        public ValueTask<(Test.VarStruct[]?, Test.VarStruct[]?)>
        opVarStructSeqAsync(Test.VarStruct[]? p1, Current current) => FromResult((p1, p1));

        public ValueTask<(optional.Test.SerializableClass?, optional.Test.SerializableClass?)>
        opSerializableAsync(optional.Test.SerializableClass? p1, Current current) => FromResult((p1, p1));

        public ValueTask<(Dictionary<int, int>?, Dictionary<int, int>?)>
        opIntIntDictAsync(Dictionary<int, int>? p1, Current current) => FromResult((p1, p1));

        public ValueTask<(Dictionary<string, int>?, Dictionary<string, int>?)>
        opStringIntDictAsync(Dictionary<string, int>? p1, Current current) => FromResult((p1, p1));

        public ValueTask<(Dictionary<int, Test.OneOptional?>?, Dictionary<int, Test.OneOptional?>?)>
        opIntOneOptionalDictAsync(Dictionary<int, Test.OneOptional?>? p1, Current current) => FromResult((p1, p1));

        public ValueTask
        opClassAndUnknownOptionalAsync(Test.A? p, Current current) => new ValueTask(Task.CompletedTask);

        public ValueTask
        sendOptionalClassAsync(bool req, Test.OneOptional? o, Current current) => new ValueTask(Task.CompletedTask);

        public ValueTask<Test.OneOptional?>
        returnOptionalClassAsync(bool req, Current current) =>
            FromResult<Test.OneOptional?>(new Test.OneOptional(53));

        public ValueTask<Test.G?> opGAsync(Test.G? g, Current current) => FromResult(g);

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
        supportsRequiredParamsAsync(Current current) => FromResult(false);

        public ValueTask<bool>
        supportsJavaSerializableAsync(Current current) => FromResult(false);

        public ValueTask<bool>
        supportsCsharpSerializableAsync(Current current) => FromResult(true);

        public ValueTask<bool>
        supportsCppStringViewAsync(Current current) => FromResult(false);

        public ValueTask<bool>
        supportsNullOptionalAsync(Current current) => FromResult(true);

        internal static ValueTask<T> FromResult<T>(T result) => new ValueTask<T>(result);
    }
}
