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

    public interface EndpointFactory
    {
	short type();
	string protocol();
	Endpoint create(string str);
	Endpoint read(BasicStream s);
	void destroy();
    }

}
