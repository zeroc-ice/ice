//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Collections.Generic;
using System.Threading.Tasks;

namespace Ice
{
    namespace optional
    {
        namespace AMD
        {
            public class InitialI : Test.Initial
            {
                public Task shutdownAsync(Current current)
                {
                    current.Adapter.Communicator.shutdown();
                    return null;
                }

                public Task<Ice.Value>
                pingPongAsync(Ice.Value obj, Current current)
                {
                    return Task.FromResult<Ice.Value>(obj);
                }

                public Task
                opOptionalExceptionAsync(int? a,
                                         string? b,
                                         Test.OneOptional? o,
                                         Current c)
                {
                    throw new Test.OptionalException(false, a, b, o);
                }

                public Task
                opDerivedExceptionAsync(int? a,
                                        string? b,
                                        Test.OneOptional? o,
                                        Current c)
                {
                    throw new Test.DerivedException(false, a, b, o, b, o);
                }

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

                public Task<Test.Initial_OpByteResult>
                opByteAsync(byte? p1, Current current)
                {
                    return Task.FromResult(new Test.Initial_OpByteResult(p1, p1));
                }

                public Task<Test.Initial_OpBoolResult>
                opBoolAsync(bool? p1, Current current)
                {
                    return Task.FromResult(new Test.Initial_OpBoolResult(p1, p1));
                }

                public Task<Test.Initial_OpShortResult>
                opShortAsync(short? p1, Current current)
                {
                    return Task.FromResult(new Test.Initial_OpShortResult(p1, p1));
                }

                public Task<Test.Initial_OpIntResult>
                opIntAsync(int? p1, Current current)
                {
                    return Task.FromResult(new Test.Initial_OpIntResult(p1, p1));
                }

                public Task<Test.Initial_OpLongResult>
                opLongAsync(long? p1, Current current)
                {
                    return Task.FromResult(new Test.Initial_OpLongResult(p1, p1));
                }

                public Task<Test.Initial_OpFloatResult>
                opFloatAsync(float? p1, Current current)
                {
                    return Task.FromResult(new Test.Initial_OpFloatResult(p1, p1));
                }

                public Task<Test.Initial_OpDoubleResult>
                opDoubleAsync(double? p1, Current current)
                {
                    return Task.FromResult(new Test.Initial_OpDoubleResult(p1, p1));
                }

                public Task<Test.Initial_OpStringResult>
                opStringAsync(string? p1, Current current)
                {
                    return Task.FromResult(new Test.Initial_OpStringResult(p1, p1));
                }

                public Task<Test.Initial_OpMyEnumResult>
                opMyEnumAsync(Test.MyEnum? p1, Current current)
                {
                    return Task.FromResult(new Test.Initial_OpMyEnumResult(p1, p1));
                }

                public Task<Test.Initial_OpSmallStructResult>
                opSmallStructAsync(Test.SmallStruct? p1, Current current)
                {
                    return Task.FromResult(new Test.Initial_OpSmallStructResult(p1, p1));
                }

                public Task<Test.Initial_OpFixedStructResult>
                opFixedStructAsync(Test.FixedStruct? p1, Current current)
                {
                    return Task.FromResult(new Test.Initial_OpFixedStructResult(p1, p1));
                }

                public Task<Test.Initial_OpVarStructResult>
                opVarStructAsync(Test.VarStruct? p1, Current current)
                {
                    return Task.FromResult(new Test.Initial_OpVarStructResult(p1, p1));
                }

                public Task<Test.Initial_OpOneOptionalResult>
                opOneOptionalAsync(Test.OneOptional? p1, Current current)
                {
                    return Task.FromResult(new Test.Initial_OpOneOptionalResult(p1, p1));
                }

                public Task<Test.Initial_OpOneOptionalProxyResult>
                opOneOptionalProxyAsync(Ice.IObjectPrx? p1, Current current)
                {
                    return Task.FromResult(new Test.Initial_OpOneOptionalProxyResult(p1, p1));
                }

                public Task<Test.Initial_OpByteSeqResult>
                opByteSeqAsync(byte[]? p1, Current current)
                {
                    return Task.FromResult(new Test.Initial_OpByteSeqResult(p1, p1));
                }

                public Task<Test.Initial_OpBoolSeqResult>
                opBoolSeqAsync(bool[]? p1, Current current)
                {
                    return Task.FromResult(new Test.Initial_OpBoolSeqResult(p1, p1));
                }

                public Task<Test.Initial_OpShortSeqResult>
                opShortSeqAsync(short[]? p1, Current current)
                {
                    return Task.FromResult(new Test.Initial_OpShortSeqResult(p1, p1));
                }

                public Task<Test.Initial_OpIntSeqResult>
                opIntSeqAsync(int[]? p1, Current current)
                {
                    return Task.FromResult(new Test.Initial_OpIntSeqResult(p1, p1));
                }

                public Task<Test.Initial_OpLongSeqResult>
                opLongSeqAsync(long[]? p1, Current current)
                {
                    return Task.FromResult(new Test.Initial_OpLongSeqResult(p1, p1));
                }

                public Task<Test.Initial_OpFloatSeqResult>
                opFloatSeqAsync(float[]? p1, Current current)
                {
                    return Task.FromResult(new Test.Initial_OpFloatSeqResult(p1, p1));
                }

                public Task<Test.Initial_OpDoubleSeqResult>
                opDoubleSeqAsync(double[]? p1, Current current)
                {
                    return Task.FromResult(new Test.Initial_OpDoubleSeqResult(p1, p1));
                }

                public Task<Test.Initial_OpStringSeqResult>
                opStringSeqAsync(string[]? p1, Current current)
                {
                    return Task.FromResult(new Test.Initial_OpStringSeqResult(p1, p1));
                }

                public Task<Test.Initial_OpSmallStructSeqResult>
                opSmallStructSeqAsync(Test.SmallStruct[]? p1, Current current)
                {
                    return Task.FromResult(new Test.Initial_OpSmallStructSeqResult(p1, p1));
                }

                public Task<Test.Initial_OpSmallStructListResult>
                opSmallStructListAsync(List<Test.SmallStruct>? p1, Current current)
                {
                    return Task.FromResult(new Test.Initial_OpSmallStructListResult(p1, p1));
                }

                public Task<Test.Initial_OpFixedStructSeqResult>
                opFixedStructSeqAsync(Test.FixedStruct[]? p1, Current current)
                {
                    return Task.FromResult(new Test.Initial_OpFixedStructSeqResult(p1, p1));
                }

                public Task<Test.Initial_OpFixedStructListResult>
                opFixedStructListAsync(LinkedList<Test.FixedStruct>? p1, Current current)
                {
                    return Task.FromResult(new Test.Initial_OpFixedStructListResult(p1, p1));
                }

                public Task<Test.Initial_OpVarStructSeqResult>
                opVarStructSeqAsync(Test.VarStruct[]? p1, Current current)
                {
                    return Task.FromResult(new Test.Initial_OpVarStructSeqResult(p1, p1));
                }

                public Task<Test.Initial_OpSerializableResult>
                opSerializableAsync(Ice.optional.Test.SerializableClass? p1, Current current)
                {
                    return Task.FromResult(new Test.Initial_OpSerializableResult(p1, p1));
                }

                public Task<Test.Initial_OpIntIntDictResult>
                opIntIntDictAsync(Dictionary<int, int>? p1, Current current)
                {
                    return Task.FromResult(new Test.Initial_OpIntIntDictResult(p1, p1));
                }

                public Task<Test.Initial_OpStringIntDictResult>
                opStringIntDictAsync(Dictionary<string, int>? p1, Current current)
                {
                    return Task.FromResult(new Test.Initial_OpStringIntDictResult(p1, p1));
                }

                public Task<Test.Initial_OpIntOneOptionalDictResult>
                opIntOneOptionalDictAsync(Dictionary<int, Test.OneOptional>? p1, Current current)
                {
                    return Task.FromResult(new Test.Initial_OpIntOneOptionalDictResult(p1, p1));
                }

                public Task
                opClassAndUnknownOptionalAsync(Test.A p, Current current)
                {
                    return null;
                }

                public Task
                sendOptionalClassAsync(bool req, Test.OneOptional? o, Current current)
                {
                    return null;
                }

                public Task<Test.OneOptional?>
                returnOptionalClassAsync(bool req, Current current)
                {
                    return Task.FromResult<Test.OneOptional?>(new Test.OneOptional(53));
                }

                public Task<Test.G>
                opGAsync(Test.G g, Current current)
                {
                    return Task.FromResult(g);
                }

                public Task
                opVoidAsync(Current current)
                {
                    return null;
                }

                public async Task<Test.Initial_OpMStruct1MarshaledResult>
                opMStruct1Async(Current current)
                {
                    await Task.Delay(0);
                    return new Test.Initial_OpMStruct1MarshaledResult(new Test.SmallStruct(), current);
                }

                public async Task<Test.Initial_OpMStruct2MarshaledResult>
                opMStruct2Async(Test.SmallStruct? p1, Current current)
                {
                    await Task.Delay(0);
                    return new Test.Initial_OpMStruct2MarshaledResult(p1, p1, current);
                }

                public async Task<Test.Initial_OpMSeq1MarshaledResult>
                opMSeq1Async(Current current)
                {
                    await Task.Delay(0);
                    return new Test.Initial_OpMSeq1MarshaledResult(new string[0], current);
                }

                public async Task<Test.Initial_OpMSeq2MarshaledResult>
                opMSeq2Async(string[]? p1, Current current)
                {
                    await Task.Delay(0);
                    return new Test.Initial_OpMSeq2MarshaledResult(p1, p1, current);
                }

                public async Task<Test.Initial_OpMDict1MarshaledResult>
                opMDict1Async(Current current)
                {
                    await Task.Delay(0);
                    return new Test.Initial_OpMDict1MarshaledResult(new Dictionary<string, int>(), current);
                }

                public async Task<Test.Initial_OpMDict2MarshaledResult>
                opMDict2Async(Dictionary<string, int>? p1, Current current)
                {
                    await Task.Delay(0);
                    return new Test.Initial_OpMDict2MarshaledResult(p1, p1, current);
                }

                public async Task<Test.Initial_OpMG1MarshaledResult>
                opMG1Async(Current current)
                {
                    await Task.Delay(0);
                    return new Test.Initial_OpMG1MarshaledResult(new Test.G(), current);
                }

                public async Task<Test.Initial_OpMG2MarshaledResult>
                opMG2Async(Test.G? p1, Current current)
                {
                    await Task.Delay(0);
                    return new Test.Initial_OpMG2MarshaledResult(p1, p1, current);
                }

                public Task<bool>
                supportsRequiredParamsAsync(Current current)
                {
                    return Task.FromResult(false);
                }

                public Task<bool>
                supportsJavaSerializableAsync(Current current)
                {
                    return Task.FromResult(false);
                }

                public Task<bool>
                supportsCsharpSerializableAsync(Current current)
                {
                    return Task.FromResult(true);
                }

                public Task<bool>
                supportsCppStringViewAsync(Current current)
                {
                    return Task.FromResult(false);
                }

                public Task<bool>
                supportsNullOptionalAsync(Current current)
                {
                    return Task.FromResult(true);
                }
            }
        }
    }
}
