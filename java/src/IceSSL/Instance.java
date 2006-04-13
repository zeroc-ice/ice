// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceSSL;

class Instance
{
    Instance(Ice.Communicator communicator)
    {
	_facade = Ice.Util.getProtocolPluginFacade(communicator);
	_securityTraceLevel = communicator.getProperties().getPropertyAsIntWithDefault("IceSSL.Trace.Security", 0);
	_securityTraceCategory = "Security";

	// 
	// Initialize the plugin, unless IceSSL.DelayInit=1.
	//
	if(communicator.getProperties().getPropertyAsInt("IceSSL.DelayInit") == 0)
	{   
	    initialize(null, null);
	}

	// 
	// Register the endpoint factory.
	//
	_facade.addEndpointFactory(new EndpointFactoryI(this));
    }

    void
    initialize(javax.net.ssl.SSLContext clientContext, javax.net.ssl.SSLContext serverContext)
    {
	if(_clientContext != null)
	{   
	    Ice.PluginInitializationException e = new Ice.PluginInitializationException();
	    e.reason = "plugin is already initialized";
	    throw e;
	}

	//
	// If we have to initialize an SSLContext, we'll need a SecureRandom object.
	//
	java.security.SecureRandom rand = null;
	if(clientContext == null || serverContext == null)
	{
	    try
	    {
		rand = java.security.SecureRandom.getInstance("SHA1PRNG");

		//
		// We call nextInt() in order to force the object to perform any time-consuming
		// initialization tasks now.
		//
		rand.nextInt();
	    }
	    catch(java.security.GeneralSecurityException ex)
	    {
		Ice.PluginInitializationException e = new Ice.PluginInitializationException();
		e.reason = "IceSSL: unable to initialize secure PRNG";
		e.initCause(ex);
		throw e;
	    }
	}

	//
	// Create the client and server contexts. We always create both, even
	// if only one is used.
	//
	try
	{
	    _clientContext = new Context(this, true, clientContext, rand);
	}
	catch(java.security.GeneralSecurityException ex)
	{
	    Ice.PluginInitializationException e = new Ice.PluginInitializationException();
	    e.reason = "IceSSL: unable to initialize client context";
	    e.initCause(ex);
	    throw e;
	}

	try
	{
	    _serverContext = new Context(this, false, serverContext, rand);
	}
	catch(java.security.GeneralSecurityException ex)
	{
	    Ice.PluginInitializationException e = new Ice.PluginInitializationException();
	    e.reason = "IceSSL: unable to initialize server context";
	    e.initCause(ex);
	    throw e;
	}
    }

    void
    setCertificateVerifier(CertificateVerifier verifier)
    {
	_verifier = verifier;
    }

    Ice.Communicator
    communicator()
    {
	return _facade.getCommunicator();
    }

    String
    defaultHost()
    {
	return _facade.getDefaultHost();
    }

    int
    networkTraceLevel()
    {
	return _facade.getNetworkTraceLevel();
    }

    String
    networkTraceCategory()
    {
	return _facade.getNetworkTraceCategory();
    }

    int
    securityTraceLevel()
    {
	return _securityTraceLevel;
    }

    String
    securityTraceCategory()
    {
	return _securityTraceCategory;
    }

    Context
    clientContext()
    {
	if(_clientContext == null)
	{
	    Ice.PluginInitializationException e = new Ice.PluginInitializationException();
	    e.reason = "IceSSL: plugin is not fully initialized";
	    throw e;
	}
	return _clientContext;
    }

    Context
    serverContext()
    {
	if(_serverContext == null)
	{
	    Ice.PluginInitializationException e = new Ice.PluginInitializationException();
	    e.reason = "IceSSL: plugin is not fully initialized";
	    throw e;
	}
	return _serverContext;
    }

    CertificateVerifier
    certificateVerifier()
    {   
	return _verifier;
    }

    private IceInternal.ProtocolPluginFacade _facade;
    private int _securityTraceLevel;
    private String _securityTraceCategory;
    private Context _clientContext;
    private Context _serverContext;
    private CertificateVerifier _verifier;
}
