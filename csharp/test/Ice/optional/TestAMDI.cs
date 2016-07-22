// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Collections.Generic;
using Test;

public class InitialI : Test.Initial
{
    public override void shutdownAsync(Action response, Action<Exception> exception, Ice.Current current)
    {
        current.adapter.getCommunicator().shutdown();
        response();
    }

    public override void pingPongAsync(Ice.Object obj, Action<Ice.Object> response, 
                                       Action<Exception> exception, Ice.Current current)
    {
        response(obj);
    }

    public override void opOptionalExceptionAsync(Ice.Optional<int> a, Ice.Optional<string> b, Ice.Optional<OneOptional> o, 
                                                  Action action, Action<Exception> exception, Ice.Current current)
    {
        exception(new OptionalException(false, a, b, o));
    }

    public override void opDerivedExceptionAsync(Ice.Optional<int> a, Ice.Optional<string> b, Ice.Optional<OneOptional> o,
                                                 Action response, Action<Exception> exception, Ice.Current current)
    {
        exception(new DerivedException(false, a, b, o, b, o));
    }

    public override void opRequiredExceptionAsync(Ice.Optional<int> a, Ice.Optional<string> b, Ice.Optional<OneOptional> o, 
                                                  Action response, Action<Exception> exception, Ice.Current current)
    {
        var e = new RequiredException();
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
        exception(e);
    }

    public override void
    opByteAsync(Ice.Optional<byte> p1, Action<Initial_OpByteResult> response, Action<Exception> exception,
                Ice.Current current)
    {
        response(new Initial_OpByteResult(p1, p1));
    }

    public override void
    opBoolAsync(Ice.Optional<bool> p1, Action<Initial_OpBoolResult> response, Action<Exception> exception,
                Ice.Current current)
    {
        response(new Initial_OpBoolResult(p1, p1));
    }

    public override void
    opShortAsync(Ice.Optional<short> p1, Action<Initial_OpShortResult> response, Action<Exception> exception,
                 Ice.Current current)
    {
        response(new Initial_OpShortResult(p1, p1));
    }

    public override void
    opIntAsync(Ice.Optional<int> p1, Action<Initial_OpIntResult> response, Action<Exception> exception,
               Ice.Current current)
    {
        response(new Initial_OpIntResult(p1, p1));
    }

    public override void
    opLongAsync(Ice.Optional<long> p1, Action<Initial_OpLongResult> response, Action<Exception> exception,
                Ice.Current current)
    {
        response(new Initial_OpLongResult(p1, p1));
    }

    public override void
    opFloatAsync(Ice.Optional<float> p1, Action<Initial_OpFloatResult> response, Action<Exception> exception,
                 Ice.Current current)
    {
        response(new Initial_OpFloatResult(p1, p1));
    }

    public override void
    opDoubleAsync(Ice.Optional<double> p1, Action<Initial_OpDoubleResult> response, Action<Exception> exception,
                  Ice.Current current)
    {
        response(new Initial_OpDoubleResult(p1, p1));
    }

    public override void
    opStringAsync(Ice.Optional<string> p1, Action<Initial_OpStringResult> response, Action<Exception> exception,
                  Ice.Current current)
    {
        response(new Initial_OpStringResult(p1, p1));
    }

    public override void
    opMyEnumAsync(Ice.Optional<MyEnum> p1, Action<Initial_OpMyEnumResult> response, Action<Exception> exception,
                  Ice.Current current)
    {
        response(new Initial_OpMyEnumResult(p1, p1));
    }

    public override void
    opSmallStructAsync(Ice.Optional<SmallStruct> p1, Action<Initial_OpSmallStructResult> response,
                       Action<Exception> exception, Ice.Current current)
    {
        response(new Initial_OpSmallStructResult(p1, p1));
    }

    public override void
    opFixedStructAsync(Ice.Optional<FixedStruct> p1, Action<Initial_OpFixedStructResult> response,
                       Action<Exception> exception, Ice.Current current)
    {
        response(new Initial_OpFixedStructResult(p1, p1));
    }

    public override void
    opVarStructAsync(Ice.Optional<VarStruct> p1, Action<Initial_OpVarStructResult> response,
                     Action<Exception> exception, Ice.Current current)
    {
        response(new Initial_OpVarStructResult(p1, p1));
    }

    public override void
    opOneOptionalAsync(Ice.Optional<OneOptional> p1, Action<Initial_OpOneOptionalResult> response,
                       Action<Exception> exception, Ice.Current current)
    {
        response(new Initial_OpOneOptionalResult(p1, p1));
    }

    public override void
    opOneOptionalProxyAsync(Ice.Optional<OneOptionalPrx> p1, Action<Initial_OpOneOptionalProxyResult> response, 
                            Action<Exception> exception, Ice.Current current)
    {
        response(new Initial_OpOneOptionalProxyResult(p1, p1));
    }

    public override void
    opByteSeqAsync(Ice.Optional<byte[]> p1, Action<Initial_OpByteSeqResult> response, Action<Exception> exception,
                   Ice.Current current)
    {
        response(new Initial_OpByteSeqResult(p1, p1));
    }

    public override void
    opBoolSeqAsync(Ice.Optional<bool[]> p1, Action<Initial_OpBoolSeqResult> response, Action<Exception> exception,
                   Ice.Current current)
    {
        response(new Initial_OpBoolSeqResult(p1, p1));
    }

    public override void
    opShortSeqAsync(Ice.Optional<short[]> p1, Action<Initial_OpShortSeqResult> response, Action<Exception> exception,
                    Ice.Current current)
    {
        response(new Initial_OpShortSeqResult(p1, p1));
    }

    public override void
    opIntSeqAsync(Ice.Optional<int[]> p1, Action<Initial_OpIntSeqResult> response, Action<Exception> exception,
                  Ice.Current current)
    {
        response(new Initial_OpIntSeqResult(p1, p1));
    }

    public override void
    opLongSeqAsync(Ice.Optional<long[]> p1, Action<Initial_OpLongSeqResult> response, Action<Exception> exception,
                   Ice.Current current)
    {
        response(new Initial_OpLongSeqResult(p1, p1));
    }

    public override void
    opFloatSeqAsync(Ice.Optional<float[]> p1, Action<Initial_OpFloatSeqResult> response, Action<Exception> exception,
                    Ice.Current current)
    {
        response(new Initial_OpFloatSeqResult(p1, p1));
    }

    public override void
    opDoubleSeqAsync(Ice.Optional<double[]> p1, Action<Initial_OpDoubleSeqResult> response, Action<Exception> exception,
                     Ice.Current current)
    {
        response(new Initial_OpDoubleSeqResult(p1, p1));
    }

    public override void
    opStringSeqAsync(Ice.Optional<string[]> p1, Action<Initial_OpStringSeqResult> response, Action<Exception> exception,
                     Ice.Current current)
    {
        response(new Initial_OpStringSeqResult(p1, p1));
    }

    public override void
    opSmallStructSeqAsync(Ice.Optional<SmallStruct[]> p1, Action<Initial_OpSmallStructSeqResult> response,
                          Action<Exception> exception, Ice.Current current)
    {
        response(new Initial_OpSmallStructSeqResult(p1, p1));
    }

    public override void
    opSmallStructListAsync(Ice.Optional<List<SmallStruct>> p1, Action<Initial_OpSmallStructListResult> response,
                           Action<Exception> exception, Ice.Current current)
    {
        response(new Initial_OpSmallStructListResult(p1, p1));
    }

    public override void
    opFixedStructSeqAsync(Ice.Optional<FixedStruct[]> p1, Action<Initial_OpFixedStructSeqResult> response,
                          Action<Exception> exception, Ice.Current current)
    {
        response(new Initial_OpFixedStructSeqResult(p1, p1));
    }

    public override void
    opFixedStructListAsync(Ice.Optional<LinkedList<FixedStruct>> p1, Action<Initial_OpFixedStructListResult> response, 
                           Action<Exception> exception, Ice.Current current)
    {
        response(new Initial_OpFixedStructListResult(p1, p1));
    }

    public override void
    opVarStructSeqAsync(Ice.Optional<VarStruct[]> p1, Action<Initial_OpVarStructSeqResult> response,
                        Action<Exception> exception, Ice.Current current)
    {
        response(new Initial_OpVarStructSeqResult(p1, p1));
    }

    public override void
    opSerializableAsync(Ice.Optional<SerializableClass> p1, Action<Initial_OpSerializableResult> response,
                        Action<Exception> exception, Ice.Current current)
    {
        response(new Initial_OpSerializableResult(p1, p1));
    }

    public override void
    opIntIntDictAsync(Ice.Optional<Dictionary<int, int>> p1, Action<Initial_OpIntIntDictResult> response,
                      Action<Exception> exception, Ice.Current current)
    {
        response(new Initial_OpIntIntDictResult(p1, p1));
    }

    public override void
    opStringIntDictAsync(Ice.Optional<Dictionary<string, int>> p1, Action<Initial_OpStringIntDictResult> response,
                         Action<Exception> exception, Ice.Current current)
    {
        response(new Initial_OpStringIntDictResult(p1, p1));
    }

    public override void
    opIntOneOptionalDictAsync(Ice.Optional<Dictionary<int, OneOptional>> p1, Action<Initial_OpIntOneOptionalDictResult> response, 
                              Action<Exception> exception, Ice.Current current)
    {
        response(new Initial_OpIntOneOptionalDictResult(p1, p1));
    }

    public override void opClassAndUnknownOptionalAsync(A p, Action response, Action<Exception> exception, Ice.Current current)
    {
        response();
    }

    public override void sendOptionalClassAsync(bool req, Ice.Optional<OneOptional> o, Action response, 
                                                Action<Exception> exception, Ice.Current current)
    {
        response();
    }

    public override void
    returnOptionalClassAsync(bool req, Action<Ice.Optional<OneOptional>> response, Action<Exception> exception, Ice.Current current)
    {
        response(new OneOptional(53));
    }

    public override void opGAsync(G g, Action<G> response, Action<Exception> exception, Ice.Current current)
    {
        response(g);
    }

    public override void opVoidAsync(Action response, Action<Exception> exception, Ice.Current current)
    {
        response();
    }

    public override void
    supportsRequiredParamsAsync(Action<bool> response, Action<Exception> exception, Ice.Current current)
    {
        response(false);
    }

    public override void
    supportsJavaSerializableAsync(Action<bool> response, Action<Exception> exception, Ice.Current current)
    {
        response(false);
    }

    public override void
    supportsCsharpSerializableAsync(Action<bool> response, Action<Exception> exception, Ice.Current current)
    {
        response(true);
    }

    public override void
    supportsCppStringViewAsync(Action<bool> response, Action<Exception> exception, Ice.Current current)
    {
        response(false);
    }
}
