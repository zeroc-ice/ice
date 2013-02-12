// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package Ice;

import Ice.Instrumentation.InvocationObserver;

public interface _ObjectDel
{
    boolean ice_isA(String id, java.util.Map<String, String> context, InvocationObserver obsv)
        throws IceInternal.LocalExceptionWrapper;

    void ice_ping(java.util.Map<String, String> context, InvocationObserver obsv)
        throws IceInternal.LocalExceptionWrapper;

    String[] ice_ids(java.util.Map<String, String> context, InvocationObserver obsv)
        throws IceInternal.LocalExceptionWrapper;

    String ice_id(java.util.Map<String, String> context, InvocationObserver obsv)
        throws IceInternal.LocalExceptionWrapper;

    boolean ice_invoke(String operation, Ice.OperationMode mode, byte[] inParams, ByteSeqHolder outParams,
                       java.util.Map<String, String> context, InvocationObserver obsv)
        throws IceInternal.LocalExceptionWrapper;

    void ice_flushBatchRequests(InvocationObserver obsv);
    
    IceInternal.RequestHandler __getRequestHandler();
    void __setRequestHandler(IceInternal.RequestHandler handler);
}
