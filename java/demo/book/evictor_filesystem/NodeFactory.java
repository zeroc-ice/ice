// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import Filesystem.*;

public class NodeFactory implements Ice.ObjectFactory
{
    public Ice.Object
    create(String type)
    {
        if(type.equals(PersistentFile.ice_staticId()))
        {
            return new FileI();
        }
        else if(type.equals(PersistentDirectory.ice_staticId()))
        {
            return new DirectoryI();
        }
        else
        {
            assert(false);
            return null;
        }
    }

    public void
    destroy()
    {
    }
}
