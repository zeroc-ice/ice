// **********************************************************************
//
// Copyright (c) 2003-2012 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.optional;

import test.Ice.optional.Test.*;

public final class InitialI extends Initial
{
    public void
    shutdown(Ice.Current current)
    {
        current.adapter.getCommunicator().shutdown();
    }

    public Ice.Object
    pingPong(Ice.Object obj, Ice.Current current)
    {
        return obj;
    }

    public void
    opOptionalException(Ice.Optional<Integer> a, Ice.Optional<String> b, Ice.Optional<OneOptional> o,
                        Ice.Current current)
        throws OptionalException
    {
        OptionalException ex = new OptionalException();
        if(a.isSet())
        {
            ex.setA(a.get());
        }
        else
        {
            ex.clearA(); // The member "a" has a default value.
        }
        if(b.isSet())
        {
            ex.setB(b.get());
        }
        if(o.isSet())
        {
            ex.setO(o.get());
        }
        throw ex;
    }

    public Ice.Optional<Byte>
    opByte(Ice.Optional<Byte> p1, Ice.Optional<Byte> p3, Ice.Current current)
    {
        p3.set(p1);
        return p1;
    }

    public Ice.Optional<Boolean>
    opBool(Ice.Optional<Boolean> p1, Ice.Optional<Boolean> p3, Ice.Current current)
    {
        p3.set(p1);
        return p1;
    }

    public Ice.Optional<Short>
    opShort(Ice.Optional<Short> p1, Ice.Optional<Short> p3, Ice.Current current)
    {
        p3.set(p1);
        return p1;
    }

    public Ice.Optional<Integer>
    opInt(Ice.Optional<Integer> p1, Ice.Optional<Integer> p3, Ice.Current current)
    {
        p3.set(p1);
        return p1;
    }

    public Ice.Optional<Long>
    opLong(Ice.Optional<Long> p1, Ice.Optional<Long> p3, Ice.Current current)
    {
        p3.set(p1);
        return p1;
    }

    public Ice.Optional<Float>
    opFloat(Ice.Optional<Float> p1, Ice.Optional<Float> p3, Ice.Current current)
    {
        p3.set(p1);
        return p1;
    }

    public Ice.Optional<Double>
    opDouble(Ice.Optional<Double> p1, Ice.Optional<Double> p3, Ice.Current current)
    {
        p3.set(p1);
        return p1;
    }

    public Ice.Optional<String>
    opString(Ice.Optional<String> p1, Ice.Optional<String> p3, Ice.Current current)
    {
        p3.set(p1);
        return p1;
    }

    public Ice.Optional<MyEnum>
    opMyEnum(Ice.Optional<MyEnum> p1, Ice.Optional<MyEnum> p3, Ice.Current current)
    {
        p3.set(p1);
        return p1;
    }

    public Ice.Optional<SmallStruct>
    opSmallStruct(Ice.Optional<SmallStruct> p1, Ice.Optional<SmallStruct> p3, Ice.Current current)
    {
        p3.set(p1);
        return p1;
    }

    public Ice.Optional<FixedStruct>
    opFixedStruct(Ice.Optional<FixedStruct> p1, Ice.Optional<FixedStruct> p3, Ice.Current current)
    {
        p3.set(p1);
        return p1;
    }

    public Ice.Optional<VarStruct>
    opVarStruct(Ice.Optional<VarStruct> p1, Ice.Optional<VarStruct> p3, Ice.Current current)
    {
        p3.set(p1);
        return p1;
    }

    public Ice.Optional<OneOptional>
    opOneOptional(Ice.Optional<OneOptional> p1, Ice.Optional<OneOptional> p3, Ice.Current current)
    {
        p3.set(p1);
        return p1;
    }

    public Ice.Optional<OneOptionalPrx>
    opOneOptionalProxy(Ice.Optional<OneOptionalPrx> p1, Ice.Optional<OneOptionalPrx> p3, Ice.Current current)
    {
        p3.set(p1);
        return p1;
    }

    public Ice.Optional<byte[]>
    opByteSeq(Ice.Optional<byte[]> p1, Ice.Optional<byte[]> p3, Ice.Current current)
    {
        p3.set(p1);
        return p1;
    }

    public Ice.Optional<boolean[]>
    opBoolSeq(Ice.Optional<boolean[]> p1, Ice.Optional<boolean[]> p3, Ice.Current current)
    {
        p3.set(p1);
        return p1;
    }

    public Ice.Optional<short[]>
    opShortSeq(Ice.Optional<short[]> p1, Ice.Optional<short[]> p3, Ice.Current current)
    {
        p3.set(p1);
        return p1;
    }

    public Ice.Optional<int[]>
    opIntSeq(Ice.Optional<int[]> p1, Ice.Optional<int[]> p3, Ice.Current current)
    {
        p3.set(p1);
        return p1;
    }

    public Ice.Optional<long[]>
    opLongSeq(Ice.Optional<long[]> p1, Ice.Optional<long[]> p3, Ice.Current current)
    {
        p3.set(p1);
        return p1;
    }

    public Ice.Optional<float[]>
    opFloatSeq(Ice.Optional<float[]> p1, Ice.Optional<float[]> p3, Ice.Current current)
    {
        p3.set(p1);
        return p1;
    }

    public Ice.Optional<double[]>
    opDoubleSeq(Ice.Optional<double[]> p1, Ice.Optional<double[]> p3, Ice.Current current)
    {
        p3.set(p1);
        return p1;
    }

    public Ice.Optional<String[]>
    opStringSeq(Ice.Optional<String[]> p1, Ice.Optional<String[]> p3, Ice.Current current)
    {
        p3.set(p1);
        return p1;
    }

    public Ice.Optional<SmallStruct[]>
    opSmallStructSeq(Ice.Optional<SmallStruct[]> p1, Ice.Optional<SmallStruct[]> p3, Ice.Current current)
    {
        p3.set(p1);
        return p1;
    }

    public Ice.Optional<java.util.List<SmallStruct>>
    opSmallStructList(Ice.Optional<java.util.List<SmallStruct>> p1,
                      Ice.Optional<java.util.List<SmallStruct>> p3, Ice.Current current)
    {
        p3.set(p1);
        return p1;
    }

    public Ice.Optional<FixedStruct[]>
    opFixedStructSeq(Ice.Optional<FixedStruct[]> p1, Ice.Optional<FixedStruct[]> p3, Ice.Current current)
    {
        p3.set(p1);
        return p1;
    }

    public Ice.Optional<java.util.List<FixedStruct>>
    opFixedStructList(Ice.Optional<java.util.List<FixedStruct>> p1,
                      Ice.Optional<java.util.List<FixedStruct>> p3, Ice.Current current)
    {
        p3.set(p1);
        return p1;
    }

    public Ice.Optional<VarStruct[]>
    opVarStructSeq(Ice.Optional<VarStruct[]> p1, Ice.Optional<VarStruct[]> p3, Ice.Current current)
    {
        p3.set(p1);
        return p1;
    }

    public Ice.Optional<SerializableClass>
    opSerializable(Ice.Optional<SerializableClass> p1, Ice.Optional<SerializableClass> p3, Ice.Current current)
    {
        p3.set(p1);
        return p1;
    }

    public Ice.Optional<java.util.Map<Integer, Integer>>
    opIntIntDict(Ice.Optional<java.util.Map<Integer, Integer>> p1,
                 Ice.Optional<java.util.Map<Integer, Integer>> p3, Ice.Current current)
    {
        p3.set(p1);
        return p1;
    }

    public Ice.Optional<java.util.Map<String, Integer>>
    opStringIntDict(Ice.Optional<java.util.Map<String, Integer>> p1,
                    Ice.Optional<java.util.Map<String, Integer>> p3, Ice.Current current)
    {
        p3.set(p1);
        return p1;
    }

    public void
    opClassAndUnknownOptional(A p, Ice.Current current)
    {
    }
}
