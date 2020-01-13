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
            current.Adapter.Communicator.shutdown();
            return null;
        }

        public Task<Value>
        pingPongAsync(Value obj, Current current) => Task.FromResult(obj);

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

        public Task<Test.IInitial.OpByteReturnValue>
        opByteAsync(byte? p1, Current current) =>
            Task.FromResult(new Test.IInitial.OpByteReturnValue(p1, p1));

        public Task<Test.IInitial.OpBoolReturnValue>
        opBoolAsync(bool? p1, Current current) =>
            Task.FromResult(new Test.IInitial.OpBoolReturnValue(p1, p1));

        public Task<Test.IInitial.OpShortReturnValue>
        opShortAsync(short? p1, Current current) =>
            Task.FromResult(new Test.IInitial.OpShortReturnValue(p1, p1));

        public Task<Test.IInitial.OpIntReturnValue>
        opIntAsync(int? p1, Current current) =>
            Task.FromResult(new Test.IInitial.OpIntReturnValue(p1, p1));

        public Task<Test.IInitial.OpLongReturnValue>
        opLongAsync(long? p1, Current current) =>
            Task.FromResult(new Test.IInitial.OpLongReturnValue(p1, p1));

        public Task<Test.IInitial.OpFloatReturnValue>
        opFloatAsync(float? p1, Current current) =>
            Task.FromResult(new Test.IInitial.OpFloatReturnValue(p1, p1));

        public Task<Test.IInitial.OpDoubleReturnValue>
        opDoubleAsync(double? p1, Current current) =>
            Task.FromResult(new Test.IInitial.OpDoubleReturnValue(p1, p1));

        public Task<Test.IInitial.OpStringReturnValue>
        opStringAsync(string? p1, Current current) =>
            Task.FromResult(new Test.IInitial.OpStringReturnValue(p1, p1));

        public Task<Test.IInitial.OpMyEnumReturnValue>
        opMyEnumAsync(Test.MyEnum? p1, Current current) =>
            Task.FromResult(new Test.IInitial.OpMyEnumReturnValue(p1, p1));

        public Task<Test.IInitial.OpSmallStructReturnValue>
        opSmallStructAsync(Test.SmallStruct? p1, Current current) =>
            Task.FromResult(new Test.IInitial.OpSmallStructReturnValue(p1, p1));

        public Task<Test.IInitial.OpFixedStructReturnValue>
        opFixedStructAsync(Test.FixedStruct? p1, Current current) =>
            Task.FromResult(new Test.IInitial.OpFixedStructReturnValue(p1, p1));

        public Task<Test.IInitial.OpVarStructReturnValue>
        opVarStructAsync(Test.VarStruct? p1, Current current) =>
            Task.FromResult(new Test.IInitial.OpVarStructReturnValue(p1, p1));

        public Task<Test.IInitial.OpOneOptionalReturnValue>
        opOneOptionalAsync(Test.OneOptional? p1, Current current) =>
            Task.FromResult(new Test.IInitial.OpOneOptionalReturnValue(p1, p1));

        public Task<Test.IInitial.OpOneOptionalProxyReturnValue>
        opOneOptionalProxyAsync(IObjectPrx? p1, Current current) =>
            Task.FromResult(new Test.IInitial.OpOneOptionalProxyReturnValue(p1, p1));

        public Task<Test.IInitial.OpByteSeqReturnValue>
        opByteSeqAsync(byte[]? p1, Current current) =>
            Task.FromResult(new Test.IInitial.OpByteSeqReturnValue(p1, p1));

        public Task<Test.IInitial.OpBoolSeqReturnValue>
        opBoolSeqAsync(bool[]? p1, Current current) =>
            Task.FromResult(new Test.IInitial.OpBoolSeqReturnValue(p1, p1));

        public Task<Test.IInitial.OpShortSeqReturnValue>
        opShortSeqAsync(short[]? p1, Current current) =>
            Task.FromResult(new Test.IInitial.OpShortSeqReturnValue(p1, p1));

        public Task<Test.IInitial.OpIntSeqReturnValue>
        opIntSeqAsync(int[]? p1, Current current) =>
            Task.FromResult(new Test.IInitial.OpIntSeqReturnValue(p1, p1));

        public Task<Test.IInitial.OpLongSeqReturnValue>
        opLongSeqAsync(long[]? p1, Current current) =>
            Task.FromResult(new Test.IInitial.OpLongSeqReturnValue(p1, p1));

        public Task<Test.IInitial.OpFloatSeqReturnValue>
        opFloatSeqAsync(float[]? p1, Current current) =>
            Task.FromResult(new Test.IInitial.OpFloatSeqReturnValue(p1, p1));

        public Task<Test.IInitial.OpDoubleSeqReturnValue>
        opDoubleSeqAsync(double[]? p1, Current current) =>
            Task.FromResult(new Test.IInitial.OpDoubleSeqReturnValue(p1, p1));

        public Task<Test.IInitial.OpStringSeqReturnValue>
        opStringSeqAsync(string[]? p1, Current current) =>
            Task.FromResult(new Test.IInitial.OpStringSeqReturnValue(p1, p1));

        public Task<Test.IInitial.OpSmallStructSeqReturnValue>
        opSmallStructSeqAsync(Test.SmallStruct[]? p1, Current current) =>
            Task.FromResult(new Test.IInitial.OpSmallStructSeqReturnValue(p1, p1));

        public Task<Test.IInitial.OpSmallStructListReturnValue>
        opSmallStructListAsync(List<Test.SmallStruct>? p1, Current current) =>
            Task.FromResult(new Test.IInitial.OpSmallStructListReturnValue(p1, p1));

        public Task<Test.IInitial.OpFixedStructSeqReturnValue>
        opFixedStructSeqAsync(Test.FixedStruct[]? p1, Current current) =>
            Task.FromResult(new Test.IInitial.OpFixedStructSeqReturnValue(p1, p1));

        public Task<Test.IInitial.OpFixedStructListReturnValue>
        opFixedStructListAsync(LinkedList<Test.FixedStruct>? p1, Current current) =>
            Task.FromResult(new Test.IInitial.OpFixedStructListReturnValue(p1, p1));

        public Task<Test.IInitial.OpVarStructSeqReturnValue>
        opVarStructSeqAsync(Test.VarStruct[]? p1, Current current) =>
            Task.FromResult(new Test.IInitial.OpVarStructSeqReturnValue(p1, p1));

        public Task<Test.IInitial.OpSerializableReturnValue>
        opSerializableAsync(optional.Test.SerializableClass? p1, Current current) =>
            Task.FromResult(new Test.IInitial.OpSerializableReturnValue(p1, p1));

        public Task<Test.IInitial.OpIntIntDictReturnValue>
        opIntIntDictAsync(Dictionary<int, int>? p1, Current current) =>
            Task.FromResult(new Test.IInitial.OpIntIntDictReturnValue(p1, p1));

        public Task<Test.IInitial.OpStringIntDictReturnValue>
        opStringIntDictAsync(Dictionary<string, int>? p1, Current current) =>
            Task.FromResult(new Test.IInitial.OpStringIntDictReturnValue(p1, p1));

        public Task<Test.IInitial.OpIntOneOptionalDictReturnValue>
        opIntOneOptionalDictAsync(Dictionary<int, Test.OneOptional>? p1, Current current) =>
            Task.FromResult(new Test.IInitial.OpIntOneOptionalDictReturnValue(p1, p1));

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
