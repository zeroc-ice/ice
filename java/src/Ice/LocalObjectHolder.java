// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package Ice;

public final class LocalObjectHolder
{
    public
    LocalObjectHolder()
    {
    }

    public
    LocalObjectHolder(LocalObject value)
    {
        this.value = value;
    }

    public LocalObject value;
}
