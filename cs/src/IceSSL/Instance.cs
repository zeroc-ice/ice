// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Collections;
using System.Security.Cryptography.X509Certificates;

namespace IceSSL
{
    internal class Instance
    {
	internal Instance(Ice.Communicator communicator)
	{
	    Ice.Properties properties = communicator.getProperties();

	    facade_ = Ice.Util.getProtocolPluginFacade(communicator);
	    securityTraceLevel_ = properties.getPropertyAsIntWithDefault("IceSSL.Trace.Security", 0);
	    securityTraceCategory_ = "Security";

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
	    // Create the client and server contexts. We always create both, even
	    // if only one is used.
	    //
	    clientContext_ = new ClientContext(this);
	    serverContext_ = new ServerContext(this);

	    //
	    // Register the endpoint factory.
	    //
	    facade_.addEndpointFactory(new SslEndpointFactory(this));
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

	internal ClientContext clientContext()
	{
	    return clientContext_;
	}

	internal ServerContext serverContext()
	{
	    return serverContext_;
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

	internal void parseStore(string prop, string store, ref StoreLocation loc, ref string name)
	{
	    int pos = store.IndexOf('.');
	    if(pos == -1)
	    {
		string msg = "property `" + prop + "' has invalid format";
		communicator().getLogger().error("IceSSL: " + msg);
		SslException e = new SslException();
		e.ice_message_ = msg;
		throw e;
	    }

	    string sloc = store.Substring(0, pos).ToLower();
	    if(sloc == "currentuser")
	    {
		loc = StoreLocation.CurrentUser;
	    }
	    else if(sloc == "localmachine")
	    {
		loc = StoreLocation.LocalMachine;
	    }
	    else
	    {
		string msg = "unknown store location `" + sloc + "' in " + prop;
		communicator().getLogger().error("IceSSL: " + msg);
		SslException e = new SslException();
		e.ice_message_ = msg;
		throw e;
	    }

	    name = store.Substring(pos + 1);
	    if(name.Length == 0)
	    {
		string msg = "invalid store name in " + prop;
		communicator().getLogger().error("IceSSL: " + msg);
		SslException e = new SslException();
		e.ice_message_ = msg;
		throw e;
	    }
	}

	internal string[] parseFileList(string files)
	{
	    ArrayList l = new ArrayList();

	    int start = 0;
	    while(start < files.Length)
	    {
		while(start < files.Length && (files[start] == ' ' || files[start] == '\t'))
		{
		    ++start;
		}
		if(start == files.Length)
		{
		    break;
		}
		if(files[start] == '"' || files[start] == '\'')
		{
		    int end = files.IndexOf(files[start], start + 1);
		    if(end == -1)
		    {
			string msg = "unmatched quote in `" + files + "'";
			communicator().getLogger().error("IceSSL: " + msg);
			SslException e = new SslException();
			e.ice_message_ = msg;
			throw e;
		    }
		    ++start;
		    string f = files.Substring(start, end - start).Trim();
		    if(f.Length > 0)
		    {
			l.Add(f);
		    }
		    start = end + 1;
		}
		else
		{
		    int pos = files.IndexOf(',', start);
		    if(pos == -1)
		    {
			l.Add(files.Substring(start));
			start = files.Length;
		    }
		    else
		    {
			string f = files.Substring(start, pos - start).Trim();
			if(f.Length > 0)
			{
			    l.Add(f);
			}
			start = pos + 1;
		    }
		}
	    }

	    return (string[])l.ToArray(typeof(string));
	}

	private void importCertificate(string propName, string propValue)
	{
	    //
	    // Expecting a property of the following form:
	    //
	    // IceSSL.ImportCert.<location>.<name>=<file>[,<file]
	    //
	    const string prefix = "IceSSL.ImportCert.";
	    StoreLocation loc = 0;
	    string name = null;
	    parseStore(propName, propName.Substring(prefix.Length), ref loc, ref name);

	    //
	    // Open the X509 certificate store.
	    //
	    X509Store store = null;
	    try
	    {
		store = new X509Store(name, loc);
		store.Open(OpenFlags.ReadWrite);
	    }
	    catch(Exception ex)
	    {
		string msg = "failure while opening store specified by " + propName;
		communicator().getLogger().error("IceSSL: " + msg);
		SslException e = new SslException(ex);
		e.ice_message_ = msg;
		throw e;
	    }

	    //
	    // Add each certificate to the store.
	    //
	    try
	    {
		string[] files = parseFileList(propValue);
		for(int i = 0; i < files.Length; ++i)
		{
		    try
		    {
			X509Certificate2 cert = new X509Certificate2(files[i]);
			store.Add(cert);
		    }
		    catch(Exception ex)
		    {
			string msg = "failure while adding certificate `" + files[i] + "'";
			communicator().getLogger().error("IceSSL: " + msg);
			SslException e = new SslException(ex);
			e.ice_message_ = msg;
			throw e;
		    }
		}
	    }
	    finally
	    {
		store.Close();
	    }
	}

	private IceInternal.ProtocolPluginFacade facade_;
	private int securityTraceLevel_;
	private string securityTraceCategory_;
	private ClientContext clientContext_;
	private ServerContext serverContext_;
    }
}
