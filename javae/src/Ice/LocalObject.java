// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

package Ice;

public interface LocalObject
{
    boolean equals(java.lang.Object rhs);

    java.lang.Object ice_clone() throws IceUtil.CloneException;

    int ice_hash();
}
