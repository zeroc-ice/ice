// Copyright (c) ZeroC, Inc.

namespace Ice.optional.AMD;

public class InitialI : Test.InitialDisp_
{
    public override Task shutdownAsync(Current current)
    {
        current.adapter.getCommunicator().shutdown();
        return Task.CompletedTask;
    }

    public override Task<Ice.Value>
    pingPongAsync(Ice.Value obj, Current current) => Task.FromResult<Ice.Value>(obj);

    public override Task
    opOptionalExceptionAsync(int? a,
                             string b,
                             Current c) => throw new Test.OptionalException(false, a, b);

    public override Task
    opDerivedExceptionAsync(int? a,
                            string b,
                            Current c) => throw new Test.DerivedException(false, a, b, "d1", b, "d2");

    public override Task
    opRequiredExceptionAsync(int? a,
                             string b,
                             Current c)
    {
        var e = new Test.RequiredException();
        e.a = a;
        e.b = b;
        if (b is not null)
        {
            e.ss = b;
        }
        throw e;
    }

    public override Task<Test.Initial_OpByteResult>
    opByteAsync(byte? p1, Current current) => Task.FromResult(new Test.Initial_OpByteResult(p1, p1));

    public override Task<Test.Initial_OpBoolResult>
    opBoolAsync(bool? p1, Current current) => Task.FromResult(new Test.Initial_OpBoolResult(p1, p1));

    public override Task<Test.Initial_OpShortResult>
    opShortAsync(short? p1, Current current) => Task.FromResult(new Test.Initial_OpShortResult(p1, p1));

    public override Task<Test.Initial_OpIntResult>
    opIntAsync(int? p1, Current current) => Task.FromResult(new Test.Initial_OpIntResult(p1, p1));

    public override Task<Test.Initial_OpLongResult>
    opLongAsync(long? p1, Current current) => Task.FromResult(new Test.Initial_OpLongResult(p1, p1));

    public override Task<Test.Initial_OpFloatResult>
    opFloatAsync(float? p1, Current current) => Task.FromResult(new Test.Initial_OpFloatResult(p1, p1));

    public override Task<Test.Initial_OpDoubleResult>
    opDoubleAsync(double? p1, Current current) => Task.FromResult(new Test.Initial_OpDoubleResult(p1, p1));

    public override Task<Test.Initial_OpStringResult>
    opStringAsync(string p1, Current current) => Task.FromResult(new Test.Initial_OpStringResult(p1, p1));

    public override Task<Test.Initial_OpMyEnumResult>
    opMyEnumAsync(Test.MyEnum? p1, Current current) => Task.FromResult(new Test.Initial_OpMyEnumResult(p1, p1));

    public override Task<Test.Initial_OpSmallStructResult>
    opSmallStructAsync(Test.SmallStruct? p1, Current current) => Task.FromResult(new Test.Initial_OpSmallStructResult(p1, p1));

    public override Task<Test.Initial_OpFixedStructResult>
    opFixedStructAsync(Test.FixedStruct? p1, Current current) => Task.FromResult(new Test.Initial_OpFixedStructResult(p1, p1));

    public override Task<Test.Initial_OpVarStructResult>
    opVarStructAsync(Test.VarStruct p1, Current current) => Task.FromResult(new Test.Initial_OpVarStructResult(p1, p1));

    public override Task<Test.Initial_OpMyInterfaceProxyResult>
    opMyInterfaceProxyAsync(Test.MyInterfacePrx p1, Current current) => Task.FromResult(new Test.Initial_OpMyInterfaceProxyResult(p1, p1));

    public override Task<Test.Initial_OpOneOptionalResult>
    opOneOptionalAsync(Test.OneOptional p1, Current current) => Task.FromResult(new Test.Initial_OpOneOptionalResult(p1, p1));

    public override Task<Test.Initial_OpByteSeqResult>
    opByteSeqAsync(byte[] p1, Current current) => Task.FromResult(new Test.Initial_OpByteSeqResult(p1, p1));

    public override Task<Test.Initial_OpBoolSeqResult>
    opBoolSeqAsync(bool[] p1, Current current) => Task.FromResult(new Test.Initial_OpBoolSeqResult(p1, p1));

    public override Task<Test.Initial_OpShortSeqResult>
    opShortSeqAsync(short[] p1, Current current) => Task.FromResult(new Test.Initial_OpShortSeqResult(p1, p1));

    public override Task<Test.Initial_OpIntSeqResult>
    opIntSeqAsync(int[] p1, Current current) => Task.FromResult(new Test.Initial_OpIntSeqResult(p1, p1));

    public override Task<Test.Initial_OpLongSeqResult>
    opLongSeqAsync(long[] p1, Current current) => Task.FromResult(new Test.Initial_OpLongSeqResult(p1, p1));

    public override Task<Test.Initial_OpFloatSeqResult>
    opFloatSeqAsync(float[] p1, Current current) => Task.FromResult(new Test.Initial_OpFloatSeqResult(p1, p1));

    public override Task<Test.Initial_OpDoubleSeqResult>
    opDoubleSeqAsync(double[] p1, Current current) => Task.FromResult(new Test.Initial_OpDoubleSeqResult(p1, p1));

    public override Task<Test.Initial_OpStringSeqResult>
    opStringSeqAsync(string[] p1, Current current) => Task.FromResult(new Test.Initial_OpStringSeqResult(p1, p1));

    public override Task<Test.Initial_OpSmallStructSeqResult>
    opSmallStructSeqAsync(Test.SmallStruct[] p1, Current current) => Task.FromResult(new Test.Initial_OpSmallStructSeqResult(p1, p1));

    public override Task<Test.Initial_OpSmallStructListResult>
    opSmallStructListAsync(List<Test.SmallStruct> p1, Current current) => Task.FromResult(new Test.Initial_OpSmallStructListResult(p1, p1));

    public override Task<Test.Initial_OpFixedStructSeqResult>
    opFixedStructSeqAsync(Test.FixedStruct[] p1, Current current) => Task.FromResult(new Test.Initial_OpFixedStructSeqResult(p1, p1));

    public override Task<Test.Initial_OpFixedStructListResult>
    opFixedStructListAsync(LinkedList<Test.FixedStruct> p1, Current current) => Task.FromResult(new Test.Initial_OpFixedStructListResult(p1, p1));

    public override Task<Test.Initial_OpVarStructSeqResult>
    opVarStructSeqAsync(Test.VarStruct[] p1, Current current) => Task.FromResult(new Test.Initial_OpVarStructSeqResult(p1, p1));

    public override Task<Test.Initial_OpIntIntDictResult>
    opIntIntDictAsync(Dictionary<int, int> p1, Current current) => Task.FromResult(new Test.Initial_OpIntIntDictResult(p1, p1));

    public override Task<Test.Initial_OpStringIntDictResult>
    opStringIntDictAsync(Dictionary<string, int> p1, Current current) => Task.FromResult(new Test.Initial_OpStringIntDictResult(p1, p1));

    public override Task
    opClassAndUnknownOptionalAsync(Test.A p, Current current) => Task.CompletedTask;

    public override Task<Test.G> opGAsync(Test.G g, Current current) => Task.FromResult(g);

    public override Task opVoidAsync(Current current) => Task.CompletedTask;

    public override Task<Test.Initial_OpMStruct1MarshaledResult> opMStruct1Async(Current current) =>
        Task.FromResult(new Test.Initial_OpMStruct1MarshaledResult(new Test.SmallStruct(), current));

    public override Task<Test.Initial_OpMStruct2MarshaledResult> opMStruct2Async(
        Test.SmallStruct? p1,
        Current current) =>
        Task.FromResult(new Test.Initial_OpMStruct2MarshaledResult(p1, p1, current));

    public override Task<Test.Initial_OpMSeq1MarshaledResult> opMSeq1Async(Current current) =>
        Task.FromResult(new Test.Initial_OpMSeq1MarshaledResult([], current));

    public override Task<Test.Initial_OpMSeq2MarshaledResult> opMSeq2Async(string[] p1, Current current) =>
        Task.FromResult(new Test.Initial_OpMSeq2MarshaledResult(p1, p1, current));

    public override Task<Test.Initial_OpMDict1MarshaledResult> opMDict1Async(Current current) =>
        Task.FromResult(new Test.Initial_OpMDict1MarshaledResult([], current));

    public override Task<Test.Initial_OpMDict2MarshaledResult> opMDict2Async(
        Dictionary<string, int> p1,
        Current current) =>
        Task.FromResult(new Test.Initial_OpMDict2MarshaledResult(p1, p1, current));

    public override Task<bool> supportsJavaSerializableAsync(Current current) =>
        Task.FromResult(false);
}
