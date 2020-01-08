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

                public Task<Value>
                pingPongAsync(Value obj, Current current)
                {
                    return Task.FromResult(obj);
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

                public Task<Test.Initial.OpByteReturnValue>
                opByteAsync(byte? p1, Current current)
                {
                    return Task.FromResult(new Test.Initial.OpByteReturnValue(p1, p1));
                }

                public Task<Test.Initial.OpBoolReturnValue>
                opBoolAsync(bool? p1, Current current)
                {
                    return Task.FromResult(new Test.Initial.OpBoolReturnValue(p1, p1));
                }

                public Task<Test.Initial.OpShortReturnValue>
                opShortAsync(short? p1, Current current)
                {
                    return Task.FromResult(new Test.Initial.OpShortReturnValue(p1, p1));
                }

                public Task<Test.Initial.OpIntReturnValue>
                opIntAsync(int? p1, Current current)
                {
                    return Task.FromResult(new Test.Initial.OpIntReturnValue(p1, p1));
                }

                public Task<Test.Initial.OpLongReturnValue>
                opLongAsync(long? p1, Current current)
                {
                    return Task.FromResult(new Test.Initial.OpLongReturnValue(p1, p1));
                }

                public Task<Test.Initial.OpFloatReturnValue>
                opFloatAsync(float? p1, Current current)
                {
                    return Task.FromResult(new Test.Initial.OpFloatReturnValue(p1, p1));
                }

                public Task<Test.Initial.OpDoubleReturnValue>
                opDoubleAsync(double? p1, Current current)
                {
                    return Task.FromResult(new Test.Initial.OpDoubleReturnValue(p1, p1));
                }

                public Task<Test.Initial.OpStringReturnValue>
                opStringAsync(string? p1, Current current)
                {
                    return Task.FromResult(new Test.Initial.OpStringReturnValue(p1, p1));
                }

                public Task<Test.Initial.OpMyEnumReturnValue>
                opMyEnumAsync(Test.MyEnum? p1, Current current)
                {
                    return Task.FromResult(new Test.Initial.OpMyEnumReturnValue(p1, p1));
                }

                public Task<Test.Initial.OpSmallStructReturnValue>
                opSmallStructAsync(Test.SmallStruct? p1, Current current)
                {
                    return Task.FromResult(new Test.Initial.OpSmallStructReturnValue(p1, p1));
                }

                public Task<Test.Initial.OpFixedStructReturnValue>
                opFixedStructAsync(Test.FixedStruct? p1, Current current)
                {
                    return Task.FromResult(new Test.Initial.OpFixedStructReturnValue(p1, p1));
                }

                public Task<Test.Initial.OpVarStructReturnValue>
                opVarStructAsync(Test.VarStruct? p1, Current current)
                {
                    return Task.FromResult(new Test.Initial.OpVarStructReturnValue(p1, p1));
                }

                public Task<Test.Initial.OpOneOptionalReturnValue>
                opOneOptionalAsync(Test.OneOptional? p1, Current current)
                {
                    return Task.FromResult(new Test.Initial.OpOneOptionalReturnValue(p1, p1));
                }

                public Task<Test.Initial.OpOneOptionalProxyReturnValue>
                opOneOptionalProxyAsync(IObjectPrx? p1, Current current)
                {
                    return Task.FromResult(new Test.Initial.OpOneOptionalProxyReturnValue(p1, p1));
                }

                public Task<Test.Initial.OpByteSeqReturnValue>
                opByteSeqAsync(byte[]? p1, Current current)
                {
                    return Task.FromResult(new Test.Initial.OpByteSeqReturnValue(p1, p1));
                }

                public Task<Test.Initial.OpBoolSeqReturnValue>
                opBoolSeqAsync(bool[]? p1, Current current)
                {
                    return Task.FromResult(new Test.Initial.OpBoolSeqReturnValue(p1, p1));
                }

                public Task<Test.Initial.OpShortSeqReturnValue>
                opShortSeqAsync(short[]? p1, Current current)
                {
                    return Task.FromResult(new Test.Initial.OpShortSeqReturnValue(p1, p1));
                }

                public Task<Test.Initial.OpIntSeqReturnValue>
                opIntSeqAsync(int[]? p1, Current current)
                {
                    return Task.FromResult(new Test.Initial.OpIntSeqReturnValue(p1, p1));
                }

                public Task<Test.Initial.OpLongSeqReturnValue>
                opLongSeqAsync(long[]? p1, Current current)
                {
                    return Task.FromResult(new Test.Initial.OpLongSeqReturnValue(p1, p1));
                }

                public Task<Test.Initial.OpFloatSeqReturnValue>
                opFloatSeqAsync(float[]? p1, Current current)
                {
                    return Task.FromResult(new Test.Initial.OpFloatSeqReturnValue(p1, p1));
                }

                public Task<Test.Initial.OpDoubleSeqReturnValue>
                opDoubleSeqAsync(double[]? p1, Current current)
                {
                    return Task.FromResult(new Test.Initial.OpDoubleSeqReturnValue(p1, p1));
                }

                public Task<Test.Initial.OpStringSeqReturnValue>
                opStringSeqAsync(string[]? p1, Current current)
                {
                    return Task.FromResult(new Test.Initial.OpStringSeqReturnValue(p1, p1));
                }

                public Task<Test.Initial.OpSmallStructSeqReturnValue>
                opSmallStructSeqAsync(Test.SmallStruct[]? p1, Current current)
                {
                    return Task.FromResult(new Test.Initial.OpSmallStructSeqReturnValue(p1, p1));
                }

                public Task<Test.Initial.OpSmallStructListReturnValue>
                opSmallStructListAsync(List<Test.SmallStruct>? p1, Current current)
                {
                    return Task.FromResult(new Test.Initial.OpSmallStructListReturnValue(p1, p1));
                }

                public Task<Test.Initial.OpFixedStructSeqReturnValue>
                opFixedStructSeqAsync(Test.FixedStruct[]? p1, Current current)
                {
                    return Task.FromResult(new Test.Initial.OpFixedStructSeqReturnValue(p1, p1));
                }

                public Task<Test.Initial.OpFixedStructListReturnValue>
                opFixedStructListAsync(LinkedList<Test.FixedStruct>? p1, Current current)
                {
                    return Task.FromResult(new Test.Initial.OpFixedStructListReturnValue(p1, p1));
                }

                public Task<Test.Initial.OpVarStructSeqReturnValue>
                opVarStructSeqAsync(Test.VarStruct[]? p1, Current current)
                {
                    return Task.FromResult(new Test.Initial.OpVarStructSeqReturnValue(p1, p1));
                }

                public Task<Test.Initial.OpSerializableReturnValue>
                opSerializableAsync(Ice.optional.Test.SerializableClass? p1, Current current)
                {
                    return Task.FromResult(new Test.Initial.OpSerializableReturnValue(p1, p1));
                }

                public Task<Test.Initial.OpIntIntDictReturnValue>
                opIntIntDictAsync(Dictionary<int, int>? p1, Current current)
                {
                    return Task.FromResult(new Test.Initial.OpIntIntDictReturnValue(p1, p1));
                }

                public Task<Test.Initial.OpStringIntDictReturnValue>
                opStringIntDictAsync(Dictionary<string, int>? p1, Current current)
                {
                    return Task.FromResult(new Test.Initial.OpStringIntDictReturnValue(p1, p1));
                }

                public Task<Test.Initial.OpIntOneOptionalDictReturnValue>
                opIntOneOptionalDictAsync(Dictionary<int, Test.OneOptional>? p1, Current current)
                {
                    return Task.FromResult(new Test.Initial.OpIntOneOptionalDictReturnValue(p1, p1));
                }

                public Task
                opClassAndUnknownOptionalAsync(Test.A p, Current current)
                {
                    return Task.CompletedTask;
                }

                public Task
                sendOptionalClassAsync(bool req, Test.OneOptional? o, Current current)
                {
                    return Task.CompletedTask;
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
                    return Task.CompletedTask;
                }

                public async Task<Test.Initial.OpMStruct1MarshaledReturnValue>
                opMStruct1Async(Current current)
                {
                    await Task.Delay(0);
                    return new Test.Initial.OpMStruct1MarshaledReturnValue(new Test.SmallStruct(), current);
                }

                public async Task<Test.Initial.OpMStruct2MarshaledReturnValue>
                opMStruct2Async(Test.SmallStruct? p1, Current current)
                {
                    await Task.Delay(0);
                    return new Test.Initial.OpMStruct2MarshaledReturnValue(p1, p1, current);
                }

                public async Task<Test.Initial.OpMSeq1MarshaledReturnValue>
                opMSeq1Async(Current current)
                {
                    await Task.Delay(0);
                    return new Test.Initial.OpMSeq1MarshaledReturnValue(new string[0], current);
                }

                public async Task<Test.Initial.OpMSeq2MarshaledReturnValue>
                opMSeq2Async(string[]? p1, Current current)
                {
                    await Task.Delay(0);
                    return new Test.Initial.OpMSeq2MarshaledReturnValue(p1, p1, current);
                }

                public async Task<Test.Initial.OpMDict1MarshaledReturnValue>
                opMDict1Async(Current current)
                {
                    await Task.Delay(0);
                    return new Test.Initial.OpMDict1MarshaledReturnValue(new Dictionary<string, int>(), current);
                }

                public async Task<Test.Initial.OpMDict2MarshaledReturnValue>
                opMDict2Async(Dictionary<string, int>? p1, Current current)
                {
                    await Task.Delay(0);
                    return new Test.Initial.OpMDict2MarshaledReturnValue(p1, p1, current);
                }

                public async Task<Test.Initial.OpMG1MarshaledReturnValue>
                opMG1Async(Current current)
                {
                    await Task.Delay(0);
                    return new Test.Initial.OpMG1MarshaledReturnValue(new Test.G(), current);
                }

                public async Task<Test.Initial.OpMG2MarshaledReturnValue>
                opMG2Async(Test.G? p1, Current current)
                {
                    await Task.Delay(0);
                    return new Test.Initial.OpMG2MarshaledReturnValue(p1, p1, current);
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
