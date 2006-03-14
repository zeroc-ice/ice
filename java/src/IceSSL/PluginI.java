// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceSSL;

public class PluginI extends Ice.LocalObjectImpl implements Ice.Plugin
{
    public
    PluginI(Ice.Communicator communicator)
    {
	_instance = new Instance(communicator);
    }

    public void
    destroy()
    {
    }

    private Instance _instance;
}
