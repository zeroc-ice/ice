// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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
