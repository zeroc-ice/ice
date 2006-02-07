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
    using System.Net.Security;
    using System.Net.Sockets;
    using System.Security.Authentication;
    using System.Security.Cryptography.X509Certificates;
    using System.Threading;

    internal sealed class ClientContext : Context
    {
	internal SslStream authenticate(Socket fd, string host, int timeout)
	{
	    NetworkStream ns = new NetworkStream(fd, true);
	    SslStream stream = new SslStream(ns, false, new RemoteCertificateValidationCallback(validationCallback),
					     null);

	    try
	    {
		AuthInfo info = new AuthInfo();
		info.stream = stream;
		IAsyncResult ar = stream.BeginAuthenticateAsClient(host, certs_, protocols_, checkCertRevocation_,
								   new AsyncCallback(authCallback), info);
		lock(info)
		{
		    if(!Monitor.Wait(info, timeout == -1 ? Timeout.Infinite : timeout))
		    {
			throw new Ice.ConnectTimeoutException("SSL authentication timed out after " + timeout +
							      " msec");
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

	internal ClientContext(Instance instance) : base(instance)
	{
	    //const string prefix = "IceSSL.Client.";
	    //Ice.Properties properties = communicator.getProperties();

	    // TODO: Add client certificates
	    // TODO: SSLv2 cannot use client certs
	    certs_ = null;

	    // TODO: Allow user to specify protocols?
	    protocols_ = SslProtocols.Default;

	    // TODO: Allow user to control this?
	    checkCertRevocation_ = false;
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
		    info.stream.EndAuthenticateAsClient(ar);
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
	    if((errors & (int)SslPolicyErrors.RemoteCertificateNameMismatch) > 0)
	    {
		// TODO: Use a property to determine whether to ignore mismatch?
		errors ^= (int)SslPolicyErrors.RemoteCertificateNameMismatch;
		message = message + "\nremote certificate name mismatch (ignored)";
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

	private X509CertificateCollection certs_;
	private SslProtocols protocols_;
	private bool checkCertRevocation_;
    }
}
