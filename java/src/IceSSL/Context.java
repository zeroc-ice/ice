// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceSSL;

class Context
{
    Context(String ciphers, String keyStore, String password, String keyStorePassword, String certs,
	    String certsPassword, java.security.SecureRandom rand)
	throws java.security.GeneralSecurityException
    {
	java.util.ArrayList cipherList = new java.util.ArrayList();
	if(ciphers.length() > 0)
	{
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

	final String ksType = java.security.KeyStore.getDefaultType();

	javax.net.ssl.KeyManager[] keyManagers = null;
	if(keyStore != null && keyStore.length() > 0)
	{
	    _keys = java.security.KeyStore.getInstance(ksType);
	    try
	    {
		char[] pass = null;
		if(keyStorePassword != null && keyStorePassword.length() > 0)
		{
		    pass = keyStorePassword.toCharArray();
		}

		java.io.BufferedInputStream bis =
		    new java.io.BufferedInputStream(new java.io.FileInputStream(keyStore));
		_keys.load(bis, pass);
	    }
	    catch(java.io.IOException ex)
	    {
		Ice.PluginInitializationException e = new Ice.PluginInitializationException();
		e.reason = "IceSSL: unable to load keystore from `" + keyStore + "'";
		e.initCause(ex);
		throw e;
	    }

	    String algorithm = javax.net.ssl.KeyManagerFactory.getDefaultAlgorithm();
	    javax.net.ssl.KeyManagerFactory kmf = javax.net.ssl.KeyManagerFactory.getInstance(algorithm);
	    kmf.init(_keys, password.toCharArray());
	    keyManagers = kmf.getKeyManagers();
	}

	javax.net.ssl.TrustManager[] trustManagers = null;
	if(certs != null && certs.length() > 0)
	{
	    _certs = java.security.KeyStore.getInstance(ksType);
	    try
	    {
		char[] pass = null;
		if(certsPassword != null && certsPassword.length() > 0)
		{
		    pass = certsPassword.toCharArray();
		}

		java.io.BufferedInputStream bis =
		    new java.io.BufferedInputStream(new java.io.FileInputStream(certs));
		_certs.load(bis, pass);
	    }
	    catch(java.io.IOException ex)
	    {
		Ice.PluginInitializationException e = new Ice.PluginInitializationException();
		e.reason = "IceSSL: unable to load keystore from `" + certs + "'";
		e.initCause(ex);
		throw e;
	    }

	    String algorithm = javax.net.ssl.TrustManagerFactory.getDefaultAlgorithm();
	    javax.net.ssl.TrustManagerFactory tmf = javax.net.ssl.TrustManagerFactory.getInstance(algorithm);
	    tmf.init(_certs);
	    trustManagers = tmf.getTrustManagers();
	}

	_ctx = javax.net.ssl.SSLContext.getInstance("SSL");
	_ctx.init(keyManagers, trustManagers, rand);
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

    private static class CipherExpression
    {
	boolean not;
	String cipher;
	java.util.regex.Pattern re;
    }

    private CipherExpression[] _ciphers;
    private boolean _allCiphers;
    private boolean _noCiphers;
    private javax.net.ssl.SSLContext _ctx;
    private java.security.KeyStore _keys;
    private java.security.KeyStore _certs;
}
