// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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
