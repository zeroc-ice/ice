// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

public class BasicOutputStream extends BasicStream
{
    public
    BasicOutputStream(Instance instance, Ice.OutputStream out)
    {
        super(instance);
        _out = out;
    }

    public Ice.OutputStream _out;
}
