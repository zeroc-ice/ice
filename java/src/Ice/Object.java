// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package Ice;

public interface Object
{
    boolean equals(java.lang.Object rhs);

    java.lang.Object clone() throws java.lang.CloneNotSupportedException;

    int ice_hash();

    boolean ice_isA(String s);
    boolean ice_isA(String s, Current current);

    void ice_ping();
    void ice_ping(Current current);

    String[] ice_ids();
    String[] ice_ids(Current current);

    String ice_id();
    String ice_id(Current current);

    int ice_operationAttributes(String operation);

    void ice_preMarshal();
    void ice_postUnmarshal();

    DispatchStatus ice_dispatch(Request request, DispatchInterceptorAsyncCallback cb);

    DispatchStatus __dispatch(IceInternal.Incoming in, Current current);

    DispatchStatus __collocDispatch(IceInternal.Direct request);

    void __write(IceInternal.BasicStream __os);
    void __read(IceInternal.BasicStream __is, boolean __rid);

    void __write(OutputStream __outS);
    void __read(InputStream __inS, boolean __rid);
}
