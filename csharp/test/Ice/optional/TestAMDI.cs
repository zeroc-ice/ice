// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System.Collections.Generic;
using System.Threading.Tasks;
using Test;

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
    opOptionalExceptionAsync(Ice.Optional<int> a, Ice.Optional<string> b, Ice.Optional<OneOptional> o, Ice.Current c)
    {
        throw new OptionalException(false, a, b, o);
    }

    public override Task
    opDerivedExceptionAsync(Ice.Optional<int> a, Ice.Optional<string> b, Ice.Optional<OneOptional> o, Ice.Current c)
    {
        throw new DerivedException(false, a, b, o, b, o);
    }

    public override Task
    opRequiredExceptionAsync(Ice.Optional<int> a, Ice.Optional<string> b, Ice.Optional<OneOptional> o, Ice.Current c)
    {
        var e = new RequiredException();
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

    public override Task<Initial_OpByteResult>
    opByteAsync(Ice.Optional<byte> p1, Ice.Current current)
    {
        return Task.FromResult<Initial_OpByteResult>(new Initial_OpByteResult(p1, p1));
    }

    public override Task<Initial_OpBoolResult>
    opBoolAsync(Ice.Optional<bool> p1, Ice.Current current)
    {
        return Task.FromResult<Initial_OpBoolResult>(new Initial_OpBoolResult(p1, p1));
    }

    public override Task<Initial_OpShortResult>
    opShortAsync(Ice.Optional<short> p1, Ice.Current current)
    {
        return Task.FromResult<Initial_OpShortResult>(new Initial_OpShortResult(p1, p1));
    }

    public override Task<Initial_OpIntResult>
    opIntAsync(Ice.Optional<int> p1, Ice.Current current)
    {
        return Task.FromResult<Initial_OpIntResult>(new Initial_OpIntResult(p1, p1));
    }

    public override Task<Initial_OpLongResult>
    opLongAsync(Ice.Optional<long> p1, Ice.Current current)
    {
        return Task.FromResult<Initial_OpLongResult>(new Initial_OpLongResult(p1, p1));
    }

    public override Task<Initial_OpFloatResult>
    opFloatAsync(Ice.Optional<float> p1, Ice.Current current)
    {
        return Task.FromResult<Initial_OpFloatResult>(new Initial_OpFloatResult(p1, p1));
    }

    public override Task<Initial_OpDoubleResult>
    opDoubleAsync(Ice.Optional<double> p1, Ice.Current current)
    {
        return Task.FromResult<Initial_OpDoubleResult>(new Initial_OpDoubleResult(p1, p1));
    }

    public override Task<Initial_OpStringResult>
    opStringAsync(Ice.Optional<string> p1, Ice.Current current)
    {
        return Task.FromResult<Initial_OpStringResult>(new Initial_OpStringResult(p1, p1));
    }

    public override Task<Initial_OpMyEnumResult>
    opMyEnumAsync(Ice.Optional<MyEnum> p1, Ice.Current current)
    {
        return Task.FromResult<Initial_OpMyEnumResult>(new Initial_OpMyEnumResult(p1, p1));
    }

    public override Task<Initial_OpSmallStructResult>
    opSmallStructAsync(Ice.Optional<SmallStruct> p1, Ice.Current current)
    {
        return Task.FromResult<Initial_OpSmallStructResult>(new Initial_OpSmallStructResult(p1, p1));
    }

    public override Task<Initial_OpFixedStructResult>
    opFixedStructAsync(Ice.Optional<FixedStruct> p1, Ice.Current current)
    {
        return Task.FromResult<Initial_OpFixedStructResult>(new Initial_OpFixedStructResult(p1, p1));
    }

    public override Task<Initial_OpVarStructResult>
    opVarStructAsync(Ice.Optional<VarStruct> p1, Ice.Current current)
    {
        return Task.FromResult<Initial_OpVarStructResult>(new Initial_OpVarStructResult(p1, p1));
    }

    public override Task<Initial_OpOneOptionalResult>
    opOneOptionalAsync(Ice.Optional<OneOptional> p1, Ice.Current current)
    {
        return Task.FromResult<Initial_OpOneOptionalResult>(new Initial_OpOneOptionalResult(p1, p1));
    }

    public override Task<Initial_OpOneOptionalProxyResult>
    opOneOptionalProxyAsync(Ice.Optional<OneOptionalPrx> p1, Ice.Current current)
    {
        return Task.FromResult<Initial_OpOneOptionalProxyResult>(new Initial_OpOneOptionalProxyResult(p1, p1));
    }

    public override Task<Initial_OpByteSeqResult>
    opByteSeqAsync(Ice.Optional<byte[]> p1, Ice.Current current)
    {
        return Task.FromResult<Initial_OpByteSeqResult>(new Initial_OpByteSeqResult(p1, p1));
    }

    public override Task<Initial_OpBoolSeqResult>
    opBoolSeqAsync(Ice.Optional<bool[]> p1, Ice.Current current)
    {
        return Task.FromResult<Initial_OpBoolSeqResult>(new Initial_OpBoolSeqResult(p1, p1));
    }

    public override Task<Initial_OpShortSeqResult>
    opShortSeqAsync(Ice.Optional<short[]> p1, Ice.Current current)
    {
        return Task.FromResult<Initial_OpShortSeqResult>(new Initial_OpShortSeqResult(p1, p1));
    }

    public override Task<Initial_OpIntSeqResult>
    opIntSeqAsync(Ice.Optional<int[]> p1, Ice.Current current)
    {
        return Task.FromResult<Initial_OpIntSeqResult>(new Initial_OpIntSeqResult(p1, p1));
    }

    public override Task<Initial_OpLongSeqResult>
    opLongSeqAsync(Ice.Optional<long[]> p1, Ice.Current current)
    {
        return Task.FromResult<Initial_OpLongSeqResult>(new Initial_OpLongSeqResult(p1, p1));
    }

    public override Task<Initial_OpFloatSeqResult>
    opFloatSeqAsync(Ice.Optional<float[]> p1, Ice.Current current)
    {
        return Task.FromResult<Initial_OpFloatSeqResult>(new Initial_OpFloatSeqResult(p1, p1));
    }

    public override Task<Initial_OpDoubleSeqResult>
    opDoubleSeqAsync(Ice.Optional<double[]> p1, Ice.Current current)
    {
        return Task.FromResult<Initial_OpDoubleSeqResult>(new Initial_OpDoubleSeqResult(p1, p1));
    }

    public override Task<Initial_OpStringSeqResult>
    opStringSeqAsync(Ice.Optional<string[]> p1, Ice.Current current)
    {
        return Task.FromResult<Initial_OpStringSeqResult>(new Initial_OpStringSeqResult(p1, p1));
    }

    public override Task<Initial_OpSmallStructSeqResult>
    opSmallStructSeqAsync(Ice.Optional<SmallStruct[]> p1, Ice.Current current)
    {
        return Task.FromResult<Initial_OpSmallStructSeqResult>(new Initial_OpSmallStructSeqResult(p1, p1));
    }

    public override Task<Initial_OpSmallStructListResult>
    opSmallStructListAsync(Ice.Optional<List<SmallStruct>> p1, Ice.Current current)
    {
        return Task.FromResult<Initial_OpSmallStructListResult>(new Initial_OpSmallStructListResult(p1, p1));
    }

    public override Task<Initial_OpFixedStructSeqResult>
    opFixedStructSeqAsync(Ice.Optional<FixedStruct[]> p1, Ice.Current current)
    {
        return Task.FromResult<Initial_OpFixedStructSeqResult>(new Initial_OpFixedStructSeqResult(p1, p1));
    }

    public override Task<Initial_OpFixedStructListResult>
    opFixedStructListAsync(Ice.Optional<LinkedList<FixedStruct>> p1, Ice.Current current)
    {
        return Task.FromResult<Initial_OpFixedStructListResult>(new Initial_OpFixedStructListResult(p1, p1));
    }

    public override Task<Initial_OpVarStructSeqResult>
    opVarStructSeqAsync(Ice.Optional<VarStruct[]> p1, Ice.Current current)
    {
        return Task.FromResult<Initial_OpVarStructSeqResult>(new Initial_OpVarStructSeqResult(p1, p1));
    }

    public override Task<Initial_OpSerializableResult>
    opSerializableAsync(Ice.Optional<SerializableClass> p1, Ice.Current current)
    {
        return Task.FromResult<Initial_OpSerializableResult>(new Initial_OpSerializableResult(p1, p1));
    }

    public override Task<Initial_OpIntIntDictResult>
    opIntIntDictAsync(Ice.Optional<Dictionary<int, int>> p1, Ice.Current current)
    {
        return Task.FromResult<Initial_OpIntIntDictResult>(new Initial_OpIntIntDictResult(p1, p1));
    }

    public override Task<Initial_OpStringIntDictResult>
    opStringIntDictAsync(Ice.Optional<Dictionary<string, int>> p1, Ice.Current current)
    {
        return Task.FromResult<Initial_OpStringIntDictResult>(new Initial_OpStringIntDictResult(p1, p1));
    }

    public override Task<Initial_OpIntOneOptionalDictResult>
    opIntOneOptionalDictAsync(Ice.Optional<Dictionary<int, OneOptional>> p1, Ice.Current current)
    {
        return Task.FromResult<Initial_OpIntOneOptionalDictResult>(new Initial_OpIntOneOptionalDictResult(p1, p1));
    }

    public override Task
    opClassAndUnknownOptionalAsync(A p, Ice.Current current)
    {
        return null;
    }

    public override Task
    sendOptionalClassAsync(bool req, Ice.Optional<OneOptional> o, Ice.Current current)
    {
        return null;
    }

    public override Task<Ice.Optional<OneOptional>>
    returnOptionalClassAsync(bool req, Ice.Current current)
    {
        return Task.FromResult<Ice.Optional<OneOptional>>(new Ice.Optional<OneOptional>(new OneOptional(53)));
    }

    public override Task<G>
    opGAsync(G g, Ice.Current current)
    {
        return Task.FromResult<G>(g);
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
