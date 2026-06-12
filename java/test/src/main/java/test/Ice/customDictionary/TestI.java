// Copyright (c) ZeroC, Inc.

package test.Ice.customDictionary;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.Current;

import test.Ice.customDictionary.Test.A;
import test.Ice.customDictionary.Test.E;
import test.Ice.customDictionary.Test.S;
import test.Ice.customDictionary.Test.TestIntf;

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
    public void shutdown(Current current) {
        _communicator.shutdown();
    }

    private final Communicator _communicator;
    private ArrayList<A> seq;
}
