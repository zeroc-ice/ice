// **********************************************************************
//
// Copyright (c) 2003-2012 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.optional;

import test.Ice.optional.AMD.Test.*;

public final class AMDInitialI extends Initial
{
    public void
    shutdown_async(AMD_Initial_shutdown cb, Ice.Current current)
    {
        current.adapter.getCommunicator().shutdown();
        cb.ice_response();
    }

    public void
    pingPong_async(AMD_Initial_pingPong cb, Ice.Object obj, Ice.Current current)
    {
        cb.ice_response(obj);
    }

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

    public void
    opByte_async(AMD_Initial_opByte cb, Ice.ByteOptional p1, Ice.Current current)
    {
        cb.ice_response(p1.get(), p1.get());
    }

    public void
    opByteOpt_async(AMD_Initial_opByteOpt cb, Ice.ByteOptional p1, Ice.Current current)
    {
        cb.ice_response(p1, p1);
    }

    public void
    opBool_async(AMD_Initial_opBool cb, Ice.BooleanOptional p1, Ice.Current current)
    {
        cb.ice_response(p1.get(), p1.get());
    }

    public void
    opBoolOpt_async(AMD_Initial_opBoolOpt cb, Ice.BooleanOptional p1, Ice.Current current)
    {
        cb.ice_response(p1, p1);
    }

    public void
    opShort_async(AMD_Initial_opShort cb, Ice.ShortOptional p1, Ice.Current current)
    {
        cb.ice_response(p1.get(), p1.get());
    }

    public void
    opShortOpt_async(AMD_Initial_opShortOpt cb, Ice.ShortOptional p1, Ice.Current current)
    {
        cb.ice_response(p1, p1);
    }

    public void
    opInt_async(AMD_Initial_opInt cb, Ice.IntOptional p1, Ice.Current current)
    {
        cb.ice_response(p1.get(), p1.get());
    }

    public void
    opIntOpt_async(AMD_Initial_opIntOpt cb, Ice.IntOptional p1, Ice.Current current)
    {
        cb.ice_response(p1, p1);
    }

    public void
    opLong_async(AMD_Initial_opLong cb, Ice.LongOptional p1, Ice.Current current)
    {
        cb.ice_response(p1.get(), p1.get());
    }

    public void
    opLongOpt_async(AMD_Initial_opLongOpt cb, Ice.LongOptional p1, Ice.Current current)
    {
        cb.ice_response(p1, p1);
    }

    public void
    opFloat_async(AMD_Initial_opFloat cb, Ice.FloatOptional p1, Ice.Current current)
    {
        cb.ice_response(p1.get(), p1.get());
    }

    public void
    opFloatOpt_async(AMD_Initial_opFloatOpt cb, Ice.FloatOptional p1, Ice.Current current)
    {
        cb.ice_response(p1, p1);
    }

    public void
    opDouble_async(AMD_Initial_opDouble cb, Ice.DoubleOptional p1, Ice.Current current)
    {
        cb.ice_response(p1.get(), p1.get());
    }

    public void
    opDoubleOpt_async(AMD_Initial_opDoubleOpt cb, Ice.DoubleOptional p1, Ice.Current current)
    {
        cb.ice_response(p1, p1);
    }

    public void
    opString_async(AMD_Initial_opString cb, Ice.Optional<String> p1, Ice.Current current)
    {
        cb.ice_response(p1.get(), p1.get());
    }

    public void
    opStringOpt_async(AMD_Initial_opStringOpt cb, Ice.Optional<String> p1, Ice.Current current)
    {
        cb.ice_response(p1, p1);
    }

    public void
    opMyEnum_async(AMD_Initial_opMyEnum cb, Ice.Optional<MyEnum> p1, Ice.Current current)
    {
        cb.ice_response(p1.get(), p1.get());
    }

    public void
    opMyEnumOpt_async(AMD_Initial_opMyEnumOpt cb, Ice.Optional<MyEnum> p1, Ice.Current current)
    {
        cb.ice_response(p1, p1);
    }

    public void
    opSmallStruct_async(AMD_Initial_opSmallStruct cb, Ice.Optional<SmallStruct> p1, Ice.Current current)
    {
        cb.ice_response(p1.get(), p1.get());
    }

    public void
    opSmallStructOpt_async(AMD_Initial_opSmallStructOpt cb, Ice.Optional<SmallStruct> p1, Ice.Current current)
    {
        cb.ice_response(p1, p1);
    }

    public void
    opFixedStruct_async(AMD_Initial_opFixedStruct cb, Ice.Optional<FixedStruct> p1, Ice.Current current)
    {
        cb.ice_response(p1.get(), p1.get());
    }

    public void
    opFixedStructOpt_async(AMD_Initial_opFixedStructOpt cb, Ice.Optional<FixedStruct> p1, Ice.Current current)
    {
        cb.ice_response(p1, p1);
    }

    public void
    opVarStruct_async(AMD_Initial_opVarStruct cb, Ice.Optional<VarStruct> p1, Ice.Current current)
    {
        cb.ice_response(p1.get(), p1.get());
    }

    public void
    opVarStructOpt_async(AMD_Initial_opVarStructOpt cb, Ice.Optional<VarStruct> p1, Ice.Current current)
    {
        cb.ice_response(p1, p1);
    }

    public void
    opOneOptional_async(AMD_Initial_opOneOptional cb, Ice.Optional<OneOptional> p1, Ice.Current current)
    {
        cb.ice_response(p1.get(), p1.get());
    }

    public void
    opOneOptionalOpt_async(AMD_Initial_opOneOptionalOpt cb, Ice.Optional<OneOptional> p1, Ice.Current current)
    {
        cb.ice_response(p1, p1);
    }

    public void
    opOneOptionalProxy_async(AMD_Initial_opOneOptionalProxy cb, Ice.Optional<OneOptionalPrx> p1, Ice.Current current)
    {
        cb.ice_response(p1.get(), p1.get());
    }

    public void
    opOneOptionalProxyOpt_async(AMD_Initial_opOneOptionalProxyOpt cb, Ice.Optional<OneOptionalPrx> p1,
                                Ice.Current current)
    {
        cb.ice_response(p1, p1);
    }

    public void
    opByteSeq_async(AMD_Initial_opByteSeq cb, Ice.Optional<byte[]> p1, Ice.Current current)
    {
        cb.ice_response(p1.get(), p1.get());
    }

    public void
    opByteSeqOpt_async(AMD_Initial_opByteSeqOpt cb, Ice.Optional<byte[]> p1, Ice.Current current)
    {
        cb.ice_response(p1, p1);
    }

    public void
    opBoolSeq_async(AMD_Initial_opBoolSeq cb, Ice.Optional<boolean[]> p1, Ice.Current current)
    {
        cb.ice_response(p1.get(), p1.get());
    }

    public void
    opBoolSeqOpt_async(AMD_Initial_opBoolSeqOpt cb, Ice.Optional<boolean[]> p1, Ice.Current current)
    {
        cb.ice_response(p1, p1);
    }

    public void
    opShortSeq_async(AMD_Initial_opShortSeq cb, Ice.Optional<short[]> p1, Ice.Current current)
    {
        cb.ice_response(p1.get(), p1.get());
    }

    public void
    opShortSeqOpt_async(AMD_Initial_opShortSeqOpt cb, Ice.Optional<short[]> p1, Ice.Current current)
    {
        cb.ice_response(p1, p1);
    }

    public void
    opIntSeq_async(AMD_Initial_opIntSeq cb, Ice.Optional<int[]> p1, Ice.Current current)
    {
        cb.ice_response(p1.get(), p1.get());
    }

    public void
    opIntSeqOpt_async(AMD_Initial_opIntSeqOpt cb, Ice.Optional<int[]> p1, Ice.Current current)
    {
        cb.ice_response(p1, p1);
    }

    public void
    opLongSeq_async(AMD_Initial_opLongSeq cb, Ice.Optional<long[]> p1, Ice.Current current)
    {
        cb.ice_response(p1.get(), p1.get());
    }

    public void
    opLongSeqOpt_async(AMD_Initial_opLongSeqOpt cb, Ice.Optional<long[]> p1, Ice.Current current)
    {
        cb.ice_response(p1, p1);
    }

    public void
    opFloatSeq_async(AMD_Initial_opFloatSeq cb, Ice.Optional<float[]> p1, Ice.Current current)
    {
        cb.ice_response(p1.get(), p1.get());
    }

    public void
    opFloatSeqOpt_async(AMD_Initial_opFloatSeqOpt cb, Ice.Optional<float[]> p1, Ice.Current current)
    {
        cb.ice_response(p1, p1);
    }

    public void
    opDoubleSeq_async(AMD_Initial_opDoubleSeq cb, Ice.Optional<double[]> p1, Ice.Current current)
    {
        cb.ice_response(p1.get(), p1.get());
    }

    public void
    opDoubleSeqOpt_async(AMD_Initial_opDoubleSeqOpt cb, Ice.Optional<double[]> p1, Ice.Current current)
    {
        cb.ice_response(p1, p1);
    }

    public void
    opStringSeq_async(AMD_Initial_opStringSeq cb, Ice.Optional<String[]> p1, Ice.Current current)
    {
        cb.ice_response(p1.get(), p1.get());
    }

    public void
    opStringSeqOpt_async(AMD_Initial_opStringSeqOpt cb, Ice.Optional<String[]> p1, Ice.Current current)
    {
        cb.ice_response(p1, p1);
    }

    public void
    opSmallStructSeq_async(AMD_Initial_opSmallStructSeq cb, Ice.Optional<SmallStruct[]> p1, Ice.Current current)
    {
        cb.ice_response(p1.get(), p1.get());
    }

    public void
    opSmallStructSeqOpt_async(AMD_Initial_opSmallStructSeqOpt cb, Ice.Optional<SmallStruct[]> p1, Ice.Current current)
    {
        cb.ice_response(p1, p1);
    }

    public void
    opSmallStructList_async(AMD_Initial_opSmallStructList cb, Ice.Optional<java.util.List<SmallStruct>> p1,
                            Ice.Current current)
    {
        cb.ice_response(p1.get(), p1.get());
    }

    public void
    opSmallStructListOpt_async(AMD_Initial_opSmallStructListOpt cb, Ice.Optional<java.util.List<SmallStruct>> p1,
                               Ice.Current current)
    {
        cb.ice_response(p1, p1);
    }

    public void
    opFixedStructSeq_async(AMD_Initial_opFixedStructSeq cb, Ice.Optional<FixedStruct[]> p1, Ice.Current current)
    {
        cb.ice_response(p1.get(), p1.get());
    }

    public void
    opFixedStructSeqOpt_async(AMD_Initial_opFixedStructSeqOpt cb, Ice.Optional<FixedStruct[]> p1, Ice.Current current)
    {
        cb.ice_response(p1, p1);
    }

    public void
    opFixedStructList_async(AMD_Initial_opFixedStructList cb, Ice.Optional<java.util.List<FixedStruct>> p1,
                            Ice.Current current)
    {
        cb.ice_response(p1.get(), p1.get());
    }

    public void
    opFixedStructListOpt_async(AMD_Initial_opFixedStructListOpt cb, Ice.Optional<java.util.List<FixedStruct>> p1,
                               Ice.Current current)
    {
        cb.ice_response(p1, p1);
    }

    public void
    opVarStructSeq_async(AMD_Initial_opVarStructSeq cb, Ice.Optional<VarStruct[]> p1, Ice.Current current)
    {
        cb.ice_response(p1.get(), p1.get());
    }

    public void
    opVarStructSeqOpt_async(AMD_Initial_opVarStructSeqOpt cb, Ice.Optional<VarStruct[]> p1, Ice.Current current)
    {
        cb.ice_response(p1, p1);
    }

    public void
    opSerializable_async(AMD_Initial_opSerializable cb, Ice.Optional<SerializableClass> p1, Ice.Current current)
    {
        cb.ice_response(p1.get(), p1.get());
    }

    public void
    opSerializableOpt_async(AMD_Initial_opSerializableOpt cb, Ice.Optional<SerializableClass> p1, Ice.Current current)
    {
        cb.ice_response(p1, p1);
    }

    public void
    opIntIntDict_async(AMD_Initial_opIntIntDict cb, Ice.Optional<java.util.Map<Integer, Integer>> p1,
                       Ice.Current current)
    {
        cb.ice_response(p1.get(), p1.get());
    }

    public void
    opIntIntDictOpt_async(AMD_Initial_opIntIntDictOpt cb, Ice.Optional<java.util.Map<Integer, Integer>> p1,
                          Ice.Current current)
    {
        cb.ice_response(p1, p1);
    }

    public void
    opStringIntDict_async(AMD_Initial_opStringIntDict cb, Ice.Optional<java.util.Map<String, Integer>> p1,
                          Ice.Current current)
    {
        cb.ice_response(p1.get(), p1.get());
    }

    public void
    opStringIntDictOpt_async(AMD_Initial_opStringIntDictOpt cb, Ice.Optional<java.util.Map<String, Integer>> p1,
                             Ice.Current current)
    {
        cb.ice_response(p1, p1);
    }

    public void
    opClassAndUnknownOptional_async(AMD_Initial_opClassAndUnknownOptional cb, A p, Ice.Current current)
    {
        cb.ice_response();
    }
}
