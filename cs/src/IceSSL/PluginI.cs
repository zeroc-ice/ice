// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace IceSSL
{
    public sealed class PluginFactory : Ice.PluginFactory
    {
	public Ice.Plugin
	create(Ice.Communicator communicator, string name, string[] args)
	{
	    if(communicator.getProperties().getPropertyAsInt("Ice.ThreadPerConnection") == 0)
	    {
		communicator.getLogger().error("IceSSL requires Ice.ThreadPerConnection=1");
		return null;
	    }

	    return new PluginI(communicator);
	}
    }

    public sealed class PluginI : Ice.LocalObjectImpl, Ice.Plugin
    {
	public
	PluginI(Ice.Communicator communicator)
	{
	    instance_ = new Instance(communicator);
	}

	public void
	destroy()
	{
	}

	private Instance instance_;
    }
}
