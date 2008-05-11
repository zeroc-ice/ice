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
    InetSocketAddress(String address, int port)
    {
	_addr = address;
	_port = port;
    }

    String
    getAddress()
    {
	return _addr;
    }

    String
    getHostName()
    {
	String result = System.getProperty("microedition.hostname");
	if(result == null || result.length() == 0)
	{
	    return "localhost";
	}
	else
	{
	    return result;
	}
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
	return _addr + ":" + _port;
    }

    public boolean
    equals(Object rhs)
    {
	InetSocketAddress addr = (InetSocketAddress)rhs;
	return _addr.equals(addr._addr) && _port == addr._port;
    }
    
    private String _addr;
    private int _port;
}
