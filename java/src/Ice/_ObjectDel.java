// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

package Ice;

public interface _ObjectDel
{
    boolean ice_isA(String id)
        throws LocationForward, IceInternal.NonRepeatable;

    void ice_ping()
        throws LocationForward, IceInternal.NonRepeatable;

    void ice_flush();
}
