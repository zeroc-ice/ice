// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import Filesystem.*;

public class NodeFactory extends Ice.LocalObjectImpl implements Ice.ObjectFactory
{
    public Ice.Object
    create(String type)
    {
        if(type.equals("::Filesystem::PersistentFile"))
        {
            return new FileI();
        }
        else if(type.equals("::Filesystem::PersistentDirectory"))
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
