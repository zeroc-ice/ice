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

package IceInternal;

public interface EndpointFactory
{
    short type();
    String protocol();
    Endpoint create(String str);
    Endpoint read(BasicStream s);
    void destroy();
}
