// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System.Diagnostics;

namespace Ice {

    public abstract class ObjectWriter : ObjectImpl
    {
	public abstract void write(OutputStream outStream);

	public override void __write(IceInternal.BasicStream os)
	{
	    IceInternal.BasicOutputStream bos = (IceInternal.BasicOutputStream)os;
	    write(bos._out);
	}

	public override void __read(IceInternal.BasicStream istr, bool rid)
	{
	    Debug.Assert(false);
	}
    }

}
