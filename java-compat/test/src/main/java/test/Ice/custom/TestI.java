// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.custom;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.Map;
import java.nio.ByteBuffer;
import java.nio.ShortBuffer;
import java.nio.IntBuffer;
import java.nio.LongBuffer;
import java.nio.FloatBuffer;
import java.nio.DoubleBuffer;

import test.Ice.custom.Test.BoolSeqHolder;
import test.Ice.custom.Test.ByteSeqHolder;
import test.Ice.custom.Test.C;
import test.Ice.custom.Test.CArrayHolder;
import test.Ice.custom.Test.CListHolder;
import test.Ice.custom.Test.CSeqHolder;
import test.Ice.custom.Test.DSeqHolder;
import test.Ice.custom.Test.DoubleSeqHolder;
import test.Ice.custom.Test.E;
import test.Ice.custom.Test.ESeqHolder;
import test.Ice.custom.Test.FloatSeqHolder;
import test.Ice.custom.Test.IntSeqHolder;
import test.Ice.custom.Test.LongSeqHolder;
import test.Ice.custom.Test.S;
import test.Ice.custom.Test.SSeqHolder;
import test.Ice.custom.Test.ShortSeqHolder;
import test.Ice.custom.Test.StringSeqHolder;
import test.Ice.custom.Test.StringSeqSeqHolder;
import test.Ice.custom.Test._TestIntfDisp;

public final class TestI extends _TestIntfDisp
{
    public
    TestI(Ice.Communicator communicator)
    {
        _communicator = communicator;
    }

    @Override
    public List<C>
    opCArray(List<C> inSeq, CArrayHolder outSeq, Ice.Current current)
    {
        outSeq.value = inSeq;
        return inSeq;
    }

    @Override
    public List<C>
    opCList(List<C> inSeq, CListHolder outSeq, Ice.Current current)
    {
        outSeq.value = inSeq;
        return inSeq;
    }

    @Override
    public C[]
    opCSeq(C[] inSeq, CSeqHolder outSeq, Ice.Current current)
    {
        seq = new ArrayList<C>(Arrays.asList(inSeq));
        outSeq.value = new C[seq.size()];
        seq.toArray(outSeq.value);
        return outSeq.value;
    }

    @Override
    public List<Boolean>
    opBoolSeq(List<Boolean> inSeq, BoolSeqHolder outSeq, Ice.Current current)
    {
        outSeq.value = inSeq;
        return inSeq;
    }

    @Override
    public List<Byte>
    opByteSeq(List<Byte> inSeq, ByteSeqHolder outSeq, Ice.Current current)
    {
        outSeq.value = inSeq;
        return inSeq;
    }

    @Override
    public List<Map<Integer,String>>
    opDSeq(List<Map<Integer,String>> inSeq, DSeqHolder outSeq, Ice.Current current)
    {
        outSeq.value = inSeq;
        return inSeq;
    }

    @Override
    public List<Double>
    opDoubleSeq(List<Double> inSeq, DoubleSeqHolder outSeq, Ice.Current current)
    {
        outSeq.value = inSeq;
        return inSeq;
    }

    @Override
    public List<E>
    opESeq(List<E> inSeq, ESeqHolder outSeq, Ice.Current current)
    {
        outSeq.value = inSeq;
        return inSeq;
    }

    @Override
    public List<Float>
    opFloatSeq(List<Float> inSeq, FloatSeqHolder outSeq, Ice.Current current)
    {
        outSeq.value = inSeq;
        return inSeq;
    }

    @Override
    public List<Integer>
    opIntSeq(List<Integer> inSeq, IntSeqHolder outSeq, Ice.Current current)
    {
        outSeq.value = inSeq;
        return inSeq;
    }

    @Override
    public List<Long>
    opLongSeq(List<Long> inSeq, LongSeqHolder outSeq, Ice.Current current)
    {
        outSeq.value = inSeq;
        return inSeq;
    }

    @Override
    public List<S>
    opSSeq(List<S> inSeq, SSeqHolder outSeq, Ice.Current current)
    {
        outSeq.value = inSeq;
        return inSeq;
    }

    @Override
    public List<Short>
    opShortSeq(List<Short> inSeq, ShortSeqHolder outSeq, Ice.Current current)
    {
        outSeq.value = inSeq;
        return inSeq;
    }

    @Override
    public List<String>
    opStringSeq(List<String> inSeq, StringSeqHolder outSeq, Ice.Current current)
    {
        outSeq.value = inSeq;
        return inSeq;
    }

    @Override
    public List<List<String>>
    opStringSeqSeq(List<List<String>> inSeq, StringSeqSeqHolder outSeq, Ice.Current current)
    {
        outSeq.value = inSeq;
        return inSeq;
    }

    @Override
    public ByteBuffer
    opByteBufferSeq(ByteBuffer inSeq, Ice.Holder<ByteBuffer> outSeq, Ice.Current current)
    {
        byte[] arr = new byte[inSeq.limit()];
        inSeq.get(arr);
        outSeq.value = ByteBuffer.wrap(arr);
        return ByteBuffer.wrap(arr);
    }

    @Override
    public ShortBuffer
    opShortBufferSeq(ShortBuffer inSeq, Ice.Holder<ShortBuffer> outSeq, Ice.Current current)
    {
        short[] arr = new short[inSeq.limit()];
        inSeq.get(arr);
        outSeq.value = ShortBuffer.wrap(arr);
        return ShortBuffer.wrap(arr);
    }

    @Override
    public IntBuffer
    opIntBufferSeq(IntBuffer inSeq, Ice.Holder<IntBuffer> outSeq, Ice.Current current)
    {
        int[] arr = new int[inSeq.limit()];
        inSeq.get(arr);
        outSeq.value = IntBuffer.wrap(arr);
        return IntBuffer.wrap(arr);
    }

    @Override
    public LongBuffer
    opLongBufferSeq(LongBuffer inSeq, Ice.Holder<LongBuffer> outSeq, Ice.Current current)
    {
        long[] arr = new long[inSeq.limit()];
        inSeq.get(arr);
        outSeq.value = LongBuffer.wrap(arr);
        return LongBuffer.wrap(arr);
    }

    @Override
    public FloatBuffer
    opFloatBufferSeq(FloatBuffer inSeq, Ice.Holder<FloatBuffer> outSeq, Ice.Current current)
    {
        float[] arr = new float[inSeq.limit()];
        inSeq.get(arr);
        outSeq.value = FloatBuffer.wrap(arr);
        return FloatBuffer.wrap(arr);
    }

    @Override
    public DoubleBuffer
    opDoubleBufferSeq(DoubleBuffer inSeq, Ice.Holder<DoubleBuffer> outSeq, Ice.Current current)
    {
        double[] arr = new double[inSeq.limit()];
        inSeq.get(arr);
        outSeq.value = DoubleBuffer.wrap(arr);
        return DoubleBuffer.wrap(arr);
    }

    @Override
    public void
    shutdown(Ice.Current current)
    {
        _communicator.shutdown();
    }

    private Ice.Communicator _communicator;
    private java.util.ArrayList<C> seq;
}
