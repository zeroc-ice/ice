// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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

    boolean ice_invoke(String operation, Ice.OperationMode mode, byte[] inParams, ByteSeqHolder outParams,
                       java.util.Map context)
        throws IceInternal.NonRepeatable;

    Connection ice_connection();
}
