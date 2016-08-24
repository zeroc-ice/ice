// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.optional;

import java.util.concurrent.CompletionStage;
import java.util.concurrent.CompletableFuture;
import java.util.Optional;
import java.util.OptionalDouble;
import java.util.OptionalInt;
import java.util.OptionalLong;

import com.zeroc.Ice.Current;

import test.Ice.optional.AMD.Test.*;

public final class AMDInitialI implements _InitialDisp
{
    @Override
    public CompletionStage<Void> shutdownAsync(Current current)
    {
        current.adapter.getCommunicator().shutdown();
        return CompletableFuture.completedFuture((Void)null);
    }

    @Override
    public CompletionStage<com.zeroc.Ice.Value> pingPongAsync(com.zeroc.Ice.Value v, Current current)
    {
        return CompletableFuture.completedFuture(v);
    }

    @Override
    public CompletionStage<Void> opOptionalExceptionAsync(OptionalInt a, Optional<String> b, Optional<OneOptional> o,
                                                          Current current)
        throws OptionalException
    {
        OptionalException ex = new OptionalException();
        if(a.isPresent())
        {
            ex.setA(a.getAsInt());
        }
        else
        {
            ex.clearA(); // The member "a" has a default value.
        }
        if(b.isPresent())
        {
            ex.setB(b.get());
        }
        if(o.isPresent())
        {
            ex.setO(o.get());
        }
        CompletableFuture<Void> f = new CompletableFuture<>();
        f.completeExceptionally(ex);
        return f;
    }

    @Override
    public CompletionStage<Void> opDerivedExceptionAsync(OptionalInt a, Optional<String> b, Optional<OneOptional> o,
                                                         Current current)
        throws OptionalException
    {
        DerivedException ex = new DerivedException();
        if(a.isPresent())
        {
            ex.setA(a.getAsInt());
        }
        else
        {
            ex.clearA(); // The member "a" has a default value.
        }
        if(b.isPresent())
        {
            ex.setB(b.get());
            ex.setSs(b.get());
        }
        else
        {
            ex.clearSs(); // The member "ss" has a default value.
        }
        if(o.isPresent())
        {
            ex.setO(o.get());
            ex.setO2(o.get());
        }
        CompletableFuture<Void> f = new CompletableFuture<>();
        f.completeExceptionally(ex);
        return f;
    }

    @Override
    public CompletionStage<Void> opRequiredExceptionAsync(OptionalInt a, Optional<String> b, Optional<OneOptional> o,
                                                          Current current)
        throws OptionalException
    {
        RequiredException ex = new RequiredException();
        if(a.isPresent())
        {
            ex.setA(a.getAsInt());
        }
        else
        {
            ex.clearA(); // The member "a" has a default value.
        }
        if(b.isPresent())
        {
            ex.setB(b.get());
            ex.ss = b.get();
        }
        if(o.isPresent())
        {
            ex.setO(o.get());
            ex.o2 = o.get();
        }
        CompletableFuture<Void> f = new CompletableFuture<>();
        f.completeExceptionally(ex);
        return f;
    }

    @Override
    public CompletionStage<Initial.OpByteResult> opByteAsync(Optional<Byte> p1, Current current)
    {
        return CompletableFuture.completedFuture(new Initial.OpByteResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpByteReqResult> opByteReqAsync(Optional<Byte> p1, Current current)
    {
        return CompletableFuture.completedFuture(new Initial.OpByteReqResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpBoolResult> opBoolAsync(Optional<Boolean> p1, Current current)
    {
        return CompletableFuture.completedFuture(new Initial.OpBoolResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpBoolReqResult> opBoolReqAsync(Optional<Boolean> p1, Current current)
    {
        return CompletableFuture.completedFuture(new Initial.OpBoolReqResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpShortResult> opShortAsync(Optional<Short> p1, Current current)
    {
        return CompletableFuture.completedFuture(new Initial.OpShortResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpShortReqResult> opShortReqAsync(Optional<Short> p1, Current current)
    {
        return CompletableFuture.completedFuture(new Initial.OpShortReqResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpIntResult> opIntAsync(OptionalInt p1, Current current)
    {
        return CompletableFuture.completedFuture(new Initial.OpIntResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpIntReqResult> opIntReqAsync(OptionalInt p1, Current current)
    {
        return CompletableFuture.completedFuture(new Initial.OpIntReqResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpLongResult> opLongAsync(OptionalLong p1, Current current)
    {
        return CompletableFuture.completedFuture(new Initial.OpLongResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpLongReqResult> opLongReqAsync(OptionalLong p1, Current current)
    {
        return CompletableFuture.completedFuture(new Initial.OpLongReqResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpFloatResult> opFloatAsync(Optional<Float> p1, Current current)
    {
        return CompletableFuture.completedFuture(new Initial.OpFloatResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpFloatReqResult> opFloatReqAsync(Optional<Float> p1, Current current)
    {
        return CompletableFuture.completedFuture(new Initial.OpFloatReqResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpDoubleResult> opDoubleAsync(OptionalDouble p1, Current current)
    {
        return CompletableFuture.completedFuture(new Initial.OpDoubleResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpDoubleReqResult> opDoubleReqAsync(OptionalDouble p1, Current current)
    {
        return CompletableFuture.completedFuture(new Initial.OpDoubleReqResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpStringResult> opStringAsync(Optional<String> p1, Current current)
    {
        return CompletableFuture.completedFuture(new Initial.OpStringResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpStringReqResult> opStringReqAsync(Optional<String> p1, Current current)
    {
        return CompletableFuture.completedFuture(new Initial.OpStringReqResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpMyEnumResult> opMyEnumAsync(Optional<MyEnum> p1, Current current)
    {
        return CompletableFuture.completedFuture(new Initial.OpMyEnumResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpMyEnumReqResult> opMyEnumReqAsync(Optional<MyEnum> p1, Current current)
    {
        return CompletableFuture.completedFuture(new Initial.OpMyEnumReqResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpSmallStructResult> opSmallStructAsync(Optional<SmallStruct> p1, Current current)
    {
        return CompletableFuture.completedFuture(new Initial.OpSmallStructResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpSmallStructReqResult> opSmallStructReqAsync(Optional<SmallStruct> p1,
                                                                                 Current current)
    {
        return CompletableFuture.completedFuture(new Initial.OpSmallStructReqResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpFixedStructResult> opFixedStructAsync(Optional<FixedStruct> p1, Current current)
    {
        return CompletableFuture.completedFuture(new Initial.OpFixedStructResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpFixedStructReqResult> opFixedStructReqAsync(Optional<FixedStruct> p1,
                                                                                 Current current)
    {
        return CompletableFuture.completedFuture(new Initial.OpFixedStructReqResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpVarStructResult> opVarStructAsync(Optional<VarStruct> p1, Current current)
    {
        return CompletableFuture.completedFuture(new Initial.OpVarStructResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpVarStructReqResult> opVarStructReqAsync(Optional<VarStruct> p1, Current current)
    {
        return CompletableFuture.completedFuture(new Initial.OpVarStructReqResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpOneOptionalResult> opOneOptionalAsync(Optional<OneOptional> p1, Current current)
    {
        return CompletableFuture.completedFuture(new Initial.OpOneOptionalResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpOneOptionalReqResult> opOneOptionalReqAsync(Optional<OneOptional> p1,
                                                                                 Current current)
    {
        return CompletableFuture.completedFuture(new Initial.OpOneOptionalReqResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpOneOptionalProxyResult> opOneOptionalProxyAsync(
        Optional<com.zeroc.Ice.ObjectPrx> p1,
        Current current)
    {
        return CompletableFuture.completedFuture(new Initial.OpOneOptionalProxyResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpOneOptionalProxyReqResult> opOneOptionalProxyReqAsync(
        Optional<com.zeroc.Ice.ObjectPrx> p1,
        Current current)
    {
        return CompletableFuture.completedFuture(new Initial.OpOneOptionalProxyReqResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpByteSeqResult> opByteSeqAsync(Optional<byte[]> p1, Current current)
    {
        return CompletableFuture.completedFuture(new Initial.OpByteSeqResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpByteSeqReqResult> opByteSeqReqAsync(Optional<byte[]> p1, Current current)
    {
        return CompletableFuture.completedFuture(new Initial.OpByteSeqReqResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpBoolSeqResult> opBoolSeqAsync(Optional<boolean[]> p1, Current current)
    {
        return CompletableFuture.completedFuture(new Initial.OpBoolSeqResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpBoolSeqReqResult> opBoolSeqReqAsync(Optional<boolean[]> p1, Current current)
    {
        return CompletableFuture.completedFuture(new Initial.OpBoolSeqReqResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpShortSeqResult> opShortSeqAsync(Optional<short[]> p1, Current current)
    {
        return CompletableFuture.completedFuture(new Initial.OpShortSeqResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpShortSeqReqResult> opShortSeqReqAsync(Optional<short[]> p1, Current current)
    {
        return CompletableFuture.completedFuture(new Initial.OpShortSeqReqResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpIntSeqResult> opIntSeqAsync(Optional<int[]> p1, Current current)
    {
        return CompletableFuture.completedFuture(new Initial.OpIntSeqResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpIntSeqReqResult> opIntSeqReqAsync(Optional<int[]> p1, Current current)
    {
        return CompletableFuture.completedFuture(new Initial.OpIntSeqReqResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpLongSeqResult> opLongSeqAsync(Optional<long[]> p1, Current current)
    {
        return CompletableFuture.completedFuture(new Initial.OpLongSeqResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpLongSeqReqResult> opLongSeqReqAsync(Optional<long[]> p1, Current current)
    {
        return CompletableFuture.completedFuture(new Initial.OpLongSeqReqResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpFloatSeqResult> opFloatSeqAsync(Optional<float[]> p1, Current current)
    {
        return CompletableFuture.completedFuture(new Initial.OpFloatSeqResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpFloatSeqReqResult> opFloatSeqReqAsync(Optional<float[]> p1, Current current)
    {
        return CompletableFuture.completedFuture(new Initial.OpFloatSeqReqResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpDoubleSeqResult> opDoubleSeqAsync(Optional<double[]> p1, Current current)
    {
        return CompletableFuture.completedFuture(new Initial.OpDoubleSeqResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpDoubleSeqReqResult> opDoubleSeqReqAsync(Optional<double[]> p1, Current current)
    {
        return CompletableFuture.completedFuture(new Initial.OpDoubleSeqReqResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpStringSeqResult> opStringSeqAsync(Optional<String[]> p1, Current current)
    {
        return CompletableFuture.completedFuture(new Initial.OpStringSeqResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpStringSeqReqResult> opStringSeqReqAsync(Optional<String[]> p1, Current current)
    {
        return CompletableFuture.completedFuture(new Initial.OpStringSeqReqResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpSmallStructSeqResult> opSmallStructSeqAsync(Optional<SmallStruct[]> p1,
                                                                                 Current current)
    {
        return CompletableFuture.completedFuture(new Initial.OpSmallStructSeqResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpSmallStructSeqReqResult> opSmallStructSeqReqAsync(Optional<SmallStruct[]> p1,
                                                                                       Current current)
    {
        return CompletableFuture.completedFuture(new Initial.OpSmallStructSeqReqResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpSmallStructListResult> opSmallStructListAsync(
        Optional<java.util.List<SmallStruct>> p1,
        Current current)
    {
        return CompletableFuture.completedFuture(new Initial.OpSmallStructListResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpSmallStructListReqResult> opSmallStructListReqAsync(
        Optional<java.util.List<SmallStruct>> p1,
        Current current)
    {
        return CompletableFuture.completedFuture(new Initial.OpSmallStructListReqResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpFixedStructSeqResult> opFixedStructSeqAsync(
        Optional<FixedStruct[]> p1,
        Current current)
    {
        return CompletableFuture.completedFuture(new Initial.OpFixedStructSeqResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpFixedStructSeqReqResult> opFixedStructSeqReqAsync(
        Optional<FixedStruct[]> p1,
        Current current)
    {
        return CompletableFuture.completedFuture(new Initial.OpFixedStructSeqReqResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpFixedStructListResult> opFixedStructListAsync(
        Optional<java.util.List<FixedStruct>> p1,
        Current current)
    {
        return CompletableFuture.completedFuture(new Initial.OpFixedStructListResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpFixedStructListReqResult> opFixedStructListReqAsync(
        Optional<java.util.List<FixedStruct>> p1,
        Current current)
    {
        return CompletableFuture.completedFuture(new Initial.OpFixedStructListReqResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpVarStructSeqResult> opVarStructSeqAsync(Optional<VarStruct[]> p1, Current current)
    {
        return CompletableFuture.completedFuture(new Initial.OpVarStructSeqResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpVarStructSeqReqResult> opVarStructSeqReqAsync(Optional<VarStruct[]> p1,
                                                                                   Current current)
    {
        return CompletableFuture.completedFuture(new Initial.OpVarStructSeqReqResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpSerializableResult> opSerializableAsync(Optional<SerializableClass> p1,
                                                                             Current current)
    {
        return CompletableFuture.completedFuture(new Initial.OpSerializableResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpSerializableReqResult> opSerializableReqAsync(Optional<SerializableClass> p1,
                                                                                   Current current)
    {
        return CompletableFuture.completedFuture(new Initial.OpSerializableReqResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpIntIntDictResult> opIntIntDictAsync(Optional<java.util.Map<Integer, Integer>> p1,
                                                                         Current current)
    {
        return CompletableFuture.completedFuture(new Initial.OpIntIntDictResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpIntIntDictReqResult> opIntIntDictReqAsync(
        Optional<java.util.Map<Integer, Integer>> p1,
        Current current)
    {
        return CompletableFuture.completedFuture(new Initial.OpIntIntDictReqResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpStringIntDictResult> opStringIntDictAsync(
        Optional<java.util.Map<String, Integer>> p1,
        Current current)
    {
        return CompletableFuture.completedFuture(new Initial.OpStringIntDictResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpStringIntDictReqResult> opStringIntDictReqAsync(
        Optional<java.util.Map<String, Integer>> p1,
        Current current)
    {
        return CompletableFuture.completedFuture(new Initial.OpStringIntDictReqResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpIntOneOptionalDictResult> opIntOneOptionalDictAsync(
        Optional<java.util.Map<Integer, OneOptional>> p1,
        Current current)
    {
        return CompletableFuture.completedFuture(new Initial.OpIntOneOptionalDictResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpIntOneOptionalDictReqResult> opIntOneOptionalDictReqAsync(
        Optional<java.util.Map<Integer, OneOptional>> p1,
        Current current)
    {
        return CompletableFuture.completedFuture(new Initial.OpIntOneOptionalDictReqResult(p1, p1));
    }

    @Override
    public CompletionStage<Void> opClassAndUnknownOptionalAsync(A p, Current current)
    {
        return CompletableFuture.completedFuture(null);
    }

    @Override
    public CompletionStage<Void> sendOptionalClassAsync(boolean req, Optional<OneOptional> o, Current current)
    {
        return CompletableFuture.completedFuture(null);
    }

    @Override
    public CompletionStage<Optional<OneOptional>> returnOptionalClassAsync(boolean req, Current current)
    {
        return CompletableFuture.completedFuture(Optional.of(new OneOptional(53)));
    }

    @Override
    public CompletionStage<G> opGAsync(G g, Current current)
    {
        return CompletableFuture.completedFuture(g);
    }

    @Override
    public CompletionStage<Void> opVoidAsync(Current current)
    {
        return CompletableFuture.completedFuture(null);
    }

    @Override
    public CompletionStage<OpMStruct1MarshaledResult> opMStruct1Async(Current current)
    {
        return CompletableFuture.completedFuture(
            new OpMStruct1MarshaledResult(Optional.of(new SmallStruct()), current));
    }

    @Override
    public CompletionStage<OpMStruct2MarshaledResult> opMStruct2Async(Optional<SmallStruct> p1, Current current)
    {
        return CompletableFuture.completedFuture(new OpMStruct2MarshaledResult(p1, p1, current));
    }

    @Override
    public CompletionStage<OpMSeq1MarshaledResult> opMSeq1Async(Current current)
    {
        return CompletableFuture.completedFuture(new OpMSeq1MarshaledResult(Optional.of(new String[0]), current));
    }

    @Override
    public CompletionStage<OpMSeq2MarshaledResult> opMSeq2Async(Optional<String[]> p1, Current current)
    {
        return CompletableFuture.completedFuture(new OpMSeq2MarshaledResult(p1, p1, current));
    }

    @Override
    public CompletionStage<OpMDict1MarshaledResult> opMDict1Async(Current current)
    {
        return CompletableFuture.completedFuture(
            new OpMDict1MarshaledResult(Optional.of(new java.util.HashMap<>()), current));
    }

    @Override
    public CompletionStage<OpMDict2MarshaledResult> opMDict2Async(Optional<java.util.Map<String, Integer>> p1,
                                                                          Current current)
    {
        return CompletableFuture.completedFuture(new OpMDict2MarshaledResult(p1, p1, current));
    }

    @Override
    public CompletionStage<OpMG1MarshaledResult> opMG1Async(Current current)
    {
        return CompletableFuture.completedFuture(new OpMG1MarshaledResult(Optional.of(new G()), current));
    }

    @Override
    public CompletionStage<OpMG2MarshaledResult> opMG2Async(Optional<G> p1, Current current)
    {
        return CompletableFuture.completedFuture(new OpMG2MarshaledResult(p1, p1, current));
    }

    @Override
    public CompletionStage<Boolean> supportsRequiredParamsAsync(Current current)
    {
        return CompletableFuture.completedFuture(true);
    }

    @Override
    public CompletionStage<Boolean> supportsJavaSerializableAsync(Current current)
    {
        return CompletableFuture.completedFuture(true);
    }

    @Override
    public CompletionStage<Boolean> supportsCsharpSerializableAsync(Current current)
    {
        return CompletableFuture.completedFuture(false);
    }

    @Override
    public CompletionStage<Boolean> supportsCppStringViewAsync(Current current)
    {
        return CompletableFuture.completedFuture(false);
    }
}
