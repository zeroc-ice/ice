// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package Ice;

public interface Object
{
    boolean equals(java.lang.Object rhs);

    java.lang.Object ice_clone() throws IceUtil.CloneException;

    int ice_hash();

    boolean ice_isA(String s);
    boolean ice_isA(String s, Current current);

    void ice_ping();
    void ice_ping(Current current);

    String[] ice_ids();
    String[] ice_ids(Current current);

    String ice_id();
    String ice_id(Current current);

    IceInternal.DispatchStatus __dispatch(IceInternal.Incoming in, Current current);
}
