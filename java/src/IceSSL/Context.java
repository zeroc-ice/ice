// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceSSL;

class Context
{
    Context(Instance instance, javax.net.ssl.SSLContext context, java.security.SecureRandom rand)
	throws java.security.GeneralSecurityException
    {
	_instance = instance;
	_logger = instance.communicator().getLogger();

	final String prefix = "IceSSL.";
	Ice.Properties properties = instance.communicator().getProperties();
	String ciphers = properties.getProperty(prefix + "Ciphers");

	if(ciphers.length() > 0)
	{
	    parseCiphers(ciphers);
	}

	//
	// If the user doesn't supply an SSLContext, we need to create one based
	// on property settings.
	//
	_ctx = context;
	if(_ctx == null)
	{
	    //
	    // Check for a default directory. We look in this directory for
	    // files mentioned in the configuration.
	    //
	    _defaultDir = properties.getProperty(prefix + "DefaultDir");

	    //
	    // The keystore holds private keys and associated certificates.
	    //
	    Ice.StringHolder keystorePath = new Ice.StringHolder(properties.getProperty(prefix + "Keystore"));

	    //
	    // The password for the keys.
	    //
	    final String password = properties.getProperty(prefix + "Password");

	    //
	    // The password for the keystore.
	    //
	    final String keystorePassword = properties.getProperty(prefix + "KeystorePassword");

	    //
	    // The default keystore type value is "JKS", but it can also be "PKCS12".
	    //
	    final String defaultType = java.security.KeyStore.getDefaultType();
	    final String keystoreType = properties.getPropertyWithDefault(prefix + "KeystoreType", defaultType);

	    //
	    // The alias of the key to use in authentication.
	    //
	    final String alias = properties.getProperty(prefix + "Alias");

	    //
	    // The truststore holds the certificates of trusted CAs.
	    //
	    Ice.StringHolder truststorePath = new Ice.StringHolder(
		properties.getPropertyWithDefault(prefix + "Truststore", properties.getProperty(prefix + "Certs")));

	    //
	    // The password for the truststore.
	    //
	    final String truststorePassword =
		properties.getPropertyWithDefault(prefix + "TruststorePassword",
						  properties.getProperty(prefix + "CertsPassword"));

	    //
	    // The truststore type defaults to "JKS", but it can also be "PKCS12".
	    //
	    String truststoreType =
		properties.getPropertyWithDefault(prefix + "TruststoreType", java.security.KeyStore.getDefaultType());

	    //
	    // Parse the enabled protocols.
	    //
	    String protocols = properties.getProperty(prefix + "Protocols");
	    if(protocols.length() > 0)
	    {
		java.util.ArrayList l = new java.util.ArrayList();
		String[] arr = protocols.split("[ \t,]+");
		for(int i = 0; i < arr.length; ++i)
		{
		    String s = arr[i].toLowerCase();
		    if(s.equals("ssl3") || s.equals("sslv3"))
		    {
			l.add("SSLv3");
		    }
		    else if(s.equals("tls") || s.equals("tls1") || s.equals("tlsv1"))
		    {
			l.add("TLSv1");
		    }
		    else
		    {
			Ice.PluginInitializationException e = new Ice.PluginInitializationException();
			e.reason = "IceSSL: unrecognized protocol `" + arr[i] + "'";
			throw e;
		    }
		}
		_protocols = new String[l.size()];
		l.toArray(_protocols);
	    }

	    //
	    // Collect the key managers.
	    //
	    javax.net.ssl.KeyManager[] keyManagers = null;
	    if(keystorePath.value.length() > 0)
	    {
		if(!checkPath(keystorePath, false))
		{
		    Ice.PluginInitializationException e = new Ice.PluginInitializationException();
		    e.reason = "IceSSL: keystore file not found:\n" + keystorePath.value;
		    throw e;
		}
		java.security.KeyStore keys = java.security.KeyStore.getInstance(keystoreType);
		try
		{
		    char[] passwordChars = null;
		    if(keystorePassword.length() > 0)
		    {
			passwordChars = keystorePassword.toCharArray();
		    }

		    java.io.BufferedInputStream bis =
			new java.io.BufferedInputStream(new java.io.FileInputStream(keystorePath.value));
		    keys.load(bis, passwordChars);
		}
		catch(java.io.IOException ex)
		{
		    Ice.PluginInitializationException e = new Ice.PluginInitializationException();
		    e.reason = "IceSSL: unable to load keystore:\n" + keystorePath.value;
		    e.initCause(ex);
		    throw e;
		}

		String algorithm = javax.net.ssl.KeyManagerFactory.getDefaultAlgorithm();
		javax.net.ssl.KeyManagerFactory kmf = javax.net.ssl.KeyManagerFactory.getInstance(algorithm);
		kmf.init(keys, password.toCharArray());
		keyManagers = kmf.getKeyManagers();

		//
		// If the user selected a specific alias, we need to wrap the key managers
		// in order to return the desired alias.
		//
		if(alias.length() > 0)
		{
		    if(!keys.isKeyEntry(alias))
		    {
			Ice.PluginInitializationException e = new Ice.PluginInitializationException();
			e.reason = "IceSSL: keystore does not contain an entry with alias `" + alias + "'";
			throw e;
		    }

		    for(int i = 0; i < keyManagers.length; ++i)
		    {
			keyManagers[i] = new KeyManagerI((javax.net.ssl.X509KeyManager)keyManagers[i], alias);
		    }
		}
	    }

	    //
	    // Collect the trust managers.
	    //
	    javax.net.ssl.TrustManager[] trustManagers = null;
	    if(truststorePath.value.length() > 0)
	    {
		if(!checkPath(truststorePath, false))
		{
		    Ice.PluginInitializationException e = new Ice.PluginInitializationException();
		    e.reason = "IceSSL: truststore file not found:\n" + truststorePath.value;
		    throw e;
		}
		java.security.KeyStore ts = java.security.KeyStore.getInstance(truststoreType);
		try
		{
		    char[] passwordChars = null;
		    if(truststorePassword.length() > 0)
		    {
			passwordChars = truststorePassword.toCharArray();
		    }

		    java.io.BufferedInputStream bis =
			new java.io.BufferedInputStream(new java.io.FileInputStream(truststorePath.value));
		    ts.load(bis, passwordChars);
		}
		catch(java.io.IOException ex)
		{
		    Ice.PluginInitializationException e = new Ice.PluginInitializationException();
		    e.reason = "IceSSL: unable to load truststore:\n" + truststorePath.value;
		    e.initCause(ex);
		    throw e;
		}

		String algorithm = javax.net.ssl.TrustManagerFactory.getDefaultAlgorithm();
		javax.net.ssl.TrustManagerFactory tmf = javax.net.ssl.TrustManagerFactory.getInstance(algorithm);
		tmf.init(ts);
		trustManagers = tmf.getTrustManagers();
	    }

	    //
	    // Initialize the SSL context.
	    //
	    _ctx = javax.net.ssl.SSLContext.getInstance("SSL");
	    _ctx.init(keyManagers, trustManagers, rand);
	}
    }

    javax.net.ssl.SSLContext
    sslContext()
    {
	return _ctx;
    }

    String[]
    filterCiphers(String[] supportedCiphers, String[] defaultCiphers)
    {
	java.util.LinkedList result = new java.util.LinkedList();
	if(_allCiphers)
	{
	    for(int i = 0; i < supportedCiphers.length; ++i)
	    {
		result.add(supportedCiphers[i]);
	    }
	}
	else if(!_noCiphers)
	{
	    for(int i = 0; i < defaultCiphers.length; ++i)
	    {
		result.add(defaultCiphers[i]);
	    }
	}

	if(_ciphers != null)
	{
	    for(int i = 0; i < _ciphers.length; ++i)
	    {
		CipherExpression ce = (CipherExpression)_ciphers[i];
		if(ce.not)
		{
		    java.util.Iterator e = result.iterator();
		    while(e.hasNext())
		    {
			String cipher = (String)e.next();
			if(ce.cipher != null)
			{
			    if(ce.cipher.equals(cipher))
			    {
				e.remove();
			    }
			}
			else
			{
			    assert(ce.re != null);
			    java.util.regex.Matcher m = ce.re.matcher(cipher);
			    if(m.find())
			    {
				e.remove();
			    }
			}
		    }
		}
		else
		{
		    if(ce.cipher != null)
		    {
			result.add(0, ce.cipher);
		    }
		    else
		    {
			assert(ce.re != null);
			for(int j = 0; j < supportedCiphers.length; ++j)
			{
			    java.util.regex.Matcher m = ce.re.matcher(supportedCiphers[j]);
			    if(m.find())
			    {
				result.add(0, supportedCiphers[j]);
			    }
			}
		    }
		}
	    }
	}

	String[] arr = new String[result.size()];
	result.toArray(arr);
	return arr;
    }

    String[]
    getProtocols()
    {
	return _protocols;
    }

    void
    traceConnection(javax.net.ssl.SSLSocket fd, boolean incoming)
    {
	javax.net.ssl.SSLSession session = fd.getSession();
	String msg = "SSL summary for " + (incoming ? "incoming" : "outgoing") + " connection\n" +
	    "cipher = " + session.getCipherSuite() + "\n" +
	    "protocol = " + session.getProtocol() + "\n" +
	    IceInternal.Network.fdToString(fd);
	_logger.trace(_instance.securityTraceCategory(), msg);
    }

    boolean
    verifyPeer(javax.net.ssl.SSLSocket fd, String host, boolean incoming)
    {
	CertificateVerifier verifier = _instance.certificateVerifier();
	if(verifier != null)
	{
	    VerifyInfo info = new VerifyInfo();
	    info.incoming = incoming;
	    try
	    {
		info.certs = fd.getSession().getPeerCertificates();
	    }
	    catch(javax.net.ssl.SSLPeerUnverifiedException ex)
	    {
		// No peer certificates.
	    }
	    info.socket = fd;
	    info.address = host;
	    if(!verifier.verify(info))
	    {
		if(_instance.securityTraceLevel() > 0)
		{
		    _logger.trace(_instance.securityTraceCategory(),
				  (incoming ? "incoming" : "outgoing") +
				  " connection rejected by certificate verifier\n" +
				  IceInternal.Network.fdToString(fd));
		}
		return false;
	    }
	}

	return true;
    }

    private void
    parseCiphers(String ciphers)
    {
	java.util.ArrayList cipherList = new java.util.ArrayList();
	String[] expr = ciphers.split("[ \t]+");
	for(int i = 0; i < expr.length; ++i)
	{
	    if(expr[i].equals("ALL"))
	    {
		if(i != 0)
		{
		    Ice.PluginInitializationException ex = new Ice.PluginInitializationException();
		    ex.reason = "IceSSL: `ALL' must be first in cipher list `" + ciphers + "'";
		    throw ex;
		}
		_allCiphers = true;
	    }
	    else if(expr[i].equals("NONE"))
	    {
		if(i != 0)
		{
		    Ice.PluginInitializationException ex = new Ice.PluginInitializationException();
		    ex.reason = "IceSSL: `NONE' must be first in cipher list `" + ciphers + "'";
		    throw ex;
		}
		_noCiphers = true;
	    }
	    else
	    {
		CipherExpression ce = new CipherExpression();
		String exp = expr[i];
		if(exp.charAt(0) == '!')
		{
		    ce.not = true;
		    if(exp.length() > 1)
		    {
			exp = exp.substring(1);
		    }
		    else
		    {
			Ice.PluginInitializationException ex = new Ice.PluginInitializationException();
			ex.reason = "IceSSL: invalid cipher expression `" + exp + "'";
			throw ex;
		    }
		}

		if(exp.charAt(0) == '(')
		{
		    if(!exp.endsWith(")"))
		    {
			Ice.PluginInitializationException ex = new Ice.PluginInitializationException();
			ex.reason = "IceSSL: invalid cipher expression `" + exp + "'";
			throw ex;
		    }

		    try
		    {
			ce.re = java.util.regex.Pattern.compile(exp.substring(1, exp.length() - 2));
		    }
		    catch(java.util.regex.PatternSyntaxException ex)
		    {
			Ice.PluginInitializationException e = new Ice.PluginInitializationException();
			e.reason = "IceSSL: invalid cipher expression `" + exp + "'";
			e.initCause(ex);
			throw e;
		    }
		}
		else
		{
		    ce.cipher = exp;
		}

		cipherList.add(ce);
	    }
	}
	_ciphers = new CipherExpression[cipherList.size()];
	cipherList.toArray(_ciphers);
    }

    private boolean
    checkPath(Ice.StringHolder path, boolean dir)
    {
	//
	// Check if file exists. If not, try prepending the default
	// directory and check again. If the file is found, the
	// string argument is modified and true is returned. Otherwise
	// false is returned.
	//
	java.io.File f = new java.io.File(path.value);
	if(f.exists())
	{
	    return dir ? f.isDirectory() : f.isFile();
	}

	if(_defaultDir.length() > 0)
	{
	    String s = _defaultDir + java.io.File.separator + path.value;
	    f = new java.io.File(s);
	    if(f.exists() && ((!dir && f.isFile()) || (dir && f.isDirectory())))
	    {
		path.value = s;
		return true;
	    }
	}

	return false;
    }

    private static class CipherExpression
    {
	boolean not;
	String cipher;
	java.util.regex.Pattern re;
    }

    private Instance _instance;
    private Ice.Logger _logger;
    private String _defaultDir;
    private CipherExpression[] _ciphers;
    private boolean _allCiphers;
    private boolean _noCiphers;
    private String[] _protocols;
    private javax.net.ssl.SSLContext _ctx;
}
