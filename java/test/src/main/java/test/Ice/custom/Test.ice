// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#pragma once

#include <Ice/BuiltinSequences.ice>

[["java:package:test.Ice.custom"]]
module Test
{

class C {};

sequence<C> CSeq;
["java:type:java.util.ArrayList<C>"] sequence<C> CArray;
["java:type:java.util.LinkedList<C>"] sequence<C> CList;

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

enum E { E1, E2, E3 };
["java:type:java.util.ArrayList<E>"] sequence<E> ESeq;

struct S
{
    E en;
};
["java:type:java.util.ArrayList<S>"] sequence<S> SSeq;

dictionary<int, string> D;
["java:type:java.util.ArrayList<java.util.Map<Integer,String>>"] sequence<D> DSeq;

["java:type:java.util.LinkedList<java.util.List<String>>"] sequence<StringSeq> StringSeqSeq;

class TestIntf
{
    CSeq opCSeq(CSeq inSeq, out CSeq outSeq);
    CArray opCArray(CArray inSeq, out CArray outSeq);
    CList opCList(CList inSeq, out CList outSeq);
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

    void shutdown();

    ["java:type:java.util.ArrayList<C>"] CSeq seq;
};

};
