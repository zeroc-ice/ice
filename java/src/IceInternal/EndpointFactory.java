// **********************************************************************
//
// Copyright (c) 2002
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
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
