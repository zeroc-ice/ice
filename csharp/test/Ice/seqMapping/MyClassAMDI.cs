// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Collections.Generic;
using Test;

public sealed class MyClassI : MyClassDisp_
{
    public override void shutdownAsync(Action response, Action<Exception> exception, Ice.Current current)
    {
        current.adapter.getCommunicator().shutdown();
        response();
    }

    public override void
    opAByteSAsync(byte[] i, Action<MyClass_OpAByteSResult> response, Action<Exception> exception, Ice.Current current)
    {
        response(new MyClass_OpAByteSResult(i, i));
    }

    public override void
    opLByteSAsync(List<byte> i, Action<MyClass_OpLByteSResult> response, Action<Exception> exception,
                  Ice.Current current)
    {
        response(new MyClass_OpLByteSResult(i, i));
    }

    public override void
    opKByteSAsync(LinkedList<byte> i, Action<MyClass_OpKByteSResult> response, Action<Exception> exception,
                  Ice.Current current)
    {
        response(new MyClass_OpKByteSResult(i, i));
    }

    public override void
    opQByteSAsync(Queue<byte> i, Action<MyClass_OpQByteSResult> response, Action<Exception> exception,
                  Ice.Current current)
    {
        response(new MyClass_OpQByteSResult(i, i));
    }

    public override void
    opSByteSAsync(Stack<byte> i, Action<MyClass_OpSByteSResult> response, Action<Exception> exception,
                  Ice.Current current)
    {
        response(new MyClass_OpSByteSResult(i, i));
    }

    public override void
    opABoolSAsync(bool[] i, Action<MyClass_OpABoolSResult> response, Action<Exception> exception,
                  Ice.Current current)
    {
        response(new MyClass_OpABoolSResult(i, i));
    }

    public override void 
    opLBoolSAsync(List<bool> i, Action<MyClass_OpLBoolSResult> response, Action<Exception> exception,
                  Ice.Current current)
    {
        response(new MyClass_OpLBoolSResult(i, i));
    }

    public override void
    opKBoolSAsync(LinkedList<bool> i, Action<MyClass_OpKBoolSResult> response, Action<Exception> exception,
                  Ice.Current current)
    {
        response(new MyClass_OpKBoolSResult(i, i));
    }

    public override void
    opQBoolSAsync(Queue<bool> i, Action<MyClass_OpQBoolSResult> response, Action<Exception> exception,
                  Ice.Current current)
    {
        response(new MyClass_OpQBoolSResult(i, i));
    }

    public override void
    opSBoolSAsync(Stack<bool> i, Action<MyClass_OpSBoolSResult> response, Action<Exception> exception,
                  Ice.Current current)
    {
        response(new MyClass_OpSBoolSResult(i, i));
    }

    public override void
    opAShortSAsync(short[] i, Action<MyClass_OpAShortSResult> response, Action<Exception> exception,
                   Ice.Current current)
    {
        response(new MyClass_OpAShortSResult(i, i));
    }

    public override void
    opLShortSAsync(List<short> i, Action<MyClass_OpLShortSResult> response, Action<Exception> exception,
                   Ice.Current current)
    {
        response(new MyClass_OpLShortSResult(i, i));
    }

    public override void
    opKShortSAsync(LinkedList<short> i, Action<MyClass_OpKShortSResult> response, Action<Exception> exception,
                   Ice.Current current)
    {
        response(new MyClass_OpKShortSResult(i, i));
    }

    public override void
    opQShortSAsync(Queue<short> i, Action<MyClass_OpQShortSResult> response, Action<Exception> exception,
                   Ice.Current current)
    {
        response(new MyClass_OpQShortSResult(i, i));
    }

    public override void
    opSShortSAsync(Stack<short> i, Action<MyClass_OpSShortSResult> response, Action<Exception> exception,
                   Ice.Current current)
    {
        response(new MyClass_OpSShortSResult(i, i));
    }

    public override void
    opAIntSAsync(int[] i, Action<MyClass_OpAIntSResult> response, Action<Exception> exception, Ice.Current current)
    {
        response(new MyClass_OpAIntSResult(i, i));
    }

    public override void
    opLIntSAsync(List<int> i, Action<MyClass_OpLIntSResult> response, Action<Exception> exception, Ice.Current current)
    {
        response(new MyClass_OpLIntSResult(i, i));
    }

    public override void
    opKIntSAsync(LinkedList<int> i, Action<MyClass_OpKIntSResult> response, Action<Exception> exception,
                 Ice.Current current)
    {
        response(new MyClass_OpKIntSResult(i, i));
    }

    public override void
    opQIntSAsync(Queue<int> i, Action<MyClass_OpQIntSResult> response, Action<Exception> exception,
                 Ice.Current current)
    {
        response(new MyClass_OpQIntSResult(i, i));
    }

    public override void
    opSIntSAsync(Stack<int> i, Action<MyClass_OpSIntSResult> response, Action<Exception> exception,
                 Ice.Current current)
    {
        response(new MyClass_OpSIntSResult(i, i));
    }

    public override void
    opALongSAsync(long[] i, Action<MyClass_OpALongSResult> response, Action<Exception> exception, Ice.Current current)
    {
        response(new MyClass_OpALongSResult(i, i));
    }

    public override void
    opLLongSAsync(List<long> i, Action<MyClass_OpLLongSResult> response, Action<Exception> exception,
                  Ice.Current current)
    {
        response(new MyClass_OpLLongSResult(i, i));
    }

    public override void
    opKLongSAsync(LinkedList<long> i, Action<MyClass_OpKLongSResult> response, Action<Exception> exception,
                  Ice.Current current)
    {
        response(new MyClass_OpKLongSResult(i, i));
    }

    public override void
    opQLongSAsync(Queue<long> i, Action<MyClass_OpQLongSResult> response, Action<Exception> exception,
                  Ice.Current current)
    {
        response(new MyClass_OpQLongSResult(i, i));
    }

    public override void
    opSLongSAsync(Stack<long> i, Action<MyClass_OpSLongSResult> response, Action<Exception> exception,
                  Ice.Current current)
    {
        response(new MyClass_OpSLongSResult(i, i));
    }

    public override void
    opAFloatSAsync(float[] i, Action<MyClass_OpAFloatSResult> response, Action<Exception> exception,
                   Ice.Current current)
    {
        response(new MyClass_OpAFloatSResult(i, i));
    }

    public override void
    opLFloatSAsync(List<float> i, Action<MyClass_OpLFloatSResult> response, Action<Exception> exception,
                   Ice.Current current)
    {
        response(new MyClass_OpLFloatSResult(i, i));
    }

    public override void
    opKFloatSAsync(LinkedList<float> i, Action<MyClass_OpKFloatSResult> response, Action<Exception> exception,
                   Ice.Current current)
    {
        response(new MyClass_OpKFloatSResult(i, i));
    }

    public override void
    opQFloatSAsync(Queue<float> i, Action<MyClass_OpQFloatSResult> response, Action<Exception> exception,
                   Ice.Current current)
    {
        response(new MyClass_OpQFloatSResult(i, i));
    }

    public override void
    opSFloatSAsync(Stack<float> i, Action<MyClass_OpSFloatSResult> response, Action<Exception> exception,
                   Ice.Current current)
    {
        response(new MyClass_OpSFloatSResult(i, i));
    }

    public override void
    opADoubleSAsync(double[] i, Action<MyClass_OpADoubleSResult> response, Action<Exception> exception,
                    Ice.Current current)
    {
        response(new MyClass_OpADoubleSResult(i, i));
    }

    public override void
    opLDoubleSAsync(List<double> i, Action<MyClass_OpLDoubleSResult> response, Action<Exception> exception,
                    Ice.Current current)
    {
        response(new MyClass_OpLDoubleSResult(i, i));
    }

    public override void
    opKDoubleSAsync(LinkedList<double> i, Action<MyClass_OpKDoubleSResult> response, Action<Exception> exception,
                    Ice.Current current)
    {
        response(new MyClass_OpKDoubleSResult(i, i));
    }

    public override void
    opQDoubleSAsync(Queue<double> i, Action<MyClass_OpQDoubleSResult> response, Action<Exception> exception,
                    Ice.Current current)
    {
        response(new MyClass_OpQDoubleSResult(i, i));
    }

    public override void
    opSDoubleSAsync(Stack<double> i, Action<MyClass_OpSDoubleSResult> response, Action<Exception> exception,
                    Ice.Current current)
    {
        response(new MyClass_OpSDoubleSResult(i, i));
    }

    public override void
    opAStringSAsync(string[] i, Action<MyClass_OpAStringSResult> response, Action<Exception> exception,
                    Ice.Current current)
    {
        response(new MyClass_OpAStringSResult(i, i));
    }

    public override void
    opLStringSAsync(List<string> i, Action<MyClass_OpLStringSResult> response, Action<Exception> exception,
                    Ice.Current current)
    {
        response(new MyClass_OpLStringSResult(i, i));
    }

    public override void
    opKStringSAsync(LinkedList<string> i, Action<MyClass_OpKStringSResult> response, Action<Exception> exception,
                    Ice.Current current)
    {
        response(new MyClass_OpKStringSResult(i, i));
    }

    public override void
    opQStringSAsync(Queue<string> i, Action<MyClass_OpQStringSResult> response, Action<Exception> exception,
                    Ice.Current current)
    {
        response(new MyClass_OpQStringSResult(i, i));
    }

    public override void
    opSStringSAsync(Stack<string> i, Action<MyClass_OpSStringSResult> response, Action<Exception> exception,
                    Ice.Current current)
    {
        response(new MyClass_OpSStringSResult(i, i));
    }

    public override void opAObjectSAsync(Ice.Value[] i, Action<MyClass_OpAObjectSResult> response, Action<Exception> exception,
                                         Ice.Current current)
    {
        response(new MyClass_OpAObjectSResult(i, i));
    }

    public override void
    opLObjectSAsync(List<Ice.Value> i, Action<MyClass_OpLObjectSResult> response, Action<Exception> exception,
                    Ice.Current current)
    {
        response(new MyClass_OpLObjectSResult(i, i));
    }

    public override void
    opAObjectPrxSAsync(Ice.ObjectPrx[] i, Action<MyClass_OpAObjectPrxSResult> response, Action<Exception> exception,
                       Ice.Current current)
    {
        response(new MyClass_OpAObjectPrxSResult(i, i));
    }

    public override void
    opLObjectPrxSAsync(List<Ice.ObjectPrx> i, Action<MyClass_OpLObjectPrxSResult> response, Action<Exception> exception,
                       Ice.Current current)
    {
        response(new MyClass_OpLObjectPrxSResult(i, i));
    }

    public override void
    opKObjectPrxSAsync(LinkedList<Ice.ObjectPrx> i, Action<MyClass_OpKObjectPrxSResult> response, Action<Exception> exception,
                       Ice.Current current)
    {
        response(new MyClass_OpKObjectPrxSResult(i, i));
    }

    public override void
    opQObjectPrxSAsync(Queue<Ice.ObjectPrx> i, Action<MyClass_OpQObjectPrxSResult> response, Action<Exception> exception,
                       Ice.Current current)
    {
        response(new MyClass_OpQObjectPrxSResult(i, i));
    }

    public override void
    opSObjectPrxSAsync(Stack<Ice.ObjectPrx> i, Action<MyClass_OpSObjectPrxSResult> response, Action<Exception> exception,
                       Ice.Current current)
    {
        response(new MyClass_OpSObjectPrxSResult(i, i));
    }

    public override void
    opAStructSAsync(S[] i, Action<MyClass_OpAStructSResult> response, Action<Exception> exception, Ice.Current current)
    {
        response(new MyClass_OpAStructSResult(i, i));
    }

    public override void
    opLStructSAsync(List<S> i, Action<MyClass_OpLStructSResult> response, Action<Exception> exception,
                    Ice.Current current)
    {
        response(new MyClass_OpLStructSResult(i, i));
    }

    public override void
    opKStructSAsync(LinkedList<S> i, Action<MyClass_OpKStructSResult> response, Action<Exception> exception,
                    Ice.Current current)
    {
        response(new MyClass_OpKStructSResult(i, i));
    }

    public override void
    opQStructSAsync(Queue<S> i, Action<MyClass_OpQStructSResult> response, Action<Exception> exception,
                    Ice.Current current)
    {
        response(new MyClass_OpQStructSResult(i, i));
    }

    public override void
    opSStructSAsync(Stack<S> i, Action<MyClass_OpSStructSResult> response, Action<Exception> exception,
                    Ice.Current current)
    {
        response(new MyClass_OpSStructSResult(i, i));
    }

    public override void
    opAStructSDAsync(SD[] i, Action<MyClass_OpAStructSDResult> response, Action<Exception> exception,
                     Ice.Current current)
    {
        response(new MyClass_OpAStructSDResult(i, i));
    }

    public override void
    opLStructSDAsync(List<SD> i, Action<MyClass_OpLStructSDResult> response, Action<Exception> exception,
                     Ice.Current current)
    {
        response(new MyClass_OpLStructSDResult(i, i));
    }

    public override void
    opKStructSDAsync(LinkedList<SD> i, Action<MyClass_OpKStructSDResult> response, Action<Exception> exception,
                     Ice.Current current)
    {
        response(new MyClass_OpKStructSDResult(i, i));
    }

    public override void
    opQStructSDAsync(Queue<SD> i, Action<MyClass_OpQStructSDResult> response, Action<Exception> exception,
                     Ice.Current current)
    {
        response(new MyClass_OpQStructSDResult(i, i));
    }

    public override void
    opSStructSDAsync(Stack<SD> i, Action<MyClass_OpSStructSDResult> response, Action<Exception> exception,
                     Ice.Current current)
    {
        response(new MyClass_OpSStructSDResult(i, i));
    }

    public override void
    opACVSAsync(CV[] i, Action<MyClass_OpACVSResult> response, Action<Exception> exception, Ice.Current current)
    {
        response(new MyClass_OpACVSResult(i, i));
    }

    public override void
    opLCVSAsync(List<CV> i, Action<MyClass_OpLCVSResult> response, Action<Exception> exception, Ice.Current current)
    {
        response(new MyClass_OpLCVSResult(i, i));
    }

    public override void
    opACVPrxSAsync(CVPrx[] i, Action<MyClass_OpACVPrxSResult> response, Action<Exception> exception, Ice.Current current)
    {
        response(new MyClass_OpACVPrxSResult(i, i));
    }

    public override void
    opLCVPrxSAsync(List<CVPrx> i, Action<MyClass_OpLCVPrxSResult> response, Action<Exception> exception,
                   Ice.Current current)
    {
        response(new MyClass_OpLCVPrxSResult(i, i));
    }

    public override void
    opKCVPrxSAsync(LinkedList<CVPrx> i, Action<MyClass_OpKCVPrxSResult> response, Action<Exception> exception,
                   Ice.Current current)
    {
        response(new MyClass_OpKCVPrxSResult(i, i));
    }

    public override void
    opQCVPrxSAsync(Queue<CVPrx> i, Action<MyClass_OpQCVPrxSResult> response, Action<Exception> exception,
                   Ice.Current current)
    {
        response(new MyClass_OpQCVPrxSResult(i, i));
    }

    public override void
    opSCVPrxSAsync(Stack<CVPrx> i, Action<MyClass_OpSCVPrxSResult> response, Action<Exception> exception,
                   Ice.Current current)
    {
        response(new MyClass_OpSCVPrxSResult(i, i));
    }

    public override void
    opACRSAsync(CR[] i, Action<MyClass_OpACRSResult> response, Action<Exception> exception, Ice.Current current)
    {
        response(new MyClass_OpACRSResult(i, i));
    }

    public override void
    opLCRSAsync(List<CR> i, Action<MyClass_OpLCRSResult> response, Action<Exception> exception,
                Ice.Current current)
    {
        response(new MyClass_OpLCRSResult(i, i));
    }

    public override void opAEnSAsync(En[] i, Action<MyClass_OpAEnSResult> response, Action<Exception> exception,
                                     Ice.Current current)
    {
        response(new MyClass_OpAEnSResult(i, i));
    }

    public override void
    opLEnSAsync(List<En> i, Action<MyClass_OpLEnSResult> response, Action<Exception> exception, Ice.Current current)
    {
        response(new MyClass_OpLEnSResult(i, i));
    }

    public override void
    opKEnSAsync(LinkedList<En> i, Action<MyClass_OpKEnSResult> response, Action<Exception> exception,
                Ice.Current current)
    {
        response(new MyClass_OpKEnSResult(i, i));
    }

    public override void
    opQEnSAsync(Queue<En> i, Action<MyClass_OpQEnSResult> response, Action<Exception> exception, Ice.Current current)
    {
        response(new MyClass_OpQEnSResult(i, i));
    }

    public override void
    opSEnSAsync(Stack<En> i, Action<MyClass_OpSEnSResult> response, Action<Exception> exception,
                Ice.Current current)
    {
        response(new MyClass_OpSEnSResult(i, i));
    }

    public override void
    opCustomIntSAsync(Custom<int> i, Action<MyClass_OpCustomIntSResult> response, Action<Exception> exception,
                      Ice.Current current)
    {
        response(new MyClass_OpCustomIntSResult(i, i));
    }

    public override void
    opCustomCVSAsync(Custom<CV> i, Action<MyClass_OpCustomCVSResult> response, Action<Exception> exception,
                     Ice.Current current)
    {
        response(new MyClass_OpCustomCVSResult(i, i));
    }

    public override void
    opCustomIntSSAsync(Custom<Custom<int>> i, Action<MyClass_OpCustomIntSSResult> response, Action<Exception> exception,
                       Ice.Current current)
    {
        response(new MyClass_OpCustomIntSSResult(i, i));
    }

    public override void
    opCustomCVSSAsync(Custom<Custom<CV>> i, Action<MyClass_OpCustomCVSSResult> response, Action<Exception> exception,
                      Ice.Current current)
    {
        response(new MyClass_OpCustomCVSSResult(i, i));
    }

    public override void
    opSerialSmallCSharpAsync(Serialize.Small i, Action<MyClass_OpSerialSmallCSharpResult> response, Action<Exception> exception,
                             Ice.Current current)
    {
        response(new MyClass_OpSerialSmallCSharpResult(i, i));
    }

    public override void
    opSerialLargeCSharpAsync(Serialize.Large i, Action<MyClass_OpSerialLargeCSharpResult> response, Action<Exception> exception,
                             Ice.Current current)
    {
        response(new MyClass_OpSerialLargeCSharpResult(i, i));
    }

    public override void
    opSerialStructCSharpAsync(Serialize.Struct i, Action<MyClass_OpSerialStructCSharpResult> response, Action<Exception> exception,
                              Ice.Current current)
    {
        response(new MyClass_OpSerialStructCSharpResult(i, i));
    }
}
