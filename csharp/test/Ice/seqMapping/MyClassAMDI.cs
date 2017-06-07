// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Threading.Tasks;
using System.Collections.Generic;
using Test;

public sealed class MyClassI : MyClassDisp_
{
    public override Task shutdownAsync(Ice.Current current)
    {
        current.adapter.getCommunicator().shutdown();
        return null;
    }

    public override Task<MyClass_OpAByteSResult>
    opAByteSAsync(byte[] i, Ice.Current current)
    {
        return Task.FromResult<MyClass_OpAByteSResult>(new MyClass_OpAByteSResult(i, i));
    }

    public override Task<MyClass_OpLByteSResult>
    opLByteSAsync(List<byte> i, Ice.Current current)
    {
        return Task.FromResult<MyClass_OpLByteSResult>(new MyClass_OpLByteSResult(i, i));
    }

    public override Task<MyClass_OpKByteSResult>
    opKByteSAsync(LinkedList<byte> i, Ice.Current current)
    {
        return Task.FromResult<MyClass_OpKByteSResult>(new MyClass_OpKByteSResult(i, i));
    }

    public override Task<MyClass_OpQByteSResult>
    opQByteSAsync(Queue<byte> i, Ice.Current current)
    {
        return Task.FromResult<MyClass_OpQByteSResult>(new MyClass_OpQByteSResult(i, i));
    }

    public override Task<MyClass_OpSByteSResult>
    opSByteSAsync(Stack<byte> i, Ice.Current current)
    {
        return Task.FromResult<MyClass_OpSByteSResult>(new MyClass_OpSByteSResult(i, i));
    }

    public override Task<MyClass_OpABoolSResult>
    opABoolSAsync(bool[] i, Ice.Current current)
    {
        return Task.FromResult<MyClass_OpABoolSResult>(new MyClass_OpABoolSResult(i, i));
    }

    public override Task<MyClass_OpLBoolSResult>
    opLBoolSAsync(List<bool> i, Ice.Current current)
    {
        return Task.FromResult<MyClass_OpLBoolSResult>(new MyClass_OpLBoolSResult(i, i));
    }

    public override Task<MyClass_OpKBoolSResult>
    opKBoolSAsync(LinkedList<bool> i, Ice.Current current)
    {
        return Task.FromResult<MyClass_OpKBoolSResult>(new MyClass_OpKBoolSResult(i, i));
    }

    public override Task<MyClass_OpQBoolSResult>
    opQBoolSAsync(Queue<bool> i, Ice.Current current)
    {
        return Task.FromResult<MyClass_OpQBoolSResult>(new MyClass_OpQBoolSResult(i, i));
    }

    public override Task<MyClass_OpSBoolSResult>
    opSBoolSAsync(Stack<bool> i, Ice.Current current)
    {
        return Task.FromResult<MyClass_OpSBoolSResult>(new MyClass_OpSBoolSResult(i, i));
    }

    public override Task<MyClass_OpAShortSResult>
    opAShortSAsync(short[] i, Ice.Current current)
    {
        return Task.FromResult<MyClass_OpAShortSResult>(new MyClass_OpAShortSResult(i, i));
    }

    public override Task<MyClass_OpLShortSResult>
    opLShortSAsync(List<short> i, Ice.Current current)
    {
        return Task.FromResult<MyClass_OpLShortSResult>(new MyClass_OpLShortSResult(i, i));
    }

    public override Task<MyClass_OpKShortSResult>
    opKShortSAsync(LinkedList<short> i, Ice.Current current)
    {
        return Task.FromResult<MyClass_OpKShortSResult>(new MyClass_OpKShortSResult(i, i));
    }

    public override Task<MyClass_OpQShortSResult>
    opQShortSAsync(Queue<short> i, Ice.Current current)
    {
        return Task.FromResult<MyClass_OpQShortSResult>(new MyClass_OpQShortSResult(i, i));
    }

    public override Task<MyClass_OpSShortSResult>
    opSShortSAsync(Stack<short> i, Ice.Current current)
    {
        return Task.FromResult<MyClass_OpSShortSResult>(new MyClass_OpSShortSResult(i, i));
    }

    public override Task<MyClass_OpAIntSResult>
    opAIntSAsync(int[] i, Ice.Current current)
    {
        return Task.FromResult<MyClass_OpAIntSResult>(new MyClass_OpAIntSResult(i, i));
    }

    public override Task<MyClass_OpLIntSResult>
    opLIntSAsync(List<int> i, Ice.Current current)
    {
        return Task.FromResult<MyClass_OpLIntSResult>(new MyClass_OpLIntSResult(i, i));
    }

    public override Task<MyClass_OpKIntSResult>
    opKIntSAsync(LinkedList<int> i, Ice.Current current)
    {
        return Task.FromResult<MyClass_OpKIntSResult>(new MyClass_OpKIntSResult(i, i));
    }

    public override Task<MyClass_OpQIntSResult>
    opQIntSAsync(Queue<int> i, Ice.Current current)
    {
        return Task.FromResult<MyClass_OpQIntSResult>(new MyClass_OpQIntSResult(i, i));
    }

    public override Task<MyClass_OpSIntSResult>
    opSIntSAsync(Stack<int> i, Ice.Current current)
    {
        return Task.FromResult<MyClass_OpSIntSResult>(new MyClass_OpSIntSResult(i, i));
    }

    public override Task<MyClass_OpALongSResult>
    opALongSAsync(long[] i, Ice.Current current)
    {
        return Task.FromResult<MyClass_OpALongSResult>(new MyClass_OpALongSResult(i, i));
    }

    public override Task<MyClass_OpLLongSResult>
    opLLongSAsync(List<long> i, Ice.Current current)
    {
        return Task.FromResult<MyClass_OpLLongSResult>(new MyClass_OpLLongSResult(i, i));
    }

    public override Task<MyClass_OpKLongSResult>
    opKLongSAsync(LinkedList<long> i, Ice.Current current)
    {
        return Task.FromResult<MyClass_OpKLongSResult>(new MyClass_OpKLongSResult(i, i));
    }

    public override Task<MyClass_OpQLongSResult>
    opQLongSAsync(Queue<long> i, Ice.Current current)
    {
        return Task.FromResult<MyClass_OpQLongSResult>(new MyClass_OpQLongSResult(i, i));
    }

    public override Task<MyClass_OpSLongSResult>
    opSLongSAsync(Stack<long> i, Ice.Current current)
    {
        return Task.FromResult<MyClass_OpSLongSResult>(new MyClass_OpSLongSResult(i, i));
    }

    public override Task<MyClass_OpAFloatSResult>
    opAFloatSAsync(float[] i, Ice.Current current)
    {
        return Task.FromResult<MyClass_OpAFloatSResult>(new MyClass_OpAFloatSResult(i, i));
    }

    public override Task<MyClass_OpLFloatSResult>
    opLFloatSAsync(List<float> i, Ice.Current current)
    {
        return Task.FromResult<MyClass_OpLFloatSResult>(new MyClass_OpLFloatSResult(i, i));
    }

    public override Task<MyClass_OpKFloatSResult>
    opKFloatSAsync(LinkedList<float> i, Ice.Current current)
    {
        return Task.FromResult<MyClass_OpKFloatSResult>(new MyClass_OpKFloatSResult(i, i));
    }

    public override Task<MyClass_OpQFloatSResult>
    opQFloatSAsync(Queue<float> i, Ice.Current current)
    {
        return Task.FromResult<MyClass_OpQFloatSResult>(new MyClass_OpQFloatSResult(i, i));
    }

    public override Task<MyClass_OpSFloatSResult>
    opSFloatSAsync(Stack<float> i, Ice.Current current)
    {
        return Task.FromResult<MyClass_OpSFloatSResult>(new MyClass_OpSFloatSResult(i, i));
    }

    public override Task<MyClass_OpADoubleSResult>
    opADoubleSAsync(double[] i, Ice.Current current)
    {
        return Task.FromResult<MyClass_OpADoubleSResult>(new MyClass_OpADoubleSResult(i, i));
    }

    public override Task<MyClass_OpLDoubleSResult>
    opLDoubleSAsync(List<double> i, Ice.Current current)
    {
        return Task.FromResult<MyClass_OpLDoubleSResult>(new MyClass_OpLDoubleSResult(i, i));
    }

    public override Task<MyClass_OpKDoubleSResult>
    opKDoubleSAsync(LinkedList<double> i, Ice.Current current)
    {
        return Task.FromResult<MyClass_OpKDoubleSResult>(new MyClass_OpKDoubleSResult(i, i));
    }

    public override Task<MyClass_OpQDoubleSResult>
    opQDoubleSAsync(Queue<double> i, Ice.Current current)
    {
        return Task.FromResult<MyClass_OpQDoubleSResult>(new MyClass_OpQDoubleSResult(i, i));
    }

    public override Task<MyClass_OpSDoubleSResult>
    opSDoubleSAsync(Stack<double> i, Ice.Current current)
    {
        return Task.FromResult<MyClass_OpSDoubleSResult>(new MyClass_OpSDoubleSResult(i, i));
    }

    public override Task<MyClass_OpAStringSResult>
    opAStringSAsync(string[] i, Ice.Current current)
    {
        return Task.FromResult<MyClass_OpAStringSResult>(new MyClass_OpAStringSResult(i, i));
    }

    public override Task<MyClass_OpLStringSResult>
    opLStringSAsync(List<string> i, Ice.Current current)
    {
        return Task.FromResult<MyClass_OpLStringSResult>(new MyClass_OpLStringSResult(i, i));
    }

    public override Task<MyClass_OpKStringSResult>
    opKStringSAsync(LinkedList<string> i, Ice.Current current)
    {
        return Task.FromResult<MyClass_OpKStringSResult>(new MyClass_OpKStringSResult(i, i));
    }

    public override Task<MyClass_OpQStringSResult>
    opQStringSAsync(Queue<string> i, Ice.Current current)
    {
        return Task.FromResult<MyClass_OpQStringSResult>(new MyClass_OpQStringSResult(i, i));
    }

    public override Task<MyClass_OpSStringSResult>
    opSStringSAsync(Stack<string> i, Ice.Current current)
    {
        return Task.FromResult<MyClass_OpSStringSResult>(new MyClass_OpSStringSResult(i, i));
    }

    public override Task<MyClass_OpAObjectSResult> opAObjectSAsync(Ice.Value[] i, Ice.Current current)
    {
        return Task.FromResult<MyClass_OpAObjectSResult>(new MyClass_OpAObjectSResult(i, i));
    }

    public override Task<MyClass_OpLObjectSResult>
    opLObjectSAsync(List<Ice.Value> i, Ice.Current current)
    {
        return Task.FromResult<MyClass_OpLObjectSResult>(new MyClass_OpLObjectSResult(i, i));
    }

    public override Task<MyClass_OpAObjectPrxSResult>
    opAObjectPrxSAsync(Ice.ObjectPrx[] i, Ice.Current current)
    {
        return Task.FromResult<MyClass_OpAObjectPrxSResult>(new MyClass_OpAObjectPrxSResult(i, i));
    }

    public override Task<MyClass_OpLObjectPrxSResult>
    opLObjectPrxSAsync(List<Ice.ObjectPrx> i, Ice.Current current)
    {
        return Task.FromResult<MyClass_OpLObjectPrxSResult>(new MyClass_OpLObjectPrxSResult(i, i));
    }

    public override Task<MyClass_OpKObjectPrxSResult>
    opKObjectPrxSAsync(LinkedList<Ice.ObjectPrx> i, Ice.Current current)
    {
        return Task.FromResult<MyClass_OpKObjectPrxSResult>(new MyClass_OpKObjectPrxSResult(i, i));
    }

    public override Task<MyClass_OpQObjectPrxSResult>
    opQObjectPrxSAsync(Queue<Ice.ObjectPrx> i, Ice.Current current)
    {
        return Task.FromResult<MyClass_OpQObjectPrxSResult>(new MyClass_OpQObjectPrxSResult(i, i));
    }

    public override Task<MyClass_OpSObjectPrxSResult>
    opSObjectPrxSAsync(Stack<Ice.ObjectPrx> i, Ice.Current current)
    {
        return Task.FromResult<MyClass_OpSObjectPrxSResult>(new MyClass_OpSObjectPrxSResult(i, i));
    }

    public override Task<MyClass_OpAStructSResult>
    opAStructSAsync(S[] i, Ice.Current current)
    {
        return Task.FromResult<MyClass_OpAStructSResult>(new MyClass_OpAStructSResult(i, i));
    }

    public override Task<MyClass_OpLStructSResult>
    opLStructSAsync(List<S> i, Ice.Current current)
    {
        return Task.FromResult<MyClass_OpLStructSResult>(new MyClass_OpLStructSResult(i, i));
    }

    public override Task<MyClass_OpKStructSResult>
    opKStructSAsync(LinkedList<S> i, Ice.Current current)
    {
        return Task.FromResult<MyClass_OpKStructSResult>(new MyClass_OpKStructSResult(i, i));
    }

    public override Task<MyClass_OpQStructSResult>
    opQStructSAsync(Queue<S> i, Ice.Current current)
    {
        return Task.FromResult<MyClass_OpQStructSResult>(new MyClass_OpQStructSResult(i, i));
    }

    public override Task<MyClass_OpSStructSResult>
    opSStructSAsync(Stack<S> i, Ice.Current current)
    {
        return Task.FromResult<MyClass_OpSStructSResult>(new MyClass_OpSStructSResult(i, i));
    }

    public override Task<MyClass_OpAStructSDResult>
    opAStructSDAsync(SD[] i, Ice.Current current)
    {
        return Task.FromResult<MyClass_OpAStructSDResult>(new MyClass_OpAStructSDResult(i, i));
    }

    public override Task<MyClass_OpLStructSDResult>
    opLStructSDAsync(List<SD> i, Ice.Current current)
    {
        return Task.FromResult<MyClass_OpLStructSDResult>(new MyClass_OpLStructSDResult(i, i));
    }

    public override Task<MyClass_OpKStructSDResult>
    opKStructSDAsync(LinkedList<SD> i, Ice.Current current)
    {
        return Task.FromResult<MyClass_OpKStructSDResult>(new MyClass_OpKStructSDResult(i, i));
    }

    public override Task<MyClass_OpQStructSDResult>
    opQStructSDAsync(Queue<SD> i, Ice.Current current)
    {
        return Task.FromResult<MyClass_OpQStructSDResult>(new MyClass_OpQStructSDResult(i, i));
    }

    public override Task<MyClass_OpSStructSDResult>
    opSStructSDAsync(Stack<SD> i, Ice.Current current)
    {
        return Task.FromResult<MyClass_OpSStructSDResult>(new MyClass_OpSStructSDResult(i, i));
    }

    public override Task<MyClass_OpACVSResult>
    opACVSAsync(CV[] i, Ice.Current current)
    {
        return Task.FromResult<MyClass_OpACVSResult>(new MyClass_OpACVSResult(i, i));
    }

    public override Task<MyClass_OpLCVSResult>
    opLCVSAsync(List<CV> i, Ice.Current current)
    {
        return Task.FromResult<MyClass_OpLCVSResult>(new MyClass_OpLCVSResult(i, i));
    }

    public override Task<MyClass_OpAIPrxSResult>
    opAIPrxSAsync(Test.IPrx[] i, Ice.Current current)
    {
        return Task.FromResult<MyClass_OpAIPrxSResult>(new MyClass_OpAIPrxSResult(i, i));
    }

    public override Task<MyClass_OpLIPrxSResult>
    opLIPrxSAsync(List<Test.IPrx> i, Ice.Current current)
    {
        return Task.FromResult<MyClass_OpLIPrxSResult>(new MyClass_OpLIPrxSResult(i, i));
    }

    public override Task<MyClass_OpKIPrxSResult>
    opKIPrxSAsync(LinkedList<Test.IPrx> i, Ice.Current current)
    {
        return Task.FromResult<MyClass_OpKIPrxSResult>(new MyClass_OpKIPrxSResult(i, i));
    }

    public override Task<MyClass_OpQIPrxSResult>
    opQIPrxSAsync(Queue<Test.IPrx> i, Ice.Current current)
    {
        return Task.FromResult<MyClass_OpQIPrxSResult>(new MyClass_OpQIPrxSResult(i, i));
    }

    public override Task<MyClass_OpSIPrxSResult>
    opSIPrxSAsync(Stack<Test.IPrx> i, Ice.Current current)
    {
        return Task.FromResult<MyClass_OpSIPrxSResult>(new MyClass_OpSIPrxSResult(i, i));
    }

    public override Task<MyClass_OpACRSResult>
    opACRSAsync(CR[] i, Ice.Current current)
    {
        return Task.FromResult<MyClass_OpACRSResult>(new MyClass_OpACRSResult(i, i));
    }

    public override Task<MyClass_OpLCRSResult>
    opLCRSAsync(List<CR> i, Ice.Current current)
    {
        return Task.FromResult<MyClass_OpLCRSResult>(new MyClass_OpLCRSResult(i, i));
    }

    public override Task<MyClass_OpAEnSResult> opAEnSAsync(En[] i, Ice.Current current)
    {
        return Task.FromResult<MyClass_OpAEnSResult>(new MyClass_OpAEnSResult(i, i));
    }

    public override Task<MyClass_OpLEnSResult>
    opLEnSAsync(List<En> i, Ice.Current current)
    {
        return Task.FromResult<MyClass_OpLEnSResult>(new MyClass_OpLEnSResult(i, i));
    }

    public override Task<MyClass_OpKEnSResult>
    opKEnSAsync(LinkedList<En> i, Ice.Current current)
    {
        return Task.FromResult<MyClass_OpKEnSResult>(new MyClass_OpKEnSResult(i, i));
    }

    public override Task<MyClass_OpQEnSResult>
    opQEnSAsync(Queue<En> i, Ice.Current current)
    {
        return Task.FromResult<MyClass_OpQEnSResult>(new MyClass_OpQEnSResult(i, i));
    }

    public override Task<MyClass_OpSEnSResult>
    opSEnSAsync(Stack<En> i, Ice.Current current)
    {
        return Task.FromResult<MyClass_OpSEnSResult>(new MyClass_OpSEnSResult(i, i));
    }

    public override Task<MyClass_OpCustomIntSResult>
    opCustomIntSAsync(Custom<int> i, Ice.Current current)
    {
        return Task.FromResult<MyClass_OpCustomIntSResult>(new MyClass_OpCustomIntSResult(i, i));
    }

    public override Task<MyClass_OpCustomCVSResult>
    opCustomCVSAsync(Custom<CV> i, Ice.Current current)
    {
        return Task.FromResult<MyClass_OpCustomCVSResult>(new MyClass_OpCustomCVSResult(i, i));
    }

    public override Task<MyClass_OpCustomIntSSResult>
    opCustomIntSSAsync(Custom<Custom<int>> i, Ice.Current current)
    {
        return Task.FromResult<MyClass_OpCustomIntSSResult>(new MyClass_OpCustomIntSSResult(i, i));
    }

    public override Task<MyClass_OpCustomCVSSResult>
    opCustomCVSSAsync(Custom<Custom<CV>> i, Ice.Current current)
    {
        return Task.FromResult<MyClass_OpCustomCVSSResult>(new MyClass_OpCustomCVSSResult(i, i));
    }

    public override Task<MyClass_OpSerialSmallCSharpResult>
    opSerialSmallCSharpAsync(Serialize.Small i, Ice.Current current)
    {
        return Task.FromResult<MyClass_OpSerialSmallCSharpResult>(new MyClass_OpSerialSmallCSharpResult(i, i));
    }

    public override Task<MyClass_OpSerialLargeCSharpResult>
    opSerialLargeCSharpAsync(Serialize.Large i, Ice.Current current)
    {
        return Task.FromResult<MyClass_OpSerialLargeCSharpResult>(new MyClass_OpSerialLargeCSharpResult(i, i));
    }

    public override Task<MyClass_OpSerialStructCSharpResult>
    opSerialStructCSharpAsync(Serialize.Struct i, Ice.Current current)
    {
        return Task.FromResult<MyClass_OpSerialStructCSharpResult>(new MyClass_OpSerialStructCSharpResult(i, i));
    }
}
