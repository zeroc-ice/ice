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

sealed class UdpEndpointFactory : EndpointFactory
{
    internal
    UdpEndpointFactory(Instance instance)
    {
	_instance = instance;
    }
    
    public short
    type()
    {
	return UdpEndpoint.TYPE;
    }
    
    public string
    protocol()
    {
	return "udp";
    }
    
    public Endpoint
    create(string str)
    {
	return new UdpEndpoint(_instance, str);
    }
    
    public Endpoint
    read(BasicStream s)
    {
	return new UdpEndpoint(s);
    }
    
    public void
    destroy()
    {
	_instance = null;
    }
    
    private Instance _instance;
}

}
