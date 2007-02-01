// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceSSL;

public class PluginFactory implements Ice.PluginFactory
{
    public Ice.Plugin
    create(Ice.Communicator communicator, String name, String[] args)
    {
        return new PluginI(communicator);
    }
}
