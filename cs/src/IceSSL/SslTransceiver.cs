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
    using System.ComponentModel;
    using System.Diagnostics;
    using System.Net.Security;
    using System.Net.Sockets;
    using System.IO;

    sealed class SslTransceiver : IceInternal.Transceiver
    {
	public Socket fd()
	{
	    Debug.Assert(fd_ != null);
	    return fd_;
	}

	public void close()
	{
	    if(instance_.networkTraceLevel() >= 1)
	    {
		string s = "closing ssl connection\n" + ToString();
		logger_.trace(instance_.networkTraceCategory(), s);
	    }

	    lock(this)
	    {
		Debug.Assert(fd_ != null);
		Debug.Assert(stream_ != null);
		try
		{
		    //
		    // Closing the stream also closes the socket.
		    //
		    stream_.Close();
		}
		catch(IOException ex)
		{
		    throw new Ice.SocketException(ex);
		}
		finally
		{
		    fd_ = null;
		    stream_ = null;
		}
	    }
	}

	public void shutdownWrite()
	{
	    if(instance_.networkTraceLevel() >= 2)
	    {
		string s = "shutting down ssl connection for writing\n" + ToString();
		logger_.trace(instance_.networkTraceCategory(), s);
	    }

	    Debug.Assert(fd_ != null);
	    try
	    {
		fd_.Shutdown(SocketShutdown.Send);
	    }
	    catch(SocketException ex)
	    {
	        if(IceInternal.Network.notConnected(ex))
		{
		    return;
		}
	        throw new Ice.SocketException(ex);
	    }
	}

	public void shutdownReadWrite()
	{
	    if(instance_.networkTraceLevel() >= 2)
	    {
		string s = "shutting down ssl connection for reading and writing\n" + ToString();
		logger_.trace(instance_.networkTraceCategory(), s);
	    }

	    Debug.Assert(fd_ != null);
	    try
	    {
		fd_.Shutdown(SocketShutdown.Both);
	    }
	    catch(SocketException ex)
	    {
	        if(IceInternal.Network.notConnected(ex))
		{
		    return;
		}
	        throw new Ice.SocketException(ex);
	    }
	}

	public void write(IceInternal.BasicStream stream, int timeout)
	{
	    Debug.Assert(fd_ != null);

	    /*
	     * TODO: Timeouts
	    if(timeout == -1)
	    {
		timeout = System.Threading.Timeout.Infinite;
	    }
	    if(stream_.WriteTimeout != timeout)
	    {
		stream_.WriteTimeout = timeout;
	    }
	    */

	    IceInternal.ByteBuffer buf = stream.prepareWrite();
	    int remaining = buf.remaining();
	    int position = buf.position();
	    try
	    {
		stream_.Write(buf.rawBytes(), position, remaining);

		if(instance_.networkTraceLevel() >= 3)
		{
		    string s = "sent " + remaining + " of " + remaining + " bytes via ssl\n" + ToString();
		    logger_.trace(instance_.networkTraceCategory(), s);
		}
		if(stats_ != null)
		{
		    stats_.bytesSent(type(), remaining);
		}
	    }
	    catch(IOException ex)
	    {
		// TODO: Is there a better way than checking the message string?
		if(IceInternal.Network.connectionLost(ex))
		{
		    throw new Ice.ConnectionLostException(ex);
		}
		if(IceInternal.Network.timeout(ex))
		{
		    throw new Ice.TimeoutException();
		}
		throw new Ice.SocketException(ex);
	    }
	    catch(SocketException ex) // TODO: Necessary?
	    {
		if(IceInternal.Network.connectionLost(ex))
		{
		    throw new Ice.ConnectionLostException(ex);
		}
		if(IceInternal.Network.wouldBlock(ex))
		{
		    throw new Ice.TimeoutException();
		}
		throw new Ice.SocketException(ex);
	    }
	    catch(Ice.LocalException)
	    {
		throw;
	    }
	    catch(System.Exception ex)
	    {
		throw new Ice.SyscallException(ex);
	    }
	}

	public void read(IceInternal.BasicStream stream, int timeout)
	{
	    Debug.Assert(fd_ != null);

	    /*
	     * TODO: Timeouts
	    if(timeout == -1)
	    {
		timeout = System.Threading.Timeout.Infinite;
	    }
	    if(stream_.ReadTimeout != timeout)
	    {
		stream_.ReadTimeout = timeout;
	    }
	    */

	    //
	    // TODO: Is this necessary to work around a .NET 2.0 bug?
	    //
	    // http://forums.microsoft.com/MSDN/ShowPost.aspx?PostID=171136&SiteID=1
	    //
	    fd_.Blocking = true;

	    IceInternal.ByteBuffer buf = stream.prepareRead();    
	    int remaining = buf.remaining();
	    int position = buf.position();

	    try
	    {
		int ret = stream_.Read(buf.rawBytes(), position, remaining);
		if(ret == 0)
		{
		    //
		    // Try to read again; if zero is returned, the connection is lost.
		    //
		    ret = stream_.Read(buf.rawBytes(), position, remaining);
		    if(ret == 0)
		    {
			throw new Ice.ConnectionLostException();
		    }
		}
		if(instance_.networkTraceLevel() >= 3)
		{
		    string s = "received " + ret + " of " + remaining + " bytes via ssl\n" + ToString();
		    logger_.trace(instance_.networkTraceCategory(), s);
		}    
		if(stats_ != null)
		{
		    stats_.bytesReceived(type(), ret);
		}
		buf.position(position += ret);
	    }
	    catch(IOException ex)
	    {
		// TODO: Is there a better way than checking the message string?
		if(IceInternal.Network.connectionLost(ex))
		{
		    throw new Ice.ConnectionLostException(ex);
		}
		if(IceInternal.Network.timeout(ex))
		{
		    throw new Ice.TimeoutException();
		}
		throw new Ice.SocketException(ex);
	    }
	    catch(SocketException ex) // TODO: Necessary?
	    {
		if(IceInternal.Network.connectionLost(ex))
		{
		    throw new Ice.ConnectionLostException(ex);
		}
		if(IceInternal.Network.wouldBlock(ex))
		{
		    throw new Ice.TimeoutException();
		}
		throw new Ice.SocketException(ex);
	    }
	    catch(Ice.LocalException) // TODO: Necessary?
	    {
		throw;
	    }
	    catch(System.Exception ex)
	    {
		throw new Ice.SyscallException(ex);
	    }
	}

	public string type()
	{
	    return "ssl";
	}

	public override string ToString()
	{
	    return desc_;
	}

	//
	// Only for use by SslConnector, SslAcceptor.
	//
	internal SslTransceiver(Instance instance, Socket fd, SslStream stream)
	{
	    instance_ = instance;
	    fd_ = fd;
	    stream_ = stream;
	    logger_ = instance.communicator().getLogger();
	    stats_ = instance.communicator().getStats();
	    desc_ = IceInternal.Network.fdToString(fd_);
	}

#if DEBUG
	~SslTransceiver()
	{
	    lock(this)
	    {
		IceUtil.Assert.FinalizerAssert(fd_ == null);
	    }
	}
#endif

	private Instance instance_;
	private Socket fd_;
	private SslStream stream_;
	private Ice.Logger logger_;
	private Ice.Stats stats_;
	private string desc_;
    }

}
