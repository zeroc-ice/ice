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

public interface LocalObject
{
    boolean equals(java.lang.Object rhs);

    java.lang.Object clone() throws java.lang.CloneNotSupportedException;

    int ice_hash();
}
