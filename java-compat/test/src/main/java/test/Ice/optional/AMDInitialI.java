// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.optional;

import test.Ice.optional.AMD.Test.*;

public final class AMDInitialI extends _InitialDisp
{
    @Override
    public void
    shutdown_async(AMD_Initial_shutdown cb, Ice.Current current)
    {
        current.adapter.getCommunicator().shutdown();
        cb.ice_response();
    }

    @Override
    public void
    pingPong_async(AMD_Initial_pingPong cb, Ice.Object obj, Ice.Current current)
    {
        cb.ice_response(obj);
    }

    @Override
    public void
    opOptionalException_async(AMD_Initial_opOptionalException cb, Ice.IntOptional a, Ice.Optional<String> b,
                              Ice.Optional<OneOptional> o, Ice.Current current)
        throws OptionalException
    {
        OptionalException ex = new OptionalException();
        if(a.isSet())
        {
            ex.setA(a.get());
        }
        else
        {
            ex.clearA(); // The member "a" has a default value.
        }
        if(b.isSet())
        {
            ex.setB(b.get());
        }
        if(o.isSet())
        {
            ex.setO(o.get());
        }
        cb.ice_exception(ex);
    }

    @Override
    public void
    opDerivedException_async(AMD_Initial_opDerivedException cb, Ice.IntOptional a, Ice.Optional<String> b,
                             Ice.Optional<OneOptional> o, Ice.Current current)
        throws OptionalException
    {
        DerivedException ex = new DerivedException();
        if(a.isSet())
        {
            ex.setA(a.get());
        }
        else
        {
            ex.clearA(); // The member "a" has a default value.
        }
        if(b.isSet())
        {
            ex.setB(b.get());
            ex.setSs(b.get());
        }
        else
        {
            ex.clearSs(); // The member "ss" has a default value.
        }
        if(o.isSet())
        {
            ex.setO(o.get());
            ex.setO2(o.get());
        }
        cb.ice_exception(ex);
    }

    @Override
    public void
    opRequiredException_async(AMD_Initial_opRequiredException cb, Ice.IntOptional a, Ice.Optional<String> b,
                              Ice.Optional<OneOptional> o, Ice.Current current)
        throws OptionalException
    {
        RequiredException ex = new RequiredException();
        if(a.isSet())
        {
            ex.setA(a.get());
        }
        else
        {
            ex.clearA(); // The member "a" has a default value.
        }
        if(b.isSet())
        {
            ex.setB(b.get());
            ex.ss = b.get();
        }
        if(o.isSet())
        {
            ex.setO(o.get());
            ex.o2 = o.get();
        }
        cb.ice_exception(ex);
    }

    @Override
    public void
    opByte_async(AMD_Initial_opByte cb, Ice.ByteOptional p1, Ice.Current current)
    {
        cb.ice_response(p1, p1);
    }

    @Override
    public void
    opByteReq_async(AMD_Initial_opByteReq cb, Ice.ByteOptional p1, Ice.Current current)
    {
        cb.ice_response(p1.get(), p1.get());
    }

    @Override
    public void
    opBool_async(AMD_Initial_opBool cb, Ice.BooleanOptional p1, Ice.Current current)
    {
        cb.ice_response(p1, p1);
    }

    @Override
    public void
    opBoolReq_async(AMD_Initial_opBoolReq cb, Ice.BooleanOptional p1, Ice.Current current)
    {
        cb.ice_response(p1.get(), p1.get());
    }

    @Override
    public void
    opShort_async(AMD_Initial_opShort cb, Ice.ShortOptional p1, Ice.Current current)
    {
        cb.ice_response(p1, p1);
    }

    @Override
    public void
    opShortReq_async(AMD_Initial_opShortReq cb, Ice.ShortOptional p1, Ice.Current current)
    {
        cb.ice_response(p1.get(), p1.get());
    }

    @Override
    public void
    opInt_async(AMD_Initial_opInt cb, Ice.IntOptional p1, Ice.Current current)
    {
        cb.ice_response(p1, p1);
    }

    @Override
    public void
    opIntReq_async(AMD_Initial_opIntReq cb, Ice.IntOptional p1, Ice.Current current)
    {
        cb.ice_response(p1.get(), p1.get());
    }

    @Override
    public void
    opLong_async(AMD_Initial_opLong cb, Ice.LongOptional p1, Ice.Current current)
    {
        cb.ice_response(p1, p1);
    }

    @Override
    public void
    opLongReq_async(AMD_Initial_opLongReq cb, Ice.LongOptional p1, Ice.Current current)
    {
        cb.ice_response(p1.get(), p1.get());
    }

    @Override
    public void
    opFloat_async(AMD_Initial_opFloat cb, Ice.FloatOptional p1, Ice.Current current)
    {
        cb.ice_response(p1, p1);
    }

    @Override
    public void
    opFloatReq_async(AMD_Initial_opFloatReq cb, Ice.FloatOptional p1, Ice.Current current)
    {
        cb.ice_response(p1.get(), p1.get());
    }

    @Override
    public void
    opDouble_async(AMD_Initial_opDouble cb, Ice.DoubleOptional p1, Ice.Current current)
    {
        cb.ice_response(p1, p1);
    }

    @Override
    public void
    opDoubleReq_async(AMD_Initial_opDoubleReq cb, Ice.DoubleOptional p1, Ice.Current current)
    {
        cb.ice_response(p1.get(), p1.get());
    }

    @Override
    public void
    opString_async(AMD_Initial_opString cb, Ice.Optional<String> p1, Ice.Current current)
    {
        cb.ice_response(p1, p1);
    }

    @Override
    public void
    opStringReq_async(AMD_Initial_opStringReq cb, Ice.Optional<String> p1, Ice.Current current)
    {
        cb.ice_response(p1.get(), p1.get());
    }

    @Override
    public void
    opMyEnum_async(AMD_Initial_opMyEnum cb, Ice.Optional<MyEnum> p1, Ice.Current current)
    {
        cb.ice_response(p1, p1);
    }

    @Override
    public void
    opMyEnumReq_async(AMD_Initial_opMyEnumReq cb, Ice.Optional<MyEnum> p1, Ice.Current current)
    {
        cb.ice_response(p1.get(), p1.get());
    }

    @Override
    public void
    opSmallStruct_async(AMD_Initial_opSmallStruct cb, Ice.Optional<SmallStruct> p1, Ice.Current current)
    {
        cb.ice_response(p1, p1);
    }

    @Override
    public void
    opSmallStructReq_async(AMD_Initial_opSmallStructReq cb, Ice.Optional<SmallStruct> p1, Ice.Current current)
    {
        cb.ice_response(p1.get(), p1.get());
    }

    @Override
    public void
    opFixedStruct_async(AMD_Initial_opFixedStruct cb, Ice.Optional<FixedStruct> p1, Ice.Current current)
    {
        cb.ice_response(p1, p1);
    }

    @Override
    public void
    opFixedStructReq_async(AMD_Initial_opFixedStructReq cb, Ice.Optional<FixedStruct> p1, Ice.Current current)
    {
        cb.ice_response(p1.get(), p1.get());
    }

    @Override
    public void
    opVarStruct_async(AMD_Initial_opVarStruct cb, Ice.Optional<VarStruct> p1, Ice.Current current)
    {
        cb.ice_response(p1, p1);
    }

    @Override
    public void
    opVarStructReq_async(AMD_Initial_opVarStructReq cb, Ice.Optional<VarStruct> p1, Ice.Current current)
    {
        cb.ice_response(p1.get(), p1.get());
    }

    @Override
    public void
    opOneOptional_async(AMD_Initial_opOneOptional cb, Ice.Optional<OneOptional> p1, Ice.Current current)
    {
        cb.ice_response(p1, p1);
    }

    @Override
    public void
    opOneOptionalReq_async(AMD_Initial_opOneOptionalReq cb, Ice.Optional<OneOptional> p1, Ice.Current current)
    {
        cb.ice_response(p1.get(), p1.get());
    }

    @Override
    public void
    opOneOptionalProxy_async(AMD_Initial_opOneOptionalProxy cb, Ice.Optional<OneOptionalPrx> p1, Ice.Current current)
    {
        cb.ice_response(p1, p1);
    }

    @Override
    public void
    opOneOptionalProxyReq_async(AMD_Initial_opOneOptionalProxyReq cb, Ice.Optional<OneOptionalPrx> p1,
                                Ice.Current current)
    {
        cb.ice_response(p1.get(), p1.get());
    }

    @Override
    public void
    opByteSeq_async(AMD_Initial_opByteSeq cb, Ice.Optional<byte[]> p1, Ice.Current current)
    {
        cb.ice_response(p1, p1);
    }

    @Override
    public void
    opByteSeqReq_async(AMD_Initial_opByteSeqReq cb, Ice.Optional<byte[]> p1, Ice.Current current)
    {
        cb.ice_response(p1.get(), p1.get());
    }

    @Override
    public void
    opBoolSeq_async(AMD_Initial_opBoolSeq cb, Ice.Optional<boolean[]> p1, Ice.Current current)
    {
        cb.ice_response(p1, p1);
    }

    @Override
    public void
    opBoolSeqReq_async(AMD_Initial_opBoolSeqReq cb, Ice.Optional<boolean[]> p1, Ice.Current current)
    {
        cb.ice_response(p1.get(), p1.get());
    }

    @Override
    public void
    opShortSeq_async(AMD_Initial_opShortSeq cb, Ice.Optional<short[]> p1, Ice.Current current)
    {
        cb.ice_response(p1, p1);
    }

    @Override
    public void
    opShortSeqReq_async(AMD_Initial_opShortSeqReq cb, Ice.Optional<short[]> p1, Ice.Current current)
    {
        cb.ice_response(p1.get(), p1.get());
    }

    @Override
    public void
    opIntSeq_async(AMD_Initial_opIntSeq cb, Ice.Optional<int[]> p1, Ice.Current current)
    {
        cb.ice_response(p1, p1);
    }

    @Override
    public void
    opIntSeqReq_async(AMD_Initial_opIntSeqReq cb, Ice.Optional<int[]> p1, Ice.Current current)
    {
        cb.ice_response(p1.get(), p1.get());
    }

    @Override
    public void
    opLongSeq_async(AMD_Initial_opLongSeq cb, Ice.Optional<long[]> p1, Ice.Current current)
    {
        cb.ice_response(p1, p1);
    }

    @Override
    public void
    opLongSeqReq_async(AMD_Initial_opLongSeqReq cb, Ice.Optional<long[]> p1, Ice.Current current)
    {
        cb.ice_response(p1.get(), p1.get());
    }

    @Override
    public void
    opFloatSeq_async(AMD_Initial_opFloatSeq cb, Ice.Optional<float[]> p1, Ice.Current current)
    {
        cb.ice_response(p1, p1);
    }

    @Override
    public void
    opFloatSeqReq_async(AMD_Initial_opFloatSeqReq cb, Ice.Optional<float[]> p1, Ice.Current current)
    {
        cb.ice_response(p1.get(), p1.get());
    }

    @Override
    public void
    opDoubleSeq_async(AMD_Initial_opDoubleSeq cb, Ice.Optional<double[]> p1, Ice.Current current)
    {
        cb.ice_response(p1, p1);
    }

    @Override
    public void
    opDoubleSeqReq_async(AMD_Initial_opDoubleSeqReq cb, Ice.Optional<double[]> p1, Ice.Current current)
    {
        cb.ice_response(p1.get(), p1.get());
    }

    @Override
    public void
    opStringSeq_async(AMD_Initial_opStringSeq cb, Ice.Optional<String[]> p1, Ice.Current current)
    {
        cb.ice_response(p1, p1);
    }

    @Override
    public void
    opStringSeqReq_async(AMD_Initial_opStringSeqReq cb, Ice.Optional<String[]> p1, Ice.Current current)
    {
        cb.ice_response(p1.get(), p1.get());
    }

    @Override
    public void
    opSmallStructSeq_async(AMD_Initial_opSmallStructSeq cb, Ice.Optional<SmallStruct[]> p1, Ice.Current current)
    {
        cb.ice_response(p1, p1);
    }

    @Override
    public void
    opSmallStructSeqReq_async(AMD_Initial_opSmallStructSeqReq cb, Ice.Optional<SmallStruct[]> p1, Ice.Current current)
    {
        cb.ice_response(p1.get(), p1.get());
    }

    @Override
    public void
    opSmallStructList_async(AMD_Initial_opSmallStructList cb, Ice.Optional<java.util.List<SmallStruct>> p1,
                            Ice.Current current)
    {
        cb.ice_response(p1, p1);
    }

    @Override
    public void
    opSmallStructListReq_async(AMD_Initial_opSmallStructListReq cb, Ice.Optional<java.util.List<SmallStruct>> p1,
                               Ice.Current current)
    {
        cb.ice_response(p1.get(), p1.get());
    }

    @Override
    public void
    opFixedStructSeq_async(AMD_Initial_opFixedStructSeq cb, Ice.Optional<FixedStruct[]> p1, Ice.Current current)
    {
        cb.ice_response(p1, p1);
    }

    @Override
    public void
    opFixedStructSeqReq_async(AMD_Initial_opFixedStructSeqReq cb, Ice.Optional<FixedStruct[]> p1, Ice.Current current)
    {
        cb.ice_response(p1.get(), p1.get());
    }

    @Override
    public void
    opFixedStructList_async(AMD_Initial_opFixedStructList cb, Ice.Optional<java.util.List<FixedStruct>> p1,
                            Ice.Current current)
    {
        cb.ice_response(p1, p1);
    }

    @Override
    public void
    opFixedStructListReq_async(AMD_Initial_opFixedStructListReq cb, Ice.Optional<java.util.List<FixedStruct>> p1,
                               Ice.Current current)
    {
        cb.ice_response(p1.get(), p1.get());
    }

    @Override
    public void
    opVarStructSeq_async(AMD_Initial_opVarStructSeq cb, Ice.Optional<VarStruct[]> p1, Ice.Current current)
    {
        cb.ice_response(p1, p1);
    }

    @Override
    public void
    opVarStructSeqReq_async(AMD_Initial_opVarStructSeqReq cb, Ice.Optional<VarStruct[]> p1, Ice.Current current)
    {
        cb.ice_response(p1.get(), p1.get());
    }

    @Override
    public void
    opSerializable_async(AMD_Initial_opSerializable cb, Ice.Optional<SerializableClass> p1, Ice.Current current)
    {
        cb.ice_response(p1, p1);
    }

    @Override
    public void
    opSerializableReq_async(AMD_Initial_opSerializableReq cb, Ice.Optional<SerializableClass> p1, Ice.Current current)
    {
        cb.ice_response(p1.get(), p1.get());
    }

    @Override
    public void
    opIntIntDict_async(AMD_Initial_opIntIntDict cb, Ice.Optional<java.util.Map<Integer, Integer>> p1,
                       Ice.Current current)
    {
        cb.ice_response(p1, p1);
    }

    @Override
    public void
    opIntIntDictReq_async(AMD_Initial_opIntIntDictReq cb, Ice.Optional<java.util.Map<Integer, Integer>> p1,
                          Ice.Current current)
    {
        cb.ice_response(p1.get(), p1.get());
    }

    @Override
    public void
    opStringIntDict_async(AMD_Initial_opStringIntDict cb, Ice.Optional<java.util.Map<String, Integer>> p1,
                          Ice.Current current)
    {
        cb.ice_response(p1, p1);
    }

    @Override
    public void
    opStringIntDictReq_async(AMD_Initial_opStringIntDictReq cb, Ice.Optional<java.util.Map<String, Integer>> p1,
                             Ice.Current current)
    {
        cb.ice_response(p1.get(), p1.get());
    }

    @Override
    public void
    opIntOneOptionalDict_async(AMD_Initial_opIntOneOptionalDict cb,
        Ice.Optional<java.util.Map<Integer, OneOptional>> p1, Ice.Current current)
    {
        cb.ice_response(p1, p1);
    }

    @Override
    public void
    opIntOneOptionalDictReq_async(AMD_Initial_opIntOneOptionalDictReq cb,
        Ice.Optional<java.util.Map<Integer, OneOptional>> p1, Ice.Current current)
    {
        cb.ice_response(p1.get(), p1.get());
    }

    @Override
    public void
    opClassAndUnknownOptional_async(AMD_Initial_opClassAndUnknownOptional cb, A p, Ice.Current current)
    {
        cb.ice_response();
    }

    @Override
    public void
    sendOptionalClass_async(AMD_Initial_sendOptionalClass cb, boolean req, Ice.Optional<OneOptional> o,
                            Ice.Current current)
    {
        cb.ice_response();
    }

    @Override
    public void
    returnOptionalClass_async(AMD_Initial_returnOptionalClass cb, boolean req, Ice.Current current)
    {
        cb.ice_response(new Ice.Optional<OneOptional>(new OneOptional(53)));
    }

    @Override
    public void
    opG_async(AMD_Initial_opG cb, G g, Ice.Current current)
    {
        cb.ice_response(g);
    }

    @Override
    public void
    opVoid_async(AMD_Initial_opVoid cb, Ice.Current current)
    {
        cb.ice_response();
    }

    @Override
    public void
    opMStruct1_async(AMD_Initial_opMStruct1 cb, Ice.Current current)
    {
        cb.ice_response(new Ice.Optional<SmallStruct>(new SmallStruct()));
    }

    @Override
    public void
    opMStruct2_async(AMD_Initial_opMStruct2 cb, Ice.Optional<SmallStruct> p1, Ice.Current current)
    {
        cb.ice_response(p1, p1);
    }

    @Override
    public void
    opMSeq1_async(AMD_Initial_opMSeq1 cb, Ice.Current current)
    {
        cb.ice_response(new Ice.Optional<String[]>(new String[0]));
    }

    @Override
    public void
    opMSeq2_async(AMD_Initial_opMSeq2 cb, Ice.Optional<String[]> p1, Ice.Current current)
    {
        cb.ice_response(p1, p1);
    }

    @Override
    public void
    opMDict1_async(AMD_Initial_opMDict1 cb, Ice.Current current)
    {
        cb.ice_response(new Ice.Optional<java.util.Map<String, Integer>>(new java.util.HashMap<String, Integer>()));
    }

    @Override
    public void
    opMDict2_async(AMD_Initial_opMDict2 cb, Ice.Optional<java.util.Map<String, Integer>> p1, Ice.Current current)
    {
        cb.ice_response(p1, p1);
    }

    @Override
    public void
    opMG1_async(AMD_Initial_opMG1 cb, Ice.Current current)
    {
        cb.ice_response(new Ice.Optional<G>(new G()));
    }

    @Override
    public void
    opMG2_async(AMD_Initial_opMG2 cb, Ice.Optional<G> p1, Ice.Current current)
    {
        cb.ice_response(p1, p1);
    }

    @Override
    public void
    supportsRequiredParams_async(AMD_Initial_supportsRequiredParams cb, Ice.Current current)
    {
        cb.ice_response(true);
    }

    @Override
    public void
    supportsJavaSerializable_async(AMD_Initial_supportsJavaSerializable cb, Ice.Current current)
    {
        cb.ice_response(true);
    }

    @Override
    public void
    supportsCsharpSerializable_async(AMD_Initial_supportsCsharpSerializable cb, Ice.Current current)
    {
        cb.ice_response(false);
    }

    @Override
    public void
    supportsCppStringView_async(AMD_Initial_supportsCppStringView cb, Ice.Current current)
    {
        cb.ice_response(false);
    }

    @Override
    public void
    supportsNullOptional_async(AMD_Initial_supportsNullOptional cb, Ice.Current current)
    {
        cb.ice_response(true);
    }
}
