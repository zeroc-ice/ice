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
    public class BasicInputStream : BasicStream
    {
	public BasicInputStream(Instance instance, Ice.InputStream inStream)
            : base(instance)
	{
	    _in = inStream;
	}

	public Ice.InputStream _in;
    }
}
