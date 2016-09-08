// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.optional;

import java.util.Optional;
import java.util.OptionalDouble;
import java.util.OptionalInt;
import java.util.OptionalLong;

import com.zeroc.Ice.Current;

import test.Ice.optional.Test.*;

public final class InitialI implements _InitialDisp
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
    public void opOptionalException(OptionalInt a, Optional<String> b, Optional<OneOptional> o, Current current)
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
        throw ex;
    }

    @Override
    public void opDerivedException(OptionalInt a, Optional<String> b, Optional<OneOptional> o, Current current)
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
        throw ex;
    }

    @Override
    public void opRequiredException(OptionalInt a, Optional<String> b, Optional<OneOptional> o, Current current)
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
        throw ex;
    }

    @Override
    public Initial.OpByteResult opByte(Optional<Byte> p1, Current current)
    {
        return new Initial.OpByteResult(p1, p1);
    }

    @Override
    public Initial.OpByteReqResult opByteReq(Optional<Byte> p1, Current current)
    {
        return new Initial.OpByteReqResult(p1, p1);
    }

    @Override
    public Initial.OpBoolResult opBool(Optional<Boolean> p1, Current current)
    {
        return new Initial.OpBoolResult(p1, p1);
    }

    @Override
    public Initial.OpBoolReqResult opBoolReq(Optional<Boolean> p1, Current current)
    {
        return new Initial.OpBoolReqResult(p1, p1);
    }

    @Override
    public Initial.OpShortResult opShort(Optional<Short> p1, Current current)
    {
        return new Initial.OpShortResult(p1, p1);
    }

    @Override
    public Initial.OpShortReqResult opShortReq(Optional<Short> p1, Current current)
    {
        return new Initial.OpShortReqResult(p1, p1);
    }

    @Override
    public Initial.OpIntResult opInt(OptionalInt p1, Current current)
    {
        return new Initial.OpIntResult(p1, p1);
    }

    @Override
    public Initial.OpIntReqResult opIntReq(OptionalInt p1, Current current)
    {
        return new Initial.OpIntReqResult(p1, p1);
    }

    @Override
    public Initial.OpLongResult opLong(OptionalLong p1, Current current)
    {
        return new Initial.OpLongResult(p1, p1);
    }

    @Override
    public Initial.OpLongReqResult opLongReq(OptionalLong p1, Current current)
    {
        return new Initial.OpLongReqResult(p1, p1);
    }

    @Override
    public Initial.OpFloatResult opFloat(Optional<Float> p1, Current current)
    {
        return new Initial.OpFloatResult(p1, p1);
    }

    @Override
    public Initial.OpFloatReqResult opFloatReq(Optional<Float> p1, Current current)
    {
        return new Initial.OpFloatReqResult(p1, p1);
    }

    @Override
    public Initial.OpDoubleResult opDouble(OptionalDouble p1, Current current)
    {
        return new Initial.OpDoubleResult(p1, p1);
    }

    @Override
    public Initial.OpDoubleReqResult opDoubleReq(OptionalDouble p1, Current current)
    {
        return new Initial.OpDoubleReqResult(p1, p1);
    }

    @Override
    public Initial.OpStringResult opString(Optional<String> p1, Current current)
    {
        return new Initial.OpStringResult(p1, p1);
    }

    @Override
    public Initial.OpStringReqResult opStringReq(Optional<String> p1, Current current)
    {
        return new Initial.OpStringReqResult(p1, p1);
    }

    @Override
    public Initial.OpMyEnumResult opMyEnum(Optional<MyEnum> p1, Current current)
    {
        return new Initial.OpMyEnumResult(p1, p1);
    }

    @Override
    public Initial.OpMyEnumReqResult opMyEnumReq(Optional<MyEnum> p1, Current current)
    {
        return new Initial.OpMyEnumReqResult(p1, p1);
    }

    @Override
    public Initial.OpSmallStructResult opSmallStruct(Optional<SmallStruct> p1, Current current)
    {
        return new Initial.OpSmallStructResult(p1, p1);
    }

    @Override
    public Initial.OpSmallStructReqResult opSmallStructReq(Optional<SmallStruct> p1, Current current)
    {
        return new Initial.OpSmallStructReqResult(p1, p1);
    }

    @Override
    public Initial.OpFixedStructResult opFixedStruct(Optional<FixedStruct> p1, Current current)
    {
        return new Initial.OpFixedStructResult(p1, p1);
    }

    @Override
    public Initial.OpFixedStructReqResult opFixedStructReq(Optional<FixedStruct> p1, Current current)
    {
        return new Initial.OpFixedStructReqResult(p1, p1);
    }

    @Override
    public Initial.OpVarStructResult opVarStruct(Optional<VarStruct> p1, Current current)
    {
        return new Initial.OpVarStructResult(p1, p1);
    }

    @Override
    public Initial.OpVarStructReqResult opVarStructReq(Optional<VarStruct> p1, Current current)
    {
        return new Initial.OpVarStructReqResult(p1, p1);
    }

    @Override
    public Initial.OpOneOptionalResult opOneOptional(Optional<OneOptional> p1, Current current)
    {
        return new Initial.OpOneOptionalResult(p1, p1);
    }

    @Override
    public Initial.OpOneOptionalReqResult opOneOptionalReq(Optional<OneOptional> p1, Current current)
    {
        return new Initial.OpOneOptionalReqResult(p1, p1);
    }

    @Override
    public Initial.OpOneOptionalProxyResult opOneOptionalProxy(Optional<com.zeroc.Ice.ObjectPrx> p1, Current current)
    {
        return new Initial.OpOneOptionalProxyResult(p1, p1);
    }

    @Override
    public Initial.OpOneOptionalProxyReqResult opOneOptionalProxyReq(Optional<com.zeroc.Ice.ObjectPrx> p1,
                                                                     Current current)
    {
        return new Initial.OpOneOptionalProxyReqResult(p1, p1);
    }

    @Override
    public Initial.OpByteSeqResult opByteSeq(Optional<byte[]> p1, Current current)
    {
        return new Initial.OpByteSeqResult(p1, p1);
    }

    @Override
    public Initial.OpByteSeqReqResult opByteSeqReq(Optional<byte[]> p1, Current current)
    {
        return new Initial.OpByteSeqReqResult(p1, p1);
    }

    @Override
    public Initial.OpBoolSeqResult opBoolSeq(Optional<boolean[]> p1, Current current)
    {
        return new Initial.OpBoolSeqResult(p1, p1);
    }

    @Override
    public Initial.OpBoolSeqReqResult opBoolSeqReq(Optional<boolean[]> p1, Current current)
    {
        return new Initial.OpBoolSeqReqResult(p1, p1);
    }

    @Override
    public Initial.OpShortSeqResult opShortSeq(Optional<short[]> p1, Current current)
    {
        return new Initial.OpShortSeqResult(p1, p1);
    }

    @Override
    public Initial.OpShortSeqReqResult opShortSeqReq(Optional<short[]> p1, Current current)
    {
        return new Initial.OpShortSeqReqResult(p1, p1);
    }

    @Override
    public Initial.OpIntSeqResult opIntSeq(Optional<int[]> p1, Current current)
    {
        return new Initial.OpIntSeqResult(p1, p1);
    }

    @Override
    public Initial.OpIntSeqReqResult opIntSeqReq(Optional<int[]> p1, Current current)
    {
        return new Initial.OpIntSeqReqResult(p1, p1);
    }

    @Override
    public Initial.OpLongSeqResult opLongSeq(Optional<long[]> p1, Current current)
    {
        return new Initial.OpLongSeqResult(p1, p1);
    }

    @Override
    public Initial.OpLongSeqReqResult opLongSeqReq(Optional<long[]> p1, Current current)
    {
        return new Initial.OpLongSeqReqResult(p1, p1);
    }

    @Override
    public Initial.OpFloatSeqResult opFloatSeq(Optional<float[]> p1, Current current)
    {
        return new Initial.OpFloatSeqResult(p1, p1);
    }

    @Override
    public Initial.OpFloatSeqReqResult opFloatSeqReq(Optional<float[]> p1, Current current)
    {
        return new Initial.OpFloatSeqReqResult(p1, p1);
    }

    @Override
    public Initial.OpDoubleSeqResult opDoubleSeq(Optional<double[]> p1, Current current)
    {
        return new Initial.OpDoubleSeqResult(p1, p1);
    }

    @Override
    public Initial.OpDoubleSeqReqResult opDoubleSeqReq(Optional<double[]> p1, Current current)
    {
        return new Initial.OpDoubleSeqReqResult(p1, p1);
    }

    @Override
    public Initial.OpStringSeqResult opStringSeq(Optional<String[]> p1, Current current)
    {
        return new Initial.OpStringSeqResult(p1, p1);
    }

    @Override
    public Initial.OpStringSeqReqResult opStringSeqReq(Optional<String[]> p1, Current current)
    {
        return new Initial.OpStringSeqReqResult(p1, p1);
    }

    @Override
    public Initial.OpSmallStructSeqResult opSmallStructSeq(Optional<SmallStruct[]> p1, Current current)
    {
        return new Initial.OpSmallStructSeqResult(p1, p1);
    }

    @Override
    public Initial.OpSmallStructSeqReqResult opSmallStructSeqReq(Optional<SmallStruct[]> p1, Current current)
    {
        return new Initial.OpSmallStructSeqReqResult(p1, p1);
    }

    @Override
    public Initial.OpSmallStructListResult opSmallStructList(Optional<java.util.List<SmallStruct>> p1, Current current)
    {
        return new Initial.OpSmallStructListResult(p1, p1);
    }

    @Override
    public Initial.OpSmallStructListReqResult opSmallStructListReq(Optional<java.util.List<SmallStruct>> p1,
                                                                   Current current)
    {
        return new Initial.OpSmallStructListReqResult(p1, p1);
    }

    @Override
    public Initial.OpFixedStructSeqResult opFixedStructSeq(Optional<FixedStruct[]> p1, Current current)
    {
        return new Initial.OpFixedStructSeqResult(p1, p1);
    }

    @Override
    public Initial.OpFixedStructSeqReqResult opFixedStructSeqReq(Optional<FixedStruct[]> p1, Current current)
    {
        return new Initial.OpFixedStructSeqReqResult(p1, p1);
    }

    @Override
    public Initial.OpFixedStructListResult opFixedStructList(Optional<java.util.List<FixedStruct>> p1, Current current)
    {
        return new Initial.OpFixedStructListResult(p1, p1);
    }

    @Override
    public Initial.OpFixedStructListReqResult opFixedStructListReq(Optional<java.util.List<FixedStruct>> p1,
                                                                   Current current)
    {
        return new Initial.OpFixedStructListReqResult(p1, p1);
    }

    @Override
    public Initial.OpVarStructSeqResult opVarStructSeq(Optional<VarStruct[]> p1, Current current)
    {
        return new Initial.OpVarStructSeqResult(p1, p1);
    }

    @Override
    public Initial.OpVarStructSeqReqResult opVarStructSeqReq(Optional<VarStruct[]> p1, Current current)
    {
        return new Initial.OpVarStructSeqReqResult(p1, p1);
    }

    @Override
    public Initial.OpSerializableResult opSerializable(Optional<SerializableClass> p1, Current current)
    {
        return new Initial.OpSerializableResult(p1, p1);
    }

    @Override
    public Initial.OpSerializableReqResult opSerializableReq(Optional<SerializableClass> p1, Current current)
    {
        return new Initial.OpSerializableReqResult(p1, p1);
    }

    @Override
    public Initial.OpIntIntDictResult opIntIntDict(Optional<java.util.Map<Integer, Integer>> p1, Current current)
    {
        return new Initial.OpIntIntDictResult(p1, p1);
    }

    @Override
    public Initial.OpIntIntDictReqResult opIntIntDictReq(Optional<java.util.Map<Integer, Integer>> p1, Current current)
    {
        return new Initial.OpIntIntDictReqResult(p1, p1);
    }

    @Override
    public Initial.OpStringIntDictResult opStringIntDict(Optional<java.util.Map<String, Integer>> p1, Current current)
    {
        return new Initial.OpStringIntDictResult(p1, p1);
    }

    @Override
    public Initial.OpStringIntDictReqResult opStringIntDictReq(Optional<java.util.Map<String, Integer>> p1,
                                                               Current current)
    {
        return new Initial.OpStringIntDictReqResult(p1, p1);
    }

    @Override
    public Initial.OpIntOneOptionalDictResult opIntOneOptionalDict(Optional<java.util.Map<Integer, OneOptional>> p1,
                                                                   Current current)
    {
        return new Initial.OpIntOneOptionalDictResult(p1, p1);
    }

    @Override
    public Initial.OpIntOneOptionalDictReqResult opIntOneOptionalDictReq(
        Optional<java.util.Map<Integer, OneOptional>> p1,
        Current current)
    {
        return new Initial.OpIntOneOptionalDictReqResult(p1, p1);
    }

    @Override
    public void opClassAndUnknownOptional(A p, Current current)
    {
    }

    @Override
    public void sendOptionalClass(boolean req, Optional<OneOptional> o, Current current)
    {
    }

    @Override
    public Optional<OneOptional> returnOptionalClass(boolean req, Current current)
    {
        return Optional.of(new OneOptional(53));
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
        return new OpMStruct1MarshaledResult(Optional.of(new SmallStruct()), current);
    }

    @Override
    public OpMStruct2MarshaledResult opMStruct2(Optional<SmallStruct> p1, Current current)
    {
        return new OpMStruct2MarshaledResult(p1, p1, current);
    }

    @Override
    public OpMSeq1MarshaledResult opMSeq1(Current current)
    {
        return new OpMSeq1MarshaledResult(Optional.of(new String[0]), current);
    }

    @Override
    public OpMSeq2MarshaledResult opMSeq2(Optional<String[]> p1, Current current)
    {
        return new OpMSeq2MarshaledResult(p1, p1, current);
    }

    @Override
    public OpMDict1MarshaledResult opMDict1(Current current)
    {
        return new OpMDict1MarshaledResult(Optional.of(new java.util.HashMap<>()), current);
    }

    @Override
    public OpMDict2MarshaledResult opMDict2(Optional<java.util.Map<String, Integer>> p1, Current current)
    {
        return new OpMDict2MarshaledResult(p1, p1, current);
    }

    @Override
    public OpMG1MarshaledResult opMG1(Current current)
    {
        return new OpMG1MarshaledResult(Optional.of(new G()), current);
    }

    @Override
    public OpMG2MarshaledResult opMG2(Optional<G> p1, Current current)
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
        //
        // The java.util.Optional class does not support a null value. Constructing an Optional
        // using Optional.ofNullable(null) returns an optional whose value is NOT present.
        //
        return false;
    }
}
