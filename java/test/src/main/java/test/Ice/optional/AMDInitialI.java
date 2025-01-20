// Copyright (c) ZeroC, Inc.

package test.Ice.optional;

import com.zeroc.Ice.Current;

import test.Ice.optional.AMD.Test.*;

import java.util.Optional;
import java.util.OptionalDouble;
import java.util.OptionalInt;
import java.util.OptionalLong;
import java.util.concurrent.CompletableFuture;
import java.util.concurrent.CompletionStage;

public final class AMDInitialI implements Initial {
    @Override
    public CompletionStage<Void> shutdownAsync(Current current) {
        current.adapter.getCommunicator().shutdown();
        return CompletableFuture.completedFuture((Void) null);
    }

    @Override
    public CompletionStage<com.zeroc.Ice.Value> pingPongAsync(
            com.zeroc.Ice.Value v, Current current) {
        return CompletableFuture.completedFuture(v);
    }

    @Override
    public CompletionStage<Void> opOptionalExceptionAsync(
            OptionalInt a, Optional<String> b, Current current) throws OptionalException {
        OptionalException ex = new OptionalException();
        if (a.isPresent()) {
            ex.setA(a.getAsInt());
        } else {
            ex.clearA(); // The member "a" has a default value.
        }
        if (b.isPresent()) {
            ex.setB(b.get());
        }
        CompletableFuture<Void> f = new CompletableFuture<>();
        f.completeExceptionally(ex);
        return f;
    }

    @Override
    public CompletionStage<Void> opDerivedExceptionAsync(
            OptionalInt a, Optional<String> b, Current current) throws OptionalException {
        DerivedException ex = new DerivedException();
        if (a.isPresent()) {
            ex.setA(a.getAsInt());
        } else {
            ex.clearA(); // The member "a" has a default value.
        }
        if (b.isPresent()) {
            ex.setB(b.get());
            ex.setSs(b.get());
        } else {
            ex.clearSs(); // The member "ss" has a default value.
        }
        ex.d1 = "d1";
        ex.d2 = "d2";
        CompletableFuture<Void> f = new CompletableFuture<>();
        f.completeExceptionally(ex);
        return f;
    }

    @Override
    public CompletionStage<Void> opRequiredExceptionAsync(
            OptionalInt a, Optional<String> b, Current current) throws OptionalException {
        RequiredException ex = new RequiredException();
        if (a.isPresent()) {
            ex.setA(a.getAsInt());
        } else {
            ex.clearA(); // The member "a" has a default value.
        }
        if (b.isPresent()) {
            ex.setB(b.get());
            ex.ss = b.get();
        }
        CompletableFuture<Void> f = new CompletableFuture<>();
        f.completeExceptionally(ex);
        return f;
    }

    @Override
    public CompletionStage<Initial.OpByteResult> opByteAsync(Optional<Byte> p1, Current current) {
        return CompletableFuture.completedFuture(new Initial.OpByteResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpBoolResult> opBoolAsync(
            Optional<Boolean> p1, Current current) {
        return CompletableFuture.completedFuture(new Initial.OpBoolResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpShortResult> opShortAsync(
            Optional<Short> p1, Current current) {
        return CompletableFuture.completedFuture(new Initial.OpShortResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpIntResult> opIntAsync(OptionalInt p1, Current current) {
        return CompletableFuture.completedFuture(new Initial.OpIntResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpLongResult> opLongAsync(OptionalLong p1, Current current) {
        return CompletableFuture.completedFuture(new Initial.OpLongResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpFloatResult> opFloatAsync(
            Optional<Float> p1, Current current) {
        return CompletableFuture.completedFuture(new Initial.OpFloatResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpDoubleResult> opDoubleAsync(
            OptionalDouble p1, Current current) {
        return CompletableFuture.completedFuture(new Initial.OpDoubleResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpStringResult> opStringAsync(
            Optional<String> p1, Current current) {
        return CompletableFuture.completedFuture(new Initial.OpStringResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpMyEnumResult> opMyEnumAsync(
            Optional<MyEnum> p1, Current current) {
        return CompletableFuture.completedFuture(new Initial.OpMyEnumResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpSmallStructResult> opSmallStructAsync(
            Optional<SmallStruct> p1, Current current) {
        return CompletableFuture.completedFuture(new Initial.OpSmallStructResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpFixedStructResult> opFixedStructAsync(
            Optional<FixedStruct> p1, Current current) {
        return CompletableFuture.completedFuture(new Initial.OpFixedStructResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpVarStructResult> opVarStructAsync(
            Optional<VarStruct> p1, Current current) {
        return CompletableFuture.completedFuture(new Initial.OpVarStructResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpMyInterfaceProxyResult> opMyInterfaceProxyAsync(
            Optional<MyInterfacePrx> p1, Current current) {
        return CompletableFuture.completedFuture(new Initial.OpMyInterfaceProxyResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpOneOptionalResult> opOneOptionalAsync(
            OneOptional p1, Current current) {
        return CompletableFuture.completedFuture(new Initial.OpOneOptionalResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpByteSeqResult> opByteSeqAsync(
            Optional<byte[]> p1, Current current) {
        return CompletableFuture.completedFuture(new Initial.OpByteSeqResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpBoolSeqResult> opBoolSeqAsync(
            Optional<boolean[]> p1, Current current) {
        return CompletableFuture.completedFuture(new Initial.OpBoolSeqResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpShortSeqResult> opShortSeqAsync(
            Optional<short[]> p1, Current current) {
        return CompletableFuture.completedFuture(new Initial.OpShortSeqResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpIntSeqResult> opIntSeqAsync(
            Optional<int[]> p1, Current current) {
        return CompletableFuture.completedFuture(new Initial.OpIntSeqResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpLongSeqResult> opLongSeqAsync(
            Optional<long[]> p1, Current current) {
        return CompletableFuture.completedFuture(new Initial.OpLongSeqResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpFloatSeqResult> opFloatSeqAsync(
            Optional<float[]> p1, Current current) {
        return CompletableFuture.completedFuture(new Initial.OpFloatSeqResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpDoubleSeqResult> opDoubleSeqAsync(
            Optional<double[]> p1, Current current) {
        return CompletableFuture.completedFuture(new Initial.OpDoubleSeqResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpStringSeqResult> opStringSeqAsync(
            Optional<String[]> p1, Current current) {
        return CompletableFuture.completedFuture(new Initial.OpStringSeqResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpSmallStructSeqResult> opSmallStructSeqAsync(
            Optional<SmallStruct[]> p1, Current current) {
        return CompletableFuture.completedFuture(new Initial.OpSmallStructSeqResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpSmallStructListResult> opSmallStructListAsync(
            Optional<java.util.List<SmallStruct>> p1, Current current) {
        return CompletableFuture.completedFuture(new Initial.OpSmallStructListResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpFixedStructSeqResult> opFixedStructSeqAsync(
            Optional<FixedStruct[]> p1, Current current) {
        return CompletableFuture.completedFuture(new Initial.OpFixedStructSeqResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpFixedStructListResult> opFixedStructListAsync(
            Optional<java.util.List<FixedStruct>> p1, Current current) {
        return CompletableFuture.completedFuture(new Initial.OpFixedStructListResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpVarStructSeqResult> opVarStructSeqAsync(
            Optional<VarStruct[]> p1, Current current) {
        return CompletableFuture.completedFuture(new Initial.OpVarStructSeqResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpSerializableResult> opSerializableAsync(
            Optional<SerializableClass> p1, Current current) {
        return CompletableFuture.completedFuture(new Initial.OpSerializableResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpIntIntDictResult> opIntIntDictAsync(
            Optional<java.util.Map<Integer, Integer>> p1, Current current) {
        return CompletableFuture.completedFuture(new Initial.OpIntIntDictResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpStringIntDictResult> opStringIntDictAsync(
            Optional<java.util.Map<String, Integer>> p1, Current current) {
        return CompletableFuture.completedFuture(new Initial.OpStringIntDictResult(p1, p1));
    }

    @Override
    public CompletionStage<Void> opClassAndUnknownOptionalAsync(A p, Current current) {
        return CompletableFuture.completedFuture(null);
    }

    @Override
    public CompletionStage<G> opGAsync(G g, Current current) {
        return CompletableFuture.completedFuture(g);
    }

    @Override
    public CompletionStage<Void> opVoidAsync(Current current) {
        return CompletableFuture.completedFuture(null);
    }

    @Override
    public CompletionStage<OpMStruct1MarshaledResult> opMStruct1Async(Current current) {
        return CompletableFuture.completedFuture(
                new OpMStruct1MarshaledResult(Optional.of(new SmallStruct()), current));
    }

    @Override
    public CompletionStage<OpMStruct2MarshaledResult> opMStruct2Async(
            Optional<SmallStruct> p1, Current current) {
        return CompletableFuture.completedFuture(new OpMStruct2MarshaledResult(p1, p1, current));
    }

    @Override
    public CompletionStage<OpMSeq1MarshaledResult> opMSeq1Async(Current current) {
        return CompletableFuture.completedFuture(
                new OpMSeq1MarshaledResult(Optional.of(new String[0]), current));
    }

    @Override
    public CompletionStage<OpMSeq2MarshaledResult> opMSeq2Async(
            Optional<String[]> p1, Current current) {
        return CompletableFuture.completedFuture(new OpMSeq2MarshaledResult(p1, p1, current));
    }

    @Override
    public CompletionStage<OpMDict1MarshaledResult> opMDict1Async(Current current) {
        return CompletableFuture.completedFuture(
                new OpMDict1MarshaledResult(Optional.of(new java.util.HashMap<>()), current));
    }

    @Override
    public CompletionStage<OpMDict2MarshaledResult> opMDict2Async(
            Optional<java.util.Map<String, Integer>> p1, Current current) {
        return CompletableFuture.completedFuture(new OpMDict2MarshaledResult(p1, p1, current));
    }

    @Override
    public CompletionStage<Boolean> supportsJavaSerializableAsync(Current current) {
        return CompletableFuture.completedFuture(true);
    }
}
