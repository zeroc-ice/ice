// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

public sealed class ServantLocatorI : Ice.LocalObjectImpl, Ice.ServantLocator
{
    public Ice.Object locate(Ice.Current curr, out Ice.LocalObject cookie)
    {
	cookie = null;
	return null;
    }
    
    public void finished(Ice.Current curr, Ice.Object servant, Ice.LocalObject cookie)
    {
    }
    
    public void deactivate(string category)
    {
    }
}
