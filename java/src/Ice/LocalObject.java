// **********************************************************************
//
// Copyright (c) 2003 - 2004
// ZeroC, Inc.
// North Palm Beach, FL, USA
//
// All Rights Reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************


package Ice;

public interface LocalObject
{
    boolean equals(java.lang.Object rhs);

    java.lang.Object clone() throws java.lang.CloneNotSupportedException;

    int ice_hash();
}
