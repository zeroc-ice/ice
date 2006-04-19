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
    using System.Net;
    using System.Net.Security;
    using System.Net.Sockets;
    using System.Security.Authentication;
    using System.Security.Cryptography.X509Certificates;
    using System.Threading;

    sealed class ConnectorI : IceInternal.Connector
    {
	public IceInternal.Transceiver connect(int timeout)
	{
	    //
	    // The plugin may not be fully initialized.
	    //
	    Context ctx = instance_.context();

	    if(instance_.networkTraceLevel() >= 2)
	    {
		string s = "trying to establish ssl connection to " + ToString();
		logger_.trace(instance_.networkTraceCategory(), s);
	    }

	    Socket fd = IceInternal.Network.createSocket(false);
	    IceInternal.Network.setBlock(fd, true);
	    IceInternal.Network.doConnectAsync(fd, addr_, timeout);

	    SslStream stream = null;
	    try
	    {
		//
		// Create an SslStream.
		//
		NetworkStream ns = new NetworkStream(fd, true);
		ConnectorValidationCallback cb = new ConnectorValidationCallback(this, fd, host_);
		stream = new SslStream(ns, false, new RemoteCertificateValidationCallback(cb.validate), null);

		//
		// Start the validation process and wait for it to complete.
		//
		AuthInfo info = new AuthInfo();
		info.stream = stream;
		info.done = false;
		IAsyncResult ar = stream.BeginAuthenticateAsClient(host_, ctx.certs(), ctx.protocols(),
								   ctx.checkCRL(), new AsyncCallback(authCallback),
								   info);
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
		if(stream != null)
		{
		    stream.Close();
		}
		else
		{
		    IceInternal.Network.closeSocketNoThrow(fd);
		}

		throw ex;
	    }
	    catch(IOException ex)
	    {
		if(stream != null)
		{
		    stream.Close();
		}
		else
		{
		    IceInternal.Network.closeSocketNoThrow(fd);
		}

		if(IceInternal.Network.connectionLost(ex))
		{
		    throw new Ice.ConnectionLostException(ex);
		}
		throw new Ice.SyscallException(ex);
	    }
	    catch(AuthenticationException ex)
	    {
		if(stream != null)
		{
		    stream.Close();
		}
		else
		{
		    IceInternal.Network.closeSocketNoThrow(fd);
		}

		Ice.SecurityException e = new Ice.SecurityException(ex);
		e.reason = ex.Message;
		throw e;
	    }
	    catch(Exception ex)
	    {
		if(stream != null)
		{
		    stream.Close();
		}
		else
		{
		    IceInternal.Network.closeSocketNoThrow(fd);
		}

		throw new Ice.SyscallException(ex);
	    }

	    if(instance_.networkTraceLevel() >= 1)
	    {
		string s = "ssl connection established\n" + IceInternal.Network.fdToString(fd);
		logger_.trace(instance_.networkTraceCategory(), s);
	    }

	    if(instance_.securityTraceLevel() >= 1)
	    {
		instance_.traceStream(stream, IceInternal.Network.fdToString(fd));
	    }

	    return new TransceiverI(instance_, fd, stream);
	}

	public override string ToString()
	{
	    return IceInternal.Network.addrToString(addr_);
	}

	//
	// Only for use by EndpointI.
	//
	internal ConnectorI(Instance instance, string host, int port)
	{
	    instance_ = instance;
	    host_ = host;
	    logger_ = instance.communicator().getLogger();
	    addr_ = IceInternal.Network.getAddress(host, port);
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

	internal bool validate(SslStream stream, Socket fd, string host, X509Certificate certificate, X509Chain chain,
			       SslPolicyErrors sslPolicyErrors)
	{
	    string message = "";
	    int errors = (int)sslPolicyErrors;
	    if((errors & (int)SslPolicyErrors.RemoteCertificateNameMismatch) > 0)
	    {
		if(!instance_.context().checkCertName())
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
		    if(instance_.securityTraceLevel() >= 1)
		    {
			logger_.trace(instance_.securityTraceCategory(),
				      "outgoing connection rejected by certificate verifier\n" +
				      IceInternal.Network.fdToString(fd));
		    }
		    return false;
		}
	    }

	    return true;
	}

	private Instance instance_;
	private string host_;
	private Ice.Logger logger_;
	private IPEndPoint addr_;
    }

    //
    // We need to pass some additional information to the certificate validation callback.
    //
    internal class ConnectorValidationCallback
    {
	internal ConnectorValidationCallback(ConnectorI connector, Socket fd, string host)
	{
	    connector_ = connector;
	    fd_ = fd;
	    host_ = host;
	}

	internal bool validate(object sender, X509Certificate certificate, X509Chain chain,
			       SslPolicyErrors sslPolicyErrors)
	{
	    SslStream stream = (SslStream)sender;
	    return connector_.validate(stream, fd_, host_, certificate, chain, sslPolicyErrors);
	}

	private ConnectorI connector_;
	private Socket fd_;
	private string host_;
    }
}
