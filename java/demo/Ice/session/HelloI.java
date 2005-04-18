// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import Demo.*;

public class HelloI extends _HelloDisp
{
    public
    HelloI(int id)
    {
	_id = id;
    }

    public void
    sayHello(Ice.Current current)
    {
        System.out.println(_id + ": Hello World!");
    }

    final private int _id;
}
