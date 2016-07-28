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

public class InitialI : Test.InitialDisp_
{
    public override void shutdown(Ice.Current current)
    {
        current.adapter.getCommunicator().shutdown();
    }

    public override Ice.Value pingPong(Ice.Value obj, Ice.Current current)
    {
        return obj;
    }

    public override void opOptionalException(Ice.Optional<int> a, Ice.Optional<string> b,
                                             Ice.Optional<Test.OneOptional> o, Ice.Current current)
    {
        throw new Test.OptionalException(false, a, b, o);
    }

    public override void opDerivedException(Ice.Optional<int> a, Ice.Optional<string> b,
                                            Ice.Optional<Test.OneOptional> o, Ice.Current current)
    {
        throw new Test.DerivedException(false, a, b, o, b, o);
    }

    public override void opRequiredException(Ice.Optional<int> a, Ice.Optional<string> b,
                                             Ice.Optional<Test.OneOptional> o, Ice.Current current)
    {
        Test.RequiredException e = new Test.RequiredException();
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

    public override Ice.Optional<byte> opByte(Ice.Optional<byte> p1, out Ice.Optional<byte> p3, Ice.Current current)
    {
        p3 = p1;
        return p1;
    }

    public override Ice.Optional<bool> opBool(Ice.Optional<bool> p1, out Ice.Optional<bool> p3, Ice.Current current)
    {
        p3 = p1;
        return p1;
    }

    public override Ice.Optional<short> opShort(Ice.Optional<short> p1, out Ice.Optional<short> p3, Ice.Current current)
    {
        p3 = p1;
        return p1;
    }

    public override Ice.Optional<int> opInt(Ice.Optional<int> p1, out Ice.Optional<int> p3, Ice.Current current)
    {
        p3 = p1;
        return p1;
    }

    public override Ice.Optional<long> opLong(Ice.Optional<long> p1, out Ice.Optional<long> p3, Ice.Current current)
    {
        p3 = p1;
        return p1;
    }

    public override Ice.Optional<float> opFloat(Ice.Optional<float> p1, out Ice.Optional<float> p3, Ice.Current current)
    {
        p3 = p1;
        return p1;
    }

    public override Ice.Optional<double> opDouble(Ice.Optional<double> p1, out Ice.Optional<double> p3,
                                                  Ice.Current current)
    {
        p3 = p1;
        return p1;
    }

    public override Ice.Optional<string> opString(Ice.Optional<string> p1, out Ice.Optional<string> p3,
                                                  Ice.Current current)
    {
        p3 = p1;
        return p1;
    }

    public override Ice.Optional<Test.MyEnum> opMyEnum(Ice.Optional<Test.MyEnum> p1, out Ice.Optional<Test.MyEnum> p3,
                                                       Ice.Current current)
    {
        p3 = p1;
        return p1;
    }

    public override Ice.Optional<Test.SmallStruct> opSmallStruct(Ice.Optional<Test.SmallStruct> p1,
                                                                 out Ice.Optional<Test.SmallStruct> p3,
                                                                 Ice.Current current)
    {
        p3 = p1;
        return p1;
    }

    public override Ice.Optional<Test.FixedStruct> opFixedStruct(Ice.Optional<Test.FixedStruct> p1,
                                                                 out Ice.Optional<Test.FixedStruct> p3,
                                                                 Ice.Current current)
    {
        p3 = p1;
        return p1;
    }

    public override Ice.Optional<Test.VarStruct> opVarStruct(Ice.Optional<Test.VarStruct> p1,
                                                             out Ice.Optional<Test.VarStruct> p3,
                                                             Ice.Current current)
    {
        p3 = p1;
        return p1;
    }

    public override Ice.Optional<Test.OneOptional> opOneOptional(Ice.Optional<Test.OneOptional> p1,
                                                                 out Ice.Optional<Test.OneOptional> p3,
                                                                 Ice.Current current)
    {
        p3 = p1;
        return p1;
    }

    public override Ice.Optional<Test.OneOptionalPrx> opOneOptionalProxy(Ice.Optional<Test.OneOptionalPrx> p1,
                                                                         out Ice.Optional<Test.OneOptionalPrx> p3,
                                                                         Ice.Current current)
    {
        p3 = p1;
        return p1;
    }

    public override Ice.Optional<byte[]> opByteSeq(Ice.Optional<byte[]> p1, out Ice.Optional<byte[]> p3,
                                                   Ice.Current current)
    {
        p3 = p1;
        return p1;
    }

    public override Ice.Optional<bool[]> opBoolSeq(Ice.Optional<bool[]> p1, out Ice.Optional<bool[]> p3,
                                                   Ice.Current current)
    {
        p3 = p1;
        return p1;
    }

    public override Ice.Optional<short[]> opShortSeq(Ice.Optional<short[]> p1, out Ice.Optional<short[]> p3,
                                                     Ice.Current current)
    {
        p3 = p1;
        return p1;
    }

    public override Ice.Optional<int[]> opIntSeq(Ice.Optional<int[]> p1, out Ice.Optional<int[]> p3,
                                                 Ice.Current current)
    {
        p3 = p1;
        return p1;
    }

    public override Ice.Optional<long[]> opLongSeq(Ice.Optional<long[]> p1, out Ice.Optional<long[]> p3,
                                                   Ice.Current current)
    {
        p3 = p1;
        return p1;
    }

    public override Ice.Optional<float[]> opFloatSeq(Ice.Optional<float[]> p1, out Ice.Optional<float[]> p3,
                                                     Ice.Current current)
    {
        p3 = p1;
        return p1;
    }

    public override Ice.Optional<double[]> opDoubleSeq(Ice.Optional<double[]> p1, out Ice.Optional<double[]> p3,
                                                       Ice.Current current)
    {
        p3 = p1;
        return p1;
    }

    public override Ice.Optional<String[]> opStringSeq(Ice.Optional<String[]> p1, out Ice.Optional<String[]> p3,
                                                       Ice.Current current)
    {
        p3 = p1;
        return p1;
    }

    public override Ice.Optional<Test.SmallStruct[]> opSmallStructSeq(Ice.Optional<Test.SmallStruct[]> p1,
                                                                      out Ice.Optional<Test.SmallStruct[]> p3,
                                                                      Ice.Current current)
    {
        p3 = p1;
        return p1;
    }

    public override Ice.Optional<List<Test.SmallStruct>> opSmallStructList(Ice.Optional<List<Test.SmallStruct>> p1,
                                                                           out Ice.Optional<List<Test.SmallStruct>> p3,
                                                                           Ice.Current current)
    {
        p3 = p1;
        return p1;
    }

    public override Ice.Optional<Test.FixedStruct[]> opFixedStructSeq(Ice.Optional<Test.FixedStruct[]> p1,
                                                                      out Ice.Optional<Test.FixedStruct[]> p3,
                                                                      Ice.Current current)
    {
        p3 = p1;
        return p1;
    }

    public override Ice.Optional<LinkedList<Test.FixedStruct>> opFixedStructList(
        Ice.Optional<LinkedList<Test.FixedStruct>> p1,
        out Ice.Optional<LinkedList<Test.FixedStruct>> p3,
        Ice.Current current)
    {
        p3 = p1;
        return p1;
    }

    public override Ice.Optional<Test.VarStruct[]> opVarStructSeq(Ice.Optional<Test.VarStruct[]> p1,
                                                                  out Ice.Optional<Test.VarStruct[]> p3,
                                                                  Ice.Current current)
    {
        p3 = p1;
        return p1;
    }

    public override Ice.Optional<Test.SerializableClass> opSerializable(Ice.Optional<Test.SerializableClass> p1,
                                                                        out Ice.Optional<Test.SerializableClass> p3,
                                                                        Ice.Current current)
    {
        p3 = p1;
        return p1;
    }

    public override Ice.Optional<Dictionary<int, int>> opIntIntDict(Ice.Optional<Dictionary<int, int>> p1,
                                                                    out Ice.Optional<Dictionary<int, int>> p3,
                                                                    Ice.Current current)
    {
        p3 = p1;
        return p1;
    }

    public override Ice.Optional<Dictionary<string, int>> opStringIntDict(Ice.Optional<Dictionary<string, int>> p1,
                                                                          out Ice.Optional<Dictionary<string, int>> p3,
                                                                          Ice.Current current)
    {
        p3 = p1;
        return p1;
    }

    public override Ice.Optional<Dictionary<int, Test.OneOptional>> opIntOneOptionalDict(
        Ice.Optional<Dictionary<int, Test.OneOptional>> p1,
        out Ice.Optional<Dictionary<int, Test.OneOptional>> p3,
        Ice.Current current)
    {
        p3 = p1;
        return p1;
    }

    public override void opClassAndUnknownOptional(Test.A p, Ice.Current current)
    {
    }

    public override void sendOptionalClass(bool req, Ice.Optional<Test.OneOptional> o, Ice.Current current)
    {
    }

    public override void returnOptionalClass(bool req, out Ice.Optional<Test.OneOptional> o, Ice.Current current)
    {
        o = new Test.OneOptional(53);
    }

    public override Test.G opG(Test.G g, Ice.Current current)
    {
        return g;
    }

    public override void opVoid(Ice.Current current)
    {
    }

    public override bool supportsRequiredParams(Ice.Current current)
    {
        return false;
    }

    public override bool supportsJavaSerializable(Ice.Current current)
    {
        return false;
    }

    public override bool supportsCsharpSerializable(Ice.Current current)
    {
        return true;
    }

    public override bool supportsCppStringView(Ice.Current current)
    {
        return false;
    }
}
