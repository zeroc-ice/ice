// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Collections.Generic;
using Test;

public sealed class MyClassI : MyClass
{
    public override void shutdown_async(AMD_MyClass_shutdown cb, Ice.Current current)
    {
        current.adapter.getCommunicator().shutdown();
        cb.ice_response();
    }

    public override void opAByteS_async(AMD_MyClass_opAByteS cb, byte[] i, Ice.Current current)
    {
        cb.ice_response(i, i);
    }

    public override void opLByteS_async(AMD_MyClass_opLByteS cb, List<byte> i, Ice.Current current)
    {
        cb.ice_response(i, i);
    }

    public override void opKByteS_async(AMD_MyClass_opKByteS cb, LinkedList<byte> i, Ice.Current current)
    {
        cb.ice_response(i, i);
    }

    public override void opQByteS_async(AMD_MyClass_opQByteS cb, Queue<byte> i, Ice.Current current)
    {
        cb.ice_response(i, i);
    }

    public override void opSByteS_async(AMD_MyClass_opSByteS cb, Stack<byte> i, Ice.Current current)
    {
        cb.ice_response(i, i);
    }

    public override void opCByteS_async(AMD_MyClass_opCByteS cb, CByteS i, Ice.Current current)
    {
        cb.ice_response(i, i);
    }

    public override void opABoolS_async(AMD_MyClass_opABoolS cb, bool[] i, Ice.Current current)
    {
        cb.ice_response(i, i);
    }

    public override void opLBoolS_async(AMD_MyClass_opLBoolS cb, List<bool> i, Ice.Current current)
    {
        cb.ice_response(i, i);
    }

    public override void opKBoolS_async(AMD_MyClass_opKBoolS cb, LinkedList<bool> i, Ice.Current current)
    {
        cb.ice_response(i, i);
    }

    public override void opQBoolS_async(AMD_MyClass_opQBoolS cb, Queue<bool> i, Ice.Current current)
    {
        cb.ice_response(i, i);
    }

    public override void opSBoolS_async(AMD_MyClass_opSBoolS cb, Stack<bool> i, Ice.Current current)
    {
        cb.ice_response(i, i);
    }

    public override void opCBoolS_async(AMD_MyClass_opCBoolS cb, CBoolS i, Ice.Current current)
    {
        cb.ice_response(i, i);
    }

    public override void opAShortS_async(AMD_MyClass_opAShortS cb, short[] i, Ice.Current current)
    {
        cb.ice_response(i, i);
    }

    public override void opLShortS_async(AMD_MyClass_opLShortS cb, List<short> i, Ice.Current current)
    {
        cb.ice_response(i, i);
    }

    public override void opKShortS_async(AMD_MyClass_opKShortS cb, LinkedList<short> i, Ice.Current current)
    {
        cb.ice_response(i, i);
    }

    public override void opQShortS_async(AMD_MyClass_opQShortS cb, Queue<short> i, Ice.Current current)
    {
        cb.ice_response(i, i);
    }

    public override void opSShortS_async(AMD_MyClass_opSShortS cb, Stack<short> i, Ice.Current current)
    {
        cb.ice_response(i, i);
    }

    public override void opCShortS_async(AMD_MyClass_opCShortS cb, CShortS i, Ice.Current current)
    {
        cb.ice_response(i, i);
    }

    public override void opAIntS_async(AMD_MyClass_opAIntS cb, int[] i, Ice.Current current)
    {
        cb.ice_response(i, i);
    }

    public override void opLIntS_async(AMD_MyClass_opLIntS cb, List<int> i, Ice.Current current)
    {
        cb.ice_response(i, i);
    }

    public override void opKIntS_async(AMD_MyClass_opKIntS cb, LinkedList<int> i, Ice.Current current)
    {
        cb.ice_response(i, i);
    }

    public override void opQIntS_async(AMD_MyClass_opQIntS cb, Queue<int> i, Ice.Current current)
    {
        cb.ice_response(i, i);
    }

    public override void opSIntS_async(AMD_MyClass_opSIntS cb, Stack<int> i, Ice.Current current)
    {
        cb.ice_response(i, i);
    }

    public override void opCIntS_async(AMD_MyClass_opCIntS cb, CIntS i, Ice.Current current)
    {
        cb.ice_response(i, i);
    }

    public override void opALongS_async(AMD_MyClass_opALongS cb, long[] i, Ice.Current current)
    {
        cb.ice_response(i, i);
    }

    public override void opLLongS_async(AMD_MyClass_opLLongS cb, List<long> i, Ice.Current current)
    {
        cb.ice_response(i, i);
    }

    public override void opKLongS_async(AMD_MyClass_opKLongS cb, LinkedList<long> i, Ice.Current current)
    {
        cb.ice_response(i, i);
    }

    public override void opQLongS_async(AMD_MyClass_opQLongS cb, Queue<long> i, Ice.Current current)
    {
        cb.ice_response(i, i);
    }

    public override void opSLongS_async(AMD_MyClass_opSLongS cb, Stack<long> i, Ice.Current current)
    {
        cb.ice_response(i, i);
    }

    public override void opCLongS_async(AMD_MyClass_opCLongS cb, CLongS i, Ice.Current current)
    {
        cb.ice_response(i, i);
    }

    public override void opAFloatS_async(AMD_MyClass_opAFloatS cb, float[] i, Ice.Current current)
    {
        cb.ice_response(i, i);
    }

    public override void opLFloatS_async(AMD_MyClass_opLFloatS cb, List<float> i, Ice.Current current)
    {
        cb.ice_response(i, i);
    }

    public override void opKFloatS_async(AMD_MyClass_opKFloatS cb, LinkedList<float> i, Ice.Current current)
    {
        cb.ice_response(i, i);
    }

    public override void opQFloatS_async(AMD_MyClass_opQFloatS cb, Queue<float> i, Ice.Current current)
    {
        cb.ice_response(i, i);
    }

    public override void opSFloatS_async(AMD_MyClass_opSFloatS cb, Stack<float> i, Ice.Current current)
    {
        cb.ice_response(i, i);
    }

    public override void opCFloatS_async(AMD_MyClass_opCFloatS cb, CFloatS i, Ice.Current current)
    {
        cb.ice_response(i, i);
    }

    public override void opADoubleS_async(AMD_MyClass_opADoubleS cb, double[] i, Ice.Current current)
    {
        cb.ice_response(i, i);
    }

    public override void opLDoubleS_async(AMD_MyClass_opLDoubleS cb, List<double> i, Ice.Current current)
    {
        cb.ice_response(i, i);
    }

    public override void opKDoubleS_async(AMD_MyClass_opKDoubleS cb, LinkedList<double> i, Ice.Current current)
    {
        cb.ice_response(i, i);
    }

    public override void opQDoubleS_async(AMD_MyClass_opQDoubleS cb, Queue<double> i, Ice.Current current)
    {
        cb.ice_response(i, i);
    }

    public override void opSDoubleS_async(AMD_MyClass_opSDoubleS cb, Stack<double> i, Ice.Current current)
    {
        cb.ice_response(i, i);
    }

    public override void opCDoubleS_async(AMD_MyClass_opCDoubleS cb, CDoubleS i, Ice.Current current)
    {
        cb.ice_response(i, i);
    }

    public override void opAStringS_async(AMD_MyClass_opAStringS cb, string[] i, Ice.Current current)
    {
        cb.ice_response(i, i);
    }

    public override void opLStringS_async(AMD_MyClass_opLStringS cb, List<string> i, Ice.Current current)
    {
        cb.ice_response(i, i);
    }

    public override void opKStringS_async(AMD_MyClass_opKStringS cb, LinkedList<string> i, Ice.Current current)
    {
        cb.ice_response(i, i);
    }

    public override void opQStringS_async(AMD_MyClass_opQStringS cb, Queue<string> i, Ice.Current current)
    {
        cb.ice_response(i, i);
    }

    public override void opSStringS_async(AMD_MyClass_opSStringS cb, Stack<string> i, Ice.Current current)
    {
        cb.ice_response(i, i);
    }

    public override void opCStringS_async(AMD_MyClass_opCStringS cb, CStringS i, Ice.Current current)
    {
        cb.ice_response(i, i);
    }

    public override void opAObjectS_async(AMD_MyClass_opAObjectS cb, Ice.Object[] i, Ice.Current current)
    {
        cb.ice_response(i, i);
    }

    public override void opLObjectS_async(AMD_MyClass_opLObjectS cb, List<Ice.Object> i, Ice.Current current)
    {
        cb.ice_response(i, i);
    }

    public override void opCObjectS_async(AMD_MyClass_opCObjectS cb, CObjectS i, Ice.Current current)
    {
        cb.ice_response(i, i);
    }

    public override void opAObjectPrxS_async(AMD_MyClass_opAObjectPrxS cb, Ice.ObjectPrx[] i, Ice.Current current)
    {
        cb.ice_response(i, i);
    }

    public override void opLObjectPrxS_async(AMD_MyClass_opLObjectPrxS cb, List<Ice.ObjectPrx> i, Ice.Current current)
    {
        cb.ice_response(i, i);
    }

    public override void opKObjectPrxS_async(AMD_MyClass_opKObjectPrxS cb, LinkedList<Ice.ObjectPrx> i, Ice.Current current)
    {
        cb.ice_response(i, i);
    }

    public override void opQObjectPrxS_async(AMD_MyClass_opQObjectPrxS cb, Queue<Ice.ObjectPrx> i, Ice.Current current)
    {
        cb.ice_response(i, i);
    }

    public override void opSObjectPrxS_async(AMD_MyClass_opSObjectPrxS cb, Stack<Ice.ObjectPrx> i, Ice.Current current)
    {
        cb.ice_response(i, i);
    }

    public override void opCObjectPrxS_async(AMD_MyClass_opCObjectPrxS cb, CObjectPrxS i, Ice.Current current)
    {
        cb.ice_response(i, i);
    }

    public override void opAStructS_async(AMD_MyClass_opAStructS cb, S[] i, Ice.Current current)
    {
        cb.ice_response(i, i);
    }

    public override void opLStructS_async(AMD_MyClass_opLStructS cb, List<S> i, Ice.Current current)
    {
        cb.ice_response(i, i);
    }

    public override void opKStructS_async(AMD_MyClass_opKStructS cb, LinkedList<S> i, Ice.Current current)
    {
        cb.ice_response(i, i);
    }

    public override void opQStructS_async(AMD_MyClass_opQStructS cb, Queue<S> i, Ice.Current current)
    {
        cb.ice_response(i, i);
    }

    public override void opSStructS_async(AMD_MyClass_opSStructS cb, Stack<S> i, Ice.Current current)
    {
        cb.ice_response(i, i);
    }

    public override void opCStructS_async(AMD_MyClass_opCStructS cb, CStructS i, Ice.Current current)
    {
        cb.ice_response(i, i);
    }

    public override void opAStructSD_async(AMD_MyClass_opAStructSD cb, SD[] i, Ice.Current current)
    {
        cb.ice_response(i, i);
    }

    public override void opLStructSD_async(AMD_MyClass_opLStructSD cb, List<SD> i, Ice.Current current)
    {
        cb.ice_response(i, i);
    }

    public override void opKStructSD_async(AMD_MyClass_opKStructSD cb, LinkedList<SD> i, Ice.Current current)
    {
        cb.ice_response(i, i);
    }

    public override void opQStructSD_async(AMD_MyClass_opQStructSD cb, Queue<SD> i, Ice.Current current)
    {
        cb.ice_response(i, i);
    }

    public override void opSStructSD_async(AMD_MyClass_opSStructSD cb, Stack<SD> i, Ice.Current current)
    {
        cb.ice_response(i, i);
    }

    public override void opCStructSD_async(AMD_MyClass_opCStructSD cb, CStructSD i, Ice.Current current)
    {
        cb.ice_response(i, i);
    }

    public override void opACVS_async(AMD_MyClass_opACVS cb, CV[] i, Ice.Current current)
    {
        cb.ice_response(i, i);
    }

    public override void opLCVS_async(AMD_MyClass_opLCVS cb, List<CV> i, Ice.Current current)
    {
        cb.ice_response(i, i);
    }

    public override void opCCVS_async(AMD_MyClass_opCCVS cb, CCVS i, Ice.Current current)
    {
        cb.ice_response(i, i);
    }

    public override void opACVPrxS_async(AMD_MyClass_opACVPrxS cb, CVPrx[] i, Ice.Current current)
    {
        cb.ice_response(i, i);
    }

    public override void opLCVPrxS_async(AMD_MyClass_opLCVPrxS cb, List<CVPrx> i, Ice.Current current)
    {
        cb.ice_response(i, i);
    }

    public override void opKCVPrxS_async(AMD_MyClass_opKCVPrxS cb, LinkedList<CVPrx> i, Ice.Current current)
    {
        cb.ice_response(i, i);
    }

    public override void opQCVPrxS_async(AMD_MyClass_opQCVPrxS cb, Queue<CVPrx> i, Ice.Current current)
    {
        cb.ice_response(i, i);
    }

    public override void opSCVPrxS_async(AMD_MyClass_opSCVPrxS cb, Stack<CVPrx> i, Ice.Current current)
    {
        cb.ice_response(i, i);
    }

    public override void opCCVPrxS_async(AMD_MyClass_opCCVPrxS cb, CCVPrxS i, Ice.Current current)
    {
        cb.ice_response(i, i);
    }

    public override void opACRS_async(AMD_MyClass_opACRS cb, CR[] i, Ice.Current current)
    {
        cb.ice_response(i, i);
    }

    public override void opLCRS_async(AMD_MyClass_opLCRS cb, List<CR> i, Ice.Current current)
    {
        cb.ice_response(i, i);
    }

    public override void opCCRS_async(AMD_MyClass_opCCRS cb, CCRS i, Ice.Current current)
    {
        cb.ice_response(i, i);
    }

    public override void opAEnS_async(AMD_MyClass_opAEnS cb, En[] i, Ice.Current current)
    {
        cb.ice_response(i, i);
    }

    public override void opLEnS_async(AMD_MyClass_opLEnS cb, List<En> i, Ice.Current current)
    {
        cb.ice_response(i, i);
    }

    public override void opKEnS_async(AMD_MyClass_opKEnS cb, LinkedList<En> i, Ice.Current current)
    {
        cb.ice_response(i, i);
    }

    public override void opQEnS_async(AMD_MyClass_opQEnS cb, Queue<En> i, Ice.Current current)
    {
        cb.ice_response(i, i);
    }

    public override void opSEnS_async(AMD_MyClass_opSEnS cb, Stack<En> i, Ice.Current current)
    {
        cb.ice_response(i, i);
    }

    public override void opCEnS_async(AMD_MyClass_opCEnS cb, CEnS i, Ice.Current current)
    {
        cb.ice_response(i, i);
    }

    public override void opCustomIntS_async(AMD_MyClass_opCustomIntS cb, Custom<int> i, Ice.Current current)
    {
        cb.ice_response(i, i);
    }

    public override void opCustomCVS_async(AMD_MyClass_opCustomCVS cb, Custom<CV> i, Ice.Current current)
    {
        cb.ice_response(i, i);
    }

    public override void opCustomIntSS_async(AMD_MyClass_opCustomIntSS cb, Custom<Custom<int>> i, Ice.Current current)
    {
        cb.ice_response(i, i);
    }

    public override void opCustomCVSS_async(AMD_MyClass_opCustomCVSS cb, Custom<Custom<CV>> i, Ice.Current current)
    {
        cb.ice_response(i, i);
    }

    public override void opSerialSmallCSharp_async(AMD_MyClass_opSerialSmallCSharp cb,
                                                   Serialize.Small i,
                                                   Ice.Current current)
    {
        cb.ice_response(i, i);
    }

    public override void opSerialLargeCSharp_async(AMD_MyClass_opSerialLargeCSharp cb,
                                                   Serialize.Large i,
                                                   Ice.Current current)
    {
        cb.ice_response(i, i);
    }

    public override void opSerialStructCSharp_async(AMD_MyClass_opSerialStructCSharp cb,
                                                    Serialize.Struct i,
                                                    Ice.Current current)
    {
        cb.ice_response(i, i);
    }
}
