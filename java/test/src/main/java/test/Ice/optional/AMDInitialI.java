//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package test.Ice.optional;

import java.util.concurrent.CompletionStage;
import java.util.concurrent.CompletableFuture;

import org.checkerframework.checker.nullness.qual.Nullable;

import com.zeroc.Ice.Current;

import test.Ice.optional.AMD.Test.*;

public final class AMDInitialI implements Initial
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
    public CompletionStage<Void> opOptionalExceptionAsync(@Nullable Integer a, @Nullable String b,
                                                          @Nullable OneOptional o,  Current current)
        throws OptionalException
    {
        OptionalException ex = new OptionalException();
        if(a != null)
        {
            ex.setA(a);
        }
        else
        {
            ex.clearA(); // The member "a" has a default value.
        }
        if(b != null)
        {
            ex.setB(b);
        }
        if(o != null)
        {
            ex.setO(o);
        }
        CompletableFuture<Void> f = new CompletableFuture<>();
        f.completeExceptionally(ex);
        return f;
    }

    @Override
    public CompletionStage<Void> opDerivedExceptionAsync(@Nullable Integer a, @Nullable String b,
                                                         @Nullable OneOptional o, Current current)
        throws OptionalException
    {
        DerivedException ex = new DerivedException();
        if(a != null)
        {
            ex.setA(a);
        }
        else
        {
            ex.clearA(); // The member "a" has a default value.
        }
        if(b != null)
        {
            ex.setB(b);
            ex.setSs(b);
        }
        else
        {
            ex.clearSs(); // The member "ss" has a default value.
        }
        if(o != null)
        {
            ex.setO(o);
            ex.setO2(o);
        }
        CompletableFuture<Void> f = new CompletableFuture<>();
        f.completeExceptionally(ex);
        return f;
    }

    @Override
    public CompletionStage<Void> opRequiredExceptionAsync(@Nullable Integer a, @Nullable String b,
                                                          @Nullable OneOptional o, Current current)
        throws OptionalException
    {
        RequiredException ex = new RequiredException();
        if(a != null)
        {
            ex.setA(a);
        }
        else
        {
            ex.clearA(); // The member "a" has a default value.
        }
        if(b != null)
        {
            ex.setB(b);
            ex.ss = b;
        }
        if(o != null)
        {
            ex.setO(o);
            ex.o2 = o;
        }
        CompletableFuture<Void> f = new CompletableFuture<>();
        f.completeExceptionally(ex);
        return f;
    }

    @Override
    public CompletionStage<Initial.OpByteResult> opByteAsync(@Nullable Byte p1, Current current)
    {
        return CompletableFuture.completedFuture(new Initial.OpByteResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpByteReqResult> opByteReqAsync(@Nullable Byte p1, Current current)
    {
        return CompletableFuture.completedFuture(new Initial.OpByteReqResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpBoolResult> opBoolAsync(@Nullable Boolean p1, Current current)
    {
        return CompletableFuture.completedFuture(new Initial.OpBoolResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpBoolReqResult> opBoolReqAsync(@Nullable Boolean p1, Current current)
    {
        return CompletableFuture.completedFuture(new Initial.OpBoolReqResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpShortResult> opShortAsync(@Nullable Short p1, Current current)
    {
        return CompletableFuture.completedFuture(new Initial.OpShortResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpShortReqResult> opShortReqAsync(@Nullable Short p1, Current current)
    {
        return CompletableFuture.completedFuture(new Initial.OpShortReqResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpIntResult> opIntAsync(@Nullable Integer p1, Current current)
    {
        return CompletableFuture.completedFuture(new Initial.OpIntResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpIntReqResult> opIntReqAsync(@Nullable Integer p1, Current current)
    {
        return CompletableFuture.completedFuture(new Initial.OpIntReqResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpLongResult> opLongAsync(@Nullable Long p1, Current current)
    {
        return CompletableFuture.completedFuture(new Initial.OpLongResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpLongReqResult> opLongReqAsync(@Nullable Long p1, Current current)
    {
        return CompletableFuture.completedFuture(new Initial.OpLongReqResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpFloatResult> opFloatAsync(@Nullable Float p1, Current current)
    {
        return CompletableFuture.completedFuture(new Initial.OpFloatResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpFloatReqResult> opFloatReqAsync(@Nullable Float p1, Current current)
    {
        return CompletableFuture.completedFuture(new Initial.OpFloatReqResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpDoubleResult> opDoubleAsync(@Nullable Double p1, Current current)
    {
        return CompletableFuture.completedFuture(new Initial.OpDoubleResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpDoubleReqResult> opDoubleReqAsync(@Nullable Double p1, Current current)
    {
        return CompletableFuture.completedFuture(new Initial.OpDoubleReqResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpStringResult> opStringAsync(@Nullable String p1, Current current)
    {
        return CompletableFuture.completedFuture(new Initial.OpStringResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpStringReqResult> opStringReqAsync(@Nullable String p1, Current current)
    {
        return CompletableFuture.completedFuture(new Initial.OpStringReqResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpMyEnumResult> opMyEnumAsync(@Nullable MyEnum p1, Current current)
    {
        return CompletableFuture.completedFuture(new Initial.OpMyEnumResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpMyEnumReqResult> opMyEnumReqAsync(@Nullable MyEnum p1, Current current)
    {
        return CompletableFuture.completedFuture(new Initial.OpMyEnumReqResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpSmallStructResult> opSmallStructAsync(@Nullable SmallStruct p1, Current current)
    {
        return CompletableFuture.completedFuture(new Initial.OpSmallStructResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpSmallStructReqResult> opSmallStructReqAsync(@Nullable SmallStruct p1,
                                                                                 Current current)
    {
        return CompletableFuture.completedFuture(new Initial.OpSmallStructReqResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpFixedStructResult> opFixedStructAsync(@Nullable FixedStruct p1, Current current)
    {
        return CompletableFuture.completedFuture(new Initial.OpFixedStructResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpFixedStructReqResult> opFixedStructReqAsync(@Nullable FixedStruct p1,
                                                                                 Current current)
    {
        return CompletableFuture.completedFuture(new Initial.OpFixedStructReqResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpVarStructResult> opVarStructAsync(@Nullable VarStruct p1, Current current)
    {
        return CompletableFuture.completedFuture(new Initial.OpVarStructResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpVarStructReqResult> opVarStructReqAsync(@Nullable VarStruct p1, Current current)
    {
        return CompletableFuture.completedFuture(new Initial.OpVarStructReqResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpOneOptionalResult> opOneOptionalAsync(@Nullable OneOptional p1, Current current)
    {
        return CompletableFuture.completedFuture(new Initial.OpOneOptionalResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpOneOptionalReqResult> opOneOptionalReqAsync(@Nullable OneOptional p1,
                                                                                 Current current)
    {
        return CompletableFuture.completedFuture(new Initial.OpOneOptionalReqResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpOneOptionalProxyResult> opOneOptionalProxyAsync(
        com.zeroc.Ice.@Nullable ObjectPrx p1,
        Current current)
    {
        return CompletableFuture.completedFuture(new Initial.OpOneOptionalProxyResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpOneOptionalProxyReqResult> opOneOptionalProxyReqAsync(
        com.zeroc.Ice.@Nullable ObjectPrx p1,
        Current current)
    {
        return CompletableFuture.completedFuture(new Initial.OpOneOptionalProxyReqResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpByteSeqResult> opByteSeqAsync(byte @Nullable[] p1, Current current)
    {
        return CompletableFuture.completedFuture(new Initial.OpByteSeqResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpByteSeqReqResult> opByteSeqReqAsync(byte @Nullable[] p1, Current current)
    {
        return CompletableFuture.completedFuture(new Initial.OpByteSeqReqResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpBoolSeqResult> opBoolSeqAsync(boolean @Nullable[] p1, Current current)
    {
        return CompletableFuture.completedFuture(new Initial.OpBoolSeqResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpBoolSeqReqResult> opBoolSeqReqAsync(boolean @Nullable[] p1, Current current)
    {
        return CompletableFuture.completedFuture(new Initial.OpBoolSeqReqResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpShortSeqResult> opShortSeqAsync(short @Nullable[] p1, Current current)
    {
        return CompletableFuture.completedFuture(new Initial.OpShortSeqResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpShortSeqReqResult> opShortSeqReqAsync(short @Nullable[] p1, Current current)
    {
        return CompletableFuture.completedFuture(new Initial.OpShortSeqReqResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpIntSeqResult> opIntSeqAsync(int @Nullable[] p1, Current current)
    {
        return CompletableFuture.completedFuture(new Initial.OpIntSeqResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpIntSeqReqResult> opIntSeqReqAsync(int @Nullable[] p1, Current current)
    {
        return CompletableFuture.completedFuture(new Initial.OpIntSeqReqResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpLongSeqResult> opLongSeqAsync(long @Nullable[] p1, Current current)
    {
        return CompletableFuture.completedFuture(new Initial.OpLongSeqResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpLongSeqReqResult> opLongSeqReqAsync(long @Nullable[] p1, Current current)
    {
        return CompletableFuture.completedFuture(new Initial.OpLongSeqReqResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpFloatSeqResult> opFloatSeqAsync(float @Nullable[] p1, Current current)
    {
        return CompletableFuture.completedFuture(new Initial.OpFloatSeqResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpFloatSeqReqResult> opFloatSeqReqAsync(float @Nullable[] p1, Current current)
    {
        return CompletableFuture.completedFuture(new Initial.OpFloatSeqReqResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpDoubleSeqResult> opDoubleSeqAsync(double @Nullable[] p1, Current current)
    {
        return CompletableFuture.completedFuture(new Initial.OpDoubleSeqResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpDoubleSeqReqResult> opDoubleSeqReqAsync(double @Nullable[] p1, Current current)
    {
        return CompletableFuture.completedFuture(new Initial.OpDoubleSeqReqResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpStringSeqResult> opStringSeqAsync(String @Nullable[] p1, Current current)
    {
        return CompletableFuture.completedFuture(new Initial.OpStringSeqResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpStringSeqReqResult> opStringSeqReqAsync(String @Nullable[] p1, Current current)
    {
        return CompletableFuture.completedFuture(new Initial.OpStringSeqReqResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpSmallStructSeqResult> opSmallStructSeqAsync(SmallStruct @Nullable[] p1,
                                                                                 Current current)
    {
        return CompletableFuture.completedFuture(new Initial.OpSmallStructSeqResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpSmallStructSeqReqResult> opSmallStructSeqReqAsync(SmallStruct @Nullable[] p1,
                                                                                       Current current)
    {
        return CompletableFuture.completedFuture(new Initial.OpSmallStructSeqReqResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpSmallStructListResult> opSmallStructListAsync(
        java.util.@Nullable List<SmallStruct> p1,
        Current current)
    {
        return CompletableFuture.completedFuture(new Initial.OpSmallStructListResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpSmallStructListReqResult> opSmallStructListReqAsync(
        java.util.@Nullable List<SmallStruct> p1,
        Current current)
    {
        return CompletableFuture.completedFuture(new Initial.OpSmallStructListReqResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpFixedStructSeqResult> opFixedStructSeqAsync(
        FixedStruct @Nullable[] p1,
        Current current)
    {
        return CompletableFuture.completedFuture(new Initial.OpFixedStructSeqResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpFixedStructSeqReqResult> opFixedStructSeqReqAsync(
        FixedStruct @Nullable[] p1,
        Current current)
    {
        return CompletableFuture.completedFuture(new Initial.OpFixedStructSeqReqResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpFixedStructListResult> opFixedStructListAsync(
        java.util.@Nullable List<FixedStruct> p1,
        Current current)
    {
        return CompletableFuture.completedFuture(new Initial.OpFixedStructListResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpFixedStructListReqResult> opFixedStructListReqAsync(
        java.util.@Nullable List<FixedStruct> p1,
        Current current)
    {
        return CompletableFuture.completedFuture(new Initial.OpFixedStructListReqResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpVarStructSeqResult> opVarStructSeqAsync(VarStruct @Nullable[] p1, Current current)
    {
        return CompletableFuture.completedFuture(new Initial.OpVarStructSeqResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpVarStructSeqReqResult> opVarStructSeqReqAsync(VarStruct @Nullable[] p1,
                                                                                   Current current)
    {
        return CompletableFuture.completedFuture(new Initial.OpVarStructSeqReqResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpSerializableResult> opSerializableAsync(@Nullable SerializableClass p1,
                                                                             Current current)
    {
        return CompletableFuture.completedFuture(new Initial.OpSerializableResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpSerializableReqResult> opSerializableReqAsync(@Nullable SerializableClass p1,
                                                                                   Current current)
    {
        return CompletableFuture.completedFuture(new Initial.OpSerializableReqResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpIntIntDictResult> opIntIntDictAsync(java.util.@Nullable Map<Integer, Integer> p1,
                                                                         Current current)
    {
        return CompletableFuture.completedFuture(new Initial.OpIntIntDictResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpIntIntDictReqResult> opIntIntDictReqAsync(
        java.util.@Nullable Map<Integer, Integer> p1,
        Current current)
    {
        return CompletableFuture.completedFuture(new Initial.OpIntIntDictReqResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpStringIntDictResult> opStringIntDictAsync(
        java.util.@Nullable Map<String, Integer> p1,
        Current current)
    {
        return CompletableFuture.completedFuture(new Initial.OpStringIntDictResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpStringIntDictReqResult> opStringIntDictReqAsync(
        java.util.@Nullable Map<String, Integer> p1,
        Current current)
    {
        return CompletableFuture.completedFuture(new Initial.OpStringIntDictReqResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpIntOneOptionalDictResult> opIntOneOptionalDictAsync(
        java.util.@Nullable Map<Integer, OneOptional> p1,
        Current current)
    {
        return CompletableFuture.completedFuture(new Initial.OpIntOneOptionalDictResult(p1, p1));
    }

    @Override
    public CompletionStage<Initial.OpIntOneOptionalDictReqResult> opIntOneOptionalDictReqAsync(
        java.util.@Nullable Map<Integer, OneOptional> p1,
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
    public CompletionStage<Void> sendOptionalClassAsync(boolean req, @Nullable OneOptional o, Current current)
    {
        return CompletableFuture.completedFuture(null);
    }

    @Override
    public CompletionStage<@Nullable OneOptional> returnOptionalClassAsync(boolean req, Current current)
    {
        return CompletableFuture.completedFuture(new OneOptional(53));
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
            new OpMStruct1MarshaledResult(new SmallStruct(), current));
    }

    @Override
    public CompletionStage<OpMStruct2MarshaledResult> opMStruct2Async(@Nullable SmallStruct p1, Current current)
    {
        return CompletableFuture.completedFuture(new OpMStruct2MarshaledResult(p1, p1, current));
    }

    @Override
    public CompletionStage<OpMSeq1MarshaledResult> opMSeq1Async(Current current)
    {
        return CompletableFuture.completedFuture(new OpMSeq1MarshaledResult(new String[0], current));
    }

    @Override
    public CompletionStage<OpMSeq2MarshaledResult> opMSeq2Async(String @Nullable[] p1, Current current)
    {
        return CompletableFuture.completedFuture(new OpMSeq2MarshaledResult(p1, p1, current));
    }

    @Override
    public CompletionStage<OpMDict1MarshaledResult> opMDict1Async(Current current)
    {
        return CompletableFuture.completedFuture(
            new OpMDict1MarshaledResult(new java.util.HashMap<>(), current));
    }

    @Override
    public CompletionStage<OpMDict2MarshaledResult> opMDict2Async(java.util.@Nullable Map<String, Integer> p1,
                                                                          Current current)
    {
        return CompletableFuture.completedFuture(new OpMDict2MarshaledResult(p1, p1, current));
    }

    @Override
    public CompletionStage<OpMG1MarshaledResult> opMG1Async(Current current)
    {
        return CompletableFuture.completedFuture(new OpMG1MarshaledResult(new G(), current));
    }

    @Override
    public CompletionStage<OpMG2MarshaledResult> opMG2Async(@Nullable G p1, Current current)
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
