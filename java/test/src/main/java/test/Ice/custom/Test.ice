// Copyright (c) ZeroC, Inc.

#pragma once

#include "Ice/BuiltinSequences.ice"

[["java:package:test.Ice.custom"]]
module Test
{
    struct A
    {
        int i; // We need a field here since structs can't be empty.
    }

    sequence<A> ASeq;
    ["java:type:java.util.ArrayList<A>"] sequence<A> AArray;
    ["java:type:java.util.LinkedList<A>"] sequence<A> AList;

    ["java:type:java.util.ArrayList<Boolean>"] sequence<bool> BoolSeq;
    ["java:type:java.util.ArrayList<Byte>"] sequence<byte> ByteSeq;
    ["java:type:java.util.ArrayList<Short>"] sequence<short> ShortSeq;
    ["java:type:java.util.ArrayList<Integer>"] sequence<int> IntSeq;
    ["java:type:java.util.ArrayList<Long>"] sequence<long> LongSeq;
    ["java:type:java.util.ArrayList<Float>"] sequence<float> FloatSeq;
    ["java:type:java.util.ArrayList<Double>"] sequence<double> DoubleSeq;
    ["java:type:java.util.ArrayList<String>"] sequence<string> StringSeq;

    ["java:buffer"] sequence<byte> ByteBuffer;
    ["java:buffer"] sequence<short> ShortBuffer;
    ["java:buffer"] sequence<int> IntBuffer;
    ["java:buffer"] sequence<long> LongBuffer;
    ["java:buffer"] sequence<float> FloatBuffer;
    ["java:buffer"] sequence<double> DoubleBuffer;

    enum E { E1, E2, E3 }
    ["java:type:java.util.ArrayList<E>"] sequence<E> ESeq;

    struct S
    {
        E en;
    }
    ["java:type:java.util.ArrayList<S>"] sequence<S> SSeq;

    dictionary<int, string> D;
    ["java:type:java.util.ArrayList<java.util.Map<Integer,String>>"] sequence<D> DSeq;

    ["java:type:java.util.LinkedList<java.util.List<String>>"] sequence<StringSeq> StringSeqSeq;

    interface TestIntf
    {
        ASeq opASeq(ASeq inSeq, out ASeq outSeq);
        AArray opAArray(AArray inSeq, out AArray outSeq);
        AList opAList(AList inSeq, out AList outSeq);
        BoolSeq opBoolSeq(BoolSeq inSeq, out BoolSeq outSeq);
        ByteSeq opByteSeq(ByteSeq inSeq, out ByteSeq outSeq);
        ShortSeq opShortSeq(ShortSeq inSeq, out ShortSeq outSeq);
        IntSeq opIntSeq(IntSeq inSeq, out IntSeq outSeq);
        LongSeq opLongSeq(LongSeq inSeq, out LongSeq outSeq);
        FloatSeq opFloatSeq(FloatSeq inSeq, out FloatSeq outSeq);
        DoubleSeq opDoubleSeq(DoubleSeq inSeq, out DoubleSeq outSeq);
        StringSeq opStringSeq(StringSeq inSeq, out StringSeq outSeq);
        ESeq opESeq(ESeq inSeq, out ESeq outSeq);
        SSeq opSSeq(SSeq inSeq, out SSeq outSeq);
        DSeq opDSeq(DSeq inSeq, out DSeq outSeq);
        StringSeqSeq opStringSeqSeq(StringSeqSeq inSeq, out StringSeqSeq outSeq);

        ByteBuffer opByteBufferSeq(ByteBuffer inSeq, out ByteBuffer outSeq);
        ShortBuffer opShortBufferSeq(ShortBuffer inSeq, out ShortBuffer outSeq);
        IntBuffer opIntBufferSeq(IntBuffer inSeq, out IntBuffer outSeq);
        LongBuffer opLongBufferSeq(LongBuffer inSeq, out LongBuffer outSeq);
        FloatBuffer opFloatBufferSeq(FloatBuffer inSeq, out FloatBuffer outSeq);
        DoubleBuffer opDoubleBufferSeq(DoubleBuffer inSeq, out DoubleBuffer outSeq);

        optional(1) ASeq opOptASeq(optional(2) ASeq inSeq, out optional(3) ASeq outSeq);
        optional(1) AArray opOptAArray(optional(2) AArray inSeq, out optional(3) AArray outSeq);
        optional(1) AList opOptAList(optional(2) AList inSeq, out optional(3) AList outSeq);
        optional(1) BoolSeq opOptBoolSeq(optional(2) BoolSeq inSeq, out optional(3) BoolSeq outSeq);
        optional(1) ByteSeq opOptByteSeq(optional(2) ByteSeq inSeq, out optional(3) ByteSeq outSeq);
        optional(1) ShortSeq opOptShortSeq(optional(2) ShortSeq inSeq, out optional(3) ShortSeq outSeq);
        optional(1) IntSeq opOptIntSeq(optional(2) IntSeq inSeq, out optional(3) IntSeq outSeq);
        optional(1) LongSeq opOptLongSeq(optional(2) LongSeq inSeq, out optional(3) LongSeq outSeq);
        optional(1) FloatSeq opOptFloatSeq(optional(2) FloatSeq inSeq, out optional(3) FloatSeq outSeq);
        optional(1) DoubleSeq opOptDoubleSeq(optional(2) DoubleSeq inSeq, out optional(3) DoubleSeq outSeq);
        optional(1) StringSeq opOptStringSeq(optional(2) StringSeq inSeq, out optional(3) StringSeq outSeq);
        optional(1) ESeq opOptESeq(optional(2) ESeq inSeq, out optional(3) ESeq outSeq);
        optional(1) SSeq opOptSSeq(optional(2) SSeq inSeq, out optional(3) SSeq outSeq);
        optional(1) DSeq opOptDSeq(optional(2) DSeq inSeq, out optional(3) DSeq outSeq);
        optional(1) StringSeqSeq opOptStringSeqSeq(optional(2) StringSeqSeq inSeq, out optional(3) StringSeqSeq outSeq);

        optional(1) ByteBuffer opOptByteBufferSeq(optional(2) ByteBuffer inSeq, out optional(3) ByteBuffer outSeq);
        optional(1) ShortBuffer opOptShortBufferSeq(optional(2) ShortBuffer inSeq, out optional(3) ShortBuffer outSeq);
        optional(1) IntBuffer opOptIntBufferSeq(optional(2) IntBuffer inSeq, out optional(3) IntBuffer outSeq);
        optional(1) LongBuffer opOptLongBufferSeq(optional(2) LongBuffer inSeq, out optional(3) LongBuffer outSeq);
        optional(1) FloatBuffer opOptFloatBufferSeq(optional(2) FloatBuffer inSeq, out optional(3) FloatBuffer outSeq);
        optional(1) DoubleBuffer opOptDoubleBufferSeq(optional(2) DoubleBuffer inSeq, out optional(3) DoubleBuffer outSeq);

        void shutdown();
    }
}
