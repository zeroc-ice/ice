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
    using System.Diagnostics;
    using System.IO;
    using System.Net;
    using System.Net.Security;
    using System.Net.Sockets;
    using System.Security.Authentication;
    using System.Security.Cryptography.X509Certificates;
    using System.Threading;

    class AcceptorI : IceInternal.Acceptor
    {
	public virtual Socket fd()
	{
	    return fd_;
	}
	
	public virtual void close()
	{
            Socket fd;
	    lock(this)
	    {
		fd = fd_;
		fd_ = null;
	    }
	    if(fd != null)
	    {
		if(instance_.networkTraceLevel() >= 1)
		{
		    string s = "stopping to accept ssl connections at " + ToString();
		    logger_.trace(instance_.networkTraceCategory(), s);
		}
	    
		try
		{
		    fd.Close();
		}
		catch(System.Exception)
		{
		    // Ignore.
		}
	    }
	}
	
	public virtual void listen()
	{
	    IceInternal.Network.doListen(fd_, backlog_);
	    
	    if(instance_.networkTraceLevel() >= 1)
	    {
		string s = "accepting ssl connections at " + ToString();
		logger_.trace(instance_.networkTraceCategory(), s);
	    }
	}
	
	public virtual IceInternal.Transceiver accept(int timeout)
	{
	    Debug.Assert(timeout == -1); // Always called with -1 for thread-per-connection.

	    //
	    // The plugin may not be fully initialized.
	    //
	    Context ctx = instance_.context();

	    Socket fd = IceInternal.Network.doAccept(fd_, timeout);
	    IceInternal.Network.setBlock(fd, true); // SSL requires a blocking socket.

	    if(instance_.networkTraceLevel() >= 2)
	    {
		string s = "trying to validate incoming ssl connection\n" + IceInternal.Network.fdToString(fd);
		logger_.trace(instance_.networkTraceCategory(), s);
	    }

	    SslStream stream = null;
	    try
	    {
		//
		// Create an SslStream.
		//
		NetworkStream ns = new NetworkStream(fd, true);
		AcceptorValidationCallback cb = new AcceptorValidationCallback(this, fd);
		stream = new SslStream(ns, false, new RemoteCertificateValidationCallback(cb.validate), null);

		//
		// Get the certificate collection and select the first one.
		//
		X509Certificate2Collection certs = ctx.certs();
		X509Certificate2 cert = null;
		if(certs.Count > 0)
		{
		    cert = certs[0];
		}

		//
		// Start the validation process and wait for it to complete.
		//
		AuthInfo info = new AuthInfo();
		info.stream = stream;
		info.done = false;
		IAsyncResult ar = stream.BeginAuthenticateAsServer(cert, ctx.verifyPeer() > 1, ctx.protocols(),
								   ctx.checkCRL(), new AsyncCallback(authCallback),
								   info);
		lock(info)
		{
		    if(!info.done)
		    {
			if(!Monitor.Wait(info, timeout == -1 ? Timeout.Infinite : timeout))
			{
			    throw new Ice.TimeoutException("SSL authentication timed out after " + timeout + " msec");
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
		    //
		    // This situation occurs when connectToSelf is called; the "remote" end
		    // closes the socket immediately.
		    //
		    return null;
		}
		throw new Ice.SocketException(ex);
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
		string s = "accepted ssl connection\n" + IceInternal.Network.fdToString(fd);
		logger_.trace(instance_.networkTraceCategory(), s);
	    }

	    if(instance_.securityTraceLevel() >= 1)
	    {
		instance_.traceStream(stream, IceInternal.Network.fdToString(fd));
	    }

	    return new TransceiverI(instance_, fd, stream);
	}

	public virtual void connectToSelf()
	{
	    Socket fd = IceInternal.Network.createSocket(false);
	    IceInternal.Network.setBlock(fd, false);
	    IceInternal.Network.doConnect(fd, addr_, -1);
	    IceInternal.Network.closeSocket(fd);
	}

	public override string ToString()
	{
	    return IceInternal.Network.addrToString(addr_);
	}
	
	internal bool equivalent(string host, int port)
	{
	    EndPoint addr = IceInternal.Network.getAddress(host, port);
	    return addr.Equals(addr_);
	}
	
	internal virtual int effectivePort()
	{
	    return addr_.Port;
	}
	
	internal
	AcceptorI(Instance instance, string host, int port)
	{
	    instance_ = instance;
	    logger_ = instance.communicator().getLogger();
	    backlog_ = 0;

	    //
	    // .NET requires that a certificate be supplied.
	    //
	    X509Certificate2Collection certs = instance.context().certs();
	    if(certs.Count == 0)
	    {
		Ice.SecurityException ex = new Ice.SecurityException();
		ex.reason = "IceSSL: certificate required for server endpoint";
		throw ex;
	    }

	    if(backlog_ <= 0)
	    {
		backlog_ = 5;
	    }
	    
	    try
	    {
		fd_ = IceInternal.Network.createSocket(false);
		IceInternal.Network.setBlock(fd_, false);
		addr_ = IceInternal.Network.getAddress(host, port);
		if(instance_.networkTraceLevel() >= 2)
		{
		    string s = "attempting to bind to ssl socket " + ToString();
		    logger_.trace(instance_.networkTraceCategory(), s);
		}
		addr_ = IceInternal.Network.doBind(fd_, addr_);
	    }
	    catch(System.Exception)
	    {
		fd_ = null;
		throw;
	    }
	}
	
#if DEBUG
	~AcceptorI()
	{
	    lock(this)
	    {
		IceUtil.Assert.FinalizerAssert(fd_ == null);
	    }
	}
#endif

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
		    info.stream.EndAuthenticateAsServer(ar);
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

	internal bool validate(SslStream stream, Socket fd, X509Certificate certificate, X509Chain chain,
			       SslPolicyErrors sslPolicyErrors)
	{
	    string message = "";
	    int errors = (int)sslPolicyErrors;
	    if((errors & (int)SslPolicyErrors.RemoteCertificateNotAvailable) > 0)
	    {
		if(instance_.context().verifyPeer() > 1)
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
		//
		// This condition is not expected in a server.
		//
		Debug.Assert(false);
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
		info.incoming = true;
		info.cert = certificate;
		info.chain = chain;
		info.stream = stream;
		info.address = "";
		if(!verifier.verify(info))
		{
		    if(instance_.securityTraceLevel() >= 1)
		    {
			logger_.trace(instance_.securityTraceCategory(),
				      "incoming connection rejected by certificate verifier\n" +
				      IceInternal.Network.fdToString(fd));
		    }
		    return false;
		}
	    }

	    return true;
	}
	
	private Instance instance_;
	private Ice.Logger logger_;
	private Socket fd_;
	private int backlog_;
	private IPEndPoint addr_;
    }

    //
    // We need to pass some additional information to the certificate validation callback.
    //
    internal class AcceptorValidationCallback
    {
	internal AcceptorValidationCallback(AcceptorI acceptor, Socket fd)
	{
	    acceptor_ = acceptor;
	    fd_ = fd;
	}

	internal bool validate(object sender, X509Certificate certificate, X509Chain chain,
			       SslPolicyErrors sslPolicyErrors)
	{
	    SslStream stream = (SslStream)sender;
	    return acceptor_.validate(stream, fd_, certificate, chain, sslPolicyErrors);
	}

	private AcceptorI acceptor_;
	private Socket fd_;
    }
}
