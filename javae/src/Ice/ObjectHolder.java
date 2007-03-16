// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

package Ice;

public final class ObjectHolder
{
    public
    ObjectHolder()
    {
    }

    public
    ObjectHolder(Ice.Object value)
    {
        this.value = value;
    }

    public Ice.Object value;
}
