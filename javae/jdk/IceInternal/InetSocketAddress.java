// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

class InetSocketAddress
{
    InetSocketAddress(String host, int port)
    {
	try
	{
	    _addr = java.net.InetAddress.getByName(host);
	}
	catch(java.net.UnknownHostException ex)
	{
	    Ice.DNSException e = new Ice.DNSException();
	    e.host = host;
	    e.initCause(ex);
	    throw e;
	}
	catch(RuntimeException ex)
	{
	    Ice.SocketException se = new Ice.SocketException();
	    se.initCause(ex);
	    throw se;
	}
	_port = port;
    }

    InetSocketAddress(java.net.InetAddress addr, int port)
    {
	_addr = addr;
	_port = port;
    }

    java.net.InetAddress
    getAddress()
    {
	return _addr;
    }

    String
    getHostName()
    {
	return _addr.getHostName();
    }

    int
    getPort()
    {
	return _port;
    }

    public int
    hashCode()
    {
	return 5 * _addr.hashCode() + _port;
    }

    public String
    toString()
    {
	return _addr.toString() + ":" + _port;
    }

    public boolean
    equals(Object rhs)
    {
	InetSocketAddress addr = (InetSocketAddress)rhs;
	return _addr.equals(addr._addr) && _port == addr._port;
    }

    private java.net.InetAddress _addr;
    private int _port;
}
