// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Security.Authentication;
using System.Security.Cryptography.X509Certificates;

namespace IceSSL
{
    internal class Context
    {
	internal Context(Instance instance)
	{
	    instance_ = instance;
	    logger_ = instance.communicator().getLogger();
	}

	protected SslProtocols parseProtocols(string property)
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
			if(s.Equals("ssl3") || s.Equals("sslv3"))
			{
			    result |= SslProtocols.Ssl3;
			}
			else if(s.Equals("tls") || s.Equals("tls1") || s.Equals("tlsv1"))
			{
			    result |= SslProtocols.Tls;
			}
			else
			{
			    Ice.PluginInitializationException e = new Ice.PluginInitializationException();
			    e.reason = "IceSSL: unrecognized protocol `" + s + "'";
			    throw e;
			}
		    }
		}
	    }
	    return result;
	}

	protected X509Certificate2Collection findCertificates(string prop, string storeSpec, string value)
	{
	    StoreLocation storeLoc = 0;
	    StoreName storeName = 0;
	    string storeNameStr = null;
	    instance_.parseStore(prop, storeSpec, ref storeLoc, ref storeName, ref storeNameStr);

	    //
	    // Open the X509 certificate store.
	    //
	    X509Store store = null;
	    try
	    {
		if(storeNameStr != null)
		{
		    store = new X509Store(storeNameStr, storeLoc);
		}
		else
		{
		    store = new X509Store(storeName, storeLoc);
		}
		store.Open(OpenFlags.ReadOnly);
	    }
	    catch(Exception ex)
	    {
		Ice.PluginInitializationException e = new Ice.PluginInitializationException(ex);
		e.reason = "IceSSL: failure while opening store specified by " + prop;
		throw e;
	    }

	    //
	    // Start with all of the certificates in the collection and filter as necessary.
	    //
	    // - If the value is "*", return all certificates.
	    // - Otherwise, search using key:value pairs. The following keys are supported:
	    //
	    //   Issuer
	    //   IssuerDN
	    //   Serial
	    //   Subject
	    //   SubjectDN
	    //   SubjectKeyId
	    //   Thumbprint
	    //
	    //   A value must be enclosed in single or double quotes if it contains whitespace.
	    //
	    X509Certificate2Collection result = new X509Certificate2Collection();
	    result.AddRange(store.Certificates);
	    try
	    {
		if(value != "*")
		{
		    int start = 0;
		    int pos;
		    while((pos = value.IndexOf(':', start)) != -1)
		    {
			//
			// Parse the X509FindType.
			//
			string field = value.Substring(start, pos - start).Trim().ToLower();
			X509FindType findType;
			if(field.Equals("subject"))
			{
			    findType = X509FindType.FindBySubjectName;
			}
			else if(field.Equals("subjectdn"))
			{
			    findType = X509FindType.FindBySubjectDistinguishedName;
			}
			else if(field.Equals("issuer"))
			{
			    findType = X509FindType.FindByIssuerName;
			}
			else if(field.Equals("issuerdn"))
			{
			    findType = X509FindType.FindByIssuerDistinguishedName;
			}
			else if(field.Equals("thumbprint"))
			{
			    findType = X509FindType.FindByThumbprint;
			}
			else if(field.Equals("subjectkeyid"))
			{
			    findType = X509FindType.FindBySubjectKeyIdentifier;
			}
			else if(field.Equals("serial"))
			{
			    findType = X509FindType.FindBySerialNumber;
			}
			else
			{
			    Ice.PluginInitializationException e = new Ice.PluginInitializationException();
			    e.reason = "IceSSL: unknown key in `" + value + "'";
			    throw e;
			}

			//
			// Parse the argument.
			//
			start = pos + 1;
			while(start < value.Length && (value[start] == ' ' || value[start] == '\t'))
			{
			    ++start;
			}
			if(start == value.Length)
			{
			    Ice.PluginInitializationException e = new Ice.PluginInitializationException();
			    e.reason = "IceSSL: missing argument in `" + value + "'";
			    throw e;
			}

			string arg;
			if(value[start] == '"' || value[start] == '\'')
			{
			    int end = start;
			    ++end;
			    while(end < value.Length)
			    {
				if(value[end] == value[start] && value[end - 1] != '\\')
				{
				    break;
				}
				++end;
			    }
			    if(end == value.Length || value[end] != value[start])
			    {
				Ice.PluginInitializationException e = new Ice.PluginInitializationException();
				e.reason = "IceSSL: unmatched quote in `" + value + "'";
				throw e;
			    }
			    ++start;
			    arg = value.Substring(start, end - start);
			    start = end + 1;
			}
			else
			{
			    char[] ws = new char[] { ' ', '\t' };
			    int end = value.IndexOfAny(ws, start);
			    if(end == -1)
			    {
				arg = value.Substring(start);
				start = value.Length;
			    }
			    else
			    {
				arg = value.Substring(start, end - start);
				start = end + 1;
			    }
			}

			//
			// Execute the query.
			//
			// TODO: allow user to specify a value for validOnly?
			//
			bool validOnly = false;
			result = result.Find(findType, arg, validOnly);
		    }
		}
	    }
	    finally
	    {
		store.Close();
	    }

	    return result;
	}

	protected Instance instance_;
	protected Ice.Logger logger_;
    }
}
