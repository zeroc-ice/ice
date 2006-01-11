// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace IceInternal
{

    using System.Collections;
    using System.ComponentModel;
    using System.Diagnostics;
    using System.Net;
    using System.Net.Sockets;

    sealed class UdpTransceiver : Transceiver
    {
	public Socket fd()
	{
	    Debug.Assert(_fd != null);
	    return _fd;
	}
	
	public void close()
	{
	    lock(this)
	    {
		if(_traceLevels.network >= 1)
		{
		    string s = "closing udp connection\n" + ToString();
		    _logger.trace(_traceLevels.networkCat, s);
		}
	    
		if(_fd != null)
		{
		    try
		    {
			_fd.Close();
		    }
		    catch(System.IO.IOException)
		    {
		    }
		    _fd = null;
		}
	    }
	}

	public void shutdownWrite()
	{
	}

	public void shutdownReadWrite()
	{
	    if(_traceLevels.network >= 2)
	    {
		string s = "shutting down udp connection for reading and writing\n" + ToString();
		_logger.trace(_traceLevels.networkCat, s);
	    }

	    //
	    // Set a flag and then shutdown the socket in order to wake a thread that is
	    // blocked in read().
	    //
	    lock(_shutdownReadWriteMutex)
	    {
		_shutdownReadWrite = true;

		Debug.Assert(_fd != null);
		try
		{
		    _fd.Shutdown(SocketShutdown.Both);
		}
		catch(SocketException ex)
	        {
	            if(Network.notConnected(ex))
		    {
		        return;
		    }
		    throw new Ice.SocketException(ex);
		}
	    }

#if !__MonoCS__
	    //
	    // This is required to unblock the select call when using thread per connection.
	    //
	    Socket fd = Network.createSocket(true);
	    Network.setBlock(fd, false);
	    Network.doConnect(fd, _addr, -1);
	    byte[] buf = new byte[1];
	    fd.Send(buf, 0, 1, SocketFlags.None);
	    Network.closeSocket(fd);
#endif
	}
	
	public void write(BasicStream stream, int timeout)
	{
	    Debug.Assert(_fd != null);

	    ByteBuffer buf = stream.prepareWrite();
	    
	    Debug.Assert(buf.position() == 0);
	    int packetSize = System.Math.Min(_maxPacketSize, _sndSize - _udpOverhead);
	    if(packetSize < buf.limit())
	    {
		//
		// We don't log a warning here because the client gets an exception anyway.
		//
		throw new Ice.DatagramLimitException();
	    }

	    try
	    {	
		int remaining = buf.remaining();
		int ret;
		try
		{
		    ret = _fd.Send(buf.rawBytes(), 0, remaining, SocketFlags.None);
		}
		catch(Win32Exception e)
		{
		    if(Network.wouldBlock(e))
		    {
			if(timeout == 0)
			{
			    throw new Ice.TimeoutException();
			}
			ret = 0;
		    }
		    else
		    {
			throw;
		    }
		}    
		if(ret == 0)
		{
		    if(!Network.doPoll(_fd, timeout, Network.PollMode.Write))
		    {
			throw new Ice.TimeoutException();
		    }
		    ret = _fd.Send(buf.rawBytes(), 0, remaining, SocketFlags.None);
		}
		if(ret != remaining)
		{
		    throw new Ice.DatagramLimitException();
		}

		if(_traceLevels.network >= 3)
		{
		    string s = "sent " + ret + " bytes via udp\n" + ToString();
		    _logger.trace(_traceLevels.networkCat, s);
		}
		
		if(_stats != null)
		{
		    _stats.bytesSent(type(), ret);
		}

		buf.position(remaining);
	    }
	    catch(SocketException ex)
	    {
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
	
	public void read(BasicStream stream, int timeout)
	{
	    Debug.Assert(stream.pos() == 0);
	    int packetSize = System.Math.Min(_maxPacketSize, _rcvSize - _udpOverhead);
	    if(packetSize < stream.size())
	    {
		//
		// We log a warning here because this is the server side -- without the
		// the warning, there would only be silence.
		//
		if(_warn)
		{
		    _logger.warning("DatagramLimitException: maximum size of " + packetSize + " exceeded");
		}
		throw new Ice.DatagramLimitException();
	    }
	    stream.resize(packetSize, true);
	    ByteBuffer buf = stream.prepareRead();
	    buf.position(0);

	    //
	    // Check the shutdown flag.
	    //
	    lock(_shutdownReadWriteMutex)
	    {
		if(_shutdownReadWrite)
		{
		    throw new Ice.ConnectionLostException();
		}
	    }

	    try
	    {
		int ret;
		try
		{
		    Debug.Assert(_fd != null);
		    ret = _fd.Receive(buf.rawBytes(), 0, buf.limit(), SocketFlags.None);
		}
		catch(Win32Exception e)
		{
		    if(Network.wouldBlock(e))
		    {
			if(timeout == 0)
			{
			    throw new Ice.TimeoutException();
			}
			ret = 0;
		    }
		    else
		    {
			throw;
		    }
		}
		if(ret == 0)
		{
		    if(!Network.doPoll(_fd, timeout, Network.PollMode.Read))
		    {
			throw new Ice.TimeoutException();
		    }
		    ret = _fd.Receive(buf.rawBytes(), 0, buf.limit(), SocketFlags.None);
		    if(ret == 0)
		    {
			throw new Ice.ConnectionLostException();
		    }
		}
		if(_connect)
		{
		    //
		    // If we must connect, then we connect to the first peer that
		    // sends us a packet.
		    //
		    if(ret != 0)
		    {
			Network.doConnect(_fd, _fd.RemoteEndPoint, -1);
			_connect = false; // We're connected now
    				
			if(_traceLevels.network >= 1)
			{
			    string s = "connected udp socket\n" + ToString();
			    _logger.trace(_traceLevels.networkCat, s);
			}
		    }
		}
		if(_traceLevels.network >= 3)
		{
		    string s = "received " + ret + " bytes via udp\n" + ToString();
		    _logger.trace(_traceLevels.networkCat, s);
		}

		if(_stats != null)
		{
		    _stats.bytesReceived(type(), ret);
		}

		stream.resize(ret, true);
	    }
	    catch(SocketException ex)
	    {
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
	
        public string type()
        {
            return "udp";
        }

	public override string ToString()
	{
	    return Network.fdToString(_fd);
	}
	
	public bool equivalent(string host, int port)
	{
	    IPEndPoint addr = ((IPEndPoint)Network.getAddress(host, port));
	    return addr.Equals(_addr);
	}
	
	public int effectivePort()
	{
	    return _addr.Port;
	}
	
	//
	// Only for use by UdpEndpoint
	//
	internal UdpTransceiver(Instance instance, string host, int port)
	{
	    _traceLevels = instance.traceLevels();
	    _logger = instance.logger();
	    _stats = instance.stats();
	    _connect = true;
	    _warn = instance.properties().getPropertyAsInt("Ice.Warn.Datagrams") > 0;
	    
	    try
	    {
		_fd = Network.createSocket(true);
		setBufSize(instance);
		Network.setBlock(_fd, false);
		_addr = Network.getAddress(host, port);
		Network.doConnect(_fd, _addr, -1);
		_connect = false; // We're connected now
		
		if(_traceLevels.network >= 1)
		{
		    string s = "starting to send udp packets\n" + ToString();
		    _logger.trace(_traceLevels.networkCat, s);
		}
	    }
	    catch(Ice.LocalException)
	    {
		_fd = null;
		throw;
	    }
	}
	
	//
	// Only for use by UdpEndpoint
	//
	internal UdpTransceiver(Instance instance, string host, int port, bool connect)
	{
	    _traceLevels = instance.traceLevels();
	    _logger = instance.logger();
	    _stats = instance.stats();
	    _connect = connect;
	    _warn = instance.properties().getPropertyAsInt("Ice.Warn.Datagrams") > 0;
	    _shutdownReadWrite = false;
	    
	    try
	    {
		_fd = Network.createSocket(true);
		setBufSize(instance);
		Network.setBlock(_fd, false);
		_addr = Network.getAddress(host, port);
		if(_traceLevels.network >= 2)
		{
		    string s = "attempting to bind to udp socket " + Network.addrToString(_addr);
		    _logger.trace(_traceLevels.networkCat, s);
		}
		_addr = Network.doBind(_fd, _addr);
		
		if(_traceLevels.network >= 1)
		{
		    string s = "starting to receive udp packets\n" + ToString();
		    _logger.trace(_traceLevels.networkCat, s);
		}
	    }
	    catch(Ice.LocalException)
	    {
		_fd = null;
		throw;
	    }
	}
	
	private void setBufSize(Instance instance)
	{
	    lock(this)
	    {
		Debug.Assert(_fd != null);
		
		for (int i = 0; i < 2; ++i)
		{
		    string direction;
		    string prop;
		    int dfltSize;
		    if(i == 0)
		    {
			direction = "receive";
			prop = "Ice.UDP.RcvSize";
			dfltSize = Network.getRecvBufferSize(_fd);
			_rcvSize = dfltSize;
		    }
		    else
		    {
			direction = "send";
			prop = "Ice.UDP.SndSize";
			dfltSize = Network.getSendBufferSize(_fd);
			_sndSize = dfltSize;
		    }
		    
		    //
		    // Get property for buffer size and check for sanity.
		    //
		    int sizeRequested = instance.properties().getPropertyAsIntWithDefault(prop, dfltSize);
		    if(sizeRequested < _udpOverhead)
		    {
			_logger.warning("Invalid " + prop + " value of " + sizeRequested + " adjusted to " + dfltSize);
			sizeRequested = dfltSize;
		    }
		    
		    //
		    // Ice.MessageSizeMax overrides UDP buffer sizes if Ice.MessageSizeMax + _udpOverhead is less.
		    //
		    int messageSizeMax = instance.messageSizeMax();
		    if(sizeRequested > messageSizeMax + _udpOverhead)
		    {
			int newSize = System.Math.Min(messageSizeMax, _maxPacketSize) + _udpOverhead;
			_logger.warning("UDP " + direction + " buffer size: request size of " + sizeRequested +
					" adjusted to " + newSize + " (Ice.MessageSizeMax takes precedence)");
			sizeRequested = newSize;
		    }
		    
		    if(sizeRequested != dfltSize)
		    {
			//
			// Try to set the buffer size. The kernel will silently adjust
			// the size to an acceptable value. Then read the size back to
			// get the size that was actually set.
			//
			int sizeSet;
			if(i == 0)
			{
			    Network.setRecvBufferSize(_fd, sizeRequested);
			    _rcvSize = Network.getRecvBufferSize(_fd);
			    sizeSet = _rcvSize;
			}
			else
			{
			    Network.setSendBufferSize(_fd, sizeRequested);
			    _sndSize = Network.getSendBufferSize(_fd);
			    sizeSet = _sndSize;
			}
			
			//
			// Warn if the size that was set is less than the requested size.
			//
			if(sizeSet < sizeRequested)
			{
			    _logger.warning("UDP " + direction + " buffer size: requested size of " + sizeRequested +
					    " adjusted to " + sizeSet);
			}
		    }
		}
	    }
	}

#if DEBUG
	~UdpTransceiver()
	{
	    lock(this)
	    {
		IceUtil.Assert.FinalizerAssert(_fd == null);
	    }
	}
#endif
	
	private TraceLevels _traceLevels;
	private Ice.Logger _logger;
	private Ice.Stats _stats;
	private bool _connect;
	private readonly bool _warn;
	private int _rcvSize;
	private int _sndSize;
	private Socket _fd;
	private IPEndPoint _addr;
	
	//
	// The maximum IP datagram size is 65535. Subtract 20 bytes for the IP header and 8 bytes for the UDP header
	// to get the maximum payload.
	//
	private const int _udpOverhead = 20 + 8;
	private static readonly int _maxPacketSize = 65535 - _udpOverhead;

	private bool _shutdownReadWrite;
	private object _shutdownReadWriteMutex = new object();
    }

}
