//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Collections.Generic;
using System.Threading.Tasks;

namespace Ice.optional.AMD
{
    public class Initial : Test.IInitial
    {
        public Task shutdownAsync(Current current)
        {
            current.Adapter.Communicator.Shutdown();
            return null;
        }

        public Task<AnyClass>
        pingPongAsync(AnyClass obj, Current current) => Task.FromResult(obj);

        public Task
        opOptionalExceptionAsync(int? a, string? b, Test.OneOptional? o, Current c) =>
            throw new Test.OptionalException(false, a, b, o);

        public Task
        opDerivedExceptionAsync(int? a, string? b, Test.OneOptional? o, Current c) =>
            throw new Test.DerivedException(false, a, b, o, b, o);

        public Task
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

        public Task<(byte?, byte?)> opByteAsync(byte? p1, Current current) => Task.FromResult((p1, p1));

        public Task<(bool?, bool?)> opBoolAsync(bool? p1, Current current) => Task.FromResult((p1, p1));

        public Task<(short?, short?)> opShortAsync(short? p1, Current current) => Task.FromResult((p1, p1));

        public Task<(int?, int?)> opIntAsync(int? p1, Current current) => Task.FromResult((p1, p1));

        public Task<(long?, long?)> opLongAsync(long? p1, Current current) => Task.FromResult((p1, p1));

        public Task<(float?, float?)> opFloatAsync(float? p1, Current current) => Task.FromResult((p1, p1));

        public Task<(double?, double?)> opDoubleAsync(double? p1, Current current) => Task.FromResult((p1, p1));

        public Task<(string?, string?)> opStringAsync(string? p1, Current current) => Task.FromResult((p1, p1));

        public Task<(Test.MyEnum?, Test.MyEnum?)> opMyEnumAsync(Test.MyEnum? p1, Current current) =>
            Task.FromResult((p1, p1));

        public Task<(Test.SmallStruct?, Test.SmallStruct?)> opSmallStructAsync(Test.SmallStruct? p1, Current current) =>
            Task.FromResult((p1, p1));

        public Task<(Test.FixedStruct?, Test.FixedStruct?)> opFixedStructAsync(Test.FixedStruct? p1, Current current) =>
            Task.FromResult((p1, p1));

        public Task<(Test.VarStruct?, Test.VarStruct?)>
        opVarStructAsync(Test.VarStruct? p1, Current current) => Task.FromResult((p1, p1));

        public Task<(Test.OneOptional?, Test.OneOptional?)> opOneOptionalAsync(Test.OneOptional? p1, Current current) =>
            Task.FromResult((p1, p1));

        public Task<(IObjectPrx?, IObjectPrx?)> opOneOptionalProxyAsync(IObjectPrx? p1, Current current) =>
            Task.FromResult((p1, p1));

        public Task<(byte[]?, byte[]?)>
        opByteSeqAsync(byte[]? p1, Current current) => Task.FromResult((p1, p1));

        public Task<(bool[]?, bool[]?)> opBoolSeqAsync(bool[]? p1, Current current) => Task.FromResult((p1, p1));

        public Task<(short[]?, short[]?)> opShortSeqAsync(short[]? p1, Current current) =>
            Task.FromResult((p1, p1));

        public Task<(int[]?, int[]?)> opIntSeqAsync(int[]? p1, Current current) =>
            Task.FromResult((p1, p1));

        public Task<(long[]?, long[]?)> opLongSeqAsync(long[]? p1, Current current) =>
            Task.FromResult((p1, p1));

        public Task<(float[]?, float[]?)> opFloatSeqAsync(float[]? p1, Current current) =>
            Task.FromResult((p1, p1));

        public Task<(double[]?, double[]?)> opDoubleSeqAsync(double[]? p1, Current current) =>
            Task.FromResult((p1, p1));

        public Task<(string[]?, string[]?)> opStringSeqAsync(string[]? p1, Current current) =>
            Task.FromResult((p1, p1));

        public Task<(Test.SmallStruct[]?, Test.SmallStruct[]?)>
        opSmallStructSeqAsync(Test.SmallStruct[]? p1, Current current) => Task.FromResult((p1, p1));

        public Task<(List<Test.SmallStruct>?, List<Test.SmallStruct>?)>
        opSmallStructListAsync(List<Test.SmallStruct>? p1, Current current) => Task.FromResult((p1, p1));

        public Task<(Test.FixedStruct[]?, Test.FixedStruct[]?)>
        opFixedStructSeqAsync(Test.FixedStruct[]? p1, Current current) => Task.FromResult((p1, p1));

        public Task<(LinkedList<Test.FixedStruct>?, LinkedList<Test.FixedStruct>?)>
        opFixedStructListAsync(LinkedList<Test.FixedStruct>? p1, Current current) => Task.FromResult((p1, p1));

        public Task<(Test.VarStruct[]?, Test.VarStruct[]?)>
        opVarStructSeqAsync(Test.VarStruct[]? p1, Current current) => Task.FromResult((p1, p1));

        public Task<(optional.Test.SerializableClass?, optional.Test.SerializableClass?)>
        opSerializableAsync(optional.Test.SerializableClass? p1, Current current) => Task.FromResult((p1, p1));

        public Task<(Dictionary<int, int>?, Dictionary<int, int>?)>
        opIntIntDictAsync(Dictionary<int, int>? p1, Current current) => Task.FromResult((p1, p1));

        public Task<(Dictionary<string, int>?, Dictionary<string, int>?)>
        opStringIntDictAsync(Dictionary<string, int>? p1, Current current) => Task.FromResult((p1, p1));

        public Task<(Dictionary<int, Test.OneOptional>?, Dictionary<int, Test.OneOptional>?)>
        opIntOneOptionalDictAsync(Dictionary<int, Test.OneOptional>? p1, Current current) => Task.FromResult((p1, p1));

        public Task
        opClassAndUnknownOptionalAsync(Test.A p, Current current) => Task.CompletedTask;

        public Task
        sendOptionalClassAsync(bool req, Test.OneOptional? o, Current current) => Task.CompletedTask;

        public Task<Test.OneOptional?>
        returnOptionalClassAsync(bool req, Current current) =>
            Task.FromResult<Test.OneOptional?>(new Test.OneOptional(53));

        public Task<Test.G> opGAsync(Test.G g, Current current) => Task.FromResult(g);

        public Task opVoidAsync(Current current) => Task.CompletedTask;

        public async Task<Test.IInitial.OpMStruct1MarshaledReturnValue>
        opMStruct1Async(Current current)
        {
            await Task.Delay(0);
            return new Test.IInitial.OpMStruct1MarshaledReturnValue(new Test.SmallStruct(), current);
        }

        public async Task<Test.IInitial.OpMStruct2MarshaledReturnValue>
        opMStruct2Async(Test.SmallStruct? p1, Current current)
        {
            await Task.Delay(0);
            return new Test.IInitial.OpMStruct2MarshaledReturnValue(p1, p1, current);
        }

        public async Task<Test.IInitial.OpMSeq1MarshaledReturnValue>
        opMSeq1Async(Current current)
        {
            await Task.Delay(0);
            return new Test.IInitial.OpMSeq1MarshaledReturnValue(new string[0], current);
        }

        public async Task<Test.IInitial.OpMSeq2MarshaledReturnValue>
        opMSeq2Async(string[]? p1, Current current)
        {
            await Task.Delay(0);
            return new Test.IInitial.OpMSeq2MarshaledReturnValue(p1, p1, current);
        }

        public async Task<Test.IInitial.OpMDict1MarshaledReturnValue>
        opMDict1Async(Current current)
        {
            await Task.Delay(0);
            return new Test.IInitial.OpMDict1MarshaledReturnValue(new Dictionary<string, int>(), current);
        }

        public async Task<Test.IInitial.OpMDict2MarshaledReturnValue>
        opMDict2Async(Dictionary<string, int>? p1, Current current)
        {
            await Task.Delay(0);
            return new Test.IInitial.OpMDict2MarshaledReturnValue(p1, p1, current);
        }

        public async Task<Test.IInitial.OpMG1MarshaledReturnValue>
        opMG1Async(Current current)
        {
            await Task.Delay(0);
            return new Test.IInitial.OpMG1MarshaledReturnValue(new Test.G(), current);
        }

        public async Task<Test.IInitial.OpMG2MarshaledReturnValue>
        opMG2Async(Test.G? p1, Current current)
        {
            await Task.Delay(0);
            return new Test.IInitial.OpMG2MarshaledReturnValue(p1, p1, current);
        }

        public Task<bool>
        supportsRequiredParamsAsync(Current current) => Task.FromResult(false);

        public Task<bool>
        supportsJavaSerializableAsync(Current current) => Task.FromResult(false);

        public Task<bool>
        supportsCsharpSerializableAsync(Current current) => Task.FromResult(true);

        public Task<bool>
        supportsCppStringViewAsync(Current current) => Task.FromResult(false);

        public Task<bool>
        supportsNullOptionalAsync(Current current) => Task.FromResult(true);
    }
}
