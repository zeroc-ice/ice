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

    sealed class UdpEndpointFactory : EndpointFactory
    {
	internal UdpEndpointFactory(Instance instance)
	{
	    _instance = instance;
	}
	
	public short type()
	{
	    return UdpEndpoint.TYPE;
	}
	
	public string protocol()
	{
	    return "udp";
	}
	
	public Endpoint create(string str)
	{
	    return new UdpEndpoint(_instance, str);
	}
	
	public Endpoint read(BasicStream s)
	{
	    return new UdpEndpoint(s);
	}
	
	public void destroy()
	{
	    _instance = null;
	}
	
	private Instance _instance;
    }

}
