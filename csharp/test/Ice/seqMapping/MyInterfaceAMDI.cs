// Copyright (c) ZeroC, Inc.

namespace Ice.seqMapping.AMD;

public sealed class MyInterfaceI : Test.AsyncMyInterfaceDisp_
{
    public override Task shutdownAsync(Ice.Current current)
    {
        current.adapter.getCommunicator().shutdown();
        return Task.CompletedTask;
    }

    public override Task<Test.MyInterface_OpAByteSResult>
    opAByteSAsync(byte[] i, Ice.Current current) => Task.FromResult(new Test.MyInterface_OpAByteSResult(i, i));

    public override Task<Test.MyInterface_OpLByteSResult>
    opLByteSAsync(List<byte> i, Ice.Current current) => Task.FromResult(new Test.MyInterface_OpLByteSResult(i, i));

    public override Task<Test.MyInterface_OpKByteSResult>
    opKByteSAsync(LinkedList<byte> i, Ice.Current current) =>
        Task.FromResult(new Test.MyInterface_OpKByteSResult(i, i));

    public override Task<Test.MyInterface_OpQByteSResult>
    opQByteSAsync(Queue<byte> i, Ice.Current current) =>
        Task.FromResult(new Test.MyInterface_OpQByteSResult(i, i));

    public override Task<Test.MyInterface_OpSByteSResult>
    opSByteSAsync(Stack<byte> i, Ice.Current current) => Task.FromResult(new Test.MyInterface_OpSByteSResult(i, i));

    public override Task<Test.MyInterface_OpABoolSResult>
    opABoolSAsync(bool[] i, Ice.Current current) => Task.FromResult(new Test.MyInterface_OpABoolSResult(i, i));

    public override Task<Test.MyInterface_OpLBoolSResult>
    opLBoolSAsync(List<bool> i, Ice.Current current) => Task.FromResult(new Test.MyInterface_OpLBoolSResult(i, i));

    public override Task<Test.MyInterface_OpKBoolSResult>
    opKBoolSAsync(LinkedList<bool> i, Ice.Current current) =>
        Task.FromResult(new Test.MyInterface_OpKBoolSResult(i, i));

    public override Task<Test.MyInterface_OpQBoolSResult>
    opQBoolSAsync(Queue<bool> i, Ice.Current current) =>
        Task.FromResult(new Test.MyInterface_OpQBoolSResult(i, i));

    public override Task<Test.MyInterface_OpSBoolSResult>
    opSBoolSAsync(Stack<bool> i, Ice.Current current) =>
        Task.FromResult(new Test.MyInterface_OpSBoolSResult(i, i));

    public override Task<Test.MyInterface_OpAShortSResult>
    opAShortSAsync(short[] i, Ice.Current current) => Task.FromResult(new Test.MyInterface_OpAShortSResult(i, i));

    public override Task<Test.MyInterface_OpLShortSResult>
    opLShortSAsync(List<short> i, Ice.Current current) =>
        Task.FromResult(new Test.MyInterface_OpLShortSResult(i, i));

    public override Task<Test.MyInterface_OpKShortSResult>
    opKShortSAsync(LinkedList<short> i, Ice.Current current) =>
        Task.FromResult(new Test.MyInterface_OpKShortSResult(i, i));

    public override Task<Test.MyInterface_OpQShortSResult>
    opQShortSAsync(Queue<short> i, Ice.Current current) =>
        Task.FromResult(new Test.MyInterface_OpQShortSResult(i, i));

    public override Task<Test.MyInterface_OpSShortSResult>
    opSShortSAsync(Stack<short> i, Ice.Current current) =>
        Task.FromResult(new Test.MyInterface_OpSShortSResult(i, i));

    public override Task<Test.MyInterface_OpAIntSResult>
    opAIntSAsync(int[] i, Ice.Current current) => Task.FromResult(new Test.MyInterface_OpAIntSResult(i, i));

    public override Task<Test.MyInterface_OpLIntSResult>
    opLIntSAsync(List<int> i, Ice.Current current) => Task.FromResult(new Test.MyInterface_OpLIntSResult(i, i));

    public override Task<Test.MyInterface_OpKIntSResult>
    opKIntSAsync(LinkedList<int> i, Ice.Current current) =>
        Task.FromResult(new Test.MyInterface_OpKIntSResult(i, i));

    public override Task<Test.MyInterface_OpQIntSResult>
    opQIntSAsync(Queue<int> i, Ice.Current current) => Task.FromResult(new Test.MyInterface_OpQIntSResult(i, i));

    public override Task<Test.MyInterface_OpSIntSResult>
    opSIntSAsync(Stack<int> i, Ice.Current current) => Task.FromResult(new Test.MyInterface_OpSIntSResult(i, i));

    public override Task<Test.MyInterface_OpALongSResult>
    opALongSAsync(long[] i, Ice.Current current) => Task.FromResult(new Test.MyInterface_OpALongSResult(i, i));

    public override Task<Test.MyInterface_OpLLongSResult>
    opLLongSAsync(List<long> i, Ice.Current current) => Task.FromResult(new Test.MyInterface_OpLLongSResult(i, i));

    public override Task<Test.MyInterface_OpKLongSResult>
    opKLongSAsync(LinkedList<long> i, Ice.Current current) =>
        Task.FromResult(new Test.MyInterface_OpKLongSResult(i, i));

    public override Task<Test.MyInterface_OpQLongSResult>
    opQLongSAsync(Queue<long> i, Ice.Current current) => Task.FromResult(new Test.MyInterface_OpQLongSResult(i, i));

    public override Task<Test.MyInterface_OpSLongSResult>
    opSLongSAsync(Stack<long> i, Ice.Current current) => Task.FromResult(new Test.MyInterface_OpSLongSResult(i, i));

    public override Task<Test.MyInterface_OpAFloatSResult>
    opAFloatSAsync(float[] i, Ice.Current current) => Task.FromResult(new Test.MyInterface_OpAFloatSResult(i, i));

    public override Task<Test.MyInterface_OpLFloatSResult>
    opLFloatSAsync(List<float> i, Ice.Current current) =>
        Task.FromResult(new Test.MyInterface_OpLFloatSResult(i, i));

    public override Task<Test.MyInterface_OpKFloatSResult>
    opKFloatSAsync(LinkedList<float> i, Ice.Current current) =>
        Task.FromResult(new Test.MyInterface_OpKFloatSResult(i, i));

    public override Task<Test.MyInterface_OpQFloatSResult>
    opQFloatSAsync(Queue<float> i, Ice.Current current) =>
        Task.FromResult(new Test.MyInterface_OpQFloatSResult(i, i));

    public override Task<Test.MyInterface_OpSFloatSResult>
    opSFloatSAsync(Stack<float> i, Ice.Current current) =>
        Task.FromResult(new Test.MyInterface_OpSFloatSResult(i, i));

    public override Task<Test.MyInterface_OpADoubleSResult>
    opADoubleSAsync(double[] i, Ice.Current current) =>
        Task.FromResult(new Test.MyInterface_OpADoubleSResult(i, i));

    public override Task<Test.MyInterface_OpLDoubleSResult>
    opLDoubleSAsync(List<double> i, Ice.Current current) =>
        Task.FromResult(new Test.MyInterface_OpLDoubleSResult(i, i));

    public override Task<Test.MyInterface_OpKDoubleSResult>
    opKDoubleSAsync(LinkedList<double> i, Ice.Current current) =>
        Task.FromResult(new Test.MyInterface_OpKDoubleSResult(i, i));

    public override Task<Test.MyInterface_OpQDoubleSResult>
    opQDoubleSAsync(Queue<double> i, Ice.Current current) =>
        Task.FromResult(new Test.MyInterface_OpQDoubleSResult(i, i));

    public override Task<Test.MyInterface_OpSDoubleSResult>
    opSDoubleSAsync(Stack<double> i, Ice.Current current) =>
        Task.FromResult(new Test.MyInterface_OpSDoubleSResult(i, i));

    public override Task<Test.MyInterface_OpAStringSResult>
    opAStringSAsync(string[] i, Ice.Current current) =>
        Task.FromResult(new Test.MyInterface_OpAStringSResult(i, i));

    public override Task<Test.MyInterface_OpLStringSResult>
    opLStringSAsync(List<string> i, Ice.Current current) =>
        Task.FromResult(new Test.MyInterface_OpLStringSResult(i, i));

    public override Task<Test.MyInterface_OpKStringSResult>
    opKStringSAsync(LinkedList<string> i, Ice.Current current) =>
        Task.FromResult(new Test.MyInterface_OpKStringSResult(i, i));

    public override Task<Test.MyInterface_OpQStringSResult>
    opQStringSAsync(Queue<string> i, Ice.Current current) =>
        Task.FromResult(new Test.MyInterface_OpQStringSResult(i, i));

    public override Task<Test.MyInterface_OpSStringSResult>
    opSStringSAsync(Stack<string> i, Ice.Current current) =>
        Task.FromResult(new Test.MyInterface_OpSStringSResult(i, i));

    public override Task<Test.MyInterface_OpAObjectSResult> opAObjectSAsync(
        Ice.Value[] i,
        Ice.Current current) =>
        Task.FromResult(new Test.MyInterface_OpAObjectSResult(i, i));

    public override Task<Test.MyInterface_OpLObjectSResult>
    opLObjectSAsync(List<Ice.Value> i, Ice.Current current) =>
        Task.FromResult(new Test.MyInterface_OpLObjectSResult(i, i));

    public override Task<Test.MyInterface_OpAObjectPrxSResult>
    opAObjectPrxSAsync(Ice.ObjectPrx[] i, Ice.Current current) =>
        Task.FromResult(new Test.MyInterface_OpAObjectPrxSResult(i, i));

    public override Task<Test.MyInterface_OpLObjectPrxSResult>
    opLObjectPrxSAsync(List<Ice.ObjectPrx> i, Ice.Current current) =>
        Task.FromResult(new Test.MyInterface_OpLObjectPrxSResult(i, i));

    public override Task<Test.MyInterface_OpKObjectPrxSResult>
    opKObjectPrxSAsync(LinkedList<Ice.ObjectPrx> i, Ice.Current current) =>
        Task.FromResult(new Test.MyInterface_OpKObjectPrxSResult(i, i));

    public override Task<Test.MyInterface_OpQObjectPrxSResult>
    opQObjectPrxSAsync(Queue<Ice.ObjectPrx> i, Ice.Current current) =>
        Task.FromResult(new Test.MyInterface_OpQObjectPrxSResult(i, i));

    public override Task<Test.MyInterface_OpSObjectPrxSResult>
    opSObjectPrxSAsync(Stack<Ice.ObjectPrx> i, Ice.Current current) =>
        Task.FromResult(new Test.MyInterface_OpSObjectPrxSResult(i, i));

    public override Task<Test.MyInterface_OpAStructSResult>
    opAStructSAsync(Test.S[] i, Ice.Current current) =>
        Task.FromResult(new Test.MyInterface_OpAStructSResult(i, i));

    public override Task<Test.MyInterface_OpLStructSResult>
    opLStructSAsync(List<Test.S> i, Ice.Current current) =>
        Task.FromResult(new Test.MyInterface_OpLStructSResult(i, i));

    public override Task<Test.MyInterface_OpKStructSResult>
    opKStructSAsync(LinkedList<Test.S> i, Ice.Current current) =>
        Task.FromResult<Test.MyInterface_OpKStructSResult>(new Test.MyInterface_OpKStructSResult(i, i));

    public override Task<Test.MyInterface_OpQStructSResult>
    opQStructSAsync(Queue<Test.S> i, Ice.Current current) =>
        Task.FromResult<Test.MyInterface_OpQStructSResult>(new Test.MyInterface_OpQStructSResult(i, i));

    public override Task<Test.MyInterface_OpSStructSResult>
    opSStructSAsync(Stack<Test.S> i, Ice.Current current) =>
        Task.FromResult<Test.MyInterface_OpSStructSResult>(new Test.MyInterface_OpSStructSResult(i, i));

    public override Task<Test.MyInterface_OpAStructSDResult>
    opAStructSDAsync(Test.SD[] i, Ice.Current current) =>
        Task.FromResult(new Test.MyInterface_OpAStructSDResult(i, i));

    public override Task<Test.MyInterface_OpLStructSDResult>
    opLStructSDAsync(List<Test.SD> i, Ice.Current current) =>
        Task.FromResult(new Test.MyInterface_OpLStructSDResult(i, i));

    public override Task<Test.MyInterface_OpKStructSDResult>
    opKStructSDAsync(LinkedList<Test.SD> i, Ice.Current current) =>
        Task.FromResult(new Test.MyInterface_OpKStructSDResult(i, i));

    public override Task<Test.MyInterface_OpQStructSDResult>
    opQStructSDAsync(Queue<Test.SD> i, Ice.Current current) =>
        Task.FromResult(new Test.MyInterface_OpQStructSDResult(i, i));

    public override Task<Test.MyInterface_OpSStructSDResult>
    opSStructSDAsync(Stack<Test.SD> i, Ice.Current current) =>
        Task.FromResult(new Test.MyInterface_OpSStructSDResult(i, i));

    public override Task<Test.MyInterface_OpACVSResult>
    opACVSAsync(Test.CV[] i, Ice.Current current) => Task.FromResult(new Test.MyInterface_OpACVSResult(i, i));

    public override Task<Test.MyInterface_OpLCVSResult>
    opLCVSAsync(List<Test.CV> i, Ice.Current current) => Task.FromResult(new Test.MyInterface_OpLCVSResult(i, i));

    public override Task<Test.MyInterface_OpAIPrxSResult>
    opAIPrxSAsync(Test.IPrx[] i, Ice.Current current) =>
        Task.FromResult(new Test.MyInterface_OpAIPrxSResult(i, i));

    public override Task<Test.MyInterface_OpLIPrxSResult>
    opLIPrxSAsync(List<Test.IPrx> i, Ice.Current current) =>
        Task.FromResult(new Test.MyInterface_OpLIPrxSResult(i, i));

    public override Task<Test.MyInterface_OpKIPrxSResult>
    opKIPrxSAsync(LinkedList<Test.IPrx> i, Ice.Current current) =>
        Task.FromResult(new Test.MyInterface_OpKIPrxSResult(i, i));

    public override Task<Test.MyInterface_OpQIPrxSResult>
    opQIPrxSAsync(Queue<Test.IPrx> i, Ice.Current current) =>
        Task.FromResult(new Test.MyInterface_OpQIPrxSResult(i, i));

    public override Task<Test.MyInterface_OpSIPrxSResult>
    opSIPrxSAsync(Stack<Test.IPrx> i, Ice.Current current) =>
        Task.FromResult(new Test.MyInterface_OpSIPrxSResult(i, i));

    public override Task<Test.MyInterface_OpACRSResult>
    opACRSAsync(Test.CR[] i, Ice.Current current) => Task.FromResult(new Test.MyInterface_OpACRSResult(i, i));

    public override Task<Test.MyInterface_OpLCRSResult>
    opLCRSAsync(List<Test.CR> i, Ice.Current current) => Task.FromResult(new Test.MyInterface_OpLCRSResult(i, i));

    public override Task<Test.MyInterface_OpAEnSResult> opAEnSAsync(Test.En[] i, Ice.Current current) =>
        Task.FromResult(new Test.MyInterface_OpAEnSResult(i, i));

    public override Task<Test.MyInterface_OpLEnSResult>
    opLEnSAsync(List<Test.En> i, Ice.Current current) => Task.FromResult(new Test.MyInterface_OpLEnSResult(i, i));

    public override Task<Test.MyInterface_OpKEnSResult>
    opKEnSAsync(LinkedList<Test.En> i, Ice.Current current) =>
        Task.FromResult(new Test.MyInterface_OpKEnSResult(i, i));

    public override Task<Test.MyInterface_OpQEnSResult>
    opQEnSAsync(Queue<Test.En> i, Ice.Current current) => Task.FromResult(new Test.MyInterface_OpQEnSResult(i, i));

    public override Task<Test.MyInterface_OpSEnSResult>
    opSEnSAsync(Stack<Test.En> i, Ice.Current current) => Task.FromResult(new Test.MyInterface_OpSEnSResult(i, i));

    public override Task<Test.MyInterface_OpCustomIntSResult>
    opCustomIntSAsync(Custom<int> i, Ice.Current current) =>
        Task.FromResult(new Test.MyInterface_OpCustomIntSResult(i, i));

    public override Task<Test.MyInterface_OpCustomCVSResult>
    opCustomCVSAsync(Custom<Test.CV> i, Ice.Current current) =>
        Task.FromResult(new Test.MyInterface_OpCustomCVSResult(i, i));

    public override Task<Test.MyInterface_OpCustomIntSSResult>
    opCustomIntSSAsync(Custom<Custom<int>> i, Ice.Current current) =>
        Task.FromResult(new Test.MyInterface_OpCustomIntSSResult(i, i));

    public override Task<Test.MyInterface_OpCustomCVSSResult>
    opCustomCVSSAsync(Custom<Custom<Test.CV>> i, Ice.Current current) =>
        Task.FromResult(new Test.MyInterface_OpCustomCVSSResult(i, i));
}
