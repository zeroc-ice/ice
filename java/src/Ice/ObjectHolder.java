// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
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

    public class Patcher implements IceInternal.Patcher
    {
        public void
        patch(Ice.Object v)
        {
            value = v;
        }

        public String
        type()
        {
            return Ice.ObjectImpl.ice_staticId();
        }
    }

    public Patcher
    getPatcher()
    {
        return new Patcher();
    }

    public Ice.Object value;
}
