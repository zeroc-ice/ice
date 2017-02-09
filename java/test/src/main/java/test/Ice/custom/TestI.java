// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
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

import test.Ice.custom.Test.C;
import test.Ice.custom.Test.E;
import test.Ice.custom.Test.S;
import test.Ice.custom.Test.TestIntf;
import test.Ice.custom.Test._TestIntfDisp;

public final class TestI implements _TestIntfDisp
{
    public TestI(com.zeroc.Ice.Communicator communicator)
    {
        _communicator = communicator;
    }

    @Override
    public TestIntf.OpCArrayResult opCArray(List<C> inSeq, com.zeroc.Ice.Current current)
    {
        return new TestIntf.OpCArrayResult(inSeq, inSeq);
    }

    @Override
    public TestIntf.OpCListResult opCList(List<C> inSeq, com.zeroc.Ice.Current current)
    {
        return new TestIntf.OpCListResult(inSeq, inSeq);
    }

    @Override
    public TestIntf.OpCSeqResult opCSeq(C[] inSeq, com.zeroc.Ice.Current current)
    {
        TestIntf.OpCSeqResult r = new TestIntf.OpCSeqResult();
        seq = new ArrayList<>(Arrays.asList(inSeq));
        r.outSeq = new C[seq.size()];
        seq.toArray(r.outSeq);
        r.returnValue = r.outSeq;
        return r;
    }

    @Override
    public TestIntf.OpBoolSeqResult opBoolSeq(List<Boolean> inSeq, com.zeroc.Ice.Current current)
    {
        return new TestIntf.OpBoolSeqResult(inSeq, inSeq);
    }

    @Override
    public TestIntf.OpByteSeqResult opByteSeq(List<Byte> inSeq, com.zeroc.Ice.Current current)
    {
        return new TestIntf.OpByteSeqResult(inSeq, inSeq);
    }

    @Override
    public TestIntf.OpDSeqResult opDSeq(List<Map<Integer,String>> inSeq, com.zeroc.Ice.Current current)
    {
        return new TestIntf.OpDSeqResult(inSeq, inSeq);
    }

    @Override
    public TestIntf.OpDoubleSeqResult opDoubleSeq(List<Double> inSeq, com.zeroc.Ice.Current current)
    {
        return new TestIntf.OpDoubleSeqResult(inSeq, inSeq);
    }

    @Override
    public TestIntf.OpESeqResult opESeq(List<E> inSeq, com.zeroc.Ice.Current current)
    {
        return new TestIntf.OpESeqResult(inSeq, inSeq);
    }

    @Override
    public TestIntf.OpFloatSeqResult opFloatSeq(List<Float> inSeq, com.zeroc.Ice.Current current)
    {
        return new TestIntf.OpFloatSeqResult(inSeq, inSeq);
    }

    @Override
    public TestIntf.OpIntSeqResult opIntSeq(List<Integer> inSeq, com.zeroc.Ice.Current current)
    {
        return new TestIntf.OpIntSeqResult(inSeq, inSeq);
    }

    @Override
    public TestIntf.OpLongSeqResult opLongSeq(List<Long> inSeq, com.zeroc.Ice.Current current)
    {
        return new TestIntf.OpLongSeqResult(inSeq, inSeq);
    }

    @Override
    public TestIntf.OpSSeqResult opSSeq(List<S> inSeq, com.zeroc.Ice.Current current)
    {
        return new TestIntf.OpSSeqResult(inSeq, inSeq);
    }

    @Override
    public TestIntf.OpShortSeqResult opShortSeq(List<Short> inSeq, com.zeroc.Ice.Current current)
    {
        return new TestIntf.OpShortSeqResult(inSeq, inSeq);
    }

    @Override
    public TestIntf.OpStringSeqResult opStringSeq(List<String> inSeq, com.zeroc.Ice.Current current)
    {
        return new TestIntf.OpStringSeqResult(inSeq, inSeq);
    }

    @Override
    public TestIntf.OpStringSeqSeqResult opStringSeqSeq(List<List<String>> inSeq, com.zeroc.Ice.Current current)
    {
        return new TestIntf.OpStringSeqSeqResult(inSeq, inSeq);
    }

    @Override
    public TestIntf.OpByteBufferSeqResult opByteBufferSeq(ByteBuffer inSeq, com.zeroc.Ice.Current current)
    {
        TestIntf.OpByteBufferSeqResult r = new TestIntf.OpByteBufferSeqResult();
        byte[] arr = new byte[inSeq.limit()];
        inSeq.get(arr);
        r.outSeq = ByteBuffer.wrap(arr);
        r.returnValue = ByteBuffer.wrap(arr);
        return r;
    }

    @Override
    public TestIntf.OpShortBufferSeqResult opShortBufferSeq(ShortBuffer inSeq, com.zeroc.Ice.Current current)
    {
        TestIntf.OpShortBufferSeqResult r = new TestIntf.OpShortBufferSeqResult();
        short[] arr = new short[inSeq.limit()];
        inSeq.get(arr);
        r.outSeq = ShortBuffer.wrap(arr);
        r.returnValue = ShortBuffer.wrap(arr);
        return r;
    }

    @Override
    public TestIntf.OpIntBufferSeqResult opIntBufferSeq(IntBuffer inSeq, com.zeroc.Ice.Current current)
    {
        TestIntf.OpIntBufferSeqResult r = new TestIntf.OpIntBufferSeqResult();
        int[] arr = new int[inSeq.limit()];
        inSeq.get(arr);
        r.outSeq = IntBuffer.wrap(arr);
        r.returnValue = IntBuffer.wrap(arr);
        return r;
    }

    @Override
    public TestIntf.OpLongBufferSeqResult opLongBufferSeq(LongBuffer inSeq, com.zeroc.Ice.Current current)
    {
        TestIntf.OpLongBufferSeqResult r = new TestIntf.OpLongBufferSeqResult();
        long[] arr = new long[inSeq.limit()];
        inSeq.get(arr);
        r.outSeq = LongBuffer.wrap(arr);
        r.returnValue = LongBuffer.wrap(arr);
        return r;
    }

    @Override
    public TestIntf.OpFloatBufferSeqResult opFloatBufferSeq(FloatBuffer inSeq, com.zeroc.Ice.Current current)
    {
        TestIntf.OpFloatBufferSeqResult r = new TestIntf.OpFloatBufferSeqResult();
        float[] arr = new float[inSeq.limit()];
        inSeq.get(arr);
        r.outSeq = FloatBuffer.wrap(arr);
        r.returnValue = FloatBuffer.wrap(arr);
        return r;
    }

    @Override
    public TestIntf.OpDoubleBufferSeqResult opDoubleBufferSeq(DoubleBuffer inSeq, com.zeroc.Ice.Current current)
    {
        TestIntf.OpDoubleBufferSeqResult r = new TestIntf.OpDoubleBufferSeqResult();
        double[] arr = new double[inSeq.limit()];
        inSeq.get(arr);
        r.outSeq = DoubleBuffer.wrap(arr);
        r.returnValue = DoubleBuffer.wrap(arr);
        return r;
    }

    @Override
    public TestIntf.OpOptCArrayResult opOptCArray(java.util.Optional<List<C>> inSeq, com.zeroc.Ice.Current current)
    {
        return new TestIntf.OpOptCArrayResult(inSeq, inSeq);
    }

    @Override
    public TestIntf.OpOptCListResult opOptCList(java.util.Optional<List<C>> inSeq, com.zeroc.Ice.Current current)
    {
        return new TestIntf.OpOptCListResult(inSeq, inSeq);
    }

    @Override
    public TestIntf.OpOptCSeqResult opOptCSeq(java.util.Optional<C[]> inSeq, com.zeroc.Ice.Current current)
    {
        return new TestIntf.OpOptCSeqResult(inSeq, inSeq);
    }

    @Override
    public TestIntf.OpOptBoolSeqResult opOptBoolSeq(java.util.Optional<List<Boolean>> inSeq, com.zeroc.Ice.Current current)
    {
        return new TestIntf.OpOptBoolSeqResult(inSeq, inSeq);
    }

    @Override
    public TestIntf.OpOptByteSeqResult opOptByteSeq(java.util.Optional<List<Byte>> inSeq, com.zeroc.Ice.Current current)
    {
        return new TestIntf.OpOptByteSeqResult(inSeq, inSeq);
    }

    @Override
    public TestIntf.OpOptDSeqResult opOptDSeq(java.util.Optional<List<Map<Integer,String>>> inSeq, com.zeroc.Ice.Current current)
    {
        return new TestIntf.OpOptDSeqResult(inSeq, inSeq);
    }

    @Override
    public TestIntf.OpOptDoubleSeqResult opOptDoubleSeq(java.util.Optional<List<Double>> inSeq, com.zeroc.Ice.Current current)
    {
        return new TestIntf.OpOptDoubleSeqResult(inSeq, inSeq);
    }

    @Override
    public TestIntf.OpOptESeqResult opOptESeq(java.util.Optional<List<E>> inSeq, com.zeroc.Ice.Current current)
    {
        return new TestIntf.OpOptESeqResult(inSeq, inSeq);
    }

    @Override
    public TestIntf.OpOptFloatSeqResult opOptFloatSeq(java.util.Optional<List<Float>> inSeq, com.zeroc.Ice.Current current)
    {
        return new TestIntf.OpOptFloatSeqResult(inSeq, inSeq);
    }

    @Override
    public TestIntf.OpOptIntSeqResult opOptIntSeq(java.util.Optional<List<Integer>> inSeq, com.zeroc.Ice.Current current)
    {
        return new TestIntf.OpOptIntSeqResult(inSeq, inSeq);
    }

    @Override
    public TestIntf.OpOptLongSeqResult opOptLongSeq(java.util.Optional<List<Long>> inSeq, com.zeroc.Ice.Current current)
    {
        return new TestIntf.OpOptLongSeqResult(inSeq, inSeq);
    }

    @Override
    public TestIntf.OpOptSSeqResult opOptSSeq(java.util.Optional<List<S>> inSeq, com.zeroc.Ice.Current current)
    {
        return new TestIntf.OpOptSSeqResult(inSeq, inSeq);
    }

    @Override
    public TestIntf.OpOptShortSeqResult opOptShortSeq(java.util.Optional<List<Short>> inSeq, com.zeroc.Ice.Current current)
    {
        return new TestIntf.OpOptShortSeqResult(inSeq, inSeq);
    }

    @Override
    public TestIntf.OpOptStringSeqResult opOptStringSeq(java.util.Optional<List<String>> inSeq, com.zeroc.Ice.Current current)
    {
        return new TestIntf.OpOptStringSeqResult(inSeq, inSeq);
    }

    @Override
    public TestIntf.OpOptStringSeqSeqResult opOptStringSeqSeq(java.util.Optional<List<List<String>>> inSeq, com.zeroc.Ice.Current current)
    {
        return new TestIntf.OpOptStringSeqSeqResult(inSeq, inSeq);
    }

    @Override
    public TestIntf.OpOptByteBufferSeqResult opOptByteBufferSeq(java.util.Optional<ByteBuffer> inSeq, com.zeroc.Ice.Current current)
    {
        return new TestIntf.OpOptByteBufferSeqResult(inSeq, inSeq);
    }

    @Override
    public TestIntf.OpOptShortBufferSeqResult opOptShortBufferSeq(java.util.Optional<ShortBuffer> inSeq, com.zeroc.Ice.Current current)
    {
        return new TestIntf.OpOptShortBufferSeqResult(inSeq, inSeq);
    }

    @Override
    public TestIntf.OpOptIntBufferSeqResult opOptIntBufferSeq(java.util.Optional<IntBuffer> inSeq, com.zeroc.Ice.Current current)
    {
        return new TestIntf.OpOptIntBufferSeqResult(inSeq, inSeq);
    }

    @Override
    public TestIntf.OpOptLongBufferSeqResult opOptLongBufferSeq(java.util.Optional<LongBuffer> inSeq, com.zeroc.Ice.Current current)
    {
        return new TestIntf.OpOptLongBufferSeqResult(inSeq, inSeq);
    }

    @Override
    public TestIntf.OpOptFloatBufferSeqResult opOptFloatBufferSeq(java.util.Optional<FloatBuffer> inSeq, com.zeroc.Ice.Current current)
    {
        return new TestIntf.OpOptFloatBufferSeqResult(inSeq, inSeq);
    }

    @Override
    public TestIntf.OpOptDoubleBufferSeqResult opOptDoubleBufferSeq(java.util.Optional<DoubleBuffer> inSeq, com.zeroc.Ice.Current current)
    {
        return new TestIntf.OpOptDoubleBufferSeqResult(inSeq, inSeq);
    }

    @Override
    public void shutdown(com.zeroc.Ice.Current current)
    {
        _communicator.shutdown();
    }

    private com.zeroc.Ice.Communicator _communicator;
    private java.util.ArrayList<C> seq;
}
