// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

package Ice;

public interface Object
{
    boolean equals(java.lang.Object rhs);

    java.lang.Object clone() throws java.lang.CloneNotSupportedException;

    int ice_hash();

    boolean ice_isA(String s, Current current);

    void ice_ping(Current current);

    String[] ice_ids(Current current);

    String ice_id(Current current);

    void ice_preMarshal();
    void ice_postUnmarshal();

    IceInternal.DispatchStatus __dispatch(IceInternal.Incoming in, Current current);

    void __write(IceInternal.BasicStream __os);
    void __read(IceInternal.BasicStream __is, boolean __rid);
}
