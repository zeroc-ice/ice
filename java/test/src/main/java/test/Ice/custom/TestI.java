// Copyright (c) ZeroC, Inc.

package test.Ice.custom;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.Current;

import test.Ice.custom.Test.A;
import test.Ice.custom.Test.E;
import test.Ice.custom.Test.S;
import test.Ice.custom.Test.TestIntf;

import java.nio.ByteBuffer;
import java.nio.DoubleBuffer;
import java.nio.FloatBuffer;
import java.nio.IntBuffer;
import java.nio.LongBuffer;
import java.nio.ShortBuffer;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.Map;
import java.util.Optional;

public final class TestI implements TestIntf {
    public TestI(Communicator communicator) {
        _communicator = communicator;
    }

    @Override
    public TestIntf.OpAArrayResult opAArray(List<A> inSeq, Current current) {
        return new TestIntf.OpAArrayResult(inSeq, inSeq);
    }

    @Override
    public TestIntf.OpAListResult opAList(List<A> inSeq, Current current) {
        return new TestIntf.OpAListResult(inSeq, inSeq);
    }

    @Override
    public TestIntf.OpASeqResult opASeq(A[] inSeq, Current current) {
        TestIntf.OpASeqResult r = new TestIntf.OpASeqResult();
        seq = new ArrayList<>(Arrays.asList(inSeq));
        r.outSeq = new A[seq.size()];
        seq.toArray(r.outSeq);
        r.returnValue = r.outSeq;
        return r;
    }

    @Override
    public TestIntf.OpBoolSeqResult opBoolSeq(List<Boolean> inSeq, Current current) {
        return new TestIntf.OpBoolSeqResult(inSeq, inSeq);
    }

    @Override
    public TestIntf.OpByteSeqResult opByteSeq(List<Byte> inSeq, Current current) {
        return new TestIntf.OpByteSeqResult(inSeq, inSeq);
    }

    @Override
    public TestIntf.OpDSeqResult opDSeq(
            List<Map<Integer, String>> inSeq, Current current) {
        return new TestIntf.OpDSeqResult(inSeq, inSeq);
    }

    @Override
    public TestIntf.OpDoubleSeqResult opDoubleSeq(
            List<Double> inSeq, Current current) {
        return new TestIntf.OpDoubleSeqResult(inSeq, inSeq);
    }

    @Override
    public TestIntf.OpESeqResult opESeq(List<E> inSeq, Current current) {
        return new TestIntf.OpESeqResult(inSeq, inSeq);
    }

    @Override
    public TestIntf.OpFloatSeqResult opFloatSeq(List<Float> inSeq, Current current) {
        return new TestIntf.OpFloatSeqResult(inSeq, inSeq);
    }

    @Override
    public TestIntf.OpIntSeqResult opIntSeq(List<Integer> inSeq, Current current) {
        return new TestIntf.OpIntSeqResult(inSeq, inSeq);
    }

    @Override
    public TestIntf.OpLongSeqResult opLongSeq(List<Long> inSeq, Current current) {
        return new TestIntf.OpLongSeqResult(inSeq, inSeq);
    }

    @Override
    public TestIntf.OpSSeqResult opSSeq(List<S> inSeq, Current current) {
        return new TestIntf.OpSSeqResult(inSeq, inSeq);
    }

    @Override
    public TestIntf.OpShortSeqResult opShortSeq(List<Short> inSeq, Current current) {
        return new TestIntf.OpShortSeqResult(inSeq, inSeq);
    }

    @Override
    public TestIntf.OpStringSeqResult opStringSeq(
            List<String> inSeq, Current current) {
        return new TestIntf.OpStringSeqResult(inSeq, inSeq);
    }

    @Override
    public TestIntf.OpStringSeqSeqResult opStringSeqSeq(
            List<List<String>> inSeq, Current current) {
        return new TestIntf.OpStringSeqSeqResult(inSeq, inSeq);
    }

    @Override
    public TestIntf.OpByteBufferSeqResult opByteBufferSeq(
            ByteBuffer inSeq, Current current) {
        TestIntf.OpByteBufferSeqResult r = new TestIntf.OpByteBufferSeqResult();
        byte[] arr = new byte[inSeq.limit()];
        inSeq.get(arr);
        r.outSeq = ByteBuffer.wrap(arr);
        r.returnValue = ByteBuffer.wrap(arr);
        return r;
    }

    @Override
    public TestIntf.OpShortBufferSeqResult opShortBufferSeq(
            ShortBuffer inSeq, Current current) {
        TestIntf.OpShortBufferSeqResult r = new TestIntf.OpShortBufferSeqResult();
        short[] arr = new short[inSeq.limit()];
        inSeq.get(arr);
        r.outSeq = ShortBuffer.wrap(arr);
        r.returnValue = ShortBuffer.wrap(arr);
        return r;
    }

    @Override
    public TestIntf.OpIntBufferSeqResult opIntBufferSeq(
            IntBuffer inSeq, Current current) {
        TestIntf.OpIntBufferSeqResult r = new TestIntf.OpIntBufferSeqResult();
        int[] arr = new int[inSeq.limit()];
        inSeq.get(arr);
        r.outSeq = IntBuffer.wrap(arr);
        r.returnValue = IntBuffer.wrap(arr);
        return r;
    }

    @Override
    public TestIntf.OpLongBufferSeqResult opLongBufferSeq(
            LongBuffer inSeq, Current current) {
        TestIntf.OpLongBufferSeqResult r = new TestIntf.OpLongBufferSeqResult();
        long[] arr = new long[inSeq.limit()];
        inSeq.get(arr);
        r.outSeq = LongBuffer.wrap(arr);
        r.returnValue = LongBuffer.wrap(arr);
        return r;
    }

    @Override
    public TestIntf.OpFloatBufferSeqResult opFloatBufferSeq(
            FloatBuffer inSeq, Current current) {
        TestIntf.OpFloatBufferSeqResult r = new TestIntf.OpFloatBufferSeqResult();
        float[] arr = new float[inSeq.limit()];
        inSeq.get(arr);
        r.outSeq = FloatBuffer.wrap(arr);
        r.returnValue = FloatBuffer.wrap(arr);
        return r;
    }

    @Override
    public TestIntf.OpDoubleBufferSeqResult opDoubleBufferSeq(
            DoubleBuffer inSeq, Current current) {
        TestIntf.OpDoubleBufferSeqResult r = new TestIntf.OpDoubleBufferSeqResult();
        double[] arr = new double[inSeq.limit()];
        inSeq.get(arr);
        r.outSeq = DoubleBuffer.wrap(arr);
        r.returnValue = DoubleBuffer.wrap(arr);
        return r;
    }

    @Override
    public TestIntf.OpOptAArrayResult opOptAArray(
            Optional<List<A>> inSeq, Current current) {
        return new TestIntf.OpOptAArrayResult(inSeq, inSeq);
    }

    @Override
    public TestIntf.OpOptAListResult opOptAList(
            Optional<List<A>> inSeq, Current current) {
        return new TestIntf.OpOptAListResult(inSeq, inSeq);
    }

    @Override
    public TestIntf.OpOptASeqResult opOptASeq(
            Optional<A[]> inSeq, Current current) {
        return new TestIntf.OpOptASeqResult(inSeq, inSeq);
    }

    @Override
    public TestIntf.OpOptBoolSeqResult opOptBoolSeq(
            Optional<List<Boolean>> inSeq, Current current) {
        return new TestIntf.OpOptBoolSeqResult(inSeq, inSeq);
    }

    @Override
    public TestIntf.OpOptByteSeqResult opOptByteSeq(
            Optional<List<Byte>> inSeq, Current current) {
        return new TestIntf.OpOptByteSeqResult(inSeq, inSeq);
    }

    @Override
    public TestIntf.OpOptDSeqResult opOptDSeq(
            Optional<List<Map<Integer, String>>> inSeq, Current current) {
        return new TestIntf.OpOptDSeqResult(inSeq, inSeq);
    }

    @Override
    public TestIntf.OpOptDoubleSeqResult opOptDoubleSeq(
            Optional<List<Double>> inSeq, Current current) {
        return new TestIntf.OpOptDoubleSeqResult(inSeq, inSeq);
    }

    @Override
    public TestIntf.OpOptESeqResult opOptESeq(
            Optional<List<E>> inSeq, Current current) {
        return new TestIntf.OpOptESeqResult(inSeq, inSeq);
    }

    @Override
    public TestIntf.OpOptFloatSeqResult opOptFloatSeq(
            Optional<List<Float>> inSeq, Current current) {
        return new TestIntf.OpOptFloatSeqResult(inSeq, inSeq);
    }

    @Override
    public TestIntf.OpOptIntSeqResult opOptIntSeq(
            Optional<List<Integer>> inSeq, Current current) {
        return new TestIntf.OpOptIntSeqResult(inSeq, inSeq);
    }

    @Override
    public TestIntf.OpOptLongSeqResult opOptLongSeq(
            Optional<List<Long>> inSeq, Current current) {
        return new TestIntf.OpOptLongSeqResult(inSeq, inSeq);
    }

    @Override
    public TestIntf.OpOptSSeqResult opOptSSeq(
            Optional<List<S>> inSeq, Current current) {
        return new TestIntf.OpOptSSeqResult(inSeq, inSeq);
    }

    @Override
    public TestIntf.OpOptShortSeqResult opOptShortSeq(
            Optional<List<Short>> inSeq, Current current) {
        return new TestIntf.OpOptShortSeqResult(inSeq, inSeq);
    }

    @Override
    public TestIntf.OpOptStringSeqResult opOptStringSeq(
            Optional<List<String>> inSeq, Current current) {
        return new TestIntf.OpOptStringSeqResult(inSeq, inSeq);
    }

    @Override
    public TestIntf.OpOptStringSeqSeqResult opOptStringSeqSeq(
            Optional<List<List<String>>> inSeq, Current current) {
        return new TestIntf.OpOptStringSeqSeqResult(inSeq, inSeq);
    }

    @Override
    public TestIntf.OpOptByteBufferSeqResult opOptByteBufferSeq(
            Optional<ByteBuffer> inSeq, Current current) {
        return new TestIntf.OpOptByteBufferSeqResult(inSeq, inSeq);
    }

    @Override
    public TestIntf.OpOptShortBufferSeqResult opOptShortBufferSeq(
            Optional<ShortBuffer> inSeq, Current current) {
        return new TestIntf.OpOptShortBufferSeqResult(inSeq, inSeq);
    }

    @Override
    public TestIntf.OpOptIntBufferSeqResult opOptIntBufferSeq(
            Optional<IntBuffer> inSeq, Current current) {
        return new TestIntf.OpOptIntBufferSeqResult(inSeq, inSeq);
    }

    @Override
    public TestIntf.OpOptLongBufferSeqResult opOptLongBufferSeq(
            Optional<LongBuffer> inSeq, Current current) {
        return new TestIntf.OpOptLongBufferSeqResult(inSeq, inSeq);
    }

    @Override
    public TestIntf.OpOptFloatBufferSeqResult opOptFloatBufferSeq(
            Optional<FloatBuffer> inSeq, Current current) {
        return new TestIntf.OpOptFloatBufferSeqResult(inSeq, inSeq);
    }

    @Override
    public TestIntf.OpOptDoubleBufferSeqResult opOptDoubleBufferSeq(
            Optional<DoubleBuffer> inSeq, Current current) {
        return new TestIntf.OpOptDoubleBufferSeqResult(inSeq, inSeq);
    }

    @Override
    public void shutdown(Current current) {
        _communicator.shutdown();
    }

    private final Communicator _communicator;
    private ArrayList<A> seq;
}
