// **********************************************************************
//
// Copyright (c) 2003 - 2004
// ZeroC, Inc.
// North Palm Beach, FL, USA
//
// All Rights Reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************


namespace IceInternal
{

    sealed class TcpEndpointFactory : EndpointFactory
    {
	internal TcpEndpointFactory(Instance instance)
	{
	    _instance = instance;
	}
	
	public short type()
	{
	    return TcpEndpoint.TYPE;
	}
	
	public string protocol()
	{
	    return "tcp";
	}
	
	public Endpoint create(string str)
	{
	    return new TcpEndpoint(_instance, str);
	}
	
	public Endpoint read(BasicStream s)
	{
	    return new TcpEndpoint(s);
	}
	
	public void destroy()
	{
	    _instance = null;
	}
	
	private Instance _instance;
    }

}
