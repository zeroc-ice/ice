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

    using System.Net;
    using System.Net.Sockets;

    sealed class TcpConnector : Connector
    {
	public Transceiver connect(int timeout)
	{
	    if(_traceLevels.network >= 2)
	    {
		string s = "trying to establish tcp connection to " + ToString();
		_logger.trace(_traceLevels.networkCat, s);
	    }
	    
	    Socket fd = Network.createSocket(false);
	    Network.setBlock(fd, false);
	    Network.doConnect(fd, _addr, timeout);
	    
	    if(_traceLevels.network >= 1)
	    {
		string s = "tcp connection established\n" + Network.fdToString(fd);
		_logger.trace(_traceLevels.networkCat, s);
	    }
	    
	    return new TcpTransceiver(instance_, fd);
	}
	
	public override string ToString()
	{
	    return Network.addrToString(_addr);
	}
	
	//
	// Only for use by TcpEndpoint
	//
	internal TcpConnector(Instance instance, string host, int port)
	{
	    instance_ = instance;
	    _traceLevels = instance.traceLevels();
	    _logger = instance.logger();
	    
	    _addr = Network.getAddress(host, port);
	}
	
	private Instance instance_;
	private TraceLevels _traceLevels;
	private Ice.Logger _logger;
	private IPEndPoint _addr;
    }

}
