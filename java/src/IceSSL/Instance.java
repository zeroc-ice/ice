// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
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

	java.security.SecureRandom rand;
	try
	{
	    //
	    // Create a SecureRandom object. We call nextInt() in order to
	    // force the object to perform any time-consuming initialization tasks now.
	    //
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

	try
	{
	    _clientContext = createContext("Client", rand);
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
	    _serverContext = createContext("Server", rand);
	}
	catch(java.security.GeneralSecurityException ex)
	{
	    Ice.PluginInitializationException e = new Ice.PluginInitializationException();
	    e.reason = "IceSSL: unable to initialize server context";
	    e.initCause(ex);
	    throw e;
	}

	_facade.addEndpointFactory(new SslEndpointFactory(this));
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
	return _clientContext;
    }

    Context
    serverContext()
    {
	return _serverContext;
    }

    private Context
    createContext(String mode, java.security.SecureRandom rand)
	throws java.security.GeneralSecurityException
    {
	final String prefix = "IceSSL." + mode + ".";
	Ice.Properties properties = communicator().getProperties();
	String ciphers = properties.getProperty(prefix + "Ciphers");
	String keyStore = properties.getProperty(prefix + "Keystore");
	String password = properties.getProperty(prefix + "Password");
	String keyStorePassword = properties.getProperty(prefix + "KeystorePassword");
	String certs = properties.getProperty(prefix + "Certs");
	String certsPassword = properties.getProperty(prefix + "CertsPassword");
	return new Context(ciphers, keyStore, password, keyStorePassword, certs, certsPassword, rand);
    }

    private IceInternal.ProtocolPluginFacade _facade;
    private int _securityTraceLevel;
    private String _securityTraceCategory;
    private Context _clientContext;
    private Context _serverContext;
}
