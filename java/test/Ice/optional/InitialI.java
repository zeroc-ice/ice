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
    opOptionalException(Ice.IntOptional a, Ice.Optional<String> b, Ice.Optional<OneOptional> o,
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

    public byte
    opByte(Ice.ByteOptional p1, Ice.ByteHolder p3, Ice.Current current)
    {
        p3.value = p1.get();
        return p1.get();
    }

    public Ice.ByteOptional
    opByteOpt(Ice.ByteOptional p1, Ice.ByteOptional p3, Ice.Current current)
    {
        p3.set(p1);
        return p1;
    }

    public boolean
    opBool(Ice.BooleanOptional p1, Ice.BooleanHolder p3, Ice.Current current)
    {
        p3.value = p1.get();
        return p1.get();
    }

    public Ice.BooleanOptional
    opBoolOpt(Ice.BooleanOptional p1, Ice.BooleanOptional p3, Ice.Current current)
    {
        p3.set(p1);
        return p1;
    }

    public short
    opShort(Ice.ShortOptional p1, Ice.ShortHolder p3, Ice.Current current)
    {
        p3.value = p1.get();
        return p1.get();
    }

    public Ice.ShortOptional
    opShortOpt(Ice.ShortOptional p1, Ice.ShortOptional p3, Ice.Current current)
    {
        p3.set(p1);
        return p1;
    }

    public int
    opInt(Ice.IntOptional p1, Ice.IntHolder p3, Ice.Current current)
    {
        p3.value = p1.get();
        return p1.get();
    }

    public Ice.IntOptional
    opIntOpt(Ice.IntOptional p1, Ice.IntOptional p3, Ice.Current current)
    {
        p3.set(p1);
        return p1;
    }

    public long
    opLong(Ice.LongOptional p1, Ice.LongHolder p3, Ice.Current current)
    {
        p3.value = p1.get();
        return p1.get();
    }

    public Ice.LongOptional
    opLongOpt(Ice.LongOptional p1, Ice.LongOptional p3, Ice.Current current)
    {
        p3.set(p1);
        return p1;
    }

    public float
    opFloat(Ice.FloatOptional p1, Ice.FloatHolder p3, Ice.Current current)
    {
        p3.value = p1.get();
        return p1.get();
    }

    public Ice.FloatOptional
    opFloatOpt(Ice.FloatOptional p1, Ice.FloatOptional p3, Ice.Current current)
    {
        p3.set(p1);
        return p1;
    }

    public double
    opDouble(Ice.DoubleOptional p1, Ice.DoubleHolder p3, Ice.Current current)
    {
        p3.value = p1.get();
        return p1.get();
    }

    public Ice.DoubleOptional
    opDoubleOpt(Ice.DoubleOptional p1, Ice.DoubleOptional p3, Ice.Current current)
    {
        p3.set(p1);
        return p1;
    }

    public String
    opString(Ice.Optional<String> p1, Ice.StringHolder p3, Ice.Current current)
    {
        p3.value = p1.get();
        return p1.get();
    }

    public Ice.Optional<String>
    opStringOpt(Ice.Optional<String> p1, Ice.Optional<String> p3, Ice.Current current)
    {
        p3.set(p1);
        return p1;
    }

    public MyEnum
    opMyEnum(Ice.Optional<MyEnum> p1, MyEnumHolder p3, Ice.Current current)
    {
        p3.value = p1.get();
        return p1.get();
    }

    public Ice.Optional<MyEnum>
    opMyEnumOpt(Ice.Optional<MyEnum> p1, Ice.Optional<MyEnum> p3, Ice.Current current)
    {
        p3.set(p1);
        return p1;
    }

    public SmallStruct
    opSmallStruct(Ice.Optional<SmallStruct> p1, SmallStructHolder p3, Ice.Current current)
    {
        p3.value = p1.get();
        return p1.get();
    }

    public Ice.Optional<SmallStruct>
    opSmallStructOpt(Ice.Optional<SmallStruct> p1, Ice.Optional<SmallStruct> p3, Ice.Current current)
    {
        p3.set(p1);
        return p1;
    }

    public FixedStruct
    opFixedStruct(Ice.Optional<FixedStruct> p1, FixedStructHolder p3, Ice.Current current)
    {
        p3.value = p1.get();
        return p1.get();
    }

    public Ice.Optional<FixedStruct>
    opFixedStructOpt(Ice.Optional<FixedStruct> p1, Ice.Optional<FixedStruct> p3, Ice.Current current)
    {
        p3.set(p1);
        return p1;
    }

    public VarStruct
    opVarStruct(Ice.Optional<VarStruct> p1, VarStructHolder p3, Ice.Current current)
    {
        p3.value = p1.get();
        return p1.get();
    }

    public Ice.Optional<VarStruct>
    opVarStructOpt(Ice.Optional<VarStruct> p1, Ice.Optional<VarStruct> p3, Ice.Current current)
    {
        p3.set(p1);
        return p1;
    }

    public OneOptional
    opOneOptional(Ice.Optional<OneOptional> p1, OneOptionalHolder p3, Ice.Current current)
    {
        p3.value = p1.get();
        return p1.get();
    }

    public Ice.Optional<OneOptional>
    opOneOptionalOpt(Ice.Optional<OneOptional> p1, Ice.Optional<OneOptional> p3, Ice.Current current)
    {
        p3.set(p1);
        return p1;
    }

    public OneOptionalPrx
    opOneOptionalProxy(Ice.Optional<OneOptionalPrx> p1, OneOptionalPrxHolder p3, Ice.Current current)
    {
        p3.value = p1.get();
        return p1.get();
    }

    public Ice.Optional<OneOptionalPrx>
    opOneOptionalProxyOpt(Ice.Optional<OneOptionalPrx> p1, Ice.Optional<OneOptionalPrx> p3, Ice.Current current)
    {
        p3.set(p1);
        return p1;
    }

    public byte[]
    opByteSeq(Ice.Optional<byte[]> p1, ByteSeqHolder p3, Ice.Current current)
    {
        p3.value = p1.get();
        return p1.get();
    }

    public Ice.Optional<byte[]>
    opByteSeqOpt(Ice.Optional<byte[]> p1, Ice.Optional<byte[]> p3, Ice.Current current)
    {
        p3.set(p1);
        return p1;
    }

    public boolean[]
    opBoolSeq(Ice.Optional<boolean[]> p1, BoolSeqHolder p3, Ice.Current current)
    {
        p3.value = p1.get();
        return p1.get();
    }

    public Ice.Optional<boolean[]>
    opBoolSeqOpt(Ice.Optional<boolean[]> p1, Ice.Optional<boolean[]> p3, Ice.Current current)
    {
        p3.set(p1);
        return p1;
    }

    public short[]
    opShortSeq(Ice.Optional<short[]> p1, ShortSeqHolder p3, Ice.Current current)
    {
        p3.value = p1.get();
        return p1.get();
    }

    public Ice.Optional<short[]>
    opShortSeqOpt(Ice.Optional<short[]> p1, Ice.Optional<short[]> p3, Ice.Current current)
    {
        p3.set(p1);
        return p1;
    }

    public int[]
    opIntSeq(Ice.Optional<int[]> p1, IntSeqHolder p3, Ice.Current current)
    {
        p3.value = p1.get();
        return p1.get();
    }

    public Ice.Optional<int[]>
    opIntSeqOpt(Ice.Optional<int[]> p1, Ice.Optional<int[]> p3, Ice.Current current)
    {
        p3.set(p1);
        return p1;
    }

    public long[]
    opLongSeq(Ice.Optional<long[]> p1, LongSeqHolder p3, Ice.Current current)
    {
        p3.value = p1.get();
        return p1.get();
    }

    public Ice.Optional<long[]>
    opLongSeqOpt(Ice.Optional<long[]> p1, Ice.Optional<long[]> p3, Ice.Current current)
    {
        p3.set(p1);
        return p1;
    }

    public float[]
    opFloatSeq(Ice.Optional<float[]> p1, FloatSeqHolder p3, Ice.Current current)
    {
        p3.value = p1.get();
        return p1.get();
    }

    public Ice.Optional<float[]>
    opFloatSeqOpt(Ice.Optional<float[]> p1, Ice.Optional<float[]> p3, Ice.Current current)
    {
        p3.set(p1);
        return p1;
    }

    public double[]
    opDoubleSeq(Ice.Optional<double[]> p1, DoubleSeqHolder p3, Ice.Current current)
    {
        p3.value = p1.get();
        return p1.get();
    }

    public Ice.Optional<double[]>
    opDoubleSeqOpt(Ice.Optional<double[]> p1, Ice.Optional<double[]> p3, Ice.Current current)
    {
        p3.set(p1);
        return p1;
    }

    public String[]
    opStringSeq(Ice.Optional<String[]> p1, StringSeqHolder p3, Ice.Current current)
    {
        p3.value = p1.get();
        return p1.get();
    }

    public Ice.Optional<String[]>
    opStringSeqOpt(Ice.Optional<String[]> p1, Ice.Optional<String[]> p3, Ice.Current current)
    {
        p3.set(p1);
        return p1;
    }

    public SmallStruct[]
    opSmallStructSeq(Ice.Optional<SmallStruct[]> p1, SmallStructSeqHolder p3, Ice.Current current)
    {
        p3.value = p1.get();
        return p1.get();
    }

    public Ice.Optional<SmallStruct[]>
    opSmallStructSeqOpt(Ice.Optional<SmallStruct[]> p1, Ice.Optional<SmallStruct[]> p3, Ice.Current current)
    {
        p3.set(p1);
        return p1;
    }

    public java.util.List<SmallStruct>
    opSmallStructList(Ice.Optional<java.util.List<SmallStruct>> p1, SmallStructListHolder p3, Ice.Current current)
    {
        p3.value = p1.get();
        return p1.get();
    }

    public Ice.Optional<java.util.List<SmallStruct>>
    opSmallStructListOpt(Ice.Optional<java.util.List<SmallStruct>> p1,
                         Ice.Optional<java.util.List<SmallStruct>> p3, Ice.Current current)
    {
        p3.set(p1);
        return p1;
    }

    public FixedStruct[]
    opFixedStructSeq(Ice.Optional<FixedStruct[]> p1, FixedStructSeqHolder p3, Ice.Current current)
    {
        p3.value = p1.get();
        return p1.get();
    }

    public Ice.Optional<FixedStruct[]>
    opFixedStructSeqOpt(Ice.Optional<FixedStruct[]> p1, Ice.Optional<FixedStruct[]> p3, Ice.Current current)
    {
        p3.set(p1);
        return p1;
    }

    public java.util.List<FixedStruct>
    opFixedStructList(Ice.Optional<java.util.List<FixedStruct>> p1, FixedStructListHolder p3, Ice.Current current)
    {
        p3.value = p1.get();
        return p1.get();
    }

    public Ice.Optional<java.util.List<FixedStruct>>
    opFixedStructListOpt(Ice.Optional<java.util.List<FixedStruct>> p1,
                         Ice.Optional<java.util.List<FixedStruct>> p3, Ice.Current current)
    {
        p3.set(p1);
        return p1;
    }

    public VarStruct[]
    opVarStructSeq(Ice.Optional<VarStruct[]> p1, VarStructSeqHolder p3, Ice.Current current)
    {
        p3.value = p1.get();
        return p1.get();
    }

    public Ice.Optional<VarStruct[]>
    opVarStructSeqOpt(Ice.Optional<VarStruct[]> p1, Ice.Optional<VarStruct[]> p3, Ice.Current current)
    {
        p3.set(p1);
        return p1;
    }

    public SerializableClass
    opSerializable(Ice.Optional<SerializableClass> p1, Ice.Holder<SerializableClass> p3, Ice.Current current)
    {
        p3.value = p1.get();
        return p1.get();
    }

    public Ice.Optional<SerializableClass>
    opSerializableOpt(Ice.Optional<SerializableClass> p1, Ice.Optional<SerializableClass> p3, Ice.Current current)
    {
        p3.set(p1);
        return p1;
    }

    public java.util.Map<Integer, Integer>
    opIntIntDict(Ice.Optional<java.util.Map<Integer, Integer>> p1, IntIntDictHolder p3, Ice.Current current)
    {
        p3.value = p1.get();
        return p1.get();
    }

    public Ice.Optional<java.util.Map<Integer, Integer>>
    opIntIntDictOpt(Ice.Optional<java.util.Map<Integer, Integer>> p1,
                    Ice.Optional<java.util.Map<Integer, Integer>> p3, Ice.Current current)
    {
        p3.set(p1);
        return p1;
    }

    public java.util.Map<String, Integer>
    opStringIntDict(Ice.Optional<java.util.Map<String, Integer>> p1, StringIntDictHolder p3, Ice.Current current)
    {
        p3.value = p1.get();
        return p1.get();
    }

    public Ice.Optional<java.util.Map<String, Integer>>
    opStringIntDictOpt(Ice.Optional<java.util.Map<String, Integer>> p1,
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
