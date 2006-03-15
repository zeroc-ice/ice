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
    using System.Security.Authentication;
    using System.Security.Cryptography;
    using System.Security.Cryptography.X509Certificates;

    internal class Context
    {
	internal Context(Instance instance)
	{
	    instance_ = instance;
	    logger_ = instance.communicator().getLogger();

	    Ice.Properties properties = instance.communicator().getProperties();

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
	}

	internal SslProtocols parseProtocols(string property)
	{
	    SslProtocols result = SslProtocols.Default;
	    string val = instance_.communicator().getProperties().getProperty(property);
	    if(val.Length > 0)
	    {
		char[] delim = new char[] {',', ' '};
		string[] arr = val.Split(delim, StringSplitOptions.RemoveEmptyEntries);
		if(arr.Length > 0)
		{
		    result = 0;
		    for(int i = 0; i < arr.Length; ++i)
		    {
			string s = arr[i].ToLower();
			if(s == "ssl2" || s == "sslv2")
			{
			    result |= SslProtocols.Ssl2;
			}
			else if(s == "ssl3" || s == "sslv3")
			{
			    result |= SslProtocols.Ssl3;
			}
			else if(s == "tls")
			{
			    result |= SslProtocols.Tls;
			}
			else
			{
			    logger_.error("IceSSL: unrecognized protocol `" + s + "'");
			    SslException e = new SslException();
			    e.ice_message_ = "unrecognized protocol `" + s + "'";
			    throw e;
			}
		    }
		}
	    }
	    return result;
	}

	private void importCertificate(string propName, string propValue)
	{
	    //
	    // Expecting a property of the following form:
	    //
	    // IceSSL.ImportCert.<location>.<name>=<file>[,<file]
	    //
	    const string prefix = "IceSSL.ImportCert.";
	    string sub = propName.Substring(prefix.Length);
	    int pos = sub.IndexOf('.');
	    if(pos == -1)
	    {
		string msg = "property `" + propName + "' has invalid format";
		logger_.error("IceSSL: " + msg);
		SslException e = new SslException();
		e.ice_message_ = msg;
		throw e;
	    }

	    string sloc = sub.Substring(0, pos).ToLower();
	    StoreLocation loc;
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
		string msg = "unknown store location `" + sloc + "' in " + propName;
		logger_.error("IceSSL: " + msg);
		SslException e = new SslException();
		e.ice_message_ = msg;
		throw e;
	    }

	    string name = sub.Substring(pos + 1);
	    if(name.Length == 0)
	    {
		string msg = "invalid store name in " + propName;
		logger_.error("IceSSL: " + msg);
		SslException e = new SslException();
		e.ice_message_ = msg;
		throw e;
	    }

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
		logger_.error("IceSSL: " + msg);
		SslException e = new SslException(ex);
		e.ice_message_ = msg;
		throw e;
	    }

	    //
	    // Add each certificate to the store.
	    //
	    try
	    {
		char[] delim = new char[] {','};
		string[] arr = propValue.Split(delim, StringSplitOptions.RemoveEmptyEntries);
		if(arr.Length > 0)
		{
		    for(int i = 0; i < arr.Length; ++i)
		    {
			try
			{
			    X509Certificate2 cert = new X509Certificate2(arr[i]);
			    store.Add(cert);
			}
			catch(Exception ex)
			{
			    string msg = "failure while adding certificate `" + arr[i];
			    logger_.error("IceSSL: " + msg);
			    SslException e = new SslException(ex);
			    e.ice_message_ = msg;
			    throw e;
			}
		    }
		}
	    }
	    finally
	    {
		store.Close();
	    }
	}

	protected Instance instance_;
	protected Ice.Logger logger_;
    }
}
