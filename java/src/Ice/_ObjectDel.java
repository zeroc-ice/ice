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

public interface _ObjectDel
{
    boolean ice_isA(String id, java.util.Map context)
        throws IceInternal.NonRepeatable;

    void ice_ping(java.util.Map context)
        throws IceInternal.NonRepeatable;

    String[] ice_ids(java.util.Map context)
        throws IceInternal.NonRepeatable;

    String ice_id(java.util.Map context)
        throws IceInternal.NonRepeatable;

    String[] ice_facets(java.util.Map context)
        throws IceInternal.NonRepeatable;

    boolean ice_invoke(String operation, Ice.OperationMode mode, byte[] inParams, ByteSeqHolder outParams,
                       java.util.Map context)
        throws IceInternal.NonRepeatable;
}
