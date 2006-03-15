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
    using System.IO;
    using System.Net.Security;
    using System.Net.Sockets;
    using System.Security.Authentication;
    using System.Security.Cryptography;
    using System.Security.Cryptography.X509Certificates;
    using System.Threading;

    internal sealed class ServerContext : Context
    {
	internal SslStream authenticate(Socket fd, int timeout)
	{
	    NetworkStream ns = new NetworkStream(fd, true);
	    SslStream stream = new SslStream(ns, false, new RemoteCertificateValidationCallback(validationCallback),
					     null); // TODO: Need callbacks?

	    try
	    {
		AuthInfo info = new AuthInfo();
		info.stream = stream;
		IAsyncResult ar = stream.BeginAuthenticateAsServer(cert_, requireClientCert_, protocols_,
								   checkCRL_, new AsyncCallback(authCallback), info);
		lock(info)
		{
		    if(!Monitor.Wait(info, timeout == -1 ? Timeout.Infinite : timeout))
		    {
			throw new Ice.TimeoutException("SSL authentication timed out after " + timeout + " msec");
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
		throw new Ice.SocketException(ex);
	    }
	    catch(AuthenticationException ex)
	    {
		stream.Close();

		SslException e = new SslException(ex);
		e.ice_message_ = ex.Message;
		throw e;
	    }
	    catch(Exception ex)
	    {
		stream.Close();
		throw new Ice.SyscallException(ex);
	    }

	    return stream;
	}

	internal bool initialized()
	{
	    return cert_ != null;
	}

	internal ServerContext(Instance instance) : base(instance)
	{
	    const string prefix = "IceSSL.Server.";
	    Ice.Properties properties = instance.communicator().getProperties();

	    // TODO: Validate filename
	    string certFile = properties.getProperty(prefix + "Cert.File");
	    string certPassword = properties.getProperty(prefix + "Cert.Password");
	    if(certFile.Length > 0)
	    {
		if(!File.Exists(certFile))
		{
		    logger_.error("IceSSL: server certificate file not found: " + certFile);
		    Ice.FileException ex = new Ice.FileException();
		    ex.path = certFile;
		    throw ex;
		}
		try
		{
		    if(certPassword.Length == 0)
		    {
			cert_ = new X509Certificate2(certFile);
		    }
		    else
		    {
			cert_ = new X509Certificate2(certFile, certPassword);
		    }
		}
		catch(CryptographicException ex)
		{
		    SslException e = new SslException(ex);
		    e.ice_message_ = "attempting to load certificate from " + certFile;
		    throw e;
		}
	    }

	    // TODO: Review default value
	    requireClientCert_ = properties.getPropertyAsIntWithDefault(prefix + "RequireClientCert", 0) > 0;

	    // TODO: Allow user to specify protocols?
	    //protocols_ = SslProtocols.Default;
	    protocols_ = parseProtocols(prefix + "Protocols");

	    // TODO: Review default value
	    checkCRL_ = properties.getPropertyAsIntWithDefault(prefix + "CheckCRL", 0) > 0;

	    // TODO: Review default value
	    ignoreCertName_ = properties.getPropertyAsIntWithDefault(prefix + "IgnoreCertName", 1) > 0;
	}

	private class AuthInfo
	{
	    internal SslStream stream;
	    internal Exception ex;
	}

	private static void authCallback(IAsyncResult ar)
	{
	    AuthInfo info = (AuthInfo)ar.AsyncState;
	    lock(info)
	    {
		try
		{
		    info.stream.EndAuthenticateAsServer(ar);
		}
		catch(Exception ex)
		{
		    info.ex = ex;
		}
		finally
		{
		    Monitor.Pulse(info);
		}
	    }
	}

	private bool validationCallback(object sender, X509Certificate certificate, X509Chain chain,
					SslPolicyErrors sslPolicyErrors)
	{
	    if(sslPolicyErrors == SslPolicyErrors.None)
	    {
		if(instance_.securityTraceLevel() >= 1)
		{
		    logger_.trace(instance_.securityTraceCategory(), "SSL certificate validation succeeded");
		}
		return true;
	    }

	    string message = "";
	    int errors = (int)sslPolicyErrors;
	    if((errors & (int)SslPolicyErrors.RemoteCertificateNotAvailable) > 0)
	    {
		if(requireClientCert_)
		{
		    if(instance_.securityTraceLevel() >= 1)
		    {
			logger_.trace(instance_.securityTraceCategory(),
				      "SSL certificate validation failed - client certificate not provided");
		    }
		    return false;
		}
		errors ^= (int)SslPolicyErrors.RemoteCertificateNotAvailable;
		message = message + "\nremote certificate not provided (ignored)";
	    }

	    if((errors & (int)SslPolicyErrors.RemoteCertificateNameMismatch) > 0)
	    {
		if(ignoreCertName_)
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

	    if(instance_.securityTraceLevel() >= 1)
	    {
		logger_.trace(instance_.securityTraceCategory(), "SSL certificate validation succeeded" + message);
	    }
	    return true;
	}

	private X509Certificate2 cert_;
	private bool requireClientCert_;
	private SslProtocols protocols_;
	private bool checkCRL_;
	private bool ignoreCertName_;
    }
}
