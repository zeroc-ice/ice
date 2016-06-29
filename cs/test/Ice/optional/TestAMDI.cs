// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Collections.Generic;

public class InitialI : Test.Initial
{
    public override void shutdown_async(Test.AMD_Initial_shutdown cb, Ice.Current current)
    {
        current.adapter.getCommunicator().shutdown();
        cb.ice_response();
    }

    public override void pingPong_async(Test.AMD_Initial_pingPong cb, Ice.Object obj, Ice.Current current)
    {
        cb.ice_response(obj);
    }

    public override void opOptionalException_async(Test.AMD_Initial_opOptionalException cb, Ice.Optional<int> a,
                                                   Ice.Optional<string> b, Ice.Optional<Test.OneOptional> o,
                                                   Ice.Current current)
    {
        cb.ice_exception(new Test.OptionalException(false, a, b, o));
    }

    public override void opDerivedException_async(Test.AMD_Initial_opDerivedException cb, Ice.Optional<int> a,
                                                  Ice.Optional<string> b, Ice.Optional<Test.OneOptional> o,
                                                  Ice.Current current)
    {
        cb.ice_exception(new Test.DerivedException(false, a, b, o, b, o));
    }

    public override void opRequiredException_async(Test.AMD_Initial_opRequiredException cb, Ice.Optional<int> a,
                                                   Ice.Optional<string> b, Ice.Optional<Test.OneOptional> o,
                                                   Ice.Current current)
    {
        Test.RequiredException e = new Test.RequiredException();
        e.a = a;
        e.b = b;
        e.o = o;
        if(b.HasValue)
        {
            e.ss = b.Value;
        }
        if(o.HasValue)
        {
            e.o2 = o.Value;
        }
        cb.ice_exception(e);
    }

    public override void opByte_async(Test.AMD_Initial_opByte cb, Ice.Optional<byte> p1, Ice.Current current)
    {
        cb.ice_response(p1, p1);
    }

    public override void opBool_async(Test.AMD_Initial_opBool cb, Ice.Optional<bool> p1, Ice.Current current)
    {
        cb.ice_response(p1, p1);
    }

    public override void opShort_async(Test.AMD_Initial_opShort cb, Ice.Optional<short> p1, Ice.Current current)
    {
        cb.ice_response(p1, p1);
    }

    public override void opInt_async(Test.AMD_Initial_opInt cb, Ice.Optional<int> p1, Ice.Current current)
    {
        cb.ice_response(p1, p1);
    }

    public override void opLong_async(Test.AMD_Initial_opLong cb, Ice.Optional<long> p1, Ice.Current current)
    {
        cb.ice_response(p1, p1);
    }

    public override void opFloat_async(Test.AMD_Initial_opFloat cb, Ice.Optional<float> p1, Ice.Current current)
    {
        cb.ice_response(p1, p1);
    }

    public override void opDouble_async(Test.AMD_Initial_opDouble cb, Ice.Optional<double> p1, Ice.Current current)
    {
        cb.ice_response(p1, p1);
    }

    public override void opString_async(Test.AMD_Initial_opString cb, Ice.Optional<string> p1, Ice.Current current)
    {
        cb.ice_response(p1, p1);
    }

    public override void opMyEnum_async(Test.AMD_Initial_opMyEnum cb, Ice.Optional<Test.MyEnum> p1, Ice.Current current)
    {
        cb.ice_response(p1, p1);
    }

    public override void opSmallStruct_async(Test.AMD_Initial_opSmallStruct cb, Ice.Optional<Test.SmallStruct> p1,
                                             Ice.Current current)
    {
        cb.ice_response(p1, p1);
    }

    public override void opFixedStruct_async(Test.AMD_Initial_opFixedStruct cb, Ice.Optional<Test.FixedStruct> p1,
                                             Ice.Current current)
    {
        cb.ice_response(p1, p1);
    }

    public override void opVarStruct_async(Test.AMD_Initial_opVarStruct cb, Ice.Optional<Test.VarStruct> p1,
                                           Ice.Current current)
    {
        cb.ice_response(p1, p1);
    }

    public override void opOneOptional_async(Test.AMD_Initial_opOneOptional cb, Ice.Optional<Test.OneOptional> p1,
                                             Ice.Current current)
    {
        cb.ice_response(p1, p1);
    }

    public override void opOneOptionalProxy_async(Test.AMD_Initial_opOneOptionalProxy cb,
                                                  Ice.Optional<Test.OneOptionalPrx> p1,
                                                  Ice.Current current)
    {
        cb.ice_response(p1, p1);
    }

    public override void opByteSeq_async(Test.AMD_Initial_opByteSeq cb, Ice.Optional<byte[]> p1, Ice.Current current)
    {
        cb.ice_response(p1, p1);
    }

    public override void opBoolSeq_async(Test.AMD_Initial_opBoolSeq cb, Ice.Optional<bool[]> p1, Ice.Current current)
    {
        cb.ice_response(p1, p1);
    }

    public override void opShortSeq_async(Test.AMD_Initial_opShortSeq cb, Ice.Optional<short[]> p1, Ice.Current current)
    {
        cb.ice_response(p1, p1);
    }

    public override void opIntSeq_async(Test.AMD_Initial_opIntSeq cb, Ice.Optional<int[]> p1, Ice.Current current)
    {
        cb.ice_response(p1, p1);
    }

    public override void opLongSeq_async(Test.AMD_Initial_opLongSeq cb, Ice.Optional<long[]> p1, Ice.Current current)
    {
        cb.ice_response(p1, p1);
    }

    public override void opFloatSeq_async(Test.AMD_Initial_opFloatSeq cb, Ice.Optional<float[]> p1, Ice.Current current)
    {
        cb.ice_response(p1, p1);
    }

    public override void opDoubleSeq_async(Test.AMD_Initial_opDoubleSeq cb, Ice.Optional<double[]> p1,
                                           Ice.Current current)
    {
        cb.ice_response(p1, p1);
    }

    public override void opStringSeq_async(Test.AMD_Initial_opStringSeq cb, Ice.Optional<string[]> p1,
                                           Ice.Current current)
    {
        cb.ice_response(p1, p1);
    }

    public override void opSmallStructSeq_async(Test.AMD_Initial_opSmallStructSeq cb,
                                                Ice.Optional<Test.SmallStruct[]> p1,
                                                Ice.Current current)
    {
        cb.ice_response(p1, p1);
    }

    public override void opSmallStructList_async(Test.AMD_Initial_opSmallStructList cb,
                                                 Ice.Optional<List<Test.SmallStruct>> p1,
                                                 Ice.Current current)
    {
        cb.ice_response(p1, p1);
    }

    public override void opFixedStructSeq_async(Test.AMD_Initial_opFixedStructSeq cb,
                                                Ice.Optional<Test.FixedStruct[]> p1,
                                                Ice.Current current)
    {
        cb.ice_response(p1, p1);
    }

    public override void opFixedStructList_async(Test.AMD_Initial_opFixedStructList cb,
                                                 Ice.Optional<LinkedList<Test.FixedStruct>> p1,
                                                 Ice.Current current)
    {
        cb.ice_response(p1, p1);
    }

    public override void opVarStructSeq_async(Test.AMD_Initial_opVarStructSeq cb, Ice.Optional<Test.VarStruct[]> p1,
                                              Ice.Current current)
    {
        cb.ice_response(p1, p1);
    }

#if COMPACT || SILVERLIGHT
    public override void opSerializable_async(Test.AMD_Initial_opSerializable cb,
                                              Ice.Optional<byte[]> p1,
                                              Ice.Current current)
    {
        cb.ice_response(p1, p1);
    }
#else
    public override void opSerializable_async(Test.AMD_Initial_opSerializable cb,
                                              Ice.Optional<Test.SerializableClass> p1,
                                              Ice.Current current)
    {
        cb.ice_response(p1, p1);
    }
#endif

    public override void opIntIntDict_async(Test.AMD_Initial_opIntIntDict cb, Ice.Optional<Dictionary<int, int>> p1,
                                            Ice.Current current)
    {
        cb.ice_response(p1, p1);
    }

    public override void opStringIntDict_async(Test.AMD_Initial_opStringIntDict cb,
                                               Ice.Optional<Dictionary<string, int>> p1,
                                               Ice.Current current)
    {
        cb.ice_response(p1, p1);
    }

    public override void opClassAndUnknownOptional_async(Test.AMD_Initial_opClassAndUnknownOptional cb, Test.A p,
                                                         Ice.Current current)
    {
        cb.ice_response();
    }

    public override void sendOptionalClass_async(Test.AMD_Initial_sendOptionalClass cb, bool req,
                                                 Ice.Optional<Test.OneOptional> o, Ice.Current current)
    {
        cb.ice_response();
    }

    public override void returnOptionalClass_async(Test.AMD_Initial_returnOptionalClass cb, bool req,
                                                   Ice.Current current)
    {
        cb.ice_response(new Test.OneOptional(53));
    }

    public override void opVoid_async(Test.AMD_Initial_opVoid cb, Ice.Current current)
    {
        cb.ice_response();
    }

    public override void supportsRequiredParams_async(Test.AMD_Initial_supportsRequiredParams cb, Ice.Current current)
    {
        cb.ice_response(false);
    }

    public override void supportsJavaSerializable_async(Test.AMD_Initial_supportsJavaSerializable cb,
                                                        Ice.Current current)
    {
        cb.ice_response(false);
    }

    public override void supportsCsharpSerializable_async(Test.AMD_Initial_supportsCsharpSerializable cb,
                                                          Ice.Current current)
    {
#if COMPACT || SILVERLIGHT
        cb.ice_response(false);
#else
        cb.ice_response(true);
#endif
    }
}
