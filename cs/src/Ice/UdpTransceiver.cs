// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
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
	    if(_traceLevels.network >= 1)
	    {
		//UPGRADE_TODO: The equivalent in .NET for method 'java.Object.toString' may return a different value. 'ms-help://MS.VSCC.2003/commoner/redir/redirect.htm?keyword="jlca1043"' // TODO
		string s = "closing udp connection\n" + ToString();
		_logger.trace(_traceLevels.networkCat, s);
	    }
	    
	    Debug.Assert(_fd != null);
	    try
	    {
		_fd.Close();
	    }
	    catch(System.IO.IOException)
	    {
	    }
	    _fd = null;
	}
	
	public void shutdown()
	{
	}
	
	public void write(BasicStream stream, int timeout)
	{
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
	    
	    byte[] bytes = buf.toArray();
	    int remaining = bytes.Length;

	    try
	    {
		Debug.Assert(_fd != null);
		int ret;
		try
		{
		    ret = _fd.Send(bytes);
		}
		catch(Win32Exception e)
		{
		    if(Network.wouldBlock(e))
		    {
			ret = 0;
		    }
		    else
		    {
			throw;
		    }
		}    
		if(ret == 0)
		{
		    if(timeout == 0)
		    {
			throw new Ice.TimeoutException();
		    }
		    ArrayList sendList = new ArrayList();
		    sendList.Add(_fd);
		    ArrayList errorList = new ArrayList();
		    errorList.Add(_fd);
		    Network.doSelect(null, sendList, errorList, timeout);
		    if(errorList.Count != 0)
		    {
			throw new Ice.SocketException();
		    }
		    if(sendList.Count == 0)
		    {
			if(timeout > 0)
			{
			    throw new Ice.TimeoutException();
			}
			else
			{
			    throw new Ice.ConnectionLostException();
			}
		    }
		    else
		    {
			try
			{
			    ret = _fd.Send(bytes);
			}
			catch(System.Exception e)
			{
			    throw new Ice.SocketException(e);
			}
		    }
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
		    _stats.bytesSent("udp", ret);
		}
	    }
	    catch(Ice.Exception)
	    {
		throw;
	    }
	    catch(SocketException ex)
	    {
		throw new Ice.SocketException(ex);
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
	    
	    int ret;
	    byte[] bytes;
	    try
	    {
		Debug.Assert(_fd != null);
		int available = _fd.Available;
		if(available == 0)
		{
		    if(timeout == 0)
		    {
			throw new Ice.TimeoutException();
		    }
		    ArrayList recvList = new ArrayList();
		    recvList.Add(_fd);
		    ArrayList errorList = new ArrayList();
		    errorList.Add(_fd);
		    Network.doSelect(recvList, null, errorList, timeout);
		    if(errorList.Count != 0)
		    {
			throw new Ice.SocketException("Select on UDP socket returned an error");
		    }
		    if(recvList.Count == 0)
		    {
			throw new Ice.TimeoutException();
		    }
		    bytes = new byte[available = _fd.Available];

		}
		else
		{
		    bytes = new byte[available];
		}
		ret = _fd.Receive(bytes);
		Debug.Assert(ret == available);

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
	    }
	    catch(Ice.Exception)
	    {
		throw;
	    }
	    catch(SocketException ex)
	    {
		Ice.SocketException se = new Ice.SocketException(ex);
		throw se;
	    }
	    catch(System.Exception ex)
	    {
		throw new Ice.SyscallException(ex);
	    }
	    
	    if(_traceLevels.network >= 3)
	    {
		string s = "received " + ret + " bytes via udp\n" + ToString();
		_logger.trace(_traceLevels.networkCat, s);
	    }
	    
	    if(_stats != null)
	    {
		_stats.bytesReceived("udp", ret);
	    }

	    stream.resize(ret, true);
	    buf.put(bytes);
	    stream.pos(ret);
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
		Network.doConnect(_fd, _addr, - 1);
		_connect = false; // We're connected now
		
		if(_traceLevels.network >= 1)
		{
		    //UPGRADE_TODO: The equivalent in .NET for method 'java.Object.toString' may return a different value. 'ms-help://MS.VSCC.2003/commoner/redir/redirect.htm?keyword="jlca1043"'
		    string s = "starting to send udp packets\n" + ToString();
		    _logger.trace(_traceLevels.networkCat, s);
		}
	    }
	    catch(Ice.LocalException ex)
	    {
		_fd = null;
		throw ex;
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
		    //UPGRADE_TODO: The equivalent in .NET for method 'java.Object.toString' may return a different value. 'ms-help://MS.VSCC.2003/commoner/redir/redirect.htm?keyword="jlca1043"'
		    string s = "starting to receive udp packets\n" + ToString();
		    _logger.trace(_traceLevels.networkCat, s);
		}
	    }
	    catch(Ice.LocalException ex)
	    {
		_fd = null;
		throw ex;
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
			_logger.warning("UDP " + direction + " buffer size: request size of " + sizeRequested + " adjusted to " + newSize + " (Ice.MessageSizeMax takes precendence)");
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
			    _logger.warning("UDP " + direction + " buffer size: requested size of " + sizeRequested + " adjusted to " + sizeSet);
			}
		    }
		}
	    }
	}

	
	~UdpTransceiver()
	{
	    Debug.Assert(_fd == null);
	}
	
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
    }

}
