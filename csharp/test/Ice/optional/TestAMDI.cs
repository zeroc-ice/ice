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

                public Task<(byte? returnValue, byte? p3)>
                opByteAsync(byte? p1, Current current)
                {
                    return Task.FromResult((p1, p1));
                }

                public Task<(bool? returnValue, bool? p3)>
                opBoolAsync(bool? p1, Current current)
                {
                    return Task.FromResult((p1, p1));
                }

                public Task<(short? returnValue, short? p3)>
                opShortAsync(short? p1, Current current)
                {
                    return Task.FromResult((p1, p1));
                }

                public Task<(int? returnValue, int? p3)>
                opIntAsync(int? p1, Current current)
                {
                    return Task.FromResult((p1, p1));
                }

                public Task<(long? returnValue, long? p3)>
                opLongAsync(long? p1, Current current)
                {
                    return Task.FromResult((p1, p1));
                }

                public Task<(float? returnValue, float? p3)>
                opFloatAsync(float? p1, Current current)
                {
                    return Task.FromResult((p1, p1));
                }

                public Task<(double? returnValue, double? p3)>
                opDoubleAsync(double? p1, Current current)
                {
                    return Task.FromResult((p1, p1));
                }

                public Task<(string? returnValue, string? p3)>
                opStringAsync(string? p1, Current current)
                {
                    return Task.FromResult((p1, p1));
                }

                public Task<(Test.MyEnum? returnValue, Test.MyEnum? p3)>
                opMyEnumAsync(Test.MyEnum? p1, Current current)
                {
                    return Task.FromResult((p1, p1));
                }

                public Task<(Test.SmallStruct? returnValue, Test.SmallStruct? p3)>
                opSmallStructAsync(Test.SmallStruct? p1, Current current)
                {
                    return Task.FromResult((p1, p1));
                }

                public Task<(Test.FixedStruct? returnValue, Test.FixedStruct? p3)>
                opFixedStructAsync(Test.FixedStruct? p1, Current current)
                {
                    return Task.FromResult((p1, p1));
                }

                public Task<(Test.VarStruct? returnValue, Test.VarStruct? p3)>
                opVarStructAsync(Test.VarStruct? p1, Current current)
                {
                    return Task.FromResult((p1, p1));
                }

                public Task<(Test.OneOptional? returnValue, Test.OneOptional? p3)>
                opOneOptionalAsync(Test.OneOptional? p1, Current current)
                {
                    return Task.FromResult((p1, p1));
                }

                public Task<(IObjectPrx? returnValue, IObjectPrx? p3)>
                opOneOptionalProxyAsync(IObjectPrx? p1, Current current)
                {
                    return Task.FromResult((p1, p1));
                }

                public Task<(byte[]? returnValue, byte[]? p3)>
                opByteSeqAsync(byte[]? p1, Current current)
                {
                    return Task.FromResult((p1, p1));
                }

                public Task<(bool[]? returnValue, bool[]? p3)>
                opBoolSeqAsync(bool[]? p1, Current current)
                {
                    return Task.FromResult((p1, p1));
                }

                public Task<(short[]? returnValue, short[]? p3)>
                opShortSeqAsync(short[]? p1, Current current)
                {
                    return Task.FromResult((p1, p1));
                }

                public Task<(int[]? returnValue, int[]? p3)>
                opIntSeqAsync(int[]? p1, Current current)
                {
                    return Task.FromResult((p1, p1));
                }

                public Task<(long[]? returnValue, long[]? p3)>
                opLongSeqAsync(long[]? p1, Current current)
                {
                    return Task.FromResult((p1, p1));
                }

                public Task<(float[]? returnValue, float[]? p3)>
                opFloatSeqAsync(float[]? p1, Current current)
                {
                    return Task.FromResult((p1, p1));
                }

                public Task<(double[]? returnValue, double[]? p3)>
                opDoubleSeqAsync(double[]? p1, Current current)
                {
                    return Task.FromResult((p1, p1));
                }

                public Task<(string[]? returnValue, string[]? p3)>
                opStringSeqAsync(string[]? p1, Current current)
                {
                    return Task.FromResult((p1, p1));
                }

                public Task<(Test.SmallStruct[]? returnValue, Test.SmallStruct[]? p3)>
                opSmallStructSeqAsync(Test.SmallStruct[]? p1, Current current)
                {
                    return Task.FromResult((p1, p1));
                }

                public Task<(List<Test.SmallStruct>? returnValue, List<Test.SmallStruct>? p3)>
                opSmallStructListAsync(List<Test.SmallStruct>? p1, Current current)
                {
                    return Task.FromResult((p1, p1));
                }

                public Task<(Test.FixedStruct[]? returnValue, Test.FixedStruct[]? p3)>
                opFixedStructSeqAsync(Test.FixedStruct[]? p1, Current current)
                {
                    return Task.FromResult((p1, p1));
                }

                public Task<(LinkedList<Test.FixedStruct>? returnValue, LinkedList<Test.FixedStruct>? p3)>
                opFixedStructListAsync(LinkedList<Test.FixedStruct>? p1, Current current)
                {
                    return Task.FromResult((p1, p1));
                }

                public Task<(Test.VarStruct[]? returnValue, Test.VarStruct[]? p3)>
                opVarStructSeqAsync(Test.VarStruct[]? p1, Current current)
                {
                    return Task.FromResult((p1, p1));
                }

                public Task<(optional.Test.SerializableClass? returnValue, optional.Test.SerializableClass? p3)>
                opSerializableAsync(Ice.optional.Test.SerializableClass? p1, Current current)
                {
                    return Task.FromResult((p1, p1));
                }

                public Task<(Dictionary<int, int>? returnValue, Dictionary<int, int>? p3)>
                opIntIntDictAsync(Dictionary<int, int>? p1, Current current)
                {
                    return Task.FromResult((p1, p1));
                }

                public Task<(Dictionary<string, int>? returnValue, Dictionary<string, int>? p3)>
                opStringIntDictAsync(Dictionary<string, int>? p1, Current current)
                {
                    return Task.FromResult((p1, p1));
                }

                public Task<(Dictionary<int, Test.OneOptional>? returnValue, Dictionary<int, Test.OneOptional>? p3)>
                opIntOneOptionalDictAsync(Dictionary<int, Test.OneOptional>? p1, Current current)
                {
                    return Task.FromResult((p1, p1));
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
