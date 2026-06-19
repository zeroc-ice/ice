// Copyright (c) ZeroC, Inc.

package test.Ice.customDictionary;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.Current;

import test.Ice.customDictionary.Test.A;
import test.Ice.customDictionary.Test.C;
import test.Ice.customDictionary.Test.E;
import test.Ice.customDictionary.Test.TestIntf;

import java.util.LinkedHashMap;
import java.util.Map;
import java.util.Optional;

public final class TestI implements TestIntf {
    public TestI(Communicator communicator) {
        _communicator = communicator;
    }

    @Override
    public TestIntf.OpByteBoolLinkedDictResult opByteBoolLinkedDict(
            Map<Byte, Boolean> inDict, Current current) {
        return new TestIntf.OpByteBoolLinkedDictResult(inDict, inDict);
    }

    @Override
    public TestIntf.OpShortIntLinkedDictResult opShortIntLinkedDict(
            Map<Short, Integer> inDict, Current current) {
        return new TestIntf.OpShortIntLinkedDictResult(inDict, inDict);
    }

    @Override
    public TestIntf.OpLongFloatLinkedDictResult opLongFloatLinkedDict(
            LinkedHashMap<Long, Float> inDict, Current current) {
        return new TestIntf.OpLongFloatLinkedDictResult(inDict, inDict);
    }

    @Override
    public TestIntf.OpStringDoubleLinkedDictResult opStringDoubleLinkedDict(
            LinkedHashMap<String, Double> inDict, Current current) {
        return new TestIntf.OpStringDoubleLinkedDictResult(inDict, inDict);
    }

    @Override
    public TestIntf.OpAIntDictResult opAIntDict(
            Map<A, Integer> inDict, Current current) {
        return new TestIntf.OpAIntDictResult(inDict, inDict);
    }

    @Override
    public TestIntf.OpAIntLinkedDictResult opAIntLinkedDict(
            Map<A, Integer> inDict, Current current) {
        return new TestIntf.OpAIntLinkedDictResult(inDict, inDict);
    }

    @Override
    public TestIntf.OpAIntFormalLinkedDictResult opAIntFormalLinkedDict(
            LinkedHashMap<A, Integer> inDict, Current current) {
        return new TestIntf.OpAIntFormalLinkedDictResult(inDict, inDict);
    }

    @Override
    public TestIntf.OpShortCDictResult opShortCDict(
            Map<Short, C> inDict, Current current) {
        return new TestIntf.OpShortCDictResult(inDict, inDict);
    }

    @Override
    public TestIntf.OpShortCLinkedDictResult opShortCLinkedDict(
            Map<Short, C> inDict, Current current) {
        return new TestIntf.OpShortCLinkedDictResult(inDict, inDict);
    }

    @Override
    public TestIntf.OpShortCFormalLinkedDictResult opShortCFormalLinkedDict(
            LinkedHashMap<Short, C> inDict, Current current) {
        return new TestIntf.OpShortCFormalLinkedDictResult(inDict, inDict);
    }

    @Override
    public TestIntf.OpStringEDictResult opStringEDict(
            Map<String, E> inDict, Current current) {
        return new TestIntf.OpStringEDictResult(inDict, inDict);
    }

    @Override
    public TestIntf.OpStringELinkedDictResult opStringELinkedDict(
            Map<String, E> inDict, Current current) {
        return new TestIntf.OpStringELinkedDictResult(inDict, inDict);
    }

    @Override
    public TestIntf.OpStringEFormalLinkedDictResult opStringEFormalLinkedDict(
            LinkedHashMap<String, E> inDict, Current current) {
        return new TestIntf.OpStringEFormalLinkedDictResult(inDict, inDict);
    }

    @Override
    public TestIntf.OpEStringSeqDictResult opEStringSeqDict(
            Map<E, String[]> inDict, Current current) {
        return new TestIntf.OpEStringSeqDictResult(inDict, inDict);
    }

    @Override
    public TestIntf.OpEStringSeqLinkedDictResult opEStringSeqLinkedDict(
            Map<E, String[]> inDict, Current current) {
        return new TestIntf.OpEStringSeqLinkedDictResult(inDict, inDict);
    }

    @Override
    public TestIntf.OpEStringSeqFormalLinkedDictResult opEStringSeqFormalLinkedDict(
            LinkedHashMap<E, String[]> inDict, Current current) {
        return new TestIntf.OpEStringSeqFormalLinkedDictResult(inDict, inDict);
    }

    @Override
    public TestIntf.OpOptByteBoolLinkedDictResult opOptByteBoolLinkedDict(
            Optional<Map<Byte, Boolean>> inDict, Current current) {
        return new TestIntf.OpOptByteBoolLinkedDictResult(inDict, inDict);
    }

    @Override
    public TestIntf.OpOptShortIntLinkedDictResult opOptShortIntLinkedDict(
            Optional<Map<Short, Integer>> inDict, Current current) {
        return new TestIntf.OpOptShortIntLinkedDictResult(inDict, inDict);
    }

    @Override
    public TestIntf.OpOptLongFloatLinkedDictResult opOptLongFloatLinkedDict(
            Optional<LinkedHashMap<Long, Float>> inDict, Current current) {
        return new TestIntf.OpOptLongFloatLinkedDictResult(inDict, inDict);
    }

    @Override
    public TestIntf.OpOptStringDoubleLinkedDictResult opOptStringDoubleLinkedDict(
            Optional<LinkedHashMap<String, Double>> inDict, Current current) {
        return new TestIntf.OpOptStringDoubleLinkedDictResult(inDict, inDict);
    }

    @Override
    public TestIntf.OpOptAIntDictResult opOptAIntDict(
            Optional<Map<A, Integer>> inDict, Current current) {
        return new TestIntf.OpOptAIntDictResult(inDict, inDict);
    }

    @Override
    public TestIntf.OpOptAIntLinkedDictResult opOptAIntLinkedDict(
            Optional<Map<A, Integer>> inDict, Current current) {
        return new TestIntf.OpOptAIntLinkedDictResult(inDict, inDict);
    }

    @Override
    public TestIntf.OpOptAIntFormalLinkedDictResult opOptAIntFormalLinkedDict(
            Optional<LinkedHashMap<A, Integer>> inDict, Current current) {
        return new TestIntf.OpOptAIntFormalLinkedDictResult(inDict, inDict);
    }

    @Override
    public TestIntf.OpOptStringEDictResult opOptStringEDict(
            Optional<Map<String, E>> inDict, Current current) {
        return new TestIntf.OpOptStringEDictResult(inDict, inDict);
    }

    @Override
    public TestIntf.OpOptStringELinkedDictResult opOptStringELinkedDict(
            Optional<Map<String, E>> inDict, Current current) {
        return new TestIntf.OpOptStringELinkedDictResult(inDict, inDict);
    }

    @Override
    public TestIntf.OpOptStringEFormalLinkedDictResult opOptStringEFormalLinkedDict(
            Optional<LinkedHashMap<String, E>> inDict, Current current) {
        return new TestIntf.OpOptStringEFormalLinkedDictResult(inDict, inDict);
    }

    @Override
    public TestIntf.OpOptEStringSeqDictResult opOptEStringSeqDict(
            Optional<Map<E, String[]>> inDict, Current current) {
        return new TestIntf.OpOptEStringSeqDictResult(inDict, inDict);
    }

    @Override
    public TestIntf.OpOptEStringSeqLinkedDictResult opOptEStringSeqLinkedDict(
            Optional<Map<E, String[]>> inDict, Current current) {
        return new TestIntf.OpOptEStringSeqLinkedDictResult(inDict, inDict);
    }

    @Override
    public TestIntf.OpOptEStringSeqFormalLinkedDictResult opOptEStringSeqFormalLinkedDict(
            Optional<LinkedHashMap<E, String[]>> inDict, Current current) {
        return new TestIntf.OpOptEStringSeqFormalLinkedDictResult(inDict, inDict);
    }

    @Override
    public OpMAIntFormalLinkedDictMarshaledResult opMAIntFormalLinkedDict(
            LinkedHashMap<A, Integer> inDict, Current current) {
        return new OpMAIntFormalLinkedDictMarshaledResult(inDict, inDict, current);
    }

    @Override
    public OpMOptEStringSeqFormalLinkedDictMarshaledResult opMOptEStringSeqFormalLinkedDict(
            Optional<LinkedHashMap<E, String[]>> inDict, Current current) {
        return new OpMOptEStringSeqFormalLinkedDictMarshaledResult(inDict, inDict, current);
    }

    @Override
    public void shutdown(Current current) {
        _communicator.shutdown();
    }

    private final Communicator _communicator;
}
