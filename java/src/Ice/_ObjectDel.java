// **********************************************************************
//
// Copyright (c) 2001
// ZeroC, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

package Ice;

public interface _ObjectDel
{
    boolean ice_isA(String __id, java.util.Map __context)
        throws LocationForward, IceInternal.NonRepeatable;

    void ice_ping(java.util.Map __context)
        throws LocationForward, IceInternal.NonRepeatable;

    String[] ice_ids(java.util.Map __context)
        throws LocationForward, IceInternal.NonRepeatable;

    String ice_id(java.util.Map __context)
        throws LocationForward, IceInternal.NonRepeatable;

    String[] ice_facets(java.util.Map __context)
        throws LocationForward, IceInternal.NonRepeatable;

    boolean ice_invoke(String operation, Ice.OperationMode mode, byte[] inParams, ByteSeqHolder outParams,
                       java.util.Map context)
        throws LocationForward, IceInternal.NonRepeatable;

    void ice_flush();
}
