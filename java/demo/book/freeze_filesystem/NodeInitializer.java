// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import Filesystem.*;

public class NodeInitializer extends Ice.LocalObjectImpl implements Freeze.ServantInitializer
{
    public void
    initialize(Ice.ObjectAdapter adapter, Ice.Identity id, String facet, Ice.Object obj)
    {
        if(obj instanceof FileI)
        {
            ((FileI)obj)._ID = id;
        }
        else if(obj instanceof DirectoryI)
        {
            ((DirectoryI)obj)._ID = id;
        }
    }
}
