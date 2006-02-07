// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace IceSSL
{
    using System;

    internal class Context
    {
	internal Context(Instance instance)
	{
	    instance_ = instance;
	    logger_ = instance.communicator().getLogger();
	}

	protected Instance instance_;
	protected Ice.Logger logger_;
    }
}
