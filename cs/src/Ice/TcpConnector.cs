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

using System.Net;
using System.Net.Sockets;

sealed class TcpConnector : Connector
{
    public Transceiver
    connect(int timeout)
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
	
	return new TcpTransceiver(_instance, fd);
    }
    
    public override string
    ToString()
    {
	return Network.addrToString(_addr);
    }
    
    //
    // Only for use by TcpEndpoint
    //
    internal
    TcpConnector(Instance instance, string host, int port)
    {
	_instance = instance;
	_traceLevels = instance.traceLevels();
	_logger = instance.logger();
	
	_addr = Network.getAddress(host, port);
    }
    
    private Instance _instance;
    private TraceLevels _traceLevels;
    private Ice.Logger _logger;
    private IPEndPoint _addr;
}

}
