//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package test.Ice.optional;

import org.checkerframework.checker.nullness.qual.Nullable;

import com.zeroc.Ice.Current;

import test.Ice.optional.Test.*;

public final class InitialI implements Initial
{
    @Override
    public void shutdown(Current current)
    {
        current.adapter.getCommunicator().shutdown();
    }

    @Override
    public com.zeroc.Ice.Value pingPong(com.zeroc.Ice.Value obj, Current current)
    {
        return obj;
    }

    @Override
    public void opOptionalException(@Nullable Integer a, @Nullable String b, @Nullable OneOptional o, Current current)
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
        throw ex;
    }

    @Override
    public void opDerivedException(@Nullable Integer a, @Nullable String b, @Nullable OneOptional o, Current current)
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
        throw ex;
    }

    @Override
    public void opRequiredException(@Nullable Integer a, @Nullable String b, @Nullable OneOptional o, Current current)
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
        throw ex;
    }

    @Override
    public Initial.OpByteResult opByte(@Nullable Byte p1, Current current)
    {
        return new Initial.OpByteResult(p1, p1);
    }

    @Override
    public Initial.OpByteReqResult opByteReq(@Nullable Byte p1, Current current)
    {
        return new Initial.OpByteReqResult(p1, p1);
    }

    @Override
    public Initial.OpBoolResult opBool(@Nullable Boolean p1, Current current)
    {
        return new Initial.OpBoolResult(p1, p1);
    }

    @Override
    public Initial.OpBoolReqResult opBoolReq(@Nullable Boolean p1, Current current)
    {
        return new Initial.OpBoolReqResult(p1, p1);
    }

    @Override
    public Initial.OpShortResult opShort(@Nullable Short p1, Current current)
    {
        return new Initial.OpShortResult(p1, p1);
    }

    @Override
    public Initial.OpShortReqResult opShortReq(@Nullable Short p1, Current current)
    {
        return new Initial.OpShortReqResult(p1, p1);
    }

    @Override
    public Initial.OpIntResult opInt(@Nullable Integer p1, Current current)
    {
        return new Initial.OpIntResult(p1, p1);
    }

    @Override
    public Initial.OpIntReqResult opIntReq(@Nullable Integer p1, Current current)
    {
        return new Initial.OpIntReqResult(p1, p1);
    }

    @Override
    public Initial.OpLongResult opLong(@Nullable Long p1, Current current)
    {
        return new Initial.OpLongResult(p1, p1);
    }

    @Override
    public Initial.OpLongReqResult opLongReq(@Nullable Long p1, Current current)
    {
        return new Initial.OpLongReqResult(p1, p1);
    }

    @Override
    public Initial.OpFloatResult opFloat(@Nullable Float p1, Current current)
    {
        return new Initial.OpFloatResult(p1, p1);
    }

    @Override
    public Initial.OpFloatReqResult opFloatReq(@Nullable Float p1, Current current)
    {
        return new Initial.OpFloatReqResult(p1, p1);
    }

    @Override
    public Initial.OpDoubleResult opDouble(@Nullable Double p1, Current current)
    {
        return new Initial.OpDoubleResult(p1, p1);
    }

    @Override
    public Initial.OpDoubleReqResult opDoubleReq(@Nullable Double p1, Current current)
    {
        return new Initial.OpDoubleReqResult(p1, p1);
    }

    @Override
    public Initial.OpStringResult opString(@Nullable String p1, Current current)
    {
        return new Initial.OpStringResult(p1, p1);
    }

    @Override
    public Initial.OpStringReqResult opStringReq(@Nullable String p1, Current current)
    {
        return new Initial.OpStringReqResult(p1, p1);
    }

    @Override
    public Initial.OpMyEnumResult opMyEnum(@Nullable MyEnum p1, Current current)
    {
        return new Initial.OpMyEnumResult(p1, p1);
    }

    @Override
    public Initial.OpMyEnumReqResult opMyEnumReq(@Nullable MyEnum p1, Current current)
    {
        return new Initial.OpMyEnumReqResult(p1, p1);
    }

    @Override
    public Initial.OpSmallStructResult opSmallStruct(@Nullable SmallStruct p1, Current current)
    {
        return new Initial.OpSmallStructResult(p1, p1);
    }

    @Override
    public Initial.OpSmallStructReqResult opSmallStructReq(@Nullable SmallStruct p1, Current current)
    {
        return new Initial.OpSmallStructReqResult(p1, p1);
    }

    @Override
    public Initial.OpFixedStructResult opFixedStruct(@Nullable FixedStruct p1, Current current)
    {
        return new Initial.OpFixedStructResult(p1, p1);
    }

    @Override
    public Initial.OpFixedStructReqResult opFixedStructReq(@Nullable FixedStruct p1, Current current)
    {
        return new Initial.OpFixedStructReqResult(p1, p1);
    }

    @Override
    public Initial.OpVarStructResult opVarStruct(@Nullable VarStruct p1, Current current)
    {
        return new Initial.OpVarStructResult(p1, p1);
    }

    @Override
    public Initial.OpVarStructReqResult opVarStructReq(@Nullable VarStruct p1, Current current)
    {
        return new Initial.OpVarStructReqResult(p1, p1);
    }

    @Override
    public Initial.OpOneOptionalResult opOneOptional(@Nullable OneOptional p1, Current current)
    {
        return new Initial.OpOneOptionalResult(p1, p1);
    }

    @Override
    public Initial.OpOneOptionalReqResult opOneOptionalReq(@Nullable OneOptional p1, Current current)
    {
        return new Initial.OpOneOptionalReqResult(p1, p1);
    }

    @Override
    public Initial.OpOneOptionalProxyResult opOneOptionalProxy(com.zeroc.Ice.@Nullable ObjectPrx p1, Current current)
    {
        return new Initial.OpOneOptionalProxyResult(p1, p1);
    }

    @Override
    public Initial.OpOneOptionalProxyReqResult opOneOptionalProxyReq(com.zeroc.Ice.@Nullable ObjectPrx p1,
                                                                     Current current)
    {
        return new Initial.OpOneOptionalProxyReqResult(p1, p1);
    }

    @Override
    public Initial.OpByteSeqResult opByteSeq(byte @Nullable[] p1, Current current)
    {
        return new Initial.OpByteSeqResult(p1, p1);
    }

    @Override
    public Initial.OpByteSeqReqResult opByteSeqReq(byte @Nullable[] p1, Current current)
    {
        return new Initial.OpByteSeqReqResult(p1, p1);
    }

    @Override
    public Initial.OpBoolSeqResult opBoolSeq(boolean @Nullable[] p1, Current current)
    {
        return new Initial.OpBoolSeqResult(p1, p1);
    }

    @Override
    public Initial.OpBoolSeqReqResult opBoolSeqReq(boolean @Nullable[] p1, Current current)
    {
        return new Initial.OpBoolSeqReqResult(p1, p1);
    }

    @Override
    public Initial.OpShortSeqResult opShortSeq(short @Nullable[] p1, Current current)
    {
        return new Initial.OpShortSeqResult(p1, p1);
    }

    @Override
    public Initial.OpShortSeqReqResult opShortSeqReq(short @Nullable[] p1, Current current)
    {
        return new Initial.OpShortSeqReqResult(p1, p1);
    }

    @Override
    public Initial.OpIntSeqResult opIntSeq(int @Nullable[] p1, Current current)
    {
        return new Initial.OpIntSeqResult(p1, p1);
    }

    @Override
    public Initial.OpIntSeqReqResult opIntSeqReq(int @Nullable[] p1, Current current)
    {
        return new Initial.OpIntSeqReqResult(p1, p1);
    }

    @Override
    public Initial.OpLongSeqResult opLongSeq(long @Nullable[] p1, Current current)
    {
        return new Initial.OpLongSeqResult(p1, p1);
    }

    @Override
    public Initial.OpLongSeqReqResult opLongSeqReq(long @Nullable[] p1, Current current)
    {
        return new Initial.OpLongSeqReqResult(p1, p1);
    }

    @Override
    public Initial.OpFloatSeqResult opFloatSeq(float @Nullable[] p1, Current current)
    {
        return new Initial.OpFloatSeqResult(p1, p1);
    }

    @Override
    public Initial.OpFloatSeqReqResult opFloatSeqReq(float @Nullable[] p1, Current current)
    {
        return new Initial.OpFloatSeqReqResult(p1, p1);
    }

    @Override
    public Initial.OpDoubleSeqResult opDoubleSeq(double @Nullable[] p1, Current current)
    {
        return new Initial.OpDoubleSeqResult(p1, p1);
    }

    @Override
    public Initial.OpDoubleSeqReqResult opDoubleSeqReq(double @Nullable[] p1, Current current)
    {
        return new Initial.OpDoubleSeqReqResult(p1, p1);
    }

    @Override
    public Initial.OpStringSeqResult opStringSeq(String @Nullable[] p1, Current current)
    {
        return new Initial.OpStringSeqResult(p1, p1);
    }

    @Override
    public Initial.OpStringSeqReqResult opStringSeqReq(String @Nullable[] p1, Current current)
    {
        return new Initial.OpStringSeqReqResult(p1, p1);
    }

    @Override
    public Initial.OpSmallStructSeqResult opSmallStructSeq(SmallStruct @Nullable[] p1, Current current)
    {
        return new Initial.OpSmallStructSeqResult(p1, p1);
    }

    @Override
    public Initial.OpSmallStructSeqReqResult opSmallStructSeqReq(SmallStruct @Nullable[] p1, Current current)
    {
        return new Initial.OpSmallStructSeqReqResult(p1, p1);
    }

    @Override
    public Initial.OpSmallStructListResult opSmallStructList(java.util.@Nullable List<SmallStruct> p1, Current current)
    {
        return new Initial.OpSmallStructListResult(p1, p1);
    }

    @Override
    public Initial.OpSmallStructListReqResult opSmallStructListReq(java.util.@Nullable List<SmallStruct> p1,
                                                                   Current current)
    {
        return new Initial.OpSmallStructListReqResult(p1, p1);
    }

    @Override
    public Initial.OpFixedStructSeqResult opFixedStructSeq(FixedStruct @Nullable[] p1, Current current)
    {
        return new Initial.OpFixedStructSeqResult(p1, p1);
    }

    @Override
    public Initial.OpFixedStructSeqReqResult opFixedStructSeqReq(FixedStruct @Nullable[] p1, Current current)
    {
        return new Initial.OpFixedStructSeqReqResult(p1, p1);
    }

    @Override
    public Initial.OpFixedStructListResult opFixedStructList(java.util.@Nullable List<FixedStruct> p1, Current current)
    {
        return new Initial.OpFixedStructListResult(p1, p1);
    }

    @Override
    public Initial.OpFixedStructListReqResult opFixedStructListReq(java.util.@Nullable List<FixedStruct> p1,
                                                                   Current current)
    {
        return new Initial.OpFixedStructListReqResult(p1, p1);
    }

    @Override
    public Initial.OpVarStructSeqResult opVarStructSeq(VarStruct @Nullable[] p1, Current current)
    {
        return new Initial.OpVarStructSeqResult(p1, p1);
    }

    @Override
    public Initial.OpVarStructSeqReqResult opVarStructSeqReq(VarStruct @Nullable[] p1, Current current)
    {
        return new Initial.OpVarStructSeqReqResult(p1, p1);
    }

    @Override
    public Initial.OpSerializableResult opSerializable(@Nullable SerializableClass p1, Current current)
    {
        return new Initial.OpSerializableResult(p1, p1);
    }

    @Override
    public Initial.OpSerializableReqResult opSerializableReq(@Nullable SerializableClass p1, Current current)
    {
        return new Initial.OpSerializableReqResult(p1, p1);
    }

    @Override
    public Initial.OpIntIntDictResult opIntIntDict(java.util.@Nullable Map<Integer, Integer> p1, Current current)
    {
        return new Initial.OpIntIntDictResult(p1, p1);
    }

    @Override
    public Initial.OpIntIntDictReqResult opIntIntDictReq(java.util.@Nullable Map<Integer, Integer> p1, Current current)
    {
        return new Initial.OpIntIntDictReqResult(p1, p1);
    }

    @Override
    public Initial.OpStringIntDictResult opStringIntDict(java.util.@Nullable Map<String, Integer> p1, Current current)
    {
        return new Initial.OpStringIntDictResult(p1, p1);
    }

    @Override
    public Initial.OpStringIntDictReqResult opStringIntDictReq(java.util.@Nullable Map<String, Integer> p1,
                                                               Current current)
    {
        return new Initial.OpStringIntDictReqResult(p1, p1);
    }

    @Override
    public Initial.OpIntOneOptionalDictResult opIntOneOptionalDict(java.util.@Nullable Map<Integer, OneOptional> p1,
                                                                   Current current)
    {
        return new Initial.OpIntOneOptionalDictResult(p1, p1);
    }

    @Override
    public Initial.OpIntOneOptionalDictReqResult opIntOneOptionalDictReq(
        java.util.@Nullable Map<Integer, OneOptional> p1,
        Current current)
    {
        return new Initial.OpIntOneOptionalDictReqResult(p1, p1);
    }

    @Override
    public void opClassAndUnknownOptional(A p, Current current)
    {
    }

    @Override
    public void sendOptionalClass(boolean req, @Nullable OneOptional o, Current current)
    {
    }

    @Override
    public @Nullable OneOptional returnOptionalClass(boolean req, Current current)
    {
        return new OneOptional(53);
    }

    @Override
    public G opG(G g, Current current)
    {
        return g;
    }

    @Override
    public void opVoid(Current current)
    {
    }

    @Override
    public OpMStruct1MarshaledResult opMStruct1(Current current)
    {
        return new OpMStruct1MarshaledResult(new SmallStruct(), current);
    }

    @Override
    public OpMStruct2MarshaledResult opMStruct2(@Nullable SmallStruct p1, Current current)
    {
        return new OpMStruct2MarshaledResult(p1, p1, current);
    }

    @Override
    public OpMSeq1MarshaledResult opMSeq1(Current current)
    {
        return new OpMSeq1MarshaledResult(new String[0], current);
    }

    @Override
    public OpMSeq2MarshaledResult opMSeq2(String @Nullable[] p1, Current current)
    {
        return new OpMSeq2MarshaledResult(p1, p1, current);
    }

    @Override
    public OpMDict1MarshaledResult opMDict1(Current current)
    {
        return new OpMDict1MarshaledResult(new java.util.HashMap<>(), current);
    }

    @Override
    public OpMDict2MarshaledResult opMDict2(java.util.@Nullable Map<String, Integer> p1, Current current)
    {
        return new OpMDict2MarshaledResult(p1, p1, current);
    }

    @Override
    public OpMG1MarshaledResult opMG1(Current current)
    {
        return new OpMG1MarshaledResult(new G(), current);
    }

    @Override
    public OpMG2MarshaledResult opMG2(@Nullable G p1, Current current)
    {
        return new OpMG2MarshaledResult(p1, p1, current);
    }

    @Override
    public boolean supportsRequiredParams(Current current)
    {
        return true;
    }

    @Override
    public boolean supportsJavaSerializable(Current current)
    {
        return true;
    }

    @Override
    public boolean supportsCsharpSerializable(Current current)
    {
        return false;
    }

    @Override
    public boolean supportsCppStringView(Current current)
    {
        return false;
    }

    @Override
    public boolean supportsNullOptional(Current current)
    {
        return false;
    }
}
