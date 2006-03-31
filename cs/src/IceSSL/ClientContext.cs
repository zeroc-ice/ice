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
    using System.Net.Security;
    using System.Net.Sockets;
    using System.Security.Authentication;
    using System.Security.Cryptography;
    using System.Security.Cryptography.X509Certificates;
    using System.Threading;

    internal sealed class ClientContext : Context
    {
	internal SslStream authenticate(Socket fd, string host, int timeout)
	{
	    NetworkStream ns = new NetworkStream(fd, true);
	    ClientValidationCallback cb = new ClientValidationCallback(this, host);
	    SslStream stream = new SslStream(ns, false, new RemoteCertificateValidationCallback(cb.validationCallback),
					     null);

	    try
	    {
		AuthInfo info = new AuthInfo();
		info.stream = stream;
		info.done = false;
		IAsyncResult ar = stream.BeginAuthenticateAsClient(host, certs_, protocols_, checkCRL_,
								   new AsyncCallback(authCallback), info);
		lock(info)
		{
		    if(!info.done)
		    {
			if(!Monitor.Wait(info, timeout == -1 ? Timeout.Infinite : timeout))
			{
			    throw new Ice.ConnectTimeoutException("SSL authentication timed out after " + timeout +
								  " msec");
			}
		    }
		    if(info.ex != null)
		    {
			throw info.ex;
		    }
		}
	    }
	    catch(Ice.LocalException ex)
	    {
		stream.Close();
		throw ex;
	    }
	    catch(IOException ex)
	    {
		stream.Close();

		if(IceInternal.Network.connectionLost(ex))
		{
		    throw new Ice.ConnectionLostException(ex);
		}
		throw new Ice.SyscallException(ex);
	    }
	    catch(AuthenticationException ex)
	    {
		stream.Close();

		Ice.SecurityException e = new Ice.SecurityException(ex);
		e.reason = ex.Message;
		throw e;
	    }
	    catch(Exception ex)
	    {
		stream.Close();
		throw new Ice.SyscallException(ex);
	    }

	    return stream;
	}

	internal ClientContext(Instance instance, X509Certificate2Collection certs) : base(instance)
	{
	    const string prefix = "IceSSL.Client.";
	    Ice.Properties properties = instance.communicator().getProperties();

	    if(certs != null)
	    {
		certs_ = certs;
	    }
	    else
	    {
		//
		// Load client certificate from file.
		//
		// TODO: tracing?
		certs_ = new X509Certificate2Collection();
		string certFile = properties.getProperty(prefix + "CertFile");
		string password = properties.getProperty(prefix + "Password");
		if(certFile.Length > 0)
		{
		    if(!File.Exists(certFile))
		    {
			Ice.PluginInitializationException e = new Ice.PluginInitializationException();
			e.reason = "IceSSL: client certificate file not found: " + certFile;
			throw e;
		    }
		    try
		    {
			X509Certificate2 cert = new X509Certificate2(certFile, password);
			certs_.Add(cert);
		    }
		    catch(CryptographicException ex)
		    {
			Ice.PluginInitializationException e = new Ice.PluginInitializationException(ex);
			e.reason = "IceSSL: error while attempting to load certificate from " + certFile;
			throw e;
		    }
		}

		//
		// Select client certificates from stores.
		//
		// TODO: tracing?
		const string findPrefix = "IceSSL.Client.FindCert.";
		Ice.PropertyDict certProps = properties.getPropertiesForPrefix(findPrefix);
		if(certProps.Count > 0)
		{
		    foreach(DictionaryEntry entry in certProps)
		    {
			string name = (string)entry.Key;
			string val = (string)entry.Value;
			if(val.Length > 0)
			{
			    string storeSpec = name.Substring(findPrefix.Length);
			    X509Certificate2Collection coll = findCertificates(name, storeSpec, val);
			    certs_.AddRange(coll);
			}
		    }
		    if(certs_.Count == 0)
		    {
			Ice.PluginInitializationException e = new Ice.PluginInitializationException();
			e.reason = "IceSSL: no client certificates found";
			throw e;
		    }
		}
	    }

	    protocols_ = parseProtocols(prefix + "Protocols");
	    checkCRL_ = properties.getPropertyAsIntWithDefault(prefix + "CheckCRL", 0) > 0;
	    checkCertName_ = properties.getPropertyAsIntWithDefault(prefix + "CheckCertName", 0) > 0;
	}

	private class AuthInfo
	{
	    internal SslStream stream;
	    volatile internal Exception ex;
	    volatile internal bool done;
	}

	private static void authCallback(IAsyncResult ar)
	{
	    AuthInfo info = (AuthInfo)ar.AsyncState;
	    lock(info)
	    {
		try
		{
		    info.stream.EndAuthenticateAsClient(ar);
		}
		catch(Exception ex)
		{
		    info.ex = ex;
		}
		finally
		{
		    info.done = true;
		    Monitor.Pulse(info);
		}
	    }
	}

	internal bool validationCallback(SslStream stream, string host, X509Certificate certificate, X509Chain chain,
					 SslPolicyErrors sslPolicyErrors)
	{
	    string message = "";
	    int errors = (int)sslPolicyErrors;
	    if((errors & (int)SslPolicyErrors.RemoteCertificateNameMismatch) > 0)
	    {
		if(!checkCertName_)
		{
		    errors ^= (int)SslPolicyErrors.RemoteCertificateNameMismatch;
		    message = message + "\nremote certificate name mismatch (ignored)";
		}
		else
		{
		    if(instance_.securityTraceLevel() >= 1)
		    {
			logger_.trace(instance_.securityTraceCategory(),
				      "SSL certificate validation failed - remote certificate name mismatch");
		    }
		    return false;
		}
	    }

	    if(errors > 0)
	    {
		if(instance_.securityTraceLevel() >= 1)
		{
		    logger_.trace(instance_.securityTraceCategory(), "SSL certificate validation failed");
		}
		return false;
	    }

	    CertificateVerifier verifier = instance_.certificateVerifier();
	    if(verifier != null)
	    {
		VerifyInfo info = new VerifyInfo();
		info.incoming = false;
		info.cert = certificate;
		info.chain = chain;
		info.stream = stream;
		info.address = host;
		if(!verifier.verify(info))
		{
		    return false;
		}
	    }

	    if(instance_.securityTraceLevel() >= 1)
	    {
		logger_.trace(instance_.securityTraceCategory(), "SSL certificate validation succeeded" + message);
	    }

	    return true;
	}

	private X509Certificate2Collection certs_;
	private SslProtocols protocols_;
	private bool checkCRL_;
	private bool checkCertName_;
    }

    //
    // We need to pass some additional information to the certificate validation callback.
    //
    internal class ClientValidationCallback
    {
	internal ClientValidationCallback(ClientContext context, string host)
	{
	    context_ = context;
	    host_ = host;
	}

	internal bool validationCallback(object sender, X509Certificate certificate, X509Chain chain,
					 SslPolicyErrors sslPolicyErrors)
	{
	    SslStream stream = (SslStream)sender;
	    return context_.validationCallback(stream, host_, certificate, chain, sslPolicyErrors);
	}

	private ClientContext context_;
	private string host_;
    }
}
