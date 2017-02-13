// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System.Collections.Generic;
using Test;

public sealed class MyClassI : Test.MyClassDisp_
{
    public override void shutdown(Ice.Current current)
    {
        current.adapter.getCommunicator().shutdown();
    }

    public override byte[] opAByteS(byte[] i, out byte[] o, Ice.Current current)
    {
        o = i;
        return i;
    }

    public override List<byte> opLByteS(List<byte> i, out List<byte> o, Ice.Current current)
    {
        o = i;
        return i;
    }

    public override LinkedList<byte> opKByteS(LinkedList<byte> i, out LinkedList<byte> o, Ice.Current current)
    {
        o = i;
        return i;
    }

    public override Queue<byte> opQByteS(Queue<byte> i, out Queue<byte> o, Ice.Current current)
    {
        o = i;
        return i;
    }

    public override Stack<byte> opSByteS(Stack<byte> i, out Stack<byte> o, Ice.Current current)
    {
        o = i;
        return i;
    }

    public override bool[] opABoolS(bool[] i, out bool[] o, Ice.Current current)
    {
        o = i;
        return i;
    }

    public override List<bool> opLBoolS(List<bool> i, out List<bool> o, Ice.Current current)
    {
        o = i;
        return i;
    }

    public override LinkedList<bool> opKBoolS(LinkedList<bool> i, out LinkedList<bool> o, Ice.Current current)
    {
        o = i;
        return i;
    }

    public override Queue<bool> opQBoolS(Queue<bool> i, out Queue<bool> o, Ice.Current current)
    {
        o = i;
        return i;
    }

    public override Stack<bool> opSBoolS(Stack<bool> i, out Stack<bool> o, Ice.Current current)
    {
        o = i;
        return i;
    }

    public override short[] opAShortS(short[] i, out short[] o, Ice.Current current)
    {
        o = i;
        return i;
    }

    public override List<short> opLShortS(List<short> i, out List<short> o, Ice.Current current)
    {
        o = i;
        return i;
    }

    public override LinkedList<short> opKShortS(LinkedList<short> i, out LinkedList<short> o, Ice.Current current)
    {
        o = i;
        return i;
    }

    public override Queue<short> opQShortS(Queue<short> i, out Queue<short> o, Ice.Current current)
    {
        o = i;
        return i;
    }

    public override Stack<short> opSShortS(Stack<short> i, out Stack<short> o, Ice.Current current)
    {
        o = i;
        return i;
    }

    public override int[] opAIntS(int[] i, out int[] o, Ice.Current current)
    {
        o = i;
        return i;
    }

    public override List<int> opLIntS(List<int> i, out List<int> o, Ice.Current current)
    {
        o = i;
        return i;
    }

    public override LinkedList<int> opKIntS(LinkedList<int> i, out LinkedList<int> o, Ice.Current current)
    {
        o = i;
        return i;
    }

    public override Queue<int> opQIntS(Queue<int> i, out Queue<int> o, Ice.Current current)
    {
        o = i;
        return i;
    }

    public override Stack<int> opSIntS(Stack<int> i, out Stack<int> o, Ice.Current current)
    {
        o = i;
        return i;
    }

    public override long[] opALongS(long[] i, out long[] o, Ice.Current current)
    {
        o = i;
        return i;
    }

    public override List<long> opLLongS(List<long> i, out List<long> o, Ice.Current current)
    {
        o = i;
        return i;
    }

    public override LinkedList<long> opKLongS(LinkedList<long> i, out LinkedList<long> o, Ice.Current current)
    {
        o = i;
        return i;
    }

    public override Queue<long> opQLongS(Queue<long> i, out Queue<long> o, Ice.Current current)
    {
        o = i;
        return i;
    }

    public override Stack<long> opSLongS(Stack<long> i, out Stack<long> o, Ice.Current current)
    {
        o = i;
        return i;
    }

    public override float[] opAFloatS(float[] i, out float[] o, Ice.Current current)
    {
        o = i;
        return i;
    }

    public override List<float> opLFloatS(List<float> i, out List<float> o, Ice.Current current)
    {
        o = i;
        return i;
    }

    public override LinkedList<float> opKFloatS(LinkedList<float> i, out LinkedList<float> o, Ice.Current current)
    {
        o = i;
        return i;
    }

    public override Queue<float> opQFloatS(Queue<float> i, out Queue<float> o, Ice.Current current)
    {
        o = i;
        return i;
    }

    public override Stack<float> opSFloatS(Stack<float> i, out Stack<float> o, Ice.Current current)
    {
        o = i;
        return i;
    }

    public override double[] opADoubleS(double[] i, out double[] o, Ice.Current current)
    {
        o = i;
        return i;
    }

    public override List<double> opLDoubleS(List<double> i, out List<double> o, Ice.Current current)
    {
        o = i;
        return i;
    }

    public override LinkedList<double> opKDoubleS(LinkedList<double> i, out LinkedList<double> o, Ice.Current current)
    {
        o = i;
        return i;
    }

    public override Queue<double> opQDoubleS(Queue<double> i, out Queue<double> o, Ice.Current current)
    {
        o = i;
        return i;
    }

    public override Stack<double> opSDoubleS(Stack<double> i, out Stack<double> o, Ice.Current current)
    {
        o = i;
        return i;
    }

    public override string[] opAStringS(string[] i, out string[] o, Ice.Current current)
    {
        o = i;
        return i;
    }

    public override List<string> opLStringS(List<string> i, out List<string> o, Ice.Current current)
    {
        o = i;
        return i;
    }

    public override LinkedList<string> opKStringS(LinkedList<string> i, out LinkedList<string> o, Ice.Current current)
    {
        o = i;
        return i;
    }

    public override Queue<string> opQStringS(Queue<string> i, out Queue<string> o, Ice.Current current)
    {
        o = i;
        return i;
    }

    public override Stack<string> opSStringS(Stack<string> i, out Stack<string> o, Ice.Current current)
    {
        o = i;
        return i;
    }

    public override Ice.Value[] opAObjectS(Ice.Value[] i, out Ice.Value[] o, Ice.Current current)
    {
        o = i;
        return i;
    }

    public override List<Ice.Value> opLObjectS(List<Ice.Value> i, out List<Ice.Value> o, Ice.Current current)
    {
        o = i;
        return i;
    }

    public override Ice.ObjectPrx[] opAObjectPrxS(Ice.ObjectPrx[] i, out Ice.ObjectPrx[] o, Ice.Current current)
    {
        o = i;
        return i;
    }

    public override List<Ice.ObjectPrx> opLObjectPrxS(List<Ice.ObjectPrx> i, out List<Ice.ObjectPrx> o, Ice.Current current)
    {
        o = i;
        return i;
    }

    public override LinkedList<Ice.ObjectPrx> opKObjectPrxS(LinkedList<Ice.ObjectPrx> i, out LinkedList<Ice.ObjectPrx> o, Ice.Current current)
    {
        o = i;
        return i;
    }

    public override Queue<Ice.ObjectPrx> opQObjectPrxS(Queue<Ice.ObjectPrx> i, out Queue<Ice.ObjectPrx> o, Ice.Current current)
    {
        o = i;
        return i;
    }

    public override Stack<Ice.ObjectPrx> opSObjectPrxS(Stack<Ice.ObjectPrx> i, out Stack<Ice.ObjectPrx> o, Ice.Current current)
    {
        o = i;
        return i;
    }

    public override S[] opAStructS(S[] i, out S[] o, Ice.Current current)
    {
        o = i;
        return i;
    }

    public override List<S> opLStructS(List<S> i, out List<S> o, Ice.Current current)
    {
        o = i;
        return i;
    }

    public override LinkedList<S> opKStructS(LinkedList<S> i, out LinkedList<S> o, Ice.Current current)
    {
        o = i;
        return i;
    }

    public override Queue<S> opQStructS(Queue<S> i, out Queue<S> o, Ice.Current current)
    {
        o = i;
        return i;
    }

    public override Stack<S> opSStructS(Stack<S> i, out Stack<S> o, Ice.Current current)
    {
        o = i;
        return i;
    }

    public override SD[] opAStructSD(SD[] i, out SD[] o, Ice.Current current)
    {
        o = i;
        return i;
    }

    public override List<SD> opLStructSD(List<SD> i, out List<SD> o, Ice.Current current)
    {
        o = i;
        return i;
    }

    public override LinkedList<SD> opKStructSD(LinkedList<SD> i, out LinkedList<SD> o, Ice.Current current)
    {
        o = i;
        return i;
    }

    public override Queue<SD> opQStructSD(Queue<SD> i, out Queue<SD> o, Ice.Current current)
    {
        o = i;
        return i;
    }

    public override Stack<SD> opSStructSD(Stack<SD> i, out Stack<SD> o, Ice.Current current)
    {
        o = i;
        return i;
    }

    public override CV[] opACVS(CV[] i, out CV[] o, Ice.Current current)
    {
        o = i;
        return i;
    }

    public override List<CV> opLCVS(List<CV> i, out List<CV> o, Ice.Current current)
    {
        o = i;
        return i;
    }

    public override CR[] opACRS(CR[] i, out CR[] o, Ice.Current current)
    {
        o = i;
        return i;
    }

    public override List<CR> opLCRS(List<CR> i, out List<CR> o, Ice.Current current)
    {
        o = i;
        return i;
    }

    public override En[] opAEnS(En[] i, out En[] o, Ice.Current current)
    {
        o = i;
        return i;
    }

    public override List<En> opLEnS(List<En> i, out List<En> o, Ice.Current current)
    {
        o = i;
        return i;
    }

    public override LinkedList<En> opKEnS(LinkedList<En> i, out LinkedList<En> o, Ice.Current current)
    {
        o = i;
        return i;
    }

    public override Queue<En> opQEnS(Queue<En> i, out Queue<En> o, Ice.Current current)
    {
        o = i;
        return i;
    }

    public override Stack<En> opSEnS(Stack<En> i, out Stack<En> o, Ice.Current current)
    {
        o = i;
        return i;
    }

    public override Ice.ObjectPrx[] opACVPrxS(Ice.ObjectPrx[] i, out Ice.ObjectPrx[] o, Ice.Current current)
    {
        o = i;
        return i;
    }

    public override List<Ice.ObjectPrx> opLCVPrxS(List<Ice.ObjectPrx> i, out List<Ice.ObjectPrx> o, Ice.Current current)
    {
        o = i;
        return i;
    }

    public override LinkedList<Ice.ObjectPrx> opKCVPrxS(LinkedList<Ice.ObjectPrx> i, out LinkedList<Ice.ObjectPrx> o, Ice.Current current)
    {
        o = i;
        return i;
    }

    public override Queue<Ice.ObjectPrx> opQCVPrxS(Queue<Ice.ObjectPrx> i, out Queue<Ice.ObjectPrx> o, Ice.Current current)
    {
        o = i;
        return i;
    }

    public override Stack<Ice.ObjectPrx> opSCVPrxS(Stack<Ice.ObjectPrx> i, out Stack<Ice.ObjectPrx> o, Ice.Current current)
    {
        o = i;
        return i;
    }

    public override Custom<int> opCustomIntS(Custom<int> i, out Custom<int> o, Ice.Current current)
    {
        o = i;
        return i;
    }

    public override Custom<CV> opCustomCVS(Custom<CV> i, out Custom<CV> o, Ice.Current current)
    {
        o = i;
        return i;
    }

    public override Custom<Custom<int>> opCustomIntSS(Custom<Custom<int>> i, out Custom<Custom<int>> o,
                                                      Ice.Current current)
    {
        o = i;
        return i;
    }

    public override Custom<Custom<CV>> opCustomCVSS(Custom<Custom<CV>> i, out Custom<Custom<CV>> o,
                                                    Ice.Current current)
    {
        o = i;
        return i;
    }

    public override Serialize.Small opSerialSmallCSharp(Serialize.Small i, out Serialize.Small o,
                                                        Ice.Current current)
    {
        o = i;
        return i;
    }

    public override Serialize.Large opSerialLargeCSharp(Serialize.Large i, out Serialize.Large o,
                                                        Ice.Current current)
    {
        o = i;
        return i;
    }

    public override Serialize.Struct opSerialStructCSharp(Serialize.Struct i, out Serialize.Struct o,
                                                          Ice.Current current)
    {
        o = i;
        return i;
    }
}
