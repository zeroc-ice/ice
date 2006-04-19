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
    using System;
    using System.Collections;
    using System.IO;
    using System.Security.Cryptography.X509Certificates;

    internal class Instance
    {
	internal Instance(Ice.Communicator communicator)
	{
	    Ice.Properties properties = communicator.getProperties();

	    facade_ = Ice.Util.getProtocolPluginFacade(communicator);
	    securityTraceLevel_ = properties.getPropertyAsIntWithDefault("IceSSL.Trace.Security", 0);
	    securityTraceCategory_ = "Security";

	    //
	    // Check for a default directory. We look in this directory for
	    // files mentioned in the configuration.
	    //
	    defaultDir_ = properties.getProperty("IceSSL.DefaultDir");

	    //
	    // Process IceSSL.ImportCert.* properties.
	    //
	    Ice.PropertyDict certs = properties.getPropertiesForPrefix("IceSSL.ImportCert.");
	    foreach(DictionaryEntry entry in certs)
	    {
		string name = (string)entry.Key;
		string val = (string)entry.Value;
		if(val.Length > 0)
		{
		    importCertificate(name, val);
		}
	    }

	    //
	    // Create the context.
	    //
	    // If IceSSL.DelayInit=1, postpone the creation of the context until
	    // the application manually initializes the plugin.
	    //
	    if(properties.getPropertyAsInt("IceSSL.DelayInit") == 0)
	    {
		context_ = new Context(this, null);
	    }

	    //
	    // Register the endpoint factory.
	    //
	    facade_.addEndpointFactory(new EndpointFactoryI(this));
	}

	internal void initialize(X509Certificate2Collection certs)
	{
	    if(context_ != null)
	    {
		Ice.PluginInitializationException e = new Ice.PluginInitializationException();
		e.reason = "plugin is already initialized";
		throw e;
	    }
	    else
	    {
		context_ = new Context(this, certs);
	    }
	}

	internal void setCertificateVerifier(CertificateVerifier verifier)
	{
	    verifier_ = verifier;
	}

	internal Ice.Communicator communicator()
	{
	    return facade_.getCommunicator();
	}

	internal string defaultHost()
	{
	    return facade_.getDefaultHost();
	}

	internal int networkTraceLevel()
	{
	    return facade_.getNetworkTraceLevel();
	}

	internal string networkTraceCategory()
	{
	    return facade_.getNetworkTraceCategory();
	}

	internal int securityTraceLevel()
	{
	    return securityTraceLevel_;
	}

	internal string securityTraceCategory()
	{
	    return securityTraceCategory_;
	}

	internal Context context()
	{
	    if(context_ == null)
	    {
		Ice.PluginInitializationException e = new Ice.PluginInitializationException();
		e.reason = "IceSSL: plugin is not fully initialized";
		throw e;
	    }
	    return context_;
	}

	internal CertificateVerifier certificateVerifier()
	{
	    return verifier_;
	}

	internal void traceStream(System.Net.Security.SslStream stream, string connInfo)
	{
	    System.Text.StringBuilder s = new System.Text.StringBuilder();
	    s.Append("SSL connection summary");
	    if(connInfo.Length > 0)
	    {
		s.Append("\n");
		s.Append(connInfo);
	    }
	    s.Append("\nauthenticated = " + (stream.IsAuthenticated ? "yes" : "no"));
	    s.Append("\nencrypted = " + (stream.IsEncrypted ? "yes" : "no"));
	    s.Append("\nsigned = " + (stream.IsSigned ? "yes" : "no"));
	    s.Append("\nmutually authenticated = " + (stream.IsMutuallyAuthenticated ? "yes" : "no"));
	    s.Append("\nhash algorithm = " + stream.HashAlgorithm + "/" + stream.HashStrength);
	    s.Append("\ncipher algorithm = " + stream.CipherAlgorithm + "/" + stream.CipherStrength);
	    s.Append("\nkey exchange algorithm = " + stream.KeyExchangeAlgorithm + "/" + stream.KeyExchangeStrength);
	    s.Append("\nprotocol = " + stream.SslProtocol);
	    communicator().getLogger().trace(securityTraceCategory_, s.ToString());
	}

	//
	// Parse a string of the form "location.name" into two parts.
	//
	internal void parseStore(string prop, string store, ref StoreLocation loc, ref StoreName name, ref string sname)
	{
	    int pos = store.IndexOf('.');
	    if(pos == -1)
	    {
		Ice.PluginInitializationException e = new Ice.PluginInitializationException();
		e.reason = "IceSSL: property `" + prop + "' has invalid format";
		throw e;
	    }

	    string sloc = store.Substring(0, pos).ToLower();
	    if(sloc.Equals("currentuser"))
	    {
		loc = StoreLocation.CurrentUser;
	    }
	    else if(sloc.Equals("localmachine"))
	    {
		loc = StoreLocation.LocalMachine;
	    }
	    else
	    {
		Ice.PluginInitializationException e = new Ice.PluginInitializationException();
		e.reason = "IceSSL: unknown store location `" + sloc + "' in " + prop;
		throw e;
	    }

	    sname = store.Substring(pos + 1);
	    if(sname.Length == 0)
	    {
		Ice.PluginInitializationException e = new Ice.PluginInitializationException();
		e.reason = "IceSSL: invalid store name in " + prop;
		throw e;
	    }

	    //
	    // Try to convert the name into the StoreName enumeration.
	    //
	    try
	    {
		name = (StoreName)Enum.Parse(typeof(StoreName), sname, true);
		sname = null;
	    }
	    catch(ArgumentException)
	    {
		// Ignore - assume the user is selecting a non-standard store.
	    }
	}

	internal bool checkPath(ref string path)
	{
	    if(File.Exists(path))
	    {
		return true;
	    }

	    if(defaultDir_.Length > 0)
	    {
		string s = defaultDir_ + Path.DirectorySeparatorChar + path;
		if(File.Exists(s))
		{
		    path = s;
		    return true;
		}
	    }

	    return false;
	}

	private void importCertificate(string propName, string propValue)
	{
	    //
	    // Expecting a property of the following form:
	    //
	    // IceSSL.ImportCert.<location>.<name>=<file>[;password]
	    //
	    const string prefix = "IceSSL.ImportCert.";
	    StoreLocation loc = 0;
	    StoreName name = 0;
	    string sname = null;
	    parseStore(propName, propName.Substring(prefix.Length), ref loc, ref name, ref sname);

	    //
	    // Extract the filename and password. Either or both can be quoted.
	    //
	    string[] arr = splitString(propValue, ';');
	    if(arr == null)
	    {
		Ice.PluginInitializationException e = new Ice.PluginInitializationException();
		e.reason = "IceSSL: unmatched quote in `" + propValue + "'";
		throw e;
	    }
	    if(arr.Length == 0)
	    {
		return;
	    }
	    string file = arr[0];
	    string password = null;
	    if(arr.Length > 1)
	    {
		password = arr[1];
	    }

	    //
	    // Open the X509 certificate store.
	    //
	    X509Store store = null;
	    try
	    {
		if(sname != null)
		{
		    store = new X509Store(sname, loc);
		}
		else
		{
		    store = new X509Store(name, loc);
		}
		store.Open(OpenFlags.ReadWrite);
	    }
	    catch(Exception ex)
	    {
		Ice.PluginInitializationException e = new Ice.PluginInitializationException(ex);
		e.reason = "IceSSL: failure while opening store specified by " + propName;
		throw e;
	    }

	    if(!checkPath(ref file))
	    {
		Ice.PluginInitializationException e = new Ice.PluginInitializationException();
		e.reason = "IceSSL: certificate file not found:\n" + file;
		throw e;
	    }

	    //
	    // Add the certificate to the store.
	    //
	    try
	    {
		X509Certificate2 cert;
		if(password != null)
		{
		    cert = new X509Certificate2(file, password);
		}
		else
		{
		    cert = new X509Certificate2(file);
		}
		store.Add(cert);
	    }
	    catch(Exception ex)
	    {
		Ice.PluginInitializationException e = new Ice.PluginInitializationException(ex);
		e.reason = "IceSSL: failure while adding certificate file:\n" + file;
		throw e;
	    }
	    finally
	    {
		store.Close();
	    }
	}

	//
	// Split strings using a delimiter. Quotes are supported.
	// Returns null for an unmatched quote.
	//
	private string[] splitString(string str, char delim)
	{
	    ArrayList l = new ArrayList();
	    char[] arr = new char[str.Length];
	    int pos = 0;

	    while(pos < str.Length)
	    {
		int n = 0;
		char quoteChar = '\0';
		if(str[pos] == '"' || str[pos] == '\'')
		{
		    quoteChar = str[pos];
		    ++pos;
		}
		bool trim = true;
		while(pos < str.Length)
		{
		    if(quoteChar != '\0' && str[pos] == '\\' && pos + 1 < str.Length && str[pos + 1] == quoteChar)
		    {
			++pos;
		    }
		    else if(quoteChar != '\0' && str[pos] == quoteChar)
		    {
			trim = false;
			++pos;
			quoteChar = '\0';
			break;
		    }
		    else if(str[pos] == delim)
		    {
			if(quoteChar == '\0')
			{
			    ++pos;
			    break;
			}
		    }
		    if(pos < str.Length)
		    {
			arr[n++] = str[pos++];
		    }
		}
		if(quoteChar != '\0')
		{
		    return null; // Unmatched quote.
		}
		if(n > 0)
		{
		    string s = new string(arr, 0, n);
		    if(trim)
		    {
			s = s.Trim();
		    }
		    if(s.Length > 0)
		    {
			l.Add(s);
		    }
		}
	    }

	    return (string[])l.ToArray(typeof(string));
	}

	private IceInternal.ProtocolPluginFacade facade_;
	private int securityTraceLevel_;
	private string securityTraceCategory_;
	private Context context_;
	private CertificateVerifier verifier_;
	private string defaultDir_;
    }
}
