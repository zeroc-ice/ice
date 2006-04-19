// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System.Security.Cryptography.X509Certificates;

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

    public sealed class PluginI : Plugin
    {
	public
	PluginI(Ice.Communicator communicator)
	{
	    instance_ = new Instance(communicator);
	}

	public override void
	destroy()
	{
	}

	public override void initialize(X509Certificate2Collection certs)
	{
	    instance_.initialize(certs);
	}

	public override void setCertificateVerifier(CertificateVerifier verifier)
	{
	    instance_.setCertificateVerifier(verifier);
	}

	private Instance instance_;
    }
}
