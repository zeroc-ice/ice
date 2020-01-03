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

                public Task<Test.Initial.opByteResult>
                opByteAsync(byte? p1, Current current)
                {
                    return Task.FromResult(new Test.Initial.opByteResult(p1, p1));
                }

                public Task<Test.Initial.opBoolResult>
                opBoolAsync(bool? p1, Current current)
                {
                    return Task.FromResult(new Test.Initial.opBoolResult(p1, p1));
                }

                public Task<Test.Initial.opShortResult>
                opShortAsync(short? p1, Current current)
                {
                    return Task.FromResult(new Test.Initial.opShortResult(p1, p1));
                }

                public Task<Test.Initial.opIntResult>
                opIntAsync(int? p1, Current current)
                {
                    return Task.FromResult(new Test.Initial.opIntResult(p1, p1));
                }

                public Task<Test.Initial.opLongResult>
                opLongAsync(long? p1, Current current)
                {
                    return Task.FromResult(new Test.Initial.opLongResult(p1, p1));
                }

                public Task<Test.Initial.opFloatResult>
                opFloatAsync(float? p1, Current current)
                {
                    return Task.FromResult(new Test.Initial.opFloatResult(p1, p1));
                }

                public Task<Test.Initial.opDoubleResult>
                opDoubleAsync(double? p1, Current current)
                {
                    return Task.FromResult(new Test.Initial.opDoubleResult(p1, p1));
                }

                public Task<Test.Initial.opStringResult>
                opStringAsync(string? p1, Current current)
                {
                    return Task.FromResult(new Test.Initial.opStringResult(p1, p1));
                }

                public Task<Test.Initial.opMyEnumResult>
                opMyEnumAsync(Test.MyEnum? p1, Current current)
                {
                    return Task.FromResult(new Test.Initial.opMyEnumResult(p1, p1));
                }

                public Task<Test.Initial.opSmallStructResult>
                opSmallStructAsync(Test.SmallStruct? p1, Current current)
                {
                    return Task.FromResult(new Test.Initial.opSmallStructResult(p1, p1));
                }

                public Task<Test.Initial.opFixedStructResult>
                opFixedStructAsync(Test.FixedStruct? p1, Current current)
                {
                    return Task.FromResult(new Test.Initial.opFixedStructResult(p1, p1));
                }

                public Task<Test.Initial.opVarStructResult>
                opVarStructAsync(Test.VarStruct? p1, Current current)
                {
                    return Task.FromResult(new Test.Initial.opVarStructResult(p1, p1));
                }

                public Task<Test.Initial.opOneOptionalResult>
                opOneOptionalAsync(Test.OneOptional? p1, Current current)
                {
                    return Task.FromResult(new Test.Initial.opOneOptionalResult(p1, p1));
                }

                public Task<Test.Initial.opOneOptionalProxyResult>
                opOneOptionalProxyAsync(IObjectPrx? p1, Current current)
                {
                    return Task.FromResult(new Test.Initial.opOneOptionalProxyResult(p1, p1));
                }

                public Task<Test.Initial.opByteSeqResult>
                opByteSeqAsync(byte[]? p1, Current current)
                {
                    return Task.FromResult(new Test.Initial.opByteSeqResult(p1, p1));
                }

                public Task<Test.Initial.opBoolSeqResult>
                opBoolSeqAsync(bool[]? p1, Current current)
                {
                    return Task.FromResult(new Test.Initial.opBoolSeqResult(p1, p1));
                }

                public Task<Test.Initial.opShortSeqResult>
                opShortSeqAsync(short[]? p1, Current current)
                {
                    return Task.FromResult(new Test.Initial.opShortSeqResult(p1, p1));
                }

                public Task<Test.Initial.opIntSeqResult>
                opIntSeqAsync(int[]? p1, Current current)
                {
                    return Task.FromResult(new Test.Initial.opIntSeqResult(p1, p1));
                }

                public Task<Test.Initial.opLongSeqResult>
                opLongSeqAsync(long[]? p1, Current current)
                {
                    return Task.FromResult(new Test.Initial.opLongSeqResult(p1, p1));
                }

                public Task<Test.Initial.opFloatSeqResult>
                opFloatSeqAsync(float[]? p1, Current current)
                {
                    return Task.FromResult(new Test.Initial.opFloatSeqResult(p1, p1));
                }

                public Task<Test.Initial.opDoubleSeqResult>
                opDoubleSeqAsync(double[]? p1, Current current)
                {
                    return Task.FromResult(new Test.Initial.opDoubleSeqResult(p1, p1));
                }

                public Task<Test.Initial.opStringSeqResult>
                opStringSeqAsync(string[]? p1, Current current)
                {
                    return Task.FromResult(new Test.Initial.opStringSeqResult(p1, p1));
                }

                public Task<Test.Initial.opSmallStructSeqResult>
                opSmallStructSeqAsync(Test.SmallStruct[]? p1, Current current)
                {
                    return Task.FromResult(new Test.Initial.opSmallStructSeqResult(p1, p1));
                }

                public Task<Test.Initial.opSmallStructListResult>
                opSmallStructListAsync(List<Test.SmallStruct>? p1, Current current)
                {
                    return Task.FromResult(new Test.Initial.opSmallStructListResult(p1, p1));
                }

                public Task<Test.Initial.opFixedStructSeqResult>
                opFixedStructSeqAsync(Test.FixedStruct[]? p1, Current current)
                {
                    return Task.FromResult(new Test.Initial.opFixedStructSeqResult(p1, p1));
                }

                public Task<Test.Initial.opFixedStructListResult>
                opFixedStructListAsync(LinkedList<Test.FixedStruct>? p1, Current current)
                {
                    return Task.FromResult(new Test.Initial.opFixedStructListResult(p1, p1));
                }

                public Task<Test.Initial.opVarStructSeqResult>
                opVarStructSeqAsync(Test.VarStruct[]? p1, Current current)
                {
                    return Task.FromResult(new Test.Initial.opVarStructSeqResult(p1, p1));
                }

                public Task<Test.Initial.opSerializableResult>
                opSerializableAsync(Ice.optional.Test.SerializableClass? p1, Current current)
                {
                    return Task.FromResult(new Test.Initial.opSerializableResult(p1, p1));
                }

                public Task<Test.Initial.opIntIntDictResult>
                opIntIntDictAsync(Dictionary<int, int>? p1, Current current)
                {
                    return Task.FromResult(new Test.Initial.opIntIntDictResult(p1, p1));
                }

                public Task<Test.Initial.opStringIntDictResult>
                opStringIntDictAsync(Dictionary<string, int>? p1, Current current)
                {
                    return Task.FromResult(new Test.Initial.opStringIntDictResult(p1, p1));
                }

                public Task<Test.Initial.opIntOneOptionalDictResult>
                opIntOneOptionalDictAsync(Dictionary<int, Test.OneOptional>? p1, Current current)
                {
                    return Task.FromResult(new Test.Initial.opIntOneOptionalDictResult(p1, p1));
                }

                public Task
                opClassAndUnknownOptionalAsync(Test.A p, Current current)
                {
                    return Task.CompletedTask;
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

                public async Task<Test.Initial.opMStruct1MarshaledResult>
                opMStruct1Async(Current current)
                {
                    await Task.Delay(0);
                    return new Test.Initial.opMStruct1MarshaledResult(new Test.SmallStruct(), current);
                }

                public async Task<Test.Initial.opMStruct2MarshaledResult>
                opMStruct2Async(Test.SmallStruct? p1, Current current)
                {
                    await Task.Delay(0);
                    return new Test.Initial.opMStruct2MarshaledResult(p1, p1, current);
                }

                public async Task<Test.Initial.opMSeq1MarshaledResult>
                opMSeq1Async(Current current)
                {
                    await Task.Delay(0);
                    return new Test.Initial.opMSeq1MarshaledResult(new string[0], current);
                }

                public async Task<Test.Initial.opMSeq2MarshaledResult>
                opMSeq2Async(string[]? p1, Current current)
                {
                    await Task.Delay(0);
                    return new Test.Initial.opMSeq2MarshaledResult(p1, p1, current);
                }

                public async Task<Test.Initial.opMDict1MarshaledResult>
                opMDict1Async(Current current)
                {
                    await Task.Delay(0);
                    return new Test.Initial.opMDict1MarshaledResult(new Dictionary<string, int>(), current);
                }

                public async Task<Test.Initial.opMDict2MarshaledResult>
                opMDict2Async(Dictionary<string, int>? p1, Current current)
                {
                    await Task.Delay(0);
                    return new Test.Initial.opMDict2MarshaledResult(p1, p1, current);
                }

                public async Task<Test.Initial.opMG1MarshaledResult>
                opMG1Async(Current current)
                {
                    await Task.Delay(0);
                    return new Test.Initial.opMG1MarshaledResult(new Test.G(), current);
                }

                public async Task<Test.Initial.opMG2MarshaledResult>
                opMG2Async(Test.G? p1, Current current)
                {
                    await Task.Delay(0);
                    return new Test.Initial.opMG2MarshaledResult(p1, p1, current);
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
