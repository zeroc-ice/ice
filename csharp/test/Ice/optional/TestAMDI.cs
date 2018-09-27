// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System.Collections.Generic;
using System.Threading.Tasks;

namespace Ice
{
    namespace optional
    {
        namespace AMD
        {
            public class InitialI : Test.InitialDisp_
            {
                public override Task shutdownAsync(Ice.Current current)
                {
                    current.adapter.getCommunicator().shutdown();
                    return null;
                }

                public override Task<Ice.Value>
                pingPongAsync(Ice.Value obj, Ice.Current current)
                {
                    return Task.FromResult<Ice.Value>(obj);
                }

                public override Task
                opOptionalExceptionAsync(Ice.Optional<int> a,
                                         Ice.Optional<string> b,
                                         Ice.Optional<Test.OneOptional> o,
                                         Ice.Current c)
                {
                    throw new Test.OptionalException(false, a, b, o);
                }

                public override Task
                opDerivedExceptionAsync(Ice.Optional<int> a,
                                        Ice.Optional<string> b,
                                        Ice.Optional<Test.OneOptional> o,
                                        Ice.Current c)
                {
                    throw new Test.DerivedException(false, a, b, o, b, o);
                }

                public override Task
                opRequiredExceptionAsync(Ice.Optional<int> a,
                                         Ice.Optional<string> b,
                                         Ice.Optional<Test.OneOptional> o,
                                         Ice.Current c)
                {
                    var e = new Test.RequiredException();
                    e.a = a;
                    e.b = b;
                    e.o = o;
                    if(b.HasValue)
                    {
                        e.ss = b.Value;
                    }
                    if(o.HasValue)
                    {
                        e.o2 = o.Value;
                    }
                    throw e;
                }

                public override Task<Test.Initial_OpByteResult>
                opByteAsync(Ice.Optional<byte> p1, Ice.Current current)
                {
                    return Task.FromResult(new Test.Initial_OpByteResult(p1, p1));
                }

                public override Task<Test.Initial_OpBoolResult>
                opBoolAsync(Ice.Optional<bool> p1, Ice.Current current)
                {
                    return Task.FromResult(new Test.Initial_OpBoolResult(p1, p1));
                }

                public override Task<Test.Initial_OpShortResult>
                opShortAsync(Ice.Optional<short> p1, Ice.Current current)
                {
                    return Task.FromResult(new Test.Initial_OpShortResult(p1, p1));
                }

                public override Task<Test.Initial_OpIntResult>
                opIntAsync(Ice.Optional<int> p1, Ice.Current current)
                {
                    return Task.FromResult(new Test.Initial_OpIntResult(p1, p1));
                }

                public override Task<Test.Initial_OpLongResult>
                opLongAsync(Ice.Optional<long> p1, Ice.Current current)
                {
                    return Task.FromResult(new Test.Initial_OpLongResult(p1, p1));
                }

                public override Task<Test.Initial_OpFloatResult>
                opFloatAsync(Ice.Optional<float> p1, Ice.Current current)
                {
                    return Task.FromResult(new Test.Initial_OpFloatResult(p1, p1));
                }

                public override Task<Test.Initial_OpDoubleResult>
                opDoubleAsync(Ice.Optional<double> p1, Ice.Current current)
                {
                    return Task.FromResult(new Test.Initial_OpDoubleResult(p1, p1));
                }

                public override Task<Test.Initial_OpStringResult>
                opStringAsync(Ice.Optional<string> p1, Ice.Current current)
                {
                    return Task.FromResult(new Test.Initial_OpStringResult(p1, p1));
                }

                public override Task<Test.Initial_OpMyEnumResult>
                opMyEnumAsync(Ice.Optional<Test.MyEnum> p1, Ice.Current current)
                {
                    return Task.FromResult(new Test.Initial_OpMyEnumResult(p1, p1));
                }

                public override Task<Test.Initial_OpSmallStructResult>
                opSmallStructAsync(Ice.Optional<Test.SmallStruct> p1, Ice.Current current)
                {
                    return Task.FromResult(new Test.Initial_OpSmallStructResult(p1, p1));
                }

                public override Task<Test.Initial_OpFixedStructResult>
                opFixedStructAsync(Ice.Optional<Test.FixedStruct> p1, Ice.Current current)
                {
                    return Task.FromResult(new Test.Initial_OpFixedStructResult(p1, p1));
                }

                public override Task<Test.Initial_OpVarStructResult>
                opVarStructAsync(Ice.Optional<Test.VarStruct> p1, Ice.Current current)
                {
                    return Task.FromResult(new Test.Initial_OpVarStructResult(p1, p1));
                }

                public override Task<Test.Initial_OpOneOptionalResult>
                opOneOptionalAsync(Ice.Optional<Test.OneOptional> p1, Ice.Current current)
                {
                    return Task.FromResult(new Test.Initial_OpOneOptionalResult(p1, p1));
                }

                public override Task<Test.Initial_OpOneOptionalProxyResult>
                opOneOptionalProxyAsync(Ice.Optional<Ice.ObjectPrx> p1, Ice.Current current)
                {
                    return Task.FromResult(new Test.Initial_OpOneOptionalProxyResult(p1, p1));
                }

                public override Task<Test.Initial_OpByteSeqResult>
                opByteSeqAsync(Ice.Optional<byte[]> p1, Ice.Current current)
                {
                    return Task.FromResult(new Test.Initial_OpByteSeqResult(p1, p1));
                }

                public override Task<Test.Initial_OpBoolSeqResult>
                opBoolSeqAsync(Ice.Optional<bool[]> p1, Ice.Current current)
                {
                    return Task.FromResult(new Test.Initial_OpBoolSeqResult(p1, p1));
                }

                public override Task<Test.Initial_OpShortSeqResult>
                opShortSeqAsync(Ice.Optional<short[]> p1, Ice.Current current)
                {
                    return Task.FromResult(new Test.Initial_OpShortSeqResult(p1, p1));
                }

                public override Task<Test.Initial_OpIntSeqResult>
                opIntSeqAsync(Ice.Optional<int[]> p1, Ice.Current current)
                {
                    return Task.FromResult(new Test.Initial_OpIntSeqResult(p1, p1));
                }

                public override Task<Test.Initial_OpLongSeqResult>
                opLongSeqAsync(Ice.Optional<long[]> p1, Ice.Current current)
                {
                    return Task.FromResult(new Test.Initial_OpLongSeqResult(p1, p1));
                }

                public override Task<Test.Initial_OpFloatSeqResult>
                opFloatSeqAsync(Ice.Optional<float[]> p1, Ice.Current current)
                {
                    return Task.FromResult(new Test.Initial_OpFloatSeqResult(p1, p1));
                }

                public override Task<Test.Initial_OpDoubleSeqResult>
                opDoubleSeqAsync(Ice.Optional<double[]> p1, Ice.Current current)
                {
                    return Task.FromResult(new Test.Initial_OpDoubleSeqResult(p1, p1));
                }

                public override Task<Test.Initial_OpStringSeqResult>
                opStringSeqAsync(Ice.Optional<string[]> p1, Ice.Current current)
                {
                    return Task.FromResult(new Test.Initial_OpStringSeqResult(p1, p1));
                }

                public override Task<Test.Initial_OpSmallStructSeqResult>
                opSmallStructSeqAsync(Ice.Optional<Test.SmallStruct[]> p1, Ice.Current current)
                {
                    return Task.FromResult(new Test.Initial_OpSmallStructSeqResult(p1, p1));
                }

                public override Task<Test.Initial_OpSmallStructListResult>
                opSmallStructListAsync(Ice.Optional<List<Test.SmallStruct>> p1, Ice.Current current)
                {
                    return Task.FromResult(new Test.Initial_OpSmallStructListResult(p1, p1));
                }

                public override Task<Test.Initial_OpFixedStructSeqResult>
                opFixedStructSeqAsync(Ice.Optional<Test.FixedStruct[]> p1, Ice.Current current)
                {
                    return Task.FromResult(new Test.Initial_OpFixedStructSeqResult(p1, p1));
                }

                public override Task<Test.Initial_OpFixedStructListResult>
                opFixedStructListAsync(Ice.Optional<LinkedList<Test.FixedStruct>> p1, Ice.Current current)
                {
                    return Task.FromResult(new Test.Initial_OpFixedStructListResult(p1, p1));
                }

                public override Task<Test.Initial_OpVarStructSeqResult>
                opVarStructSeqAsync(Ice.Optional<Test.VarStruct[]> p1, Ice.Current current)
                {
                    return Task.FromResult(new Test.Initial_OpVarStructSeqResult(p1, p1));
                }

                public override Task<Test.Initial_OpSerializableResult>
                opSerializableAsync(Ice.Optional<Ice.optional.Test.SerializableClass> p1, Ice.Current current)
                {
                    return Task.FromResult(new Test.Initial_OpSerializableResult(p1, p1));
                }

                public override Task<Test.Initial_OpIntIntDictResult>
                opIntIntDictAsync(Ice.Optional<Dictionary<int, int>> p1, Ice.Current current)
                {
                    return Task.FromResult(new Test.Initial_OpIntIntDictResult(p1, p1));
                }

                public override Task<Test.Initial_OpStringIntDictResult>
                opStringIntDictAsync(Ice.Optional<Dictionary<string, int>> p1, Ice.Current current)
                {
                    return Task.FromResult(new Test.Initial_OpStringIntDictResult(p1, p1));
                }

                public override Task<Test.Initial_OpIntOneOptionalDictResult>
                opIntOneOptionalDictAsync(Ice.Optional<Dictionary<int, Test.OneOptional>> p1, Ice.Current current)
                {
                    return Task.FromResult(new Test.Initial_OpIntOneOptionalDictResult(p1, p1));
                }

                public override Task
                opClassAndUnknownOptionalAsync(Test.A p, Ice.Current current)
                {
                    return null;
                }

                public override Task
                sendOptionalClassAsync(bool req, Ice.Optional<Test.OneOptional> o, Ice.Current current)
                {
                    return null;
                }

                public override Task<Ice.Optional<Test.OneOptional>>
                returnOptionalClassAsync(bool req, Ice.Current current)
                {
                    return Task.FromResult(new Ice.Optional<Test.OneOptional>(new Test.OneOptional(53)));
                }

                public override Task<Test.G>
                opGAsync(Test.G g, Ice.Current current)
                {
                    return Task.FromResult(g);
                }

                public override Task
                opVoidAsync(Ice.Current current)
                {
                    return null;
                }

                public async override Task<Test.Initial_OpMStruct1MarshaledResult>
                opMStruct1Async(Ice.Current current)
                {
                    await Task.Delay(0);
                    return new Test.Initial_OpMStruct1MarshaledResult(new Test.SmallStruct(), current);
                }

                public async override Task<Test.Initial_OpMStruct2MarshaledResult>
                opMStruct2Async(Ice.Optional<Test.SmallStruct> p1, Ice.Current current)
                {
                    await Task.Delay(0);
                    return new Test.Initial_OpMStruct2MarshaledResult(p1, p1, current);
                }

                public async override Task<Test.Initial_OpMSeq1MarshaledResult>
                opMSeq1Async(Ice.Current current)
                {
                    await Task.Delay(0);
                    return new Test.Initial_OpMSeq1MarshaledResult(new string[0], current);
                }

                public async override Task<Test.Initial_OpMSeq2MarshaledResult>
                opMSeq2Async(Ice.Optional<string[]> p1, Ice.Current current)
                {
                    await Task.Delay(0);
                    return new Test.Initial_OpMSeq2MarshaledResult(p1, p1, current);
                }

                public async override Task<Test.Initial_OpMDict1MarshaledResult>
                opMDict1Async(Ice.Current current)
                {
                    await Task.Delay(0);
                    return new Test.Initial_OpMDict1MarshaledResult(new Dictionary<string, int>(), current);
                }

                public async override Task<Test.Initial_OpMDict2MarshaledResult>
                opMDict2Async(Ice.Optional<Dictionary<string, int>> p1, Ice.Current current)
                {
                    await Task.Delay(0);
                    return new Test.Initial_OpMDict2MarshaledResult(p1, p1, current);
                }

                public async override Task<Test.Initial_OpMG1MarshaledResult>
                opMG1Async(Ice.Current current)
                {
                    await Task.Delay(0);
                    return new Test.Initial_OpMG1MarshaledResult(new Test.G(), current);
                }

                public async override Task<Test.Initial_OpMG2MarshaledResult>
                opMG2Async(Ice.Optional<Test.G> p1, Ice.Current current)
                {
                    await Task.Delay(0);
                    return new Test.Initial_OpMG2MarshaledResult(p1, p1, current);
                }

                public override Task<bool>
                supportsRequiredParamsAsync(Ice.Current current)
                {
                    return Task.FromResult<bool>(false);
                }

                public override Task<bool>
                supportsJavaSerializableAsync(Ice.Current current)
                {
                    return Task.FromResult<bool>(false);
                }

                public override Task<bool>
                supportsCsharpSerializableAsync(Ice.Current current)
                {
                    return Task.FromResult<bool>(true);
                }

                public override Task<bool>
                supportsCppStringViewAsync(Ice.Current current)
                {
                    return Task.FromResult<bool>(false);
                }

                public override Task<bool>
                supportsNullOptionalAsync(Ice.Current current)
                {
                    return Task.FromResult<bool>(true);
                }
            }
        }
    }
}
