//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

#include <Ice/BuiltinSequences.ice>

[["java:package:test.Ice.custom"]]
module Test
{

class C {}

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

    tag(1) CSeq opOptCSeq(tag(2) CSeq inSeq, out tag(3) CSeq outSeq);
    tag(1) CArray opOptCArray(tag(2) CArray inSeq, out tag(3) CArray outSeq);
    tag(1) CList opOptCList(tag(2) CList inSeq, out tag(3) CList outSeq);
    tag(1) BoolSeq opOptBoolSeq(tag(2) BoolSeq inSeq, out tag(3) BoolSeq outSeq);
    tag(1) ByteSeq opOptByteSeq(tag(2) ByteSeq inSeq, out tag(3) ByteSeq outSeq);
    tag(1) ShortSeq opOptShortSeq(tag(2) ShortSeq inSeq, out tag(3) ShortSeq outSeq);
    tag(1) IntSeq opOptIntSeq(tag(2) IntSeq inSeq, out tag(3) IntSeq outSeq);
    tag(1) LongSeq opOptLongSeq(tag(2) LongSeq inSeq, out tag(3) LongSeq outSeq);
    tag(1) FloatSeq opOptFloatSeq(tag(2) FloatSeq inSeq, out tag(3) FloatSeq outSeq);
    tag(1) DoubleSeq opOptDoubleSeq(tag(2) DoubleSeq inSeq, out tag(3) DoubleSeq outSeq);
    tag(1) StringSeq opOptStringSeq(tag(2) StringSeq inSeq, out tag(3) StringSeq outSeq);
    tag(1) ESeq opOptESeq(tag(2) ESeq inSeq, out tag(3) ESeq outSeq);
    tag(1) SSeq opOptSSeq(tag(2) SSeq inSeq, out tag(3) SSeq outSeq);
    tag(1) DSeq opOptDSeq(tag(2) DSeq inSeq, out tag(3) DSeq outSeq);
    tag(1) StringSeqSeq opOptStringSeqSeq(tag(2) StringSeqSeq inSeq, out tag(3) StringSeqSeq outSeq);

    tag(1) ByteBuffer opOptByteBufferSeq(tag(2) ByteBuffer inSeq, out tag(3) ByteBuffer outSeq);
    tag(1) ShortBuffer opOptShortBufferSeq(tag(2) ShortBuffer inSeq, out tag(3) ShortBuffer outSeq);
    tag(1) IntBuffer opOptIntBufferSeq(tag(2) IntBuffer inSeq, out tag(3) IntBuffer outSeq);
    tag(1) LongBuffer opOptLongBufferSeq(tag(2) LongBuffer inSeq, out tag(3) LongBuffer outSeq);
    tag(1) FloatBuffer opOptFloatBufferSeq(tag(2) FloatBuffer inSeq, out tag(3) FloatBuffer outSeq);
    tag(1) DoubleBuffer opOptDoubleBufferSeq(tag(2) DoubleBuffer inSeq, out tag(3) DoubleBuffer outSeq);

    void shutdown();
}

}
